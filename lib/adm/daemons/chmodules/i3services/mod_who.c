/* mod_who.c

   Tricky@Rock the Halo
   18-AUG-2006
   Intermud3 who service

   Last edited on June 14th, 2007 by Tricky@Rock the Halo

*/

#include <intermud3.h>

privatev object I3;

void create()
{
    I3 = find_object(I3_CHDMOD);
    I3->add_service_name("who", 1);
    I3->add_service("who-req", "req_handler");
    I3->add_service("who-reply", "reply_handler");
}

void remove()
{
    I3->remove_service_name("who");
    I3->remove_service("who-req");
    I3->remove_service("who-reply");

    destruct();
}

void remote_who(mixed user, string mudname)
{
    mapping mudlist = find_object(I3_MUDLIST)->get_mud_list();
    string array muds = ({ });

    if (objectp(user)) user = user->query_name();
    else if (stringp(user)) user = lower_case(user);
    else return;

    if (stringp(mudname) || mudname != 0)
    {
        if (mudlist[mudname][0] == -1)
            muds = ({ mudname });
    }
    else
    {
        foreach (string mud in keys(mudlist))
            if (mudlist[mud][0] == -1)
                muds += ({ mud });
    }

    if (sizeof(muds) == 0) return;

    foreach (string mud in muds)
        I3->sendToMud("who-req", user, mud, 0);
}

string get_rank(object user)
{
    object rankd = find_object("/adm/daemons/rank_d");

    if (user->is_bot()) return CYN + "Bot" + NOR;
    if (objectp(rankd))
    {
        string rank, rank_colour;

        rank = rankd->queryQualativeRank(user);
        rank_colour  = rankd->queryRankProperty(rank, "colour");
        rank_colour += capitalize(rankd->queryRankProperty(rank, "short"));
        rank_colour += NOR;

        return rank_colour;
    }

    if (adminp(user)) return HIR + "Admin" + NOR;
    else if (devp(user)) return YEL + "Developer" + NOR;
    else return GRN + "User" + NOR;

}

mixed array createWhoData(object array objs)
{
    mixed array tmp = ({ });

    tmp += map(objs, (:
      ({
        (string)capitalize($1->query_name()),
        (int)query_idle($1),
        (string)($1->query_short() + " [" + get_rank($1) + "]")
      })
    :));

    return tmp;
}

mixed array createWhoBotData(object array objs)
{
    mixed array tmp = ({ });

    tmp += map(objs, (:
      ({
        (string)capitalize($1->query_bot_name()),
        (int)$1->query_idle_time(),
        (string)($1->query_short() + " [" + get_rank($1) + "]")
      })
    :));

    return tmp;
}

int sort_by_rank(object user1, object user2)
{
    object rankd = find_object("/adm/daemons/rank_d");

    if (rankd->queryQuantativeRank(user1) > rankd->queryQuantativeRank(user2))
        return -1;
    else if (rankd->queryQuantativeRank(user1) < rankd->queryQuantativeRank(user2))
        return 1;
    else return strcmp(user1->query_name(), user2->query_name());
}

void req_handler(mixed *packet)
{
    mixed array msg = ({ });
    object array bots;
    object i3_ucache = find_object(I3_UCACHE);
    object rankd = find_object("/adm/daemons/rank_d");
    string visname;

    visname = i3_ucache->getVisname(packet[I3_O_MUD], packet[I3_O_USER]);
    i3_ucache->checkUser(packet[I3_O_MUD], packet[I3_O_USER], visname, 0);
    visname = i3_ucache->getVisname(packet[I3_O_MUD], packet[I3_O_USER]);

    if (stringp(packet[I3_T_MUD]))
    {
        if (packet[I3_O_MUD] != mud_name())
        {
            ANNOUNCE_CHDMOD->rec_m_msg(
              "intermud3",
              "I3_WHO",
              "Direct RWHO by " + HIG + visname + "@" + packet[I3_O_MUD] + NOR
            );
        }

        I3LOG(
          "Notice",
          "I3_WHO",
          "who-req",
          "Direct RWHO by '" + visname + "@" + packet[I3_O_MUD] + "'"
        );
    }
    else
    {
        if (packet[I3_O_MUD] != mud_name())
        {
            ANNOUNCE_CHDMOD->rec_m_msg(
              "intermud3",
              "I3_WHO",
              "Broadcast RWHO by " + HIG + visname + "@" + packet[I3_O_MUD] + NOR
            );
        }

        I3LOG(
          "Notice",
          "I3_WHO",
          "who-req",
          "Broadcast RWHO by '" + visname + "@" + packet[I3_O_MUD] + "'"
        );
    }

    if (objectp(rankd))
    {
        object array usrs;

        usrs = sort_array(users(), "sort_by_rank");
        msg += createWhoData(usrs);
    }
    else
    {
        object array adms, devs, plrs;

        adms = sort_array(
          filter(users(), (: adminp($1) :) ),
          (: strcmp($1->query_name(), $2->query_name()) :)
        );
        devs = sort_array(
          filter(users(), (: devp($1) && !adminp($1) :) ),
          (: strcmp($1->query_name(), $2->query_name()) :)
        );
        plrs = sort_array(
          filter(users(), (: !(devp($1) || adminp($1)) :) ),
          (: strcmp($1->query_name(), $2->query_name()) :)
        );

        msg += createWhoData(adms);
        msg += createWhoData(devs);
        msg += createWhoData(plrs);
    }

    bots = sort_array(
      filter(livings(), (: environment($1) && $1->is_bot() :) ),
      (: strcmp($1->query_bot_name(), $2->query_bot_name()) :)
    );
    msg += createWhoBotData(bots);

    I3->sendToUser("who-reply", 0, packet[I3_O_MUD], packet[I3_O_USER], ({ msg }) );
}

