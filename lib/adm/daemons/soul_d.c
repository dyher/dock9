/* soul_daemon.c

Parthenon @ LPUniversity
23-JUN-06
LPUni soul daemon

*/

/* Last updated: July 16th, 2006 by Tacitus */

#include <intermud3.h>

/* Defines */

#define SAVE_PATH "/data/daemons/soul_d/soul_d.o"

privatev mapping players;

/* Global Variables */

mapping emotes;

/********************************/
/********** PROTOTYPES **********/
/********************************/

nomask varargs int request_emote(string emote, string who);
nomask varargs string array request_channel_emote(string emote, string who);
nomask varargs void do_emote(string emote, string mods, object target);
nomask varargs string array do_channel_emote(string emote, string mods, mixed target);
nomask void delete_emote(string emote);
nomask void do_save();
nomask void set_emote(string emote, string *msgs);
nomask varargs string build_emote(string msg, string emote, string mods, object target, int plural, int to_player);
nomask varargs string build_channel_emote(string msg, string emote, string mods, mixed target, int plural, int to_player);
nomask string *query_emotes();
nomask string *query_emote_msgs(string emote);

/* Functions */


void create()
{
    emotes = ([]);
    restore_object(SAVE_PATH);
}

/*********************************************/
/********** EMOTE PERFORM FUNCTIONS **********/
/*********************************************/

nomask varargs int request_emote(string emote, string arg)
{
    object target;
    int i;
    string *tmp_array, *mods_array;
    string mods;

    if(stringp(arg))
    {
        tmp_array = explode(arg, " ");
        target = present(tmp_array[0], environment(this_player()));
    }

    if(target)
    {
        if(member_array(emote+"/t", keys(emotes)) == -1) return 0;

        if(!present(target, environment(this_player())))
            return notify_fail("That person is not present\n");

        if(!living(target))
            return notify_fail("You can't do that to an inanimate object!\n");

        emote = emote+"/t";
    }
    else
    if(member_array(emote, keys(emotes)) == -1) return 0;

    if((target && (sizeof(tmp_array) > 1)) || (!target && (sizeof(tmp_array) > 0)))
    {
        for((target ? i = 1 : i = 0); i < sizeof(tmp_array); i++)
        {
            if(!mods_array) mods_array = ({ tmp_array[i] });
            else mods_array += ({ tmp_array[i] });
        }

        if(sizeof(mods_array) > 0) mods = implode(mods_array, " ");
    }
    else
    {
        if(target) mods = emotes[emote][3];
        else mods = emotes[emote][2];
    }

    if(target) do_emote(emote, mods, target);
    else do_emote(emote, mods);

    return 1;
}

int sort_time(mixed a, mixed b)
{

    if(players[a] < players[b]) return -1;
    else
    if(players[a] > players[b]) return 1;

    return 0;
}

nomask varargs string array request_channel_emote(string emote, string arg)
{
    mixed target;
    string array tmp_array;
    string array mods_array;
    string array return_msgs = ({ });
    string mods;
    int i;

    if(stringp(arg))
    {
        string u, m;
        tmp_array = explode(arg, " ");

        sscanf(tmp_array[0], "%s@%s", u, m);

        if(stringp(u))
        {
            mapping ucache = find_object(I3_UCACHE)->getUserCache();
            string array plrs = ({ });

            players = ([ ]);

            foreach(string mudname, mapping usernames in ucache)
                foreach(string username, mixed data in usernames)
                    if(data[I3_UC_LASTACTIVE] > 0) players[data[I3_UC_VISNAME] + "@" + mudname] = data[I3_UC_LASTACTIVE];

            foreach(string plr, int timestamp in players)
            {
                float fuzz;

                if(m == "")
                    fuzz = fuzzymatch(lower_case(u), lower_case(plr[0..strsrch(plr, "@") - 1]));
                else
                    fuzz = fuzzymatch(tmp_array[0], lower_case(plr));

                if(fuzz >= 97.5)
                {
                    plrs = ({ plr });
                    break;
                }
                else
                if(fuzz >= 200.0 / 3.0)
                {
                    string array tmp;

                    tmp = plrs - ({ plr });
                    plrs = tmp + ({ plr });
                }

            }

            if(sizeof(plrs) == 1) target = plrs[0];
            else
            if(sizeof(plrs) > 1)
            {
                plrs = sort_array(plrs, "sort_time");
                target = plrs[<1];
            }
            else target = 0;

        }
        else
            target = find_player(lower_case(tmp_array[0]));

    }

    if(target)
    {
        if(member_array(emote + "/t", keys(emotes)) == -1) return return_msgs;

        emote = emote + "/t";
    }
    else
    if(member_array(emote, keys(emotes)) == -1) return return_msgs;

    if((target && (sizeof(tmp_array) > 1)) || (!target && (sizeof(tmp_array) > 0)))
    {

        for((target ? i = 1 : i = 0); i < sizeof(tmp_array); i++)
        {
            if(!mods_array) mods_array = ({ tmp_array[i] });
            else mods_array += ({ tmp_array[i] });
        }

        if(sizeof(mods_array) > 0) mods = implode(mods_array, " ");

    }
    else
    {
        if(target) mods = emotes[emote][3];
        else mods = emotes[emote][2];
    }

    if(target) return_msgs = do_channel_emote(emote, mods, target);
    else return_msgs = do_channel_emote(emote, mods);

    return return_msgs;
}

