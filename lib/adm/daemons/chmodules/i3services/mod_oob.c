/* mod_oob.c

   Tricky@Rock the Halo
   4-JUL-2007
   Intermud3 OOB service

   Last edited on June 15th, 2007 by Tricky@Rock the Halo

*/

#include <intermud3.h>

/* Socket stuff */
#include <socket.h>

#ifndef SOCKET_D
#define SOCKET_D "/adm/daemons/socket_d"
#endif

#define OOB_INCOMING -1
#define OOB_OUTGOING 1

privatev object       i3;
privatev mapping      oob_services;
privatev mapping      oob_queue, oob_unanswered;
privatev mapping      o_muds, t_muds, direction;
privatev int          pid;

void installOOBService(string service);
void installOOBServices();
void uninstallOOBService(string service);
void uninstallOOBServices();
void add_oob_service(string type, string callback);
void remove_oob_service(string type);
mapping query_oob_services();

int setup_oob_socket();
void oob_connect(string t_mudname);
void oob_close_callback(int fd);
void handle_oob_packet(int fd, string mudname, string queued_packet, mixed packet);

void create()
{
    int fd;

    if((fd = setup_oob_socket()) < 0)
    {

        if(fd == EEISCONN)
        {
            OOBLOG("Warning [OOB/create]: Already connected, maybe still closing. Will try again.\n");
            call_out("create", 2);

            return;
        }

        OOBLOG("Error [OOB/create]: Unable to setup socket - " + socket_error(fd) + "\n");
        /* Don't need to call remove() so we call destruct() this way. */
        destruct();

        return;
    }

    i3 = find_object(I3_CHDMOD);
    i3->add_service("oob-req", "oob_req_handler");

    pid = 0;
    oob_queue = ([ ]);
    oob_unanswered = ([ ]);
    o_muds = ([ ]);
    t_muds = ([ ]);
    direction = ([ ]);
    oob_services = ([ ]);

    installOOBServices();
    set_heart_beat(30);
}

void remove()
{
    object sock = find_object(SOCKET_D);

    uninstallOOBServices();

    i3 = find_object(I3_CHDMOD);
    i3->remove_service("oob-req");

    foreach(int fd, string mudname in o_muds)
        if(stringp(mudname))
            sock->close(fd);

    foreach(int fd, string mudname in t_muds)
        if(stringp(mudname))
            sock->close(fd);

    sock->close("OOB");
    destruct();
}

void heart_beat()
{
    object sock = find_object(SOCKET_D);
    mapping session_keys = find_object(I3_AUTH)->query_session_keys();

    foreach(int fd, string mudname in o_muds)
        if(stringp(mudname))
            if(session_keys[mudname])
                if(time() - session_keys[mudname]["registered"] > 10 * 60)
                    sock->close(fd);

    foreach(int fd, string mudname in t_muds)
        if(stringp(mudname))
            if(session_keys[mudname])
                if(time() - session_keys[mudname]["registered"] > 10 * 60)
                    sock->close(fd);

}

void installOOBService(string service_fname)
{
    mixed file_stat;
    string fname = __DIR__ + service_fname;

    file_stat = stat(fname + ".c");

    if(sizeof(file_stat))
    {

        if(file_stat[2] != 0)
        {
            OOBLOG("Notice [OOB/installService]: Unloading service '" + service_fname + "'\n");
            destruct(find_object(fname));
        }

        OOBLOG("Notice [OOB/installService]: Loading service '" + service_fname + "'\n");
        catch(load_object(fname));
    }

}

void installOOBServices()
{

    foreach(string fname in get_dir(__DIR__ + "oob_mod_*.c"))
        installOOBService(fname[0..<3]);

}

void uninstallOOBService(string service_fname)
{
    mixed file_stat;
    string fname = __DIR__ + service_fname;

    file_stat = stat(fname + ".c");

    if(!objectp(find_object(fname))) return;

    if(arrayp(file_stat) && sizeof(file_stat) && file_stat[2] != 0)
    {
        OOBLOG("Notice [OOB/uninstallService]: Unloading service '" + service_fname + "'\n");
        destruct(find_object(fname));
    }

}

void uninstallOOBServices()
{
    foreach(string fname in get_dir(__DIR__ + "oob_mod_*.c"))
        uninstallOOBService(fname[0..<3]);
}

void add_oob_service(string type, string callback)
{
    oob_services[type] = ([
      "owner": previous_object(),
      "func": callback,
    ]);
}

