/*
 * chan_d.c
 *
 * Tacitus @ LPUniversity
 * 30-SEPT-05
 * Modular Channel Daemon
 *
 */

/* Private Global variables */
privatev mapping Channels, Modules;

/* Prototypes */
int registerModule(string name, string path);
int registerCh(string moduleName, string chName);
int removeModule(string modulename);
int removeCh(string chName);
int tune(string channel, string user, int flag);
int valid_ch(string chName);
int valid_module(string moduleName);
int snd_msg(string channel, string user, string msg);
int filter_listing(string element);
string array getChls(string moduleName);
string array getMods();
string array getTuned(string argument);
int rec_msg(string channel, string msg);

/*
 * Function name: create
 * Description:   Initialize the channel daemon
 */
void create()
{
    string array modInstalled;

    Channels = ([ ]);
    Modules = ([ ]);

    modInstalled = explode(
      read_file("/adm/daemons/chmodules/modInstalled"),
      "\n"
    );

    if (!sizeof(modInstalled)) return;

    modInstalled = filter(
      modInstalled,
      (: $1[0] != '#' && $1[0..0] != "" && sizeof($1) :)
    );

    write("\n");

    for (int i = 0 ; i < sizeof(modInstalled) ; i++)
    {
        mixed err;
#ifdef __HAS_RUSAGE__
        mapping before, after;
#endif
        int cmd_time = 0;

        if (find_object(modInstalled[i])) destruct(find_object(modInstalled[i]));

        write("  Loading channel module: " + modInstalled[i] + "...");

#ifdef __HAS_RUSAGE__
        before = rusage();
#else
        cmd_time = time_expression
        {
#endif
            err = catch (load_object(modInstalled[i]));
#ifndef __HAS_RUSAGE__
        };
#else
        after = rusage();

        if (sizeof(before) && sizeof(after))
            cmd_time = after["utime"] - before["utime"];

        cmd_time *= 1000;
#endif

        if (stringp(err))
            write("\n\tError " + err + " when loading " + modInstalled[i] + "\n");
        else
        {
            if (cmd_time == 0)
                write("\n");
            else if (cmd_time >= 1000000)
                write(sprintf("(%.3f s)\n", cmd_time / 1000000.0));
            else if (cmd_time >= 1000)
                write(sprintf("(%.3f ms)\n", cmd_time / 1000.0));
            else
                write(sprintf("(%.3f us)\n", cmd_time * 1.0));
        }
    }
}

int registerModule(string moduleName, string path)
{
    if (!moduleName || !path) return 0;

    moduleName = lower_case(moduleName);

    if (valid_module(moduleName))
    {
        if (Modules[moduleName] == path) return 1;
        else return -1;
    }

    Modules[moduleName] = path;
    return 1;
}

int unregisterModule(string moduleName)
{
    if (!moduleName) return 0;

    moduleName = lower_case(moduleName);

    if (!valid_module(moduleName)) return -1;
    if (Modules[moduleName] != file_name(previous_object())) return -1;

    foreach (mixed key, mixed value in Channels)
        if (value["module"] == moduleName)
            map_delete(Channels, key);

    map_delete(Modules, moduleName);
    return 1;
}

int registerCh(string moduleName, string chName)
{
    string newName = chName;

    if (!moduleName) return 0;

    moduleName = lower_case(moduleName);

    if (!valid_module(moduleName)) return -1;
    if (Modules[moduleName] != file_name(previous_object())) return -1;

    if (valid_ch(chName))
    {
        if (Channels[chName]["module"] == moduleName)
        {
            Channels[chName]["listeners"] = ({ });
            return 1;
        }
        else newName = moduleName[0..3] + chName;
    }

    Channels[newName] = ([
      "module": moduleName,
      "real_name" : chName,
      "listeners" : ({ }),
      "colour": NOR,
      "prefix": "",
    ]);
    return 1;
}

int removeCh(string chName)
{
    if (member_array(chName, keys(Channels)) == -1) return 0;

    map_delete(Channels, chName);
    return 1;
}

