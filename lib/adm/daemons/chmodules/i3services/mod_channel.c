/*
 * mod_channel.c
 *
 * Tricky@Rock the Halo
 * 18-AUG-2006
 * Intermud3 channel service
 *
 * Last edited on November 29th, 2007 by Tricky
 */

#include <intermud3.h>
#include <ansi.h>

#ifndef ORA
#define ORA "%^ORANGE%^"
#endif
#ifndef HIK
#define HIK "%^BOLD%^%^BLACK%^"
#endif

privatev object       I3, ChanD, HistD;
privatev mapping      ChanIndex, AllowList;
privatev string array Listening;

mapping ChanList;

privatef int load_allow_list();
void send_channel_listen(string channel, int flag);
string channel_colour(string channel);
int fix_chan_index();
void register_channel(string channel);
varargs void unregister_channel(string channel, int);
int channel_filter(string channel);

void create()
{
    if (!objectp(ChanD = find_object(CHAN_D))) return;
    if (!objectp(load_object(HISTORY_D))) return;

    ChanD->registerModule("I3", file_name(this_object()));
    load_allow_list();

    if (file_exists(CHANINDEX_FILE + ".o"))
        ChanIndex = restore_variable(read_file(CHANINDEX_FILE + ".o"));
    else ChanIndex = 0;

    if (file_exists(I3_CHANLIST_FILE + ".o")) restore_object(I3_CHANLIST_FILE);
    if (!mapp(ChanList)) ChanList = ([ ]);

    Listening = ({ });

    I3 = find_object(I3_CHDMOD);

    I3->add_service_name("channel", 1);
    I3->add_service("chanlist-reply", "chanlist_handler");
    I3->add_service("chan-who-req", "chan_who_req_handler");
    I3->add_service("chan-who-reply", "chan_who_reply_handler");
    I3->add_service("chan-user-req", "chan_user_req_handler");
    I3->add_service("chan-user-reply", "chan_user_reply_handler");
    I3->add_service("channel-m", "channel_m_handler");
    I3->add_service("channel-e", "channel_e_handler");
    I3->add_service("channel-t", "channel_t_handler");

    foreach (string channel in keys(ChanList))
        if (stringp(channel)) register_channel(channel);

    fix_chan_index();
    save_object(I3_CHANLIST_FILE);
}

void remove()
{
    save_object(I3_CHANLIST_FILE);

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    foreach (string channel in ChanD->getChls("I3"))
    {
        channel = ChanD->query_real_channel_name(channel);
        send_channel_listen(channel, 0);
        ChanD->removeCh(AllowList[channel]["local"]);
        HistD->rem_history(channel);
        HistD->rem_file_history(channel);
    }

    I3 = find_object(I3_CHDMOD);

    I3->remove_service_name("channel");
    I3->remove_service("chanlist-reply");
    I3->remove_service("chan-who-req");
    I3->remove_service("chan-who-reply");
    I3->remove_service("chan-user-req");
    I3->remove_service("chan-user-reply");
    I3->remove_service("channel-m");
    I3->remove_service("channel-e");
    I3->remove_service("channel-t");

    ChanD->unregisterModule("I3");

    destruct();
}

privatef int load_allow_list()
{
    string array lines;
    string file = read_file("/adm/etc/i3_channels");

    AllowList = ([ ]);

    if (!stringp(file) || file == "") return 0;

    lines = explode(file, "\n");
    lines = filter(lines, (: $1[0..0] != "#" && sizeof(trim($1)) :) );

    if (!sizeof(lines)) return 0;

    foreach (string line in lines)
    {
        string array parts = explode(line, ":");
        string i3_chan, local_chan, rd, wr, col;

        switch (sizeof(parts))
        {
            case 5: col = parts[4];
            case 4: wr = parts[3];
            case 3: rd = parts[2];
            case 2: i3_chan = parts[1];
            case 1: local_chan = parts[0];
        }

        if (stringp(local_chan) && local_chan != "")
        {
            string channel;

            if (stringp(i3_chan) && i3_chan != "")
                channel = i3_chan;
            else
                channel = local_chan;

            AllowList[channel] = ([ ]);
            AllowList[channel]["local"] = local_chan;

            if (stringp(rd) && rd != "")
                AllowList[channel]["read"] = rd;
            else
                AllowList[channel]["read"] = "intermud";

            if (stringp(wr) && wr != "")
                AllowList[channel]["write"] = wr;
            else
                AllowList[channel]["write"] = "intermud";

            if (stringp(col) && col != "")
                AllowList[channel]["colour"] = col;
            else
                AllowList[channel]["colour"] = channel_colour(channel);
        }
    }

    return 1;
}