void remove_oob_service(string type) { map_delete(oob_services, type); }

mapping query_oob_services()
{
    return copy(oob_services);
}

int setup_oob_socket()
{
    object sock = find_object(SOCKET_D);

    sock->set_socket_type(MUD);
    sock->set_read_cb("oob_server_read_callback");
    sock->set_listen_cb("oob_listen_callback");
    sock->set_close_cb("oob_close_callback");
    sock->set_log_file(LOG_OOB);

    return sock->server_create("OOB", OOB_TCP_PORT);
}

varargs void send_oob_packet(string t_mudname, mixed packet, int quiet)
{

    if(!quiet) OOBLOG("Notice [OOB/send_oob_packet]: Sending " + sprintf("%O", packet) + "\n");

    if(oob_queue[t_mudname]) oob_queue[t_mudname] += ({ packet });
    else oob_queue[t_mudname] = ({ packet });

    if(oob_unanswered[t_mudname]) oob_unanswered[t_mudname] += ({ packet[0] });
    else oob_unanswered[t_mudname] = ({ packet[0] });

    if(member_array(t_mudname, values(o_muds)) == -1 && member_array(t_mudname, values(t_muds)) == -1)
    {
        mapping mudlist = find_object(I3_MUDLIST)->get_mud_list();

        i3 = find_object(I3_CHDMOD);

        if(member_array("auth", keys(mudlist[t_mudname][11])) != -1)
            i3->sendToMud("auth-mud-req", 0, t_mudname, ({ }) );
        else
        {
            i3->sendToMud("oob-req", 0, t_mudname, ({ }) );
            call_out("oob_connect", 2, t_mudname);
        }

    }
    else
        OOBLOG("Notice [OOB/send_oob_packet]: Using already opened connection.\n");

}

varargs void send_oob_packets(string t_mudname, mixed *packets, int quiet)
{

    if(!quiet) OOBLOG("Notice [OOB/send_oob_packet]: Sending " + sprintf("%O", packets) + "\n");

    foreach(mixed array packet in packets)
    {

        if(oob_queue[t_mudname]) oob_queue[t_mudname] += ({ packet });
        else oob_queue[t_mudname] = ({ packet });

        if(oob_unanswered[t_mudname]) oob_unanswered[t_mudname] += ({ packet[0] });
        else oob_unanswered[t_mudname] = ({ packet[0] });

    }

    if(member_array(t_mudname, values(o_muds)) == -1 && member_array(t_mudname, values(t_muds)) == -1)
    {
        mapping mudlist = find_object(I3_MUDLIST)->get_mud_list();

        i3 = find_object(I3_CHDMOD);

        if(member_array("auth", keys(mudlist[t_mudname][11])) != -1)
            i3->sendToMud("auth-mud-req", 0, t_mudname, ({ }) );
        else
        {
            i3->sendToMud("oob-req", 0, t_mudname, ({ }) );
            call_out("oob_connect", 2, t_mudname);
        }

    }
    else
        OOBLOG("Notice [OOB/send_oob_packet]: Using already opened connection.\n");

}

varargs void send_oob_ack_packet(string t_mudname, mixed packet, int quiet)
{
    object sock = find_object(SOCKET_D);

    if(!quiet) OOBLOG("Notice [OOB/send_oob_ack_packet]: Sending " + sprintf("%O", packet) + "\n");

    foreach(int fd, int in_out in direction)
    {

        if(in_out > 0 && t_muds[fd] == t_mudname)
        {
            sock->client_write(fd, packet);
            break;
        }
        else
        if(in_out < 0 && o_muds[fd] == t_mudname)
        {
            sock->server_write(fd, packet, 0);
            break;
        }

    }

}

varargs void send_oob_ack_packets(string t_mudname, mixed *packets, int quiet)
{
    foreach(mixed packet in packets) send_oob_ack_packet(t_mudname, packet, quiet);
}

void oob_authorized(string t_mudname)
{

    if(member_array(t_mudname, values(o_muds)) == -1 && member_array(t_mudname, values(t_muds)) == -1)
        oob_connect(t_mudname);
    else
        OOBLOG("Notice [OOB/oob_authorized]: Using already opened connection.\n");

}

void oob_req_handler(mixed *packet)
{
    string o_mud = packet[I3_O_MUD];

    OOBLOG("Notice [OOB_AUTH/oob-req]: Received OOB request from '" + o_mud + "'\n");
    find_object(I3_AUTH)->set_session_key(o_mud, -1);
}