int tune(string channel, string user, int flag)
{
    object modObj;

    if (!valid_ch(channel)) return 0;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return 0;
    }

    if (!objectp(modObj = find_object(Modules[Channels[channel]["module"]])))
        return 0;
    if (!modObj->isAllowed(Channels[channel]["real_name"], user, flag))
        return 0;
    if (flag == 1 && member_array(user, Channels[channel]["listeners"]) == -1)
        Channels[channel]["listeners"] += ({ user });
    if (flag == 0 && member_array(user, Channels[channel]["listeners"]) != -1)
        Channels[channel]["listeners"] -= ({ user });

    return 1;
}

int valid_ch(string chName)
{
    return (member_array(chName, keys(Channels)) != -1);
}

int valid_module(string moduleName)
{
    if (!moduleName) return 0;

    moduleName = lower_case(moduleName);

    return (member_array(moduleName, keys(Modules)) != -1);
}

int set_local_channel_name(string channel, string newChannel)
{
    object modObj;

    foreach (string tmp in keys(Channels))
    {
        if (channel == Channels[tmp]["real_name"])
        {
            channel = tmp;
            break;
        }
    }

    if (!valid_ch(channel) || valid_ch(newChannel)) return 0;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return 0;
    }

    if (!objectp(modObj = find_object(Modules[Channels[channel]["module"]])))
        return 0;
    if (modObj != previous_object()) return 0;

    Channels[newChannel] = copy(Channels[channel]);
    map_delete(Channels, channel);
    return 1;
}

mixed query_real_channel_name(string channel)
{
    if (!valid_ch(channel)) return 0;

    return Channels[channel]["real_name"];
}

int set_channel_prefix(string channel, string prefix)
{
    if (!valid_ch(channel)) return 0;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return 0;
    }

    if (!objectp(find_object(Modules[Channels[channel]["module"]])))
        return 0;

    Channels[channel]["prefix"] = prefix;
    return 1;
}

string query_channel_prefix(string channel, string prefix)
{
    if (!valid_ch(channel)) return "";
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return "";
    }

    if (!objectp(find_object(Modules[Channels[channel]["module"]])))
        return "";

    return Channels[channel]["prefix"];
}

int remove_channel_prefix(string channel)
{
    if (!valid_ch(channel)) return 0;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return 0;
    }

    if (!objectp(find_object(Modules[Channels[channel]["module"]])))
        return 0;

    Channels[channel]["prefix"] = "";
    return 1;
}

int set_chanCol(string channel, string colour)
{
    object modObj;

    if (!valid_ch(channel)) return 0;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return 0;
    }

    if (!objectp(modObj = find_object(Modules[Channels[channel]["module"]])))
        return 0;

    Channels[channel]["colour"] = colour;
    return 1;
}

string query_chanCol(string channel)
{
    object modObj;

    if (!valid_ch(channel)) return NOR;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return NOR;
    }

    if (!objectp(modObj = find_object(Modules[Channels[channel]["module"]])))
        return NOR;

    return Channels[channel]["colour"];
}

int remove_chanCol(string channel)
{
    object modObj;

    if (!valid_ch(channel)) return 0;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return 0;
    }

    if (!objectp(modObj = find_object(Modules[Channels[channel]["module"]])))
        return 0;

    Channels[channel]["colour"] = NOR;
    return 1;
}