string array query_listening()
{
    return copy(Listening);
}

varargs int isAllowed(string channel, string usr, int flag)
{
    mapping allow;
    string i3_chan = channel;

    if (undefinedp(channel) || undefinedp(usr)) return 0;

    ChanD = find_object(CHAN_D);

    if (!mapp(AllowList[channel]))
    {
        if (member_array(channel, keys(ChanList)) != -1)
            i3_chan = "*";
        else
            return 0;
    }

    allow = copy(AllowList[i3_chan]);

    if (allow["read"])
    {
        if (!isMember(usr, allow["read"]))
            return 0;
    }
    else if (!isMember(usr, "intermud")) return 0;

    /* If we have got this far and flag is undefined then just return 1 as
       we are ok. */
    if (undefinedp(flag)) return 1;
    if (flag != 0)
    {
        if (member_array(channel, Listening) == -1)
        {
            send_channel_listen(channel, 1);
            find_object(I3_CHDMOD)->setData("reconnect_wait", MIN_RETRY_TIME);
        }

        find_object(I3_UCACHE)->send_ucache_update(usr);
    }
    else
    {
        mixed array tuned;

        tuned = find_object(CHAN_D)->getTuned(AllowList[channel]["local"]);

        if ((!arrayp(tuned) || sizeof(tuned) == 0)
         && member_array(channel, Listening) != -1)
            send_channel_listen(channel, 0);
    }

    return 1;
}

int set_local_channel_name(string i3_chan, string local_chan)
{
    ChanD = find_object(CHAN_D);

    if (!ChanD->set_local_channel_name(i3_chan, local_chan))
        return 0;

    AllowList[i3_chan]["local"] = local_chan;
    return 1;
}

