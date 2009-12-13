/*
 * localNet.c
 *
 * Tacitus @ LPUniversity
 * 30-SEPT-05
 * Channel Daemon Module - Local Net
 *
 * Version 2.11
 *
 * Last edited by Tricky on November 12th, 2007
 *
 */

#include <mudlib.h>

privatev object ChanD, HistD;
privatev mapping channels = ([
  "admin": ([
    "colour": RED,
    "read": "admin",
    "write": "admin",
  ]),
  "dev": ([
    "colour": HIY,
    "read": "developer",
    "write": "developer",
  ]),
  "wiz": ([
    "colour": HIC,
    "read": "wizard",
    "write": "wizard",
  ]),
  "chat": ([
    "colour": HIB,
  ]),
]);

void create()
{
    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    ChanD->registerModule("localNet", file_name(this_object()));

    foreach (string channel, mapping data in channels)
    {
        ChanD->registerCh("localNet", channel);

        if (data["colour"])
            ChanD->set_chanCol(channel, data["colour"]);
        if (channel == "admin")
            HistD->set_history_dir(channel, "/adm/logs");

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

    ChanD->unregisterModule("localNet");

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
                tell_object(ob, "LocalNet: Channel " + channel + " has no history yet.\n");
            else
                foreach (string histLine in history[(sizeof(history) - numLines)..<1])
                    tell_object(ob, histLine);

            return 1;
        }
        case "/all":
        {
            object ob = find_player(originator);

            if (!sizeof(history))
                tell_object(ob, "LocalNet: Channel " + channel + " has no history yet.\n");
            else
                foreach (string histLine in history[0..<1])
                    tell_object(ob, histLine);

            return 1;
        }
        case "/morse":
        {
            if (rest == "") return 1;
            if (is_emote(rest))
            {
                if (is_smiley(rest)) rest = " " + rest;
                msg = ":" + translate_morse(rest[1..<1]);
            }
            else msg = translate_morse(rest);

            break;
        }
        case "/piglatin":
        {
            if (rest == "") return 1;
            if (is_emote(rest))
            {
                if (is_smiley(rest)) rest = " " + rest;
                msg = ":" + translate_iglatinpay(rest[1..<1]);
            }
            else msg = translate_iglatinpay(rest);

            break;
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

int isAllowed(string channel, string usr, int flag)
{
    if (channels[channel]["read"])
        if (!isMember(usr, channels[channel]["read"]))
            return 0;

    return 1;
}
