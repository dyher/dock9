/*
 * mod_ping.c
 *
 * Tricky@Rock the Halo
 * 31-JAN-2007
 * Intermud3 ping service
 *
 * Last edited on May 3rd, 2008 by Tricky@Rock the Halo
 *
 */

#include <intermud3.h>

privatev object I3;
privatev int    PingReplied;

void test_connection();

void create()
{
    I3 = find_object(I3_CHDMOD);
    I3->add_service_name("ping", 1);
    I3->add_service("ping-req", "req_handler");
    I3->add_service("ping-reply", "reply_handler");

    PingReplied = random(5) + 5;
    set_heart_beat(30);
}

void remove()
{
    I3 = find_object(I3_CHDMOD);
    I3->remove_service_name("ping");
    I3->remove_service("ping-req");
    I3->remove_service("ping-reply");

    destruct();
}

void heart_beat()
{
    if (time() - I3->getIdleTimeout() >= PingReplied * 60)
    {
        if (PingReplied != 0)
        {
            PingReplied = 0;
            I3->setData("routerConnected", 0);
            I3->setData("reconnect_wait", MIN_RETRY_TIME);
            test_connection();
        }
        else
        {
            I3LOG("Warning", "I3_PING", "heart_beat", "Not connected to the router. Re-connecting.");
            I3->reconnect();
        }
    }
}

void test_connection()
{
    int hash = random(470831) + ((time() & 0x00ffffff) ^ 0x00a5a5a5);

    I3->sendToMud("ping-req", 0, I3_MUD_NAME, hash);
}

void req_handler(mixed *packet)
{
    mixed extra = ({ });

    if (sizeof(packet) >= 7) extra = packet[6..<1];

    PingReplied = random(5) + 5;
    I3->setData("routerConnected", 1);
    I3->sendToMud(
      "ping-reply",
      (string)0,
      packet[I3_O_MUD],
      extra
    );
}

void reply_handler(mixed *packet)
{
    PingReplied = random(5) + 5;
    I3->setData("routerConnected", 1);

    /* Nothing more for now. */
}