int rec_m_msg(string channel, string originator, string msg)
{
    mapping history;
    mixed array packet;
    string cmd, rest = "", i3Type, chan_msg, visname;
    string real_originator;
    int lines = 15;

    originator = lower_case(originator);
    visname = capitalize(originator);

    if (find_living(originator)
     && find_living(originator)->is_bot())
    {
        visname = capitalize(find_living(originator)->query_bot_name(originator));
//        originator = lower_case(find_living(originator)->query_bot_name());
    }
    else
    {
        if (this_interactive())
            real_originator = query_privs(this_interactive());
        else real_originator = query_privs(previous_object());

        if (!real_originator) real_originator = originator;
        if (mapp(AllowList[channel]) && AllowList[channel]["write"])
        {
            if (!isMember(real_originator, AllowList[channel]["write"]))
                return 0;
        }
        else if (!isMember(real_originator, "intermud")) return 0;
    }

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);
    history = HistD->get_history();

    if (sscanf(msg, "%s %s", cmd, rest) != 2) cmd = msg;

    switch (cmd) /* We could do some neat stuff here! */
    {

        case "/last":
        {
            object ob = find_player(originator);

            if (rest != "") lines = to_int(rest);
            if (lines > HISTORY_SIZE) lines = HISTORY_SIZE;

            if (!sizeof(history[channel]))
                tell_object(ob, "Notice [I3]: Channel " + channel + " has no history yet.\n");
            else
                foreach (string histLine in history[channel][(sizeof(history[channel]) - lines)..<1])
                    tell_object(ob, replace_string(histLine, channel, AllowList[channel]["local"]));

            return 1;
        }
        case "/all":
        {
            object ob = find_player(originator);

            if (!sizeof(history[channel]))
                tell_object(ob, "Notice [I3]: Channel " + channel + " has no history yet.\n");
            else
                foreach (string histLine in history[channel][0..<1])
                    tell_object(ob, replace_string(histLine, channel, AllowList[channel]["local"]));

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
            else msg = translate_morse("(ENCODED): " + rest);

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

    packet = ({
      channel,
      visname,
    });

    if (is_emote(msg))
    {
        i3Type = "channel-e";

        if (is_smiley(msg)) msg = " " + msg;

        packet += ({ "$N " + msg[1..<1] });
        chan_msg = " " + msg[1..<1];

        if (is_morse(msg)) msg += "\n(MORSE) " + translate_morse(msg[1..<1], 1);

        ChanD->rec_e_msg(AllowList[channel]["local"], visname, "", "$N " + msg[1..<1]);
    }
    else
    {
        i3Type = "channel-m";
        packet += ({ msg });
        chan_msg = ": " + msg;

        if (is_morse(msg)) msg += "\n(MORSE): " + translate_morse(msg, 1);

        ChanD->rec_m_msg(AllowList[channel]["local"], visname, msg);
    }

    find_object(I3_CHDMOD)->sendToAll(i3Type, originator, packet);

    chan_msg = sprintf("[%s] %s%s\n",
      ChanD->query_chanCol(AllowList[channel]["local"]) + channel + NOR,
      visname,
      chan_msg
    );
    HistD->add_history(channel, ({ chan_msg }) );
    fix_chan_index();

    return 1;
}

int rec_e_msg(string channel, string originator, string o_msg, string other_msg)
{
    mixed array packet;
    string visname;
    string real_originator;

    originator = lower_case(originator);
    visname = capitalize(originator);

    if (find_living(originator)
     && find_living(originator)->is_bot())
    {
        visname = capitalize(find_living(originator)->query_bot_name(originator));
//        originator = lower_case(find_living(originator)->query_bot_name());
    }
    else
    {
        if (this_interactive())
            real_originator = query_privs(this_interactive());
        else real_originator = query_privs(previous_object());

        if (!real_originator) real_originator = originator;
        if (mapp(AllowList[channel]) && AllowList[channel]["write"])
        {
            if (!isMember(real_originator, AllowList[channel]["write"]))
                return 0;
        }
        else if (!isMember(real_originator, "intermud")) return 0;
    }

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);
    originator = lower_case(originator);

    ChanD->rec_e_msg(AllowList[channel]["local"], originator, o_msg, other_msg);

    packet = ({
      channel,
      visname,
      other_msg,
    });

    find_object(I3_CHDMOD)->sendToAll("channel-e", originator, packet);
    other_msg = replace_string(other_msg, "$N", visname);
    other_msg = sprintf("[%s] %s\n",
      ChanD->query_chanCol(AllowList[channel]["local"]) + channel + NOR,
      other_msg
    );
    HistD->add_history(channel, ({ other_msg }) );
    fix_chan_index();

    return 1;
}

int rec_t_msg(string channel, string originator, string target, string o_msg, string t_msg, string other_msg)
{
    mixed array packet;
    string t_user, t_mud, visname;
    string real_originator;

    originator = lower_case(originator);
    visname = capitalize(originator);

    if (find_living(originator)
     && find_living(originator)->is_bot())
    {
        visname = capitalize(find_living(originator)->query_bot_name(originator));
//        originator = lower_case(find_living(originator)->query_bot_name());
    }
    else
    {
        if (this_interactive())
            real_originator = query_privs(this_interactive());
        else real_originator = query_privs(previous_object());

        if (!real_originator) real_originator = originator;
        if (mapp(AllowList[channel]) && AllowList[channel]["write"])
        {
            if (!isMember(real_originator, AllowList[channel]["write"]))
                return 0;
        }
        else if (!isMember(real_originator, "intermud")) return 0;
    }

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);
    originator = lower_case(originator);

    ChanD->rec_t_msg(AllowList[channel]["local"], originator, target, o_msg, t_msg, other_msg);

    sscanf(target, "%s@%s", t_user, t_mud);

    if (!stringp(t_user)) t_user = target;
    if (!stringp(t_mud) || t_mud == "") t_mud = mud_name();

    packet = ({
      channel,
      t_mud,
      lower_case(t_user),
      other_msg,
      t_msg,
      visname,
      t_user,
    });

    find_object(I3_CHDMOD)->sendToAll("channel-t", originator, packet);
    o_msg = replace_string(t_msg, "$N", visname);
    other_msg = replace_string(other_msg, "$O", target);
    other_msg = replace_string(other_msg, "$N", visname);
    t_msg = replace_string(t_msg, "$N", visname);
    other_msg = sprintf("[%s] %s\n",
      ChanD->query_chanCol(AllowList[channel]["local"]) + channel + NOR,
      other_msg
    );
    HistD->add_history(channel, ({ other_msg }) );
    fix_chan_index();

    return 1;
}