nomask varargs void do_emote(string emote, string mods, object target)
{
    string *msgs;

    msgs = emotes[emote];

    if(emote[<2..<1] == "/t") emote = emote[0..<3];

    if(!target)
    {
        tell_object(this_player(), build_emote(msgs[0], emote, mods, 0, 0, 1) + "\n");
        tell_room(environment(this_player()), build_emote(msgs[1], emote, mods, 0, 1, 0)
             + "\n", this_player());
    }
    else
    {
        tell_object(this_player(), build_emote(msgs[0], emote, mods, target, 0, 1) + "\n");
        tell_room(environment(this_player()), build_emote(msgs[1], emote, mods, target, 1, 0) + "\n", ({ this_player(), target }) );

        if(target != this_player()) tell_object(target, build_emote(msgs[2], emote,
            mods, target, 1, 2) + "\n");
    }

    return;
}

nomask varargs string array do_channel_emote(string emote, string mods, mixed target)
{
    string array msgs;
    string array return_msgs = ({ });

    msgs = emotes[emote];

    if(emote[<2..<1] == "/t") emote = emote[0..<3];

    if(!target)
    {
        return_msgs += ({ build_channel_emote(msgs[0], emote, mods, 0, 0, 1) });
        return_msgs += ({ build_channel_emote(msgs[1], emote, mods, 0, 1, 0) });
    }
    else
    {
        return_msgs += ({ build_channel_emote(msgs[0], emote, mods, target, 0, 1) });

        if(target != this_player())
            return_msgs += ({ build_channel_emote(msgs[2], emote, mods, target, 1, 2) });

        return_msgs += ({ build_channel_emote(msgs[1], emote, mods, target, 1, 0) });
        return_msgs += ({ target });
    }

    return return_msgs;
}

