/* mod_beep.c

   Tricky@Rock the Halo
   18-AUG-2006
   Intermud3 beep service

   Last edited on march 12th, 2007 by Tricky@Rock the Halo

   http://www.darkwoodinc.com/~tim/i3todo.html

*/

#include <intermud3.h>

privatev object i3;

void create()
{
    i3 = find_object(I3_CHDMOD);
    i3->add_service_name("beep", 1);
    i3->add_service("beep", "req_handler");
}

void remove()
{
    i3 = find_object(I3_CHDMOD);
    i3->remove_service_name("beep");
    i3->remove_service("beep");

    destruct();
}

void send_beep(object o_user, mixed t_user, string t_mud)
{

    if(objectp(t_user)) t_user = lower_case(t_user->query_name());
    else
    if(stringp(t_user)) t_user = lower_case(t_user);
    else return;

    find_object(I3_CHDMOD)->sendToUser("beep", o_user->query_name(), t_mud, t_user, 0);
}

void req_handler(mixed *packet)
{
    object o_user;

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

    tell_object(o_user, "%^BOLD%^BEEP!%^RESET%^%^BEEP%^\n");
    tell_object(o_user, capitalize(packet[6]) + "@" + packet[I3_O_MUD] + " just beeped you!\n");
}