string channel_colour(string channel)
{
    string array channelColours = ({
      HIR, GRN, ORA, HIY,
      HIB, MAG, CYN, BRED + WHT,
      HIG, HIM, HIC, HIW,
      BBLU + HIY, BBLU + HIC, BRED + HIW, BRED + HIY,
    });
    int col = 0, len = strlen(channel);

    if (mapp(AllowList[channel]) && AllowList[channel]["colour"])
        return AllowList[channel]["colour"];

    for (int i = 0 ; i < len ; i++)
        col += ((channel[i] & 0x7f) + ((i / 2) + 1));

    col %= sizeof(channelColours);

    return channelColours[col];
}

mapping getChannelList()
{
    return copy(ChanList);
}

string array getChannels()
{
    return keys(ChanList);
}

void setChanIndex(string channel, int index)
{
    ChanIndex[channel] = index;
    write_file(CHANINDEX_FILE + ".o", save_variable(ChanIndex), 1);
}

void removeChanIndex(string channel)
{
    map_delete(ChanIndex, channel);
    write_file(CHANINDEX_FILE + ".o", save_variable(ChanIndex), 1);
}

int queryChanIndex(string channel)
{
    if (ChanIndex && member_array(channel, keys(ChanIndex)) != -1) return ChanIndex[channel];
    return -1;
}

void setChanIndexes(int index)
{
    foreach (string channel in keys(ChanList)) setChanIndex(channel, index);
}

int fix_chan_index()
{
    HistD = find_object(HISTORY_D);
    ChanIndex = HistD->fix_chan_index();

    foreach (string channel in keys(ChanIndex))
        if (member_array(channel, keys(ChanList)) == -1)
            map_delete(ChanIndex, channel);

    return write_file(CHANINDEX_FILE + ".o", save_variable(ChanIndex), 1);
}

void register_channel(string channel)
{
    string array names = ({ });
    string array i3_usrs = ({ });
    string array tmp1 = ({ "all", "I3", "+I3", "-I3", });

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    if (!objectp(ChanD) || !objectp(HistD)) return;

    if (!mapp(AllowList[channel]))
    {
        AllowList[channel] = ([ ]);
        AllowList[channel]["local"] = channel;

        if (mapp(AllowList["*"]))
        {
            AllowList[channel]["read"] = AllowList["*"]["read"];
            AllowList[channel]["write"] = AllowList["*"]["write"];
        }
        else
        {
            AllowList[channel]["read"] = "intermud";
            AllowList[channel]["write"] = "intermud";
        }

        AllowList[channel]["colour"] = channel_colour(channel);
    }

    /* This will always be true until channel_filter is re-written.
    if (!channel_filter(channel)) return;
    */

    ChanD->registerCh("I3", AllowList[channel]["local"]);

    if (member_array(AllowList[channel]["local"], ChanD->getChls("I3")) == -1)
        AllowList[channel]["local"] = "i3" + AllowList[channel]["local"];

    ChanD->set_chanCol(channel, AllowList[channel]["colour"]);
    ChanD->set_channel_prefix(channel, "I3-" + I3->getData("router_list")[0][0][1..<1] + ":");
    HistD->set_history_dir(channel, HISTORY_DIR);
    HistD->set_history_size(channel, HISTORY_SIZE);

    foreach (string t in copy(Listening))
        tmp1 += ({ t, "+" + t, "-" + t });

    /* Auto tune in all I3 users that want to be auto tuned */
    i3_usrs = filter(users(), (: isMember($1->query_name(), "intermud") :) );

    foreach (object o_user in i3_usrs)
    {
        string array user_channels = ({ });
        string array tmp = ({ });
        string array tmp2 = ({ });
        string auto_tune = o_user->query_env("auto_tune");
        int tuneIn = 0;

        if (!stringp(auto_tune)) continue;

        tmp2 = explode(auto_tune, " ");

        foreach (string chan_or_mod in tmp2)
            if (member_array(chan_or_mod, tmp1) == -1)
                tmp2 -= ({ chan_or_mod });

        foreach (string chan_or_mod in tmp2)
        {
            if (chan_or_mod == "all" || chan_or_mod == "+all")
                user_channels = ChanD->getChls("I3");
            else if (chan_or_mod == "-all")
                user_channels = ({ });
            else if (chan_or_mod == "I3" || chan_or_mod == "+I3")
                user_channels = ChanD->getChls("I3");
            else if (chan_or_mod == "-I3")
                user_channels = ({ });
            else if (chan_or_mod[0] == '+')
            {
                if (channel == chan_or_mod[1..<1])
                {
                    tmp = user_channels - ({ chan_or_mod[1..<1] });
                    user_channels = tmp + ({ chan_or_mod[1..<1] });
                }
            }
            else if (chan_or_mod[0] == '-')
            {
                if (channel == chan_or_mod[1..<1])
                    user_channels -= ({ chan_or_mod[1..<1] });
            }
            else if (channel == chan_or_mod)
            {
                tmp = user_channels - ({ chan_or_mod });
                user_channels = tmp + ({ chan_or_mod });
            }
        }

        if (member_array(AllowList[channel]["local"], user_channels) != -1)
            tuneIn = 1;
        if (tuneIn) names += ({ o_user->query_name() });
    }

    if (sizeof(names) > 0)
    {
        string registered;

        names = sort_array(names, 1);

        foreach (string name in names)
            ChanD->tune(AllowList[channel]["local"], name, 1);

        if (sizeof(names) > 1)
            registered = implode(names[0..<2], "', '") + "' and '" + names[<1];
        else registered = names[0];

        I3LOG(
          "Notice",
          "I3_CHANNEL",
          "register_channel",
          sprintf("Tuning in '%s' to channel '%s', local name '%s'",
            registered,
            channel,
            AllowList[channel]["local"]
          )
        );
    }

    HistD->load_channel_history(channel);
}