nomask varargs string build_emote(string msg, string emote, string mods, object target, int plural, int to_player)
{
    string array tmp1 = ({ });
    string array tmp2 = ({ });
    int i, sz;

    if(target == this_player())
    {

        if(to_player == 1)
        {
            msg = replace_string(msg, "$PI", "your");
            msg = replace_string(msg, "$I", "You");
        }
        else msg = replace_string(msg, "$I", capitalize(this_player()->query_name()));

        if(to_player == 0)
        {

            switch(this_player()->query("gender"))
            {
                case "male":
                    msg = replace_string(msg, "$T", "himself");
                    break;
                case "female":
                    msg = replace_string(msg, "$T", "herself");
                    break;
                default:
                    msg = replace_string(msg, "$T", "itself");
            }

        }
        else if(to_player == 1) msg = replace_string(msg, "$T", "yourself");

    }

    if(target)
    {
        if(to_player == 1) msg = replace_string(msg, "$I", "You");
        else msg = replace_string(msg, "$I", capitalize(this_player()->query_name()));

        if(to_player == 1) msg = replace_string(msg, "$PI", "your");
        else
        {

            switch(this_player()->query("gender"))
            {
                case "male":
                    msg = replace_string(msg, "$PI", "his");
                    break;
                case "female":
                    msg = replace_string(msg, "$PI", "her");
                    break;
                default:
                    msg = replace_string(msg, "$PI", "its");
            }

        }

        switch(this_player()->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$SI", "he");
                break;
            case "female":
                msg = replace_string(msg, "$SI", "she");
                break;
            default:
                msg = replace_string(msg, "$SI", "it");
        }

        switch(this_player()->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$OI", "him");
                break;
            case "female":
                msg = replace_string(msg, "$OI", "her");
                break;
            default:
                msg = replace_string(msg, "$OI", "it");
        }

        if(to_player == 2) msg = replace_string(msg, "$T", "you");
        else msg = replace_string(msg, "$T", capitalize(target->query_name()) );

        if(to_player == 2) msg = replace_string(msg, "$PT", "your");
        else
        {

            switch(target->query("gender"))
            {
                case "male":
                    msg = replace_string(msg, "$PT", "his");
                    break;
                case "female":
                    msg = replace_string(msg, "$PT", "her");
                    break;
                default:
                    msg = replace_string(msg, "$PT", "its");
            }

        }

        switch(target->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$ST", "he");
                break;
            case "female":
                msg = replace_string(msg, "$ST", "she");
                break;
            default:
                msg = replace_string(msg, "$ST", "it");
        }

        switch(target->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$OT", "him");
                break;
            case "female":
                msg = replace_string(msg, "$OT", "her");
                break;
            default:
                msg = replace_string(msg, "$OT", "it");
        }

        msg = replace_string(msg, "$M", mods);

        if(plural) msg = replace_string(msg, "$V", ((emote[<1] == 's' || emote[<2..<1] == "sh") ? emote+"es" : (emote[<2..<1] == "ch" ? emote+"es" : (emote == "goes" ? "go" : emote+"s"))));
        else msg = replace_string(msg, "$V", emote);

    }
    else
    {
        if(to_player == 1) msg = replace_string(msg, "$I", "You");
        else if(to_player == 0) msg = replace_string(msg, "$I", capitalize(this_player()->query_name()));

        switch(this_player()->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$PI", "his");
                break;
            case "female":
                msg = replace_string(msg, "$PI", "her");
                break;
            default:
                msg = replace_string(msg, "$PI", "its");
        }

        switch(this_player()->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$SI", "he");
                break;
            case "female":
                msg = replace_string(msg, "$SI", "she");
                break;
            default:
                msg = replace_string(msg, "$SI", "it");
        }

        switch(this_player()->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$OI", "him");
                break;
            case "female":
                msg = replace_string(msg, "$OI", "her");
                break;
            default:
                msg = replace_string(msg, "$OI", "it");
        }

        msg = replace_string(msg, "$M", mods);

        if(plural)
            msg = replace_string(msg, "$V", ((emote[<1] == 's' || emote[<2..<1] == "sh") ? emote + "es" : (emote[<2..<1] == "ch" ? emote + "es" : (emote == "goes" ? "go" : emote + "s"))));
        else msg = replace_string(msg, "$V", emote);
    }

    tmp1 = explode(msg, " ");

    for(i = 0, sz = sizeof(tmp1); i < sz; i++)
        if(tmp1[i] != "") tmp2 += ({ tmp1[i] });

    msg = implode(tmp2, " ");

    if(msg[<1] != '.' && msg[<1] != '!' && msg[<1] != '?' &&
       msg[<2..<1] != ".\"" && msg[<2..<1] != "!\"" && msg[<2..<1] != "?\"" &&
       msg[<2..<1] != ".'" && msg[<2..<1] != "!'" && msg[<2..<1] != "?'")
        return msg + ".";
    else return msg;
}

