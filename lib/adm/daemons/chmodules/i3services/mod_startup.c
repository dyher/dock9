/* mod_startup.c

   Tricky@Rock the Halo
   7-FEB-2007
   Intermud3 startup service

   Last edited on June 14th, 2007 by Tricky@Rock the Halo

*/

#include <intermud3.h>

privatev object i3;

void create()
{
    i3 = find_object(I3_CHDMOD);
    i3->add_service("startup-reply", "reply_handler");
}

void remove()
{
    i3 = find_object(I3_CHDMOD);
    i3->remove_service("startup-reply");

    destruct();
}

void reply_handler(mixed *packet)
{
    mixed router_list;

    i3 = find_object(I3_CHDMOD);

    router_list = i3->getData("router_list");

    if(packet[I3_O_MUD] != router_list[0][0])
    {
        I3LOG("Warning", "I3_STARTUP", "startup-reply", "Illegal access. Not from the router.");
        I3LOG("Warning", "I3_STARTUP", "startup-reply", sprintf("%O", packet));

        return;
    }

    I3LOG("Notice", "I3_STARTUP", "startup-reply", "Initialising connection to the I3 router.");

    if(sizeof(packet) != 8)
    {
        I3LOG("Error", "I3_STARTUP", "startup-reply", "We don't like startup-reply packet size. Should be 8 but is " + sizeof(packet));

        return;
    }

    if(!sizeof(packet[6]))
    {
        I3LOG("Error", "I3_STARTUP", "startup-reply", "We don't like the absence of packet element 6.");

        return;
    }

    if(packet[6][0][0] == router_list[0][0] && packet[6][0][1] == router_list[0][1])
    {
        i3->setData("router_list", packet[6]);
        i3->setData("router_password", packet[7]);
    }
    else
    {
        i3->setData("router_list", packet[6]);
        I3LOG("Notice", "I3_STARTUP", "startup-reply", "Changing router details to " + packet[6][0][0] + ", " + packet[6][0][1]);

        /* Connect to the new server */
        i3->shutDown(0);
        i3->reconnect();

        return;
    }

    I3LOG("Success", "I3_STARTUP", "startup-reply", "Connection established to I3 router.");
}