varargs void unregister_channel(string channel, int tuneOut)
{
    string local_channel = AllowList[channel]["local"];

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    if (!objectp(ChanD) || !objectp(HistD)) return;

    HistD->rem_history(channel);
    HistD->rem_file_history(channel);

    if (ChanIndex && member_array(channel, keys(ChanIndex)) != -1)
        removeChanIndex(channel);
    if (ChanList && member_array(channel, keys(ChanList)) != -1)
        map_delete(ChanList, channel);
    if (AllowList && member_array(channel, keys(AllowList)) != -1)
        map_delete(AllowList, channel);

    if (!undefinedp(tuneOut))
    {
        string array names = map(users(),
          (: isMember($1->query_name(), "intermud") && $1->query_name() :)
        );

        if (sizeof(names) > 0)
        {
            string unregistered;

            names = sort_array(names, 1);

            foreach (string name in names)
                ChanD->tune(local_channel, name, 0);

            if (sizeof(names) > 1)
                unregistered = implode(names[0..<2], "', '") + "' and '" + names[<1];
            else unregistered = names[0];

            I3LOG(
              "Notice",
              "I3_CHANNEL",
              "unregister_channel",
              sprintf("Tuning out '%s' from channel '%s', local name '%s'",
                unregistered,
                channel,
                local_channel
              )
            );
        }

//        send_channel_listen(channel, 0);
        Listening -= ({ channel });
    }

    ChanD->removeCh(local_channel);
    save_object(I3_CHANLIST_FILE);
}

int channel_filter(string channel)
{
    /* THIS FUNCTION ALLOWS US TO DISALLOW CERTAIN CHANNELS MUD-WIDE */
    /* FOR NOW ALL CHANNELS WILL HAVE TO BE WHITELISTED */
    /* COMMENT EVERYTHING OUT AND RETURN 1 TO DISABLE */
    /* FUTURE RELEASE WILL INCLUDE ADMIN-FRONTEND */

    if (!mapp(AllowList[channel])) return 0;

    return 1;
}

void send_channel_listen(string channel, int flag)
{
    find_object(I3_CHDMOD)->sendToRouter("channel-listen", 0, ({
      channel,
      flag
    }) );

    if (!flag) Listening -= ({ channel });
    else       Listening += ({ channel });

    save_object(I3_CHANLIST_FILE);
}

