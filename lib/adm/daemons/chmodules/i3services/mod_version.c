/* mod_version.c

   Tricky@Rock the Halo
   13-JAN-2007
   Intermud3 version service

   Last edited on June 14th, 2007 by Tricky@Rock the Halo

*/

#include <intermud3.h>

privatev object i3;

void create()
{
    i3 = find_object(I3_CHDMOD);
    i3->add_service_name("version", 1);
    i3->add_service("version-req", "req_handler");
    i3->add_service("version-reply", "reply_handler");
}

void remove()
{
    i3 = find_object(I3_CHDMOD);
    i3->remove_service_name("version");
    i3->remove_service("version-req");
    i3->remove_service("version-reply");

    destruct();
}

void req_handler(mixed *packet)
{
    mixed version_packet;

    version_packet = ({
      LIB_VERSION,
      BASELIB_VERSION,
      INTERMUD_VER + "." + INTERMUD_REV + " " + INTERMUD_PATCH,
    });

    find_object(I3_CHDMOD)->sendToUser("version-reply", 0, packet[I3_O_MUD], packet[I3_O_USER], version_packet);
}

void reply_handler(mixed *packet)
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

    tell_object(o_user, "%^GREEN%^     Lib version for %^BOLD%^" + packet[I3_O_MUD] + "%^NORMAL%^ is %^BOLD%^" + packet[6] + "%^RESET%^\n");
    tell_object(o_user, "%^GREEN%^ Baselib version for %^BOLD%^" + packet[I3_O_MUD] + "%^NORMAL%^ is %^BOLD%^" + packet[7] + "%^RESET%^\n");
    tell_object(o_user, "%^GREEN%^Intermud version for %^BOLD%^" + packet[I3_O_MUD] + "%^NORMAL%^ is %^BOLD%^" + packet[8] + "%^RESET%^\n");
}