void oob_connect(string t_mudname)
{
    object sock = find_object(SOCKET_D);
    mapping mudlist = find_object(I3_MUDLIST)->get_mud_list();
    string tmud_ip;
    int tmud_port, fd, session_key;

    if (mudlist[t_mudname][1] == mudlist[mud_name()][1])
        tmud_ip = "127.0.0.1";
    else
        tmud_ip = mudlist[t_mudname][1];

    tmud_port = mudlist[t_mudname][3];

    if(!tmud_port) return;

    OOBLOG("Notice [OOB/oob_connect]: Connecting to " + tmud_ip + " " + tmud_port + "\n");

    sock->set_socket_type(MUD);
    sock->set_read_cb("oob_client_read_callback");
    sock->set_close_cb("oob_close_callback");
    sock->set_log_file(LOG_OOB);

    fd = sock->client_create("OOB." + t_mudname + "." + pid, tmud_ip, tmud_port);

    if(fd < 0)
    {

        if(fd == EEISCONN)
        {
            OOBLOG("Warning [OOB/oob_connect]: Already connected, maybe still closing. Will try again.\n");
            call_out("oob_connect", 2, t_mudname);

            return;
        }

        OOBLOG("Error [OOB/oob_connect]: Unable to setup socket - " + socket_error(fd) + "\n");

        return;
    }

    OOBLOG("Notice [OOB/oob_connect]: Connected to " + tmud_ip + " " + tmud_port + " on fd: " + fd + "\n");

    t_muds[fd] = t_mudname;
    session_key = find_object(I3_AUTH)->query_session_key(t_mudname);
    sock->client_write(fd, ({ "oob-begin", mud_name(), session_key > 0, session_key }) );

    pid++;
}

void oob_close_callback(int fd)
{
    OOBLOG("Notice [OOB/oob_close_callback]: Closing connection to " + (o_muds[fd] || t_muds[fd]) + "\n");

    map_delete(oob_unanswered, o_muds[fd] || t_muds[fd]);
    map_delete(oob_queue, o_muds[fd] || t_muds[fd]);
    o_muds[fd] = 0;
    t_muds[fd] = 0;
    direction[fd] = 0;
}

void oob_listen_callback(int fd, int remote_fd)
{
    OOBLOG("Notice [OOB/oob_listen_callback]: fd: " + fd + ", remote_fd: " + remote_fd + "\n");
}

void oob_server_read_callback(int fd, mixed packet)
{
    object sock = find_object(SOCKET_D);
    mixed oobType;
    string mudname, queued_packet;

    if(!sizeof(packet)) return;

    if(!arrayp(packet))
    {
        OOBLOG("Error [OOB/oob_server_read_callback]: fd: " + fd + ", packet: " + sprintf("%O", packet) + "\n");

        return;
    }

    direction[fd] = OOB_INCOMING;
    oobType = packet[0];

    switch(oobType)
    {

        case "oob-begin":
        {
            string o_mud = packet[1];

            OOBLOG("Notice [OOB/oob_server_read_callback]: fd: " + fd + ", packet: " + sprintf("%O", packet) + "\n");

            if(find_object(I3_AUTH)->query_session_key(o_mud) == packet[3])
            {
                o_muds[fd] = o_mud;
                sock->server_write(fd, ({ "oob-begin", mud_name(), 0, 0 }), 0);
            }
            else
            {
                sock->close(fd);
                oob_close_callback(fd);
            }

            return;
        }
        case "oob-end":
        {
            mixed tmp;

            OOBLOG("Notice [OOB/oob_server_read_callback]: fd: " + fd + ", packet: " + sprintf("%O", packet) + "\n");

            if(oob_queue[o_muds[fd]])
            {

                foreach(tmp in oob_queue[t_muds[fd]])
                    sock->server_write(fd, tmp, 0);

                map_delete(oob_queue, o_muds[fd]);
            }

            sock->server_write(fd, ({ "oob-end", mud_name() }), 0);
            find_object(I3_AUTH)->remove_session_key(o_muds[fd]);

            return;
        }

    }

    mudname = o_muds[fd];

    if(oob_unanswered[mudname])
        queued_packet = oob_unanswered[mudname][0];
    else
        queued_packet = "";

    handle_oob_packet(fd, mudname, queued_packet, packet);
}