void send_chan_who_req(mixed t_user, string t_mud, string channel)
{
    if (objectp(t_user)) t_user = lower_case(t_user->query_name());
    else if (stringp(t_user)) t_user = lower_case(t_user);
    else return;

    find_object(I3_CHDMOD)->sendToMud("chan-who-req", t_user, t_mud, ({ channel }) );
}

void send_chan_user_req(string t_mud, string t_user)
{
    find_object(I3_CHDMOD)->sendToMud("chan-user-req", 0, t_mud, ({ t_user }) );
}

void send_channel_add(object o_user, string channel, int type)
{
    tell_object(o_user, "Adding/Updating '%^BOLD%^" + channel + "%^RESET%^' to the router.\n");

    find_object(I3_CHDMOD)->sendToRouter("channel-add", o_user->query_name(), ({ channel, type }) );
    send_channel_listen(channel, 1);
    register_channel(channel);
}

void send_channel_remove(object o_user, string channel)
{
    if (ChanList[channel][0] != mud_name()) return;

    tell_object(o_user, "Removing '%^BOLD%^" + channel + "%^RESET%^' from the router.\n");

    find_object(I3_CHDMOD)->sendToRouter("channel-remove", o_user->query_name(), ({ channel }) );
    send_channel_listen(channel, 0);
    unregister_channel(channel);
}

void send_channel_admin(object o_user, string channel, string array add, string array remove)
{
    if (ChanList[channel][0] != mud_name()) return;

    if (sizeof(add) == 0) add = ({ });
    else
    {
        tell_object(o_user, "Adding these muds to the allowed/banned list for '%^BOLD%^" + channel + "%^RESET%^'\n");
        tell_object(o_user, sprintf("  %-=70s\n", implode(add, ", ")));
    }

    if (sizeof(remove) == 0) remove = ({ });
    else
    {
        tell_object(o_user, "Removing these muds from the allowed/banned list for '%^BOLD%^" + channel + "%^RESET%^'\n");
        tell_object(o_user, sprintf("  %-=70s\n", implode(remove, ", ")));
    }

    find_object(I3_CHDMOD)->sendToRouter("channel-admin", o_user->query_name(), ({ channel, add, remove }) );
}

void chanlist_handler(mixed array packet)
{
    string log_msg;

    I3 = find_object(I3_CHDMOD);

    ANNOUNCE_CHDMOD->rec_m_msg(
      "intermud3",
      "I3_CHANNEL",
      "Start eval_cost = " + eval_cost()
    );
    I3LOG(
      "Warning",
      "I3_CHANNEL",
      "chanlist-reply",
      "Start eval_cost = " + eval_cost()
    );

    if (packet[I3_O_MUD] != I3->getData("router_list")[0][0])
    {
        ANNOUNCE_CHDMOD->rec_m_msg(
          "intermud3",
          "I3_CHANNEL",
          "Spoof attack, packet = " + sprintf("%=O", packet)
        );
        I3LOG(
          "Warning",
          "I3_CHANNEL",
          "chanlist-reply",
          "Spoof attack, packet = " + sprintf("%=O", packet)
        );
        return;
    }

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    foreach (string channel in copy(Listening))
    {
        if (member_array(channel, keys(packet[7])) == -1)
        {
            ChanD->removeCh(AllowList[channel]["local"]);
            HistD->rem_history(channel);
            HistD->rem_file_history(channel);
            Listening -= ({ channel });
        }
    }

    I3->setData("chanlist_id", packet[6]);
    ANNOUNCE_CHDMOD->rec_m_msg(
      "intermud3",
      "I3_CHANNEL",
      "chanlist_id: " + packet[6]
    );

    if (!ChanIndex)
        if (file_exists(CHANINDEX_FILE + ".o"))
            ChanIndex = restore_variable(read_file(CHANINDEX_FILE + ".o"));

    foreach (mixed channel, mixed host_info in packet[7])
    {
        log_msg = "";
        channel = "" + channel;

        if (!host_info && ChanList[channel])
        {
            log_msg = "Deleting channel '" + channel + "' from the chanlist.";
            unregister_channel(channel, 1);
        }
        else if (host_info)
        {
            if (ChanList)
            {

                if (ChanList[channel] && ChanList[channel] != host_info)
                    log_msg = "Updating data for channel '" + channel + "' in the chanlist.";
                else if (!ChanList[channel])
                    log_msg = "Adding channel '" + channel + "' to the chanlist.";

            }
            else
            {
                ChanList = ([ ]);
                log_msg = "Creating chanlist. Adding channel '" + channel + "' to the chanlist.";
            }

            ChanList[channel] = host_info;
            send_channel_listen(channel, 1);
            register_channel(channel);
        }

        ANNOUNCE_CHDMOD->rec_m_msg(
          "intermud3",
          "I3_CHANNEL",
          log_msg
        );
        I3LOG(
          "Notice",
          "I3_CHANNEL",
          "chanlist-reply",
          TERMINAL_D->parse_pinkfish(log_msg, "dumb")
        );
    }

    fix_chan_index();
    ANNOUNCE_CHDMOD->rec_m_msg(
      "intermud3",
      "I3_CHANNEL",
      "End eval_cost = " + eval_cost()
    );
    I3LOG(
      "Warning",
      "I3_CHANNEL",
      "chanlist-reply",
      "End eval_cost = " + eval_cost()
    );

    save_object(I3_CHANLIST_FILE);
}