int snd_msg(string channel, string user, string msg)
{
    object modObj;
    string array tmp = ({ });
    string emote, arg;

    if (!valid_ch(channel)) return 0;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return 0;
    }

    if (!objectp(modObj = find_object(Modules[Channels[channel]["module"]])))
        return 0;
    if (member_array(user, Channels[channel]["listeners"]) == -1) return 0;
    if (!modObj->isAllowed(Channels[channel]["real_name"], user)) return 0;
    if (!msg) return(notify_fail("Syntax: <channel> <msg>\n"));
    if (sscanf(msg, "%s %s", emote, arg) == 2)
        tmp = SOUL_D->request_channel_emote(emote, arg);
    else tmp = SOUL_D->request_channel_emote(msg);

    if (sizeof(tmp) > 0)
    {
        string target, o_msg, t_msg, other_msg;
        int result;

        switch (sizeof(tmp))
        {
            case 2:
            case 3:
                o_msg = tmp[0];
                other_msg = tmp[1];

                if (strsrch(other_msg, "$N") == -1)
                    other_msg = "$N -- " + other_msg;

                result = modObj->rec_e_msg(
                  Channels[channel]["real_name"],
                  user,
                  o_msg,
                  other_msg
                );
                break;
            case 4:
                o_msg = tmp[0];
                t_msg = tmp[1];
                other_msg = tmp[2];

                if (strsrch(t_msg, "$N") == -1)
                    t_msg = "$N -- " + t_msg;
                if (strsrch(other_msg, "$N") == -1)
                    other_msg = "$N -- " + other_msg;
                if (objectp(tmp[3])) target = tmp[3]->query_cap_name();
                else target = tmp[3];

                result = modObj->rec_t_msg(
                  Channels[channel]["real_name"],
                  user,
                  target,
                  o_msg,
                  t_msg,
                  other_msg
                );
                break;
            default:
                return 0;
        }

        return result;
    }

    if (modObj->rec_m_msg(Channels[channel]["real_name"], user, msg)) return 1;
    else return 0;
}

string array getChls(string moduleName)
{
    string array ret = ({ });
    string array chans;
    int i;

    if (!moduleName) return ret;

    moduleName = lower_case(moduleName);

    chans = keys(Channels);

    if (moduleName == "all") ret = chans;
    else
    {
        for (i = 0 ; i < sizeof(chans) ; i++)
            if (Channels[chans[i]]["module"] == moduleName)
                ret += ({ chans[i] });
    }

    ret = filter_array(ret, "filter_listing", this_object());
    ret = sort_array(ret, 1);
    return ret;
}

string array getTuned(string channel)
{
    string array ret = ({ });

    if (!channel) return ret;
    if (sizeof(Channels[channel]["listeners"]) <= 0) return ret;

    ret = Channels[channel]["listeners"];

    foreach (string name in ret)
        if (find_living(name))
            if (!interactive(find_living(name))) ret -= ({ name });

    return ret;
}

int filter_listing(string channel)
{
    object modObj = find_object(Modules[Channels[channel]["module"]]);
    string name;

    if (modObj && objectp(modObj))
    {
        if (this_user()) name = this_user()->query_name();
        else name = master()->privs_file(file_name(previous_object()));

        if (modObj->isAllowed(Channels[channel]["real_name"], name)) return 1;
    }

    return 0;
}

string array getMods()
{
    return copy(keys(Modules));
}

int rec_msg(string channel, string msg)
{
    object living;
    string array listeners;
    int i;

    if (!valid_ch(channel)) return -1;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return -2;
    }

    listeners = Channels[channel]["listeners"];

    for (i = 0 ; i < sizeof(listeners) ; i ++)
    {
        living = find_living(listeners[i]);

        if (!objectp(living))
        {
            Channels[channel]["listeners"] -= ({ listeners[i] });
            continue;
        }

        if (living->query_env("chantime"))
            tell_object(living, ctime(time())[11..15] + " " + msg + NOR);
        else tell_object(living, msg + NOR);

    }

    return 1;
}

int rec_m_msg(string channel, string originator, string msg)
{
    object living;
    string array listeners;
    string colour, prefix;
    int i;

    if (!valid_ch(channel)) return 0;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return 0;
    }

    listeners = Channels[channel]["listeners"];
    colour = Channels[channel]["colour"];
    prefix = Channels[channel]["prefix"];

    if (strsrch(originator, "@") == -1)
        originator = capitalize(originator);

    msg = sprintf("[%s%s%s%s] %s: %s\n",
      prefix,
      colour,
      channel,
      NOR,
      originator,
      msg
    );

    for (i = 0 ; i < sizeof(listeners) ; i ++)
    {
        string ct = "";

        living = find_living(listeners[i]);

        if (!objectp(living))
        {
            Channels[channel]["listeners"] -= ({ listeners[i] });
            continue;
        }

        if (arrayp(living->query_earmuffed()) && member_array(lower_case(originator), living->query_earmuffed()) != -1)
            continue;

        if (living->query_env("chantime")) ct = ctime(time())[11..15] + " ";

        tell_object(living, ct + msg + NOR);
    }

    return 1;
}

