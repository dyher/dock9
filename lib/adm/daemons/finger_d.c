/* finger_d.c
 *
 * Tacitus @ LPUniversity
 * 26-MAR-06
 * Finger Daemon
 *
 */

/*
 * The finger daemon provides an interface to load user objects
 * to gather information.
 */

#include <socket.h>
#define BORDER1 "=+=--=+=--=+=--=+=--=+=--=--=+=--=+=--=+=--=+=--=+=--=+=--=+=--=+=--=+=--=+=\n"

string build_finger_info(mixed array finger_data);
string get_rank(object user);
mixed array get_finger_data(string username);
mixed get_user(string username);
mixed get_body(object user);

varargs string finger_user(string username, string where)
{
    mixed user, body;
    mixed array packet;
    string array *users;
    string ret, group;
    string rank, last_t, last;

    username = lower_case(username);

    if (where)
    {
        packet = ({
          "finger-req",
          5,
          mud_name(),
          query_privs(this_player()),
          where,
          0,
          username
        });
        "/adm/daemons/chmodules/i3.c"->send_packet(packet);
        return "";
    }

    if (sscanf(username, "(%s)", group))
    {
        users = master()->query_group(group);

        if (!arrayp(users) || sizeof(users) <= 0)
            return "%^RED%^Error:%^RESET%^ Group '" + group 
             + "' does not exist or has no members.\n";

        ret = BORDER1;
        ret += "\t\t%^BOLD%^GROUP:%^RESET%^ " + group + "\n";
        ret += BORDER1;
        ret += sprintf("%-20s %-40s %s\n", "Login", "Rank", "On");
        ret += BORDER1;

        foreach (username in users)
        {
            if (sscanf(username, "(%*s)"))
                ret += sprintf("%-20s %-40s %s\n", username, "(GROUP)", "---");
            else
            {
                if (find_player(username))
                {
                    last_t = "On Since";
                    body = find_player(username);
                    last = ctime(body->query("last_login"));
                }
                else
                {
                    user = get_user(username);
                    body = get_body(user);

                    if (objectp(user)) destruct(user);
                    if (!objectp(body)) continue;
                    else
                    {
                        last_t = "Last on";
                        last =  ctime(body->query("last_login"));
                        destruct(body);
                    }
                }

                rank = RANK_D->queryQualativeRank(username);
                rank = RANK_D->queryRankProperty(rank, "colour") +
                  capitalize(RANK_D->queryRankProperty(rank, "short")) +
                  NOR;

                ret += sprintf("%-20s %-40s %s\n",
                  capitalize(username), rank, last_t + " " + last);
            }
        }

        return ret += BORDER1;
    }
    else
    {
        mixed array finger_data = get_finger_data(username);

        if (!finger_data) return "That user does not exist.\n";
        else if (!sizeof(finger_data)) return "User data unavailable.\n";

        ret = build_finger_info(finger_data);
    }

    return ret;
}

string build_finger_info(mixed array finger_data)
{
    string ret, tmp, idle;
    int len, slen, sdiff, idleTime;

    tmp = HIW + (finger_data[0] != "" ? capitalize(finger_data[0]) : "<unknown>") + NOR;
    len = strlen(tmp);
    slen = strlen(find_object(TERMINAL_D)->parse_pinkfish(tmp, 1));
    sdiff = len - slen;
    ret = sprintf("\nName: %-" + (25 + sdiff) + "sRank: %s\n",
      tmp,
      finger_data[7] != "" ? "" + finger_data[7] : "<unknown>");

    if (finger_data[2] != "")
        ret += sprintf("%' '31sIn real life: %s\n", "", finger_data[2]);

    if (finger_data[3] != "")
        ret += sprintf("%' '31sEmail: %s\n", "", finger_data[3]);

    ret += sprintf("\nTitle: %s\n",
      finger_data[1] != "" ? replace_string(finger_data[1], "$N", finger_data[0] != "" ? capitalize(finger_data[0]) : "<unknown>") : "<unknown>");

    if (finger_data[5] == -1)
    {
        if ((stringp(finger_data[4]) && finger_data[4] != "")
         && (stringp(finger_data[6]) && finger_data[6] != ""))
            ret += sprintf("Left at %s from %s\n", finger_data[4], finger_data[6]);
        else if (stringp(finger_data[4]) && finger_data[4] != "")
            ret += sprintf("Left at %s\n", finger_data[4]);
        else if (stringp(finger_data[6]) && finger_data[6] != "")
            ret += sprintf("Last login from %s\n", finger_data[6]);
        else ret += "Not logged on.\n";
    }
    else
    {
        idleTime = finger_data[5];

        if (idleTime > 60)
        {
            int weeks, days, hours, mins, secs;

            weeks = idleTime / (86400 * 7);
            days = (idleTime / 86400) % 7;
            hours = (idleTime / 3600) % 24;
            mins = (idleTime / 60) % 60;
            secs = idleTime % 60;

            idle = YEL + "Idle: ";

            if (weeks) idle += sprintf("%dw ", weeks);
            if (days || (!days && weeks)) idle += sprintf("%dd ", days);
            if (hours || (!hours && days) || (!hours && weeks)) idle += sprintf("%dh ", hours);
            if (mins || (!mins && hours) || (!mins && days) || (!mins && weeks)) idle += sprintf("%dm ", mins);
            if (secs || (!secs && mins) || (!secs && hours) || (!secs && days) || (!secs && weeks)) idle += sprintf("%ds", secs);

            idle += NOR + "\n";
        }
        else idle = "";

        if ((stringp(finger_data[4]) && finger_data[4] != "")
         && (stringp(finger_data[6]) && finger_data[6] != ""))
            ret += sprintf("On since %s from %s\n%s", finger_data[4], finger_data[6], idle);
        else if (stringp(finger_data[4]) && finger_data[4] != "")
            ret += sprintf("On since %s\n%s", finger_data[4], idle);
        else if (stringp(finger_data[6]) && finger_data[6] != "")
            ret += sprintf("On from %s\n%s", finger_data[6], idle);
    }

    if (finger_data[8] != "") ret += "\n" + finger_data[8];

    return ret;
}