void channel_m_handler(mixed array packet)
{
    object i3_ucache = find_object(I3_UCACHE);
    string chan_msg, channel, visname, username, message, tmp, stmp;

    username = explode(packet[I3_O_USER], "!")[0];

    i3_ucache->checkUser(packet[I3_O_MUD], username, packet[7], 1);
    visname = i3_ucache->getVisname(packet[I3_O_MUD], username);

    if (packet[I3_O_MUD] == I3_MUD_NAME) return;

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    channel = packet[6];
    tmp = message = packet[8];
    stmp = TERMINAL_D->parse_pinkfish(tmp, "dumb");

    if (is_morse(stmp)) tmp += "\n(MORSE): " + translate_morse(stmp, 1);

    ChanD->rec_m_msg(AllowList[channel]["local"], visname + "@" + packet[I3_O_MUD], tmp);

    chan_msg = sprintf("[%s] %s@%s: %s\n",
      ChanD->query_chanCol(AllowList[channel]["local"]) + channel + NOR,
      visname,
      packet[I3_O_MUD],
      message
    );
    HistD->add_history(channel, ({ chan_msg }) );
    fix_chan_index();
}

void channel_e_handler(mixed array packet)
{
    object i3_ucache = find_object(I3_UCACHE);
    string message, chan_msg, channel, visname, username, tmp, stmp;

    username = explode(packet[I3_O_USER], "!")[0];

    i3_ucache->checkUser(packet[I3_O_MUD], username, packet[7], 1);
    visname = i3_ucache->getVisname(packet[I3_O_MUD], username);

    if (packet[I3_O_MUD] == I3_MUD_NAME) return;

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    channel = packet[6];
    tmp = message = packet[8];
    stmp = TERMINAL_D->parse_pinkfish(tmp, "dumb");

    if (is_morse(replace_string(stmp, "$N", "")))
        tmp += "\n(MORSE): " + translate_morse(replace_string(stmp, "$N", ""), 1);

    ChanD->rec_e_msg(AllowList[channel]["local"], visname + "@" + packet[I3_O_MUD], "", tmp);

    message = replace_string(message, "$N", sprintf("%s@%s", visname, packet[I3_O_MUD]));
    chan_msg = sprintf("[%s] %s\n",
      ChanD->query_chanCol(AllowList[channel]["local"]) + channel + NOR,
      message
    );
    HistD->add_history(channel, ({ chan_msg }) );
    fix_chan_index();
}

