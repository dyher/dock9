/* mod_emoteto.c

   Tricky@Rock the Halo
   18-AUG-2006
   Intermud3 emoteto service

   Last edited on October 6th, 2007 by Tricky@Rock the Halo

*/

#include <intermud3.h>

privatev object i3;

void create()
{
    i3 = find_object(I3_CHDMOD);
    i3->add_service_name("emoteto", 1);
    i3->add_service("emoteto", "req_handler");
}

void remove()
{
    i3 = find_object(I3_CHDMOD);
    i3->remove_service_name("emoteto");
    i3->remove_service("emoteto");

    destruct();
}

void send_emoteto(object o_user, mixed t_user, string t_mud, string msg)
{

    if(objectp(t_user)) t_user = lower_case(t_user->query_name());
    else
    if(stringp(t_user)) t_user = lower_case(t_user);
    else return;

    find_object(I3_CHDMOD)->sendToUser("emoteto", o_user->query_name(), t_mud, t_user, ({
      capitalize(o_user->query_name()),
      "$N " + msg
    }) );
}

void req_handler(mixed *packet)
{
    object o_user;
    string message;

    if(!packet[I3_T_USER] || !(o_user = find_player(packet[I3_T_USER])))
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

    message = packet[7];
    message = replace_string(message, "$N", sprintf("%s@%s", "%^CYAN%^" + packet[6], packet[I3_O_MUD] + "%^RESET%^"));
    tell_object(o_user, message + "\n");
    o_user->set("reply", packet[6] + "@" + packet[I3_O_MUD]);

    if(o_user->query_env("away") != 0)
    {
        send_emoteto(o_user, packet[I3_O_USER], packet[I3_O_MUD], "is away from the keyboard!");
        find_object(I3_TELL)->send_tell(o_user, packet[I3_O_USER], packet[I3_O_MUD], o_user->query_env("away"));
    }

}
