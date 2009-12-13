/* mod_finger.c

   Tricky@Rock the Halo
   18-AUG-2006
   Intermud3 finger service

   Last edited on June 15th, 2007 by Tricky@Rock the Halo

*/

#include <intermud3.h>

privatev object i3;

void create()
{
    i3 = find_object(I3_CHDMOD);
    i3->add_service_name("finger", 1);
    i3->add_service("finger-req", "req_handler");
    i3->add_service("finger-reply", "reply_handler");
}

void remove()
{
    i3 = find_object(I3_CHDMOD);
    i3->remove_service_name("finger");
    i3->remove_service("finger-req");
    i3->remove_service("finger-reply");

    destruct();
}

string get_rank(object user)
{
    object rankd = find_object("/adm/daemons/rank_d");

    if(objectp(rankd))
    {
        string rank, rank_colour;

        rank = rankd->queryQualativeRank(user);
        rank_colour  = rankd->queryRankProperty(rank, "colour");
        rank_colour += capitalize(rankd->queryRankProperty(rank, "long"));
        rank_colour += NOR;

        return rank_colour;
    }

    if(adminp(user)) return HIR + "Admin" + NOR;
    else
    if(devp(user)) return YEL + "Developer" + NOR;
    else
    return GRN + "User" + NOR;

}

void req_handler(mixed *packet)
{
    object i3_ucache = find_object(I3_UCACHE);
    object i3error = find_object(I3_ERROR);
    mixed array finger_data;
    string username, visname;

    username = packet[6];

    visname = i3_ucache->getVisname(packet[I3_O_MUD], packet[I3_O_USER]);
    i3_ucache->checkUser(packet[I3_O_MUD], packet[I3_O_USER], visname, 0);
    visname = i3_ucache->getVisname(packet[I3_O_MUD], packet[I3_O_USER]);

    ANNOUNCE_CHDMOD->rec_m_msg(
      "intermud3",
      "I3_FINGER",
      "Finger for " + HIG + username + NOR + " by " + HIG + visname + "@" + packet[I3_O_MUD] + NOR
    );

    I3LOG("Notice", "I3_FINGER", "finger-req", "Finger for '" + username + "' by '" + visname + "@" + packet[I3_O_MUD] + "'");

    finger_data = FINGER_D->get_finger_data(username);

    if (!finger_data)
        i3error->send_error(
          0,
          packet[I3_O_MUD],
          packet[I3_O_USER],
          "unk-user",
          "The user " + username + " has not registered here.",
          packet
        );
    else if (!sizeof(finger_data))
        i3error->send_error(
          0,
          packet[I3_O_MUD],
          packet[I3_O_USER],
          "unk-user",
          "Data for " + username + "@" + packet[I3_T_MUD] + " is unavailable.",
          packet
        );
    else
        find_object(I3_CHDMOD)->sendToUser("finger-reply", 0, packet[I3_O_MUD], packet[I3_O_USER], ({
          finger_data[0],
          finger_data[1],
          finger_data[2],
          finger_data[3],
          finger_data[4],
          finger_data[5],
          finger_data[6],
          finger_data[7],
          finger_data[8],
        }) );
}

void reply_handler(mixed *packet)
{
    object o_user;
    mixed finger_data;
    string mesg;

    if (!packet[I3_T_USER] || !(o_user = find_player(packet[I3_T_USER])))
    {
        find_object(I3_ERROR)->send_error(
          0,
          packet[I3_O_MUD],
          packet[I3_O_USER],
          "unk-user",
          "finger-reply can not be returned to user " + packet[I3_T_USER] + "@" + packet[I3_T_MUD],
          packet
        );

        return;
    }

    /* Get the rest of the finger data */
    finger_data = packet[I3_T_USER + 1..<1];

    /* Make sure it is the correct size by padding */
    if (sizeof(finger_data) < 9) finger_data += allocate(9 - sizeof(finger_data));

    if (!stringp(finger_data[0])) finger_data[0] = "";
    if (!stringp(finger_data[1])) finger_data[1] = "";
    if (!stringp(finger_data[2])) finger_data[2] = "";
    if (!stringp(finger_data[3])) finger_data[3] = "";
    if (!stringp(finger_data[4])) finger_data[4] = "";
    if (!intp(finger_data[5]))    finger_data[5] = 0;
    if (!stringp(finger_data[6])) finger_data[6] = "";
    if (!stringp(finger_data[7])) finger_data[7] = "";
    if (!stringp(finger_data[8])) finger_data[8] = "";

    mesg  = HIM + packet[I3_O_MUD] + NOR;
    mesg += FINGER_D->build_finger_info(finger_data);

    if (mesg[<1..<1] != "\n") mesg += "\n";

    tell_object(o_user, mesg);
}
