/* chdmod_announce.c
 *
 * Tacitus @ LPUniversity
 * 28-JUNE-06
 * Announce channel module
 *
 */

#include <mudlib.h>

privatev object ChanD, HistD;
privatev mapping channels = ([
  "announce": ([
    "colour": MAG,
    "write": "admin",
  ]),
  "warning": ([
    "colour": ORA,
    "read": "admin",
    "write": "admin",
  ]),
  "error": ([
    "colour": RED,
    "read": "admin",
    "write": "admin",
  ]),
  "intermud3": ([
    "colour": GRN,
    "read": "i3developer",
    "write": "i3developer",
  ]),
]);

void create()
{
    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    ChanD->registerModule("announceNet", file_name(this_object()));

    foreach (string channel, mapping data in channels)
    {
        ChanD->registerCh("announceNet", channel);

        if (data["colour"])
            ChanD->set_chanCol(channel, data["colour"]);

        HistD->load_channel_history(channel);
    }
}

void remove()
{
    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    foreach (string channel, mapping data in channels)
    {
        ChanD->removeCh(channel);
        HistD->rem_history(channel);
        HistD->rem_file_history(channel);
    }

    ChanD->unregisterModule("announceNet");

    destruct();
}

int rec_m_msg(string channel, string originator, string msg)
{
    string array history = ({ });
    string cmd, rest = "", chan_msg, real_originator;
    int numLines = 15;

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);
    history = HistD->get_history()[channel];

    if (sscanf(msg, "%s %s", cmd, rest) != 2) cmd = msg;

    switch (cmd) /* We could do some neat stuff here! */
    {

        case "/last":
        {
            object ob = find_player(originator);

            if (rest != "") numLines = to_int(rest);
            if (numLines > sizeof(history)) numLines = sizeof(history);
            if (!sizeof(history))
                tell_object(ob, "AnnounceNet: Channel " + channel + " has no history yet.\n");
            else
                foreach (string histLine in history[(sizeof(history) - numLines)..<1])
                    tell_object(ob, histLine);

            return 1;
        }
        case "/all":
        {
            object ob = find_player(originator);

            if (!sizeof(history))
                tell_object(ob, "AnnounceNet: Channel " + channel + " has no history yet.\n");
            else
                foreach (string histLine in history[0..<1])
                    tell_object(ob, histLine);

            return 1;
        }
    }

    if (this_interactive())
        real_originator = query_privs(this_interactive());
    else real_originator = query_privs(previous_object());

    if (!real_originator) real_originator = originator;
    if (channels[channel]["write"])
        if (!isMember(real_originator, channels[channel]["write"])) return 0;

    if (is_emote(msg))
    {
        if (is_smiley(msg)) msg = " " + msg;

        chan_msg = " " + msg[1..<1];

        if (is_morse(msg)) msg += "\n(MORSE) " + translate_morse(msg[1..<1], 1);

        ChanD->rec_e_msg(channel, originator, "", "$N " + msg[1..<1]);
    }
    else
    {
        chan_msg = ": " + msg;

        if (is_morse(msg)) msg += "\n(MORSE): " + translate_morse(msg, 1);

        ChanD->rec_m_msg(channel, originator, msg);
    }

    chan_msg = sprintf("[%s] %s%s\n",
      ChanD->query_chanCol(channel) + channel + NOR,
      capitalize(originator),
      chan_msg
    );
    HistD->add_history(channel, ({ chan_msg }) );

    return 1;
}

int rec_e_msg(string channel, string originator, string o_msg, string t_msg)
{
    string real_originator;

    if (this_interactive())
        real_originator = query_privs(this_interactive());
    else real_originator = query_privs(previous_object());

    if (!real_originator) real_originator = originator;
    if (channels[channel]["write"])
        if (!isMember(real_originator, channels[channel]["write"])) return 0;

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    ChanD->rec_e_msg(channel, originator, o_msg, t_msg);

    t_msg = replace_string(t_msg, "$N", capitalize(originator));
    t_msg = sprintf("[%s] %s\n", ChanD->query_chanCol(channel) + channel + NOR, t_msg);
    HistD->add_history(channel, ({ t_msg }) );

    return 1;
}

int rec_t_msg(string channel, string originator, string target, string o_msg, string t_msg, string other_msg)
{
    string real_originator;

    if (this_interactive())
        real_originator = query_privs(this_interactive());
    else real_originator = query_privs(previous_object());

    if (!real_originator) real_originator = originator;
    if (channels[channel]["write"])
        if (!isMember(real_originator, channels[channel]["write"])) return 0;

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    ChanD->rec_t_msg(channel, originator, target, o_msg, t_msg, other_msg);

    other_msg = replace_string(other_msg, "$O", target);
    other_msg = replace_string(other_msg, "$N", capitalize(originator));
    other_msg = sprintf("[%s] %s\n", ChanD->query_chanCol(channel) + channel + NOR, other_msg);
    HistD->add_history(channel, ({ other_msg }) );

    return 1;
}

void announce_login(string user)
{
    string msg = sprintf("%s has logged into %s.", capitalize(user), mud_name());

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    ChanD->rec_m_msg(
      "announce",
      RED + "System" + NOR,
      msg
    );

    msg = sprintf("[%s] %s: %s\n",
      ChanD->query_chanCol("announce") + "announce" + NOR,
      RED + "System" + NOR,
      msg
    );
    HistD->add_history("announce", ({ msg }) );
}

void announce_logoff(string user)
{
    string msg = sprintf("%s has left %s.", capitalize(user), mud_name());

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    ChanD->rec_m_msg(
      "announce",
      RED + "System" + NOR,
      msg
    );

    msg = sprintf("[%s] %s: %s\n",
      ChanD->query_chanCol("announce") + "announce" + NOR,
      RED + "System" + NOR,
      msg
    );
    HistD->add_history("announce", ({ msg }) );
}

int isAllowed(string channel, string usr, int flag)
{
    if (channels[channel]["read"])
        if (!isMember(usr, channels[channel]["read"]))
            return 0;

    return 1;
}