void channel_t_handler(mixed array packet)
{
    object i3_ucache = find_object(I3_UCACHE);
    string other_msg, t_msg, channel, chan_msg, target, username, visname;

    username = explode(packet[I3_O_USER], "!")[0];

    i3_ucache->checkUser(packet[I3_O_MUD], username, packet[11], 1);
    visname = i3_ucache->getVisname(packet[I3_O_MUD], username);

    if (packet[I3_O_MUD] == I3_MUD_NAME) return;

    ChanD = find_object(CHAN_D);
    HistD = find_object(HISTORY_D);

    channel = packet[6];

    if (packet[7] != mud_name()) target = packet[12] + "@" + packet[7];
    else target = packet[12];

    other_msg = packet[9];
    t_msg = packet[10];

    ChanD->rec_t_msg(AllowList[channel]["local"], visname + "@" + packet[I3_O_MUD], target, "", t_msg, other_msg);

    other_msg = replace_string(other_msg, "$N", sprintf("%s@%s", packet[11], packet[I3_O_MUD]));
    other_msg = replace_string(other_msg, "$O", packet[12] + "@" + packet[7]);
    chan_msg = sprintf("[%s] %s\n",
      ChanD->query_chanCol(AllowList[channel]["local"]) + channel + NOR,
      other_msg
    );
    HistD->add_history(channel, ({ chan_msg }) );
    fix_chan_index();
}

void chan_who_req_handler(mixed array packet)
{
    string channel = packet[6];
    string array listeners = ({ });
    string array tuned;

    ChanD = find_object(CHAN_D);
    tuned = ChanD->getTuned(AllowList[channel]["local"]);

    if (!arrayp(tuned) || tuned == 0)
    {
        find_object(I3_ERROR)->send_error(
          0,
          packet[I3_O_MUD],
          packet[I3_O_USER],
          "unk-channel",
          "Never heard of '" + channel + "'",
          packet
        );

        return;
    }

    foreach (string user in tuned) listeners += ({ capitalize(user) });

    find_object(I3_CHDMOD)->sendToUser("chan-who-reply", 0, packet[I3_O_MUD], packet[I3_O_USER], ({
      channel,
      listeners
    }) );
}

void chan_who_reply_handler(mixed array packet)
{
    object       user;
    string array listeners = packet[7];
    string       channel = packet[6];

    if (!(user = find_player(packet[I3_T_USER])))
    {
        find_object(I3_ERROR)->send_error(
          0,
          packet[I3_O_MUD],
          packet[I3_O_USER],
          "unk-user",
          packet[I3_T_USER] + "@" + packet[I3_T_MUD] + " was not found!",
          packet
        );

        return;
    }

    if (sizeof(listeners) == 0)
    {
/*
        tell_object(user, sprintf("There is nobody Listening on '%s' to '%s'\n",
          "%^BOLD%^" + packet[I3_O_MUD] + "%^RESET%^",
          "%^BOLD%^" + channel + "%^RESET%^"
        ) );
*/
        return;
    }

    tell_object(user, sprintf("The following user%s at '%s' %s tuned into '%s':\n",
      sizeof(listeners) == 1 ? "" : "s",
      "%^BOLD%^" + packet[I3_O_MUD] + "%^RESET%^",
      sizeof(listeners) == 1 ? "is" : "are",
      "%^BOLD%^" + channel + "%^RESET%^"
    ) );
    tell_object(user, sprintf("  %-=70s\n", implode(listeners, ", ")) );
}

void chan_user_req_handler(mixed array packet)
{
    object user = find_player(packet[6]);
    string capname;
    int gender;

    if (!user) user = find_living(packet[6]);

    if (!user)
    {
        find_object(I3_ERROR)->send_error(
          0,
          packet[I3_O_MUD],
          packet[I3_O_USER],
          "unk-user",
          packet[6] + "@" + packet[I3_T_MUD] + " was not found!",
          packet
        );

        return;
    }

    if (user->is_bot())
        capname = capitalize(user->query_bot_name(packet[6]));
    if (!capname) capname = capitalize(user->query_name());

    if (user->query("gender") == "male" || user->query_gender() == "male") gender = 0;
    else if (user->query("gender") == "female" || user->query_gender() == "female") gender = 1;
    else gender = 2;

    find_object(I3_CHDMOD)->sendToMud("chan-user-reply", 0, packet[I3_O_MUD], ({
      user->is_bot() ? packet[6] : user->query_name(),
      capname,
      gender
    }) );
}

void chan_user_reply_handler(mixed array packet)
{
    find_object(I3_UCACHE)->addUserCache(packet[I3_O_MUD], packet[6], packet[7], packet[8]);
}