void oob_client_read_callback(int fd, mixed packet)
{
    object sock = find_object(SOCKET_D);
    mixed oobType;
    string mudname, queued_packet;

    if(!sizeof(packet)) return;

    if(!arrayp(packet))
    {
        OOBLOG("Error [OOB/oob_client_read_callback]: fd: " + fd + ", packet: " + sprintf("%O", packet) + "\n");

        return;
    }

#if 0
    OOBLOG("Notice [OOB/oob_client_read_callback]: fd: " + fd + ", packet: " + sprintf("%O", packet) + "\n");
#endif

    direction[fd] = OOB_OUTGOING;
    oobType = packet[0];

    switch(oobType)
    {

        case "oob-begin":
        {
            OOBLOG("Notice [OOB/oob_client_read_callback]: fd: " + fd + ", packet: " + sprintf("%O", packet) + "\n");
#if 0 
            OOBLOG("Notice [OOB/oob_client_read_callback]: oob_queue: " + sprintf("%O", oob_queue) + "\n");
            OOBLOG("Notice [OOB/oob_client_read_callback]: t_muds[" + fd + "]: " + sprintf("%O", t_muds[fd]) + "\n");
#endif
            if(oob_queue[t_muds[fd]])
            {
                mixed tmp;

                foreach(tmp in oob_queue[t_muds[fd]])
                    sock->client_write(fd, tmp);

                map_delete(oob_queue, t_muds[fd]);
            }

            sock->client_write(fd, ({ "oob-end", mud_name() }) );

            return;
        }
        case "oob-end":
        {
            mixed tmp;

            OOBLOG("Notice [OOB/oob_client_read_callback]: fd: " + fd + ", packet: " + sprintf("%O", packet) + "\n");

            if(oob_queue[t_muds[fd]])
            {

                foreach(tmp in oob_queue[t_muds[fd]])
                    sock->client_write(fd, tmp);

                sock->client_write(fd, ({ "oob-end", mud_name() }) );
                map_delete(oob_queue, t_muds[fd]);
            }
            else
            {
                sock->close(fd);
                oob_close_callback(fd);
            }

            return;
        }

    }

    OOBLOG("Notice [OOB/oob_client_read_callback]: oob_unanswered: " + sprintf("%O", oob_unanswered) + "\n");

    mudname = t_muds[fd];

    if(oob_unanswered[mudname])
        queued_packet = oob_unanswered[mudname][0];
    else
        queued_packet = "";

    handle_oob_packet(fd, mudname, queued_packet, packet);
}

void handle_oob_answered(string t_mudname)
{
    oob_unanswered[t_mudname] = oob_unanswered[t_mudname][1..<1];
}

void handle_oob_packet(int fd, string mudname, string queued_packet, mixed packet)
{
    object owner = this_object();
    mixed oobType;
    string func = "dummy";

    oobType = packet[0];

    if(intp(oobType))
    {
        owner = oob_services["news-read-reply"]["owner"];
        func = oob_services["news-read-reply"]["func"];
    }
    else
    if(member_array(oobType, keys(oob_services)) == -1)
    {

        if(queued_packet == "new-grplist-req")
            handle_oob_answered(mudname);

        owner = oob_services["news-grplist-reply"]["owner"];
        func = oob_services["news-grplist-reply"]["func"];
    }
    else
    if(member_array(oobType, keys(oob_services)) != -1)
    {

        switch(oobType)
        {
            case "mail-ack":
                if(queued_packet == "mail")
                    handle_oob_answered(mudname);
                break;
            case "file-list-reply":
                if(queued_packet == "file-list-req")
                    handle_oob_answered(mudname);
                break;
            case "file-put-ack":
                if(queued_packet == "file-put")
                    handle_oob_answered(mudname);
                break;
            case "file-get-reply":
                if(queued_packet == "file-get-req")
                    handle_oob_answered(mudname);
                break;
        }

        owner = oob_services[oobType]["owner"];
        func = oob_services[oobType]["func"];
    }

    if(function_exists(func, owner))
        call_other(owner, func, mudname, packet);
    else
        call_other(this_object(), "oob_rec_unsupported", mudname, packet);

}

void oob_rec_unsupported(string o_mudname, mixed *packet)
{
    OOBLOG("Fatal [OOB/unsupported]: Service handler for OOB packet '" + sprintf("%O", packet) + "' not available.\n");
}