int rec_e_msg(string channel, string originator, string o_msg, string other_msg)
{
    object living;
    string array listeners;
    string colour, prefix;
    int i;

    if (!valid_ch(channel)) return 0;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return 0;
    }

    listeners = Channels[channel]["listeners"];
    colour = Channels[channel]["colour"];
    prefix = Channels[channel]["prefix"];

    if (o_msg != "")
        o_msg = sprintf("[%s%s%s%s] %s\n", prefix, colour, channel, NOR, o_msg);

    other_msg = sprintf("[%s%s%s%s] %s\n", prefix, colour, channel, NOR, other_msg);

    if (strsrch(originator, "@") == -1)
        other_msg = replace_string(other_msg, "$N", capitalize(originator));
    else other_msg = replace_string(other_msg, "$N", originator);

    for (i = 0 ; i < sizeof(listeners) ; i ++)
    {
        string ct = "";

        living = find_living(listeners[i]);

        if (!objectp(living))
        {
            Channels[channel]["listeners"] -= ({ listeners[i] });
            continue;
        }

        if (arrayp(living->query_earmuffed()) && member_array(lower_case(originator), living->query_earmuffed()) != -1)
            continue;

        if (living->query_env("chantime")) ct = ctime(time())[11..15] + " ";
        if (listeners[i] == originator && o_msg != "")
            tell_object(living, ct + o_msg + NOR);
        else tell_object(living, ct + other_msg + NOR);
    }

    return 1;
}

int rec_t_msg(string channel, string originator, string target, string o_msg, string t_msg, string other_msg)
{
    object living;
    string array listeners;
    string colour, prefix, t_user, t_mud;
    int i;

    if (!valid_ch(channel)) return 0;
    if (!valid_module(Channels[channel]["module"]))
    {
        map_delete(Channels, channel);
        return 0;
    }

    listeners = Channels[channel]["listeners"];
    colour = Channels[channel]["colour"];
    prefix = Channels[channel]["prefix"];

    if (o_msg != "")
    {
        o_msg = sprintf("[%s%s%s%s] %s\n", prefix, colour, channel, NOR, o_msg);
        o_msg = replace_string(o_msg, "$O", target);
    }

    t_msg = sprintf("[%s%s%s%s] %s\n", prefix, colour, channel, NOR, t_msg);

    if (strsrch(originator, "@") == -1)
        t_msg = replace_string(t_msg, "$N", capitalize(originator));
    else t_msg = replace_string(t_msg, "$N", originator);

    t_msg = replace_string(t_msg, "$O", target);

    if (other_msg != "")
    {
        other_msg = sprintf("[%s%s%s%s] %s\n", prefix, colour, channel, NOR, other_msg);

        if (strsrch(originator, "@") == -1)
            other_msg = replace_string(other_msg, "$N", capitalize(originator));
        else other_msg = replace_string(other_msg, "$N", originator);

        other_msg = replace_string(other_msg, "$O", target);
    }

    sscanf(target, "%s@%s", t_user, t_mud);

    if (!stringp(t_user)) t_user = "";
    else t_user = lower_case(t_user);

    if (!stringp(t_mud)) t_mud = "";

    for (i = 0 ; i < sizeof(listeners) ; i ++)
    {
        string ct = "";

        living = find_living(listeners[i]);

        if (!objectp(living))
        {
            Channels[channel]["listeners"] -= ({ listeners[i] });
            continue;
        }

        if (arrayp(living->query_earmuffed()) && member_array(lower_case(originator), living->query_earmuffed()) != -1)
            continue;

        if (living->query_env("chantime")) ct = ctime(time())[11..15] + " ";
        if (listeners[i] == originator) tell_object(living, ct + o_msg + NOR);
        else if (listeners[i] == lower_case(target)
          || (listeners[i] == t_user && mud_name() == t_mud))
            tell_object(living, ct + t_msg + NOR);
        else tell_object(living, ct + other_msg + NOR);
    }

    return 1;
}