nomask varargs string build_channel_emote(string msg, string emote, string mods, mixed target, int plural, int to_player)
{
    string array tmp1 = ({ });
    string array tmp2 = ({ });
    int i, sz;

    if(objectp(target) && target == this_player())
    {

        if(to_player == 1)
        {
            msg = replace_string(msg, "$PI", "your");
            msg = replace_string(msg, "$I", "You");
        }
        else msg = replace_string(msg, "$I", "$N");

        if(to_player == 0)
        {

            switch(this_player()->query("gender"))
            {
                case "male":
                    msg = replace_string(msg, "$T", "himself");
                    break;
                case "female":
                    msg = replace_string(msg, "$T", "herself");
                    break;
                default:
                    msg = replace_string(msg, "$T", "itself");
            }

        }
        else if(to_player == 1) msg = replace_string(msg, "$T", "yourself");

    }

    if(target)
    {
        if(to_player == 1) msg = replace_string(msg, "$I", "You");
        else msg = replace_string(msg, "$I", "$N");

        if(to_player == 1) msg = replace_string(msg, "$PI", "your");
        else
        {

            switch(this_player()->query("gender"))
            {
                case "male":
                    msg = replace_string(msg, "$PI", "his");
                    break;
                case "female":
                    msg = replace_string(msg, "$PI", "her");
                    break;
                default:
                    msg = replace_string(msg, "$PI", "its");
            }

        }

        switch(this_player()->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$SI", "he");
                break;
            case "female":
                msg = replace_string(msg, "$SI", "she");
                break;
            default:
                msg = replace_string(msg, "$SI", "it");
        }

        switch(this_player()->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$OI", "him");
                break;
            case "female":
                msg = replace_string(msg, "$OI", "her");
                break;
            default:
                msg = replace_string(msg, "$OI", "it");
        }

        if(to_player == 2) msg = replace_string(msg, "$T", "you");
        else msg = replace_string(msg, "$T", "$O");

        if(to_player == 2) msg = replace_string(msg, "$PT", "your");
        else if(stringp(target))
        {
            string mudname, username;
            int gender;

            sscanf(target, "%s@%s", username, mudname);
            gender = find_object(I3_UCACHE)->getGender(mudname, username);

            switch(gender)
            {
                case 0:
                    msg = replace_string(msg, "$PT", "his");
                    break;
                case 1:
                    msg = replace_string(msg, "$PT", "her");
                    break;
                default:
                    msg = replace_string(msg, "$PT", "its");
            }

            switch(gender)
            {
                case 0:
                    msg = replace_string(msg, "$ST", "he");
                    break;
                case 1:
                    msg = replace_string(msg, "$ST", "she");
                    break;
                default:
                    msg = replace_string(msg, "$ST", "it");
            }

            switch(gender)
            {
                case 0:
                    msg = replace_string(msg, "$OT", "him");
                    break;
                case 1:
                    msg = replace_string(msg, "$OT", "her");
                    break;
                default:
                    msg = replace_string(msg, "$OT", "it");
            }

        }
        else
        {

            switch(target->query("gender"))
            {
                case "male":
                    msg = replace_string(msg, "$PT", "his");
                    break;
                case "female":
                    msg = replace_string(msg, "$PT", "her");
                    break;
                default:
                    msg = replace_string(msg, "$PT", "its");
            }

            switch(target->query("gender"))
            {
                case "male":
                    msg = replace_string(msg, "$ST", "he");
                    break;
                case "female":
                    msg = replace_string(msg, "$ST", "she");
                    break;
                default:
                    msg = replace_string(msg, "$ST", "it");
            }

            switch(target->query("gender"))
            {
                case "male":
                    msg = replace_string(msg, "$OT", "him");
                    break;
                case "female":
                    msg = replace_string(msg, "$OT", "her");
                    break;
                default:
                    msg = replace_string(msg, "$OT", "it");
            }

        }

        msg = replace_string(msg, "$M", mods);

        if(plural) msg = replace_string(msg, "$V", ((emote[<1] == 's' || emote[<2..<1] == "sh") ? emote+"es" : (emote[<2..<1] == "ch" ? emote+"es" : (emote == "goes" ? "go" : emote+"s"))));
        else msg = replace_string(msg, "$V", emote);

    }
    else
    {
        if(to_player == 1) msg = replace_string(msg, "$I", "You");
        else if(to_player == 0) msg = replace_string(msg, "$I", "$N");

        switch(this_player()->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$PI", "his");
                break;
            case "female":
                msg = replace_string(msg, "$PI", "her");
                break;
            default:
                msg = replace_string(msg, "$PI", "its");
        }

        switch(this_player()->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$SI", "he");
                break;
            case "female":
                msg = replace_string(msg, "$SI", "she");
                break;
            default:
                msg = replace_string(msg, "$SI", "it");
        }

        switch(this_player()->query("gender"))
        {
            case "male":
                msg = replace_string(msg, "$OI", "him");
                break;
            case "female":
                msg = replace_string(msg, "$OI", "her");
                break;
            default:
                msg = replace_string(msg, "$OI", "it");
        }

        msg = replace_string(msg, "$M", mods);

        if(plural)
            msg = replace_string(msg, "$V", ((emote[<1] == 's' || emote[<2..<1] == "sh") ? emote + "es" : (emote[<2..<1] == "ch" ? emote + "es" : (emote == "goes" ? "go" : emote + "s"))));
        else msg = replace_string(msg, "$V", emote);
    }

    tmp1 = explode(msg, " ");

    for(i = 0, sz = sizeof(tmp1); i < sz; i++)
        if(tmp1[i] != "") tmp2 += ({ tmp1[i] });

    msg = implode(tmp2, " ");

    if(msg[<1] != '.' && msg[<1] != '!' && msg[<1] != '?' &&
       msg[<2..<1] != ".\"" && msg[<2..<1] != "!\"" && msg[<2..<1] != "?\"" &&
       msg[<2..<1] != ".'" && msg[<2..<1] != "!'" && msg[<2..<1] != "?'")
        return msg + ".";
    else return msg;
}

/***********************************************/
/********** EMOTE INTERFACE FUNCTIONS **********/
/***********************************************/

nomask void delete_emote(string emote)
{
    if(get_base(previous_object()) + ".c" != OBJ_SOUL_TOOL) return;

    map_delete(emotes, emote);
    do_save();
}

nomask void set_emote(string emote, string *msgs)
{
    if(get_base(previous_object()) + ".c" != OBJ_SOUL_TOOL) return;

    emotes[emote] = msgs;
    do_save();
}

nomask string *query_emotes()
{
    if(!emotes) restore_object(SAVE_PATH);

    return keys(emotes);
}

nomask string *query_emote_msgs(string emote)
{
    if(!emotes) restore_object(SAVE_PATH);

    return emotes[emote];
}

nomask void reset_emotes()
{
    if(get_base(previous_object()) + ".c" != OBJ_SOUL_TOOL) return;
    if(!emotes) restore_object(SAVE_PATH);

    emotes = ([]);
    save_object(SAVE_PATH);
}

nomask void do_save() { save_object(SAVE_PATH); }
