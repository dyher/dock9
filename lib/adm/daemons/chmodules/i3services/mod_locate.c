/* mod_locate.c
 *
 *   Tricky@Rock the Halo
 *  18-AUG-2006
 *  Intermud3 locate service
 *
 */

#include <intermud3.h>

privatev object i3;

void create()
{
    i3 = find_object(I3_CHDMOD);
    i3->add_service_name("locate", 1);
    i3->add_service("locate-req", "req_handler");
    i3->add_service("locate-reply", "reply_handler");
}

void remove()
{
    i3->remove_service_name("locate");
    i3->remove_service("locate-req");
    i3->remove_service("locate-reply");

    destruct();
}

void req_handler(mixed *packet)
{
    object i3_ucache = find_object(I3_UCACHE);
    mixed user;
    string visname, username;

    username = packet[6];

    visname = i3_ucache->getVisname(packet[I3_O_MUD], packet[I3_O_USER]);
    i3_ucache->checkUser(packet[I3_O_MUD], packet[I3_O_USER], visname, 0);
    visname = i3_ucache->getVisname(packet[I3_O_MUD], packet[I3_O_USER]);

    ANNOUNCE_CHDMOD->rec_m_msg(
      "intermud3",
      "I3_LOCATE",
      "Locate " + HIG + username + NOR + " by " + HIG + visname + "@" + packet[I3_O_MUD] + NOR
    );

    I3LOG(
      "Notice",
      "I3_LOCATE",
      "locate-req",
      "Locate '" + username + "' by '" + visname + "@" + packet[I3_O_MUD] + "'"
    );

    if (objectp(user = find_player(username)))
    {
        string status;

        if (!interactive(user)) status = "link-dead";
        else
        if (user->query_env("away"))
        {
            status = "afk";

            if (devp(lower_case(username)))
                tell_object(
                  user,
                  GRN + "Notice [I3_LOCATE/locate-req]: You were located by " +
                    HIG + visname + "@" + packet[I3_O_MUD] + NOR + "\n"
                );

        }
        else
        if (in_edit(user)) status = "editing";
        else
        if (in_input(user)) status = "input";
        else
        if (query_idle(user) >= 60) status = "idle";
        else
        {
            status = "online";

            if (devp(lower_case(username)))
                tell_object(
                  user,
                  GRN + "Notice [I3_LOCATE/locate-req]: You were located by " +
                    HIG + visname + "@" + packet[I3_O_MUD] + NOR + "\n"
                );

        }

        i3->sendToUser(
          "locate-reply",
          0,
          packet[I3_O_MUD],
          packet[I3_O_USER],
          ({
            I3_MUD_NAME,
            user->query_cap_name(),
            (interactive(user) ? query_idle(user) : -1),
            (status ? status : 0)
          })
        );

        return;
    }

}

void reply_handler(mixed *packet)
{
    object o_user;
    string m;
    int idl, weeks, days, hours, mins, secs;

    if (!packet[I3_T_USER] || !objectp(o_user = find_player(packet[I3_T_USER])))
        return;

    m = HIW + packet[7] + NOR + " was located on " + HIW + packet[6] + NOR;

    idl = to_int(packet[8]);
    weeks = idl / (86400 * 7);
    days = (idl / 86400) % 7;
    hours = (idl / 3600) % 24;
    mins = (idl / 60) % 60;
    secs = idl % 60;

    m += " (";

    if (packet[8] < 0)
        m += packet[9] + ")";
    else
    {

        if (!stringp(packet[9]))
            m += "none";
        else
            m += packet[9];

        m += ": ";

        if (idl < 60)
            m += GRN + "active" + NOR + ")";
        else
        {
            m += YEL;

            if (weeks) m += sprintf("%dw ", weeks);
            if (days || (!days && weeks)) m += sprintf("%dd ", days);
            if (hours || (!hours && days) || (!hours && weeks)) m += sprintf("%dh ", hours);
            if (mins || (!mins && hours) || (!mins && days) || (!mins && weeks)) m += sprintf("%dm ", mins);
            if (secs || (!secs && mins) || (!secs && hours) || (!secs && days) || (!secs && weeks)) m += sprintf("%ds", secs);

            m += NOR + ")";
        }

    }

    tell_object(o_user, m + "\n");
}