string get_rank(object user)
{
    object rankd = find_object("/adm/daemons/rank_d");

    if (objectp(rankd))
    {
        string rank, rank_colour;

        rank = rankd->queryQualativeRank(user);
        rank_colour  = rankd->queryRankProperty(rank, "colour");
        rank_colour += capitalize(rankd->queryRankProperty(rank, "long"));
        rank_colour += NOR;

        return rank_colour;
    }

    if (adminp(user)) return HIR + "Admin" + NOR;
    else if (devp(user)) return YEL + "Developer" + NOR;
    else return GRN + "User" + NOR;
}

mixed array get_finger_data(string username)
{
    mixed user, body;
    string visname, real_name;
    string title, e_mail, loginout_time, ip_name, level, extra;
    string idleMsg, away_msg, plan;
    int idleTime;

    away_msg = 0;

    if (find_player(username))
    {
        body = find_player(username);
        user = get_user(username);

        if (!interactive(body))
        {
            idleTime = -1;
            idleMsg = "(" + HIK + "Link-Dead" + NOR + ")";
        }
        else if (stringp(body->query_env("away")))
        {
            idleMsg = "(" + YEL + "afk" + NOR + ")";

            if (body->query_env("away") != "")
                away_msg = "AFK:\n" + body->query_env("away");
        }
        else if (in_edit(body)) idleMsg = "(" + CYN + "editing" + NOR + ")";
        else if (in_input(body)) idleMsg = "(" + CYN + "input" + NOR + ")";
        else
        {
            idleTime = query_idle(body);

            if(idleTime > 60) idleMsg = "(" + YEL + "Idle" + NOR + ")";
            else idleMsg = "(" + HIG + "Online" + NOR + ")";
        }

        visname = body->query_cap_name();
        title = body->query_short() + " " + idleMsg;
        real_name = "";
        e_mail = user->query("email");
        loginout_time = body->query("last_login") ? ctime(body->query("last_login")) : 0;
        ip_name = query_ip_name(body);
        level = get_rank(user);

        plan = read_file("/home/" + user->query_name()[0..0] + "/" + user->query_name() + "/.plan");

        if(!plan) plan = " This user has no plan.\n";

        plan = "Plan:\n" + plan;

        extra = plan + (away_msg != 0 ? "\n" + away_msg : "");

        if(objectp(user)) destruct(user);
    }
    else if (find_living(username) && find_living(username)->is_bot())
    {
        body = find_living(username);

        idleTime = body->query_idle_time();
        idleMsg = "(" + HIG + "Online" + NOR + ")";

        visname = capitalize(body->query_bot_name(username));
        title = body->query_short() + " " + idleMsg;

        if (visname != capitalize(body->query_bot_name()))
            real_name = capitalize(body->query_bot_name());
        else real_name = "";

        e_mail = "";
        loginout_time = ctime(stat(explode(file_name(body), "#")[0] + ".c")[2]);
        ip_name = "localhost";
        level = CYN + "Bot" + NOR;
        extra = body->query_long();
    }
    else
    {
        if (!objectp(user = FINGER_D->get_user(username))
          || !objectp(body = FINGER_D->get_body(user)))
        {
            if (user == -2) return 0;
            else return ({ });
        }

        idleTime = -1;
        idleMsg = "(" + RED + "Offline" + NOR + ")";

        visname = body->query_cap_name();
        title = body->query_short() + " " + idleMsg;
        real_name = "";
        e_mail = user->query("email");
        loginout_time = body->query("last_login") ? ctime(body->query("last_login")) : 0;
        ip_name = query_ip_name(body);
        level = get_rank(user);

        plan = read_file("/home/" + user->query_name()[0..0] + "/" + user->query_name() + "/.plan");

        if(!plan) plan = " This user has no plan.\n";

        plan = "Plan:\n" + plan;

        extra = plan + (away_msg != 0 ? "\n" + away_msg : "");

        if (objectp(user)) destruct(user);
        if (!interactive(body)) destruct(body);
    }

    return ({
      visname,
      title,
      real_name,
      e_mail,
      loginout_time,
      idleTime,
      ip_name,
      level,
      extra,
    });
}

mixed get_user(string username)
{
    object user;
    string error;

    if (origin() != "local" && !adminp(query_privs(previous_object())))
        return 0;

    error = catch(user = new(USER_OB));

    if (error) return -1;
    if (!file_exists(user_data_file(username) + ".o"))
    {
        destruct(user);
        return -2;
    }

    set_privs(user, username);
    user->set_name(username);
    user->restore_user();

    return user;
}

mixed get_body(object user)
{
    object body;
    string error;

    if (origin() != "local" && !adminp(query_privs(previous_object())))
        return 0;

    error = catch(body = new(user->query_bodyPath()));

    if (error) return -1;
    if (!file_exists(user_mob_data(query_privs(user)) + ".o"))
    {
        destruct(body);
        return -2;
    }

    set_privs(body, query_privs(user));
    body->set_name(user->query_name());
    body->restore_user();

    return body;
}

