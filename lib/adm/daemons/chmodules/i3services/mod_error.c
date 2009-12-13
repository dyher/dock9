/* mod_error.c

   Tricky@Rock the Halo
   18-AUG-2006
   Intermud3 error service

   Last edited on June 14th, 2007 by Tricky@Rock the Halo

*/

#include <intermud3.h>

privatev object i3;

void create()
{
    i3 = find_object(I3_CHDMOD);
    i3->add_service("error", "req_handler");
}

void remove()
{
    i3 = find_object(I3_CHDMOD);
    i3->remove_service("error");

    destruct();
}

varargs void send_error(mixed o_user, string t_mud, mixed t_user, string err_code, string err_msg, mixed *packet)
{

    if(intp(o_user)) o_user = 0;
    else
    if(objectp(o_user)) o_user = o_user->query_name();
    else
    if(stringp(o_user)) o_user = lower_case(o_user);
    else o_user = 0;

    if(intp(t_user)) t_user = 0;
    else
    if(objectp(t_user)) t_user = t_user->query_name();
    else
    if(stringp(t_user)) t_user = lower_case(t_user);
    else t_user = 0;

    find_object(I3_CHDMOD)->sendToUser("error", o_user, t_mud, t_user, ({
      err_code,
      err_msg,
      packet ? packet : 0
    }) );
}

void req_handler(mixed *packet)
{
    object o_user;
    string error_code = packet[6];
    string error_message = packet[7];

    i3 = find_object(I3_CHDMOD);

    if(stringp(packet[I3_O_MUD])) error_message += sprintf(" -- %s", packet[I3_O_MUD]);

    if(error_code == "bad-proto" && error_message == "MUD already connected")
    {
        ANNOUNCE_CHDMOD->rec_m_msg(
          "error",
          "I3_ERROR",
          HIR + error_code + "/" + error_message + NOR
        );
        ANNOUNCE_CHDMOD->rec_m_msg(
          "error",
          "I3_ERROR",
          YEL + "Try reconnecting in a few minutes." + NOR
        );

        i3->setData("reconnect_wait", i3->getData("reconnect_wait") - 10);
        i3->reconnect(305);
        i3->shutDown(300);

        return;
    }
    else
    if(error_code == "bad-mojo")
    {
        ANNOUNCE_CHDMOD->rec_m_msg(
          "error",
          "I3_ERROR",
          HIR + error_code + "/" + error_message + NOR
        );
        ANNOUNCE_CHDMOD->rec_m_msg(
          "error",
          "I3_ERROR",
          YEL + "Try reconnecting in a few minutes." + NOR
        );

        i3->setData("reconnect_wait", i3->getData("reconnect_wait") - 10);
        i3->reconnect(125);
        i3->shutDown(120);

        return;
    }
    else
    if(error_code == "not-allowed")
    {

        if(arrayp(packet[8]) && packet[8][I3_TYPE] == "channel-listen")
        {
            string channel = packet[8][6];

            ANNOUNCE_CHDMOD->rec_m_msg(
              "intermud3",
              "I3_ERROR",
              ORA + error_message + NOR
            );
            I3LOG("Warning", "I3_ERROR", "req_handler", error_message);
            find_object(I3_CHANNEL)->unregister_channel(channel, 1);

            return;
        }

    }
    else
    if(error_code == "unk-channel")
    {

        if(arrayp(packet[8]) && packet[8][I3_TYPE] == "channel-listen")
        {
            string channel = packet[8][6];

            ANNOUNCE_CHDMOD->rec_m_msg(
              "intermud3",
              "I3_ERROR",
              ORA + error_message + NOR
            );
            I3LOG("Warning", "I3_ERROR", "error", error_message);
            find_object(I3_CHANNEL)->unregister_channel(channel);

            return;
        }

    }

    if (stringp(packet[I3_T_USER]) && objectp(o_user = find_player(packet[I3_T_USER])))
        message("all", ORA + "I3_ERROR: " + error_message + NOR + "\n", ({ o_user }) );
    else
        ANNOUNCE_CHDMOD->rec_m_msg(
          "intermud3",
          "I3_ERROR",
          ORA + error_message + NOR
        );
}