void reply_handler(mixed *packet)
{
    mapping mudlist = find_object(I3_MUDLIST)->get_mud_list();
    object o_user;
    string list = HIM + packet[I3_O_MUD] + NOR + "\n";

    if (!packet[I3_T_USER] || !(o_user = find_player(packet[I3_T_USER])))
        return;
    if (sizeof(packet[6]))
    {
        foreach(mixed *who in packet[6])
        {
            string idleMsg;
            int idleTime = intp(who[1]) ? who[1] : to_int(who[1]);
            int weeks = idleTime / (86400 * 7);
            int days = (idleTime / 86400) % 7;
            int hours = (idleTime / 3600) % 24;
            int mins = (idleTime / 60) % 60;
            int secs = idleTime % 60;

            /* Emerald MUD */
            if (who[1] == "never")
                list += implode(explode(who[2], "\n")[1..<1], "\n") + NOR + "\n";
            /* IMC2 client who-reply */
            else if (idleTime == 0 && who[2] == "" &&
                     strsrch(lower_case(mudlist[packet[I3_O_MUD]][7]), "imc2 ") != -1)
                list += TERMINAL_D->imc2_to_pinkfish(who[0]) + NOR + "\n";
            /* AFK I3 Driver who-reply */
            else if (idleTime < 0)
                list += who[0] + " " + who[2] + NOR + "\n";
            else if (idleTime == 9999)
                list += who[0] + " " + who[2] + NOR + "\n\n";
            else if (idleTime == 2 * 9999)
                list += "\n" + who[0] + " " + who[2] + NOR + "\n";
            else if (idleTime == 3 * 9999)
                list += "\n" + who[0] + " " + who[2] + NOR + "\n\n";
            /* Check for names with a <SPC> in them. Generally means it is a header in who[2]. */
            else if (strsrch(who[0], " ") != -1
              && (strsrch(lower_case(mudlist[packet[I3_O_MUD]][5]), "circle") != -1
               || strsrch(lower_case(mudlist[packet[I3_O_MUD]][7]), "afkmud") != -1))
                list += "* " + HIW + who[2] + NOR + "\n";
            /* Regular I3 who-reply */
            else
            {
                if (idleTime < 60)
                    idleMsg = GRN + "active" + NOR;
                else
                {
                    idleMsg  = HIK + "idle ";

                    if (weeks) idleMsg += sprintf("%dw ", weeks);
                    if (days || (!days && weeks)) idleMsg += sprintf("%dd ", days);
                    if (hours || (!hours && days) || (!hours && weeks)) idleMsg += sprintf("%dh ", hours);
                    if (mins || (!mins && hours) || (!mins && days) || (!mins && weeks)) idleMsg += sprintf("%dm ", mins);
                    if (secs || (!secs && mins) || (!secs && hours) || (!secs && days) || (!secs && weeks)) idleMsg += sprintf("%ds", secs);

                    idleMsg += NOR;
                }

                list += who[0] + " (" + idleMsg + "): " + who[2] + NOR + "\n";
            }

        }

    }
    else list += "Nobody logged on.\n";

    foreach (string line in explode(list, "\n"))
        tell_object(o_user, line + "\n");
}
