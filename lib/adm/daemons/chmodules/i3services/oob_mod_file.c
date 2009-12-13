/* mod_file.c

   Tricky@Rock the Halo
   28-MAY-2007
   Intermud3 OOB file service

   Last edited on June 14th, 2007 by Tricky@Rock the Halo

*/

#include <driver/runtime_config.h>
#include <intermud3.h>

#define TRUNCATED "***TRUNCATED***\n"

privatev object i3, oob;
privatev mapping ls_ids, put_ids, get_ids;
privatev string put_status, get_status;
privatev int put_packet_size, put_ctr, put_success;
privatev int put_ack_packet_size, put_ack_ctr;
privatev int get_packet_size, get_ctr;

void create()
{
    string array ftpdir_array;
    string ftpdir = "";

    ls_ids = ([ ]);
    put_ids = ([ ]);
    get_ids = ([ ]);

    put_packet_size = 0;

    if(file_size(OOB_DIR) == -1)
    {
        ftpdir_array = explode(OOB_DIR, "/");

        foreach(string dir in ftpdir_array)
        {
            ftpdir += "/" + dir;

            if(file_size(ftpdir) != -2) mkdir(ftpdir);
            if(file_size(ftpdir) != -2) return;
        }

    }

    i3 = find_object(I3_CHDMOD);
    i3->add_service_name("file", 1);

    oob = find_object(I3_OOB);
    oob->add_oob_service("file-list-req", "list_req_handler");
    oob->add_oob_service("file-list-reply", "list_reply_handler");
    oob->add_oob_service("file-put", "put_handler");
    oob->add_oob_service("file-put-ack", "put_ack_handler");
    oob->add_oob_service("file-get-req", "get_req_handler");
    oob->add_oob_service("file-get-reply", "get_reply_handler");
}

void remove()
{
    i3 = find_object(I3_CHDMOD);
    i3->remove_service_name("file");

    oob = find_object(I3_OOB);
    oob->remove_oob_service("file-list-req");
    oob->remove_oob_service("file-list-reply");
    oob->remove_oob_service("file-put");
    oob->remove_oob_service("file-put-ack");
    oob->remove_oob_service("file-get-req");
    oob->remove_oob_service("file-get-reply");

    destruct();
}

int send_file_list_req(string t_mudname, string o_user, string dir)
{
    mapping mudlist = find_object(I3_MUDLIST)->get_mud_list();

    if(mudlist[t_mudname][0] != -1) return -1;
    if(member_array("file", keys(mudlist[t_mudname][11])) == -1) return 0;

    if(ls_ids[o_user]) ls_ids[o_user] += ({ dir == "" ? "" : "/" + dir, t_mudname });
    else ls_ids[o_user] = ({ dir == "" ? "" : "/" + dir, t_mudname });

    oob = find_object(I3_OOB);
    oob->send_oob_packet(t_mudname, ({ "file-list-req", mud_name(), o_user, dir }) );

    return 1;
}

int send_file_put_req(string t_mudname, string o_user, string r_fname, string l_fname)
{
    mapping mudlist = find_object(I3_MUDLIST)->get_mud_list();
    string array packets = ({ });
    string contents;
    int new_id;

    if(mudlist[t_mudname][0] != -1) return -1;
    if(member_array("file", keys(mudlist[t_mudname][11])) == -1) return -2;

    l_fname = resolve_path(this_player()->query("cwd"), l_fname);

    if(file_size(l_fname) < 0) return (file_size(l_fname) - 2);

    contents = read_file(l_fname);

    if(!stringp(contents) || contents == 0) return 0;

    new_id = 0;

    for(int i = 0; i < strlen(t_mudname); i++) new_id += (t_mudname[i] + ((i / 2) + 1));
    new_id = (new_id * 1000) + random(470831) + ((time() & 0x00ffffff) ^ 0x005a5a5a);

    while(put_ids[new_id]) new_id++;

    put_ids[new_id] = ({ find_player(o_user), r_fname, l_fname, t_mudname });
    put_ctr = 0;
    put_ack_ctr = 0;
    put_ack_packet_size = 1;

    if(sizeof(contents) < 4000)
    {
        packets += ({ ({ "file-put", new_id, mud_name(), o_user, r_fname, contents }) });
    }
    else
    {
        string tmp1 = contents;

        while(sizeof(tmp1))
        {
            string tmp2 = "";

            while(sizeof(tmp2) < 4000 && sizeof(tmp1))
            {
                tmp2 += tmp1[0..3999];
                tmp1 = tmp1[4000..<1];
            }

            put_ctr++;

            if(put_ctr == 1)
                packets += ({ ({ "file-put", new_id, mud_name(), o_user, r_fname, tmp2, (sizeof(contents) / 4000) + 1 }) });
            else
                packets += ({ ({ "file-put", new_id, mud_name(), o_user, r_fname, tmp2 }) });

        }

    }

    oob = find_object(I3_OOB);
    oob->send_oob_packets(t_mudname, packets, 1);

    return 1;
}

int send_file_get_req(string t_mudname, string o_user, string r_fname, string l_fname)
{
    mapping mudlist = find_object(I3_MUDLIST)->get_mud_list();
    int new_id;

    if(find_player(o_user) != this_interactive()) return 0;
    if(mudlist[t_mudname][0] != -1) return -1;
    if(member_array("file", keys(mudlist[t_mudname][11])) == -1) return 0;

    l_fname = resolve_path(this_player()->query("cwd"), l_fname);
    if(file_size(l_fname) > -1) return -2;

    new_id = 0;

    for(int i = 0; i < strlen(t_mudname); i++) new_id += (t_mudname[i] + ((i / 2) + 1));
    new_id = (new_id * 1000) + random(470831) + ((time() & 0x00ffffff) ^ 0x005a5a5a);

    while(arrayp(get_ids[new_id])) new_id++;

    get_ids[new_id] = ({ find_player(o_user), r_fname, l_fname, t_mudname });
    get_ctr = 0;
    get_packet_size = 1;

    oob = find_object(I3_OOB);
    oob->send_oob_packet(t_mudname, ({ "file-get-req", new_id, mud_name(), o_user, r_fname }) );

    return 1;
}

void list_req_handler(string o_mudname, mixed array packet)
{
    mixed array dirstat;
    string dir = packet[3];

    if(dir == "" || dir == "/") dir = ".";
    if(dir[0..0] == "/") dir = dir[1..<1];

    dir = resolve_path(OOB_DIR, dir);

    switch(file_size(dir))
    {

        case -2:
        {
            if(dir[<1..<1] != "/") dir += "/";

            dir += "*";
            dirstat = get_dir(dir, -1);

            break;
        }
        case -1:
        {

            if(sizeof(dirstat = get_dir(dir, -1)) > 0)
                break;

            oob = find_object(I3_OOB);
            oob->send_oob_ack_packet(packet[1], ({ "file-list-reply", packet[2], ({ }) }) );

            return;
        }
        default:
        {
            dirstat = get_dir(dir, -1);

            break;
        }

    }

    if(!arrayp(dirstat)) dirstat = ({ });

    oob = find_object(I3_OOB);
    oob->send_oob_ack_packet(packet[1], ({ "file-list-reply", packet[2], dirstat }) );
}

void list_reply_handler(string o_mudname, mixed array packet)
{
    object user;
    mixed array dir;
    mixed array ls_data;
    string filelist;
    int total_fsize = 0, total_fnames = 0;

    if(!objectp(user = find_player(packet[1]))) return;
    if(!arrayp(ls_data = ls_ids[packet[1]])) return;

    dir = packet[2];

    if (!arrayp(dir) || sizeof(dir) < 1)
    {
        tell_object(user, "OOB File: Cannot access: No such file or directory.\n");
        return;
    }

    foreach(mixed array file in dir)
    {
        string fname = file[0];
        int    fsize = file[1];

        if(fname == "." || fname == "..") continue;

        total_fnames++;

        if(fsize > -1) total_fsize += fsize;

    }

    filelist  = sprintf("%s:%s%s\n", ls_data[1], (ls_data[0] == "" ? "/" : ""), ls_data[0]);
    filelist += sprintf("%dK (%d bytes) in %d file(s)\n", total_fsize / 1024, total_fsize, total_fnames);

    foreach(mixed array file in dir)
    {
        string fname = file[0];
        int    fsize = file[1];
        int    ftime = file[2];
        int    is_dir = 0;

        if(fname == "." || fname == "..") continue;

        if(fsize == -2)
        {
            fname = "%^BOLD%^BLUE%^" + fname + "%^RESET%^";
            is_dir = 1;
        }

        filelist += sprintf("%c %9s %-s    %-s\n",
          is_dir ? 'd' : '-',
          is_dir ? "" : "" + fsize,
          ctime(ftime),
          fname
        );

        if(strlen(filelist) + (strlen(TRUNCATED) * 2) >= get_config(__MAX_STRING_LENGTH__))
        {
            filelist += TRUNCATED;
            break;
        }

    }

    tell_object(user, filelist + "\n");
    map_delete(ls_ids, packet[1]);
}

void put_handler(string o_mudname, mixed array packet)
{
    string fname = packet[4];

    if(fname[0..0] == "/") fname = fname[1..<1];

    fname = OOB_DIR + packet[2] + "/" + packet[3] + "/" + fname;

    if(!put_packet_size)
    {

        if(sizeof(packet) == 6) put_packet_size = 1;
        else put_packet_size = packet[6];

        put_ctr = 0;
    }

    put_ctr++;

    if(!master()->valid_write(fname, this_object(), "write_file"))
        put_success = -3;
    else
    {

        if(file_size(OOB_DIR + packet[2]) != -2)
            mkdir(OOB_DIR + packet[2]);
        if(file_size(OOB_DIR + packet[2] + "/" + packet[3]) != -2)
            mkdir(OOB_DIR + packet[2] + "/" + packet[3]);

        if(put_ctr == 1)
        {
            if(catch(put_success = write_file(fname, packet[5], 1)))
                put_success = -3;
        }
        else
        if(catch(put_success = write_file(fname, packet[5])))
            put_success = -3;

    }

    oob = find_object(I3_OOB);

    if(put_ctr == 1 && sizeof(packet) > 6)
        oob->send_oob_ack_packet(packet[2], ({ "file-put-ack", packet[1], put_success, put_packet_size }) );
    else
        oob->send_oob_ack_packet(packet[2], ({ "file-put-ack", packet[1], put_success }) );

    if(put_ctr == put_packet_size)
        put_packet_size = 0;

}

void put_ack_handler(string o_mudname, mixed array packet)
{
    object user;
    mixed array put_data;

    if(!arrayp(put_data = put_ids[packet[1]])) return;
    if(!objectp(user = put_data[0])) return;

    if(sizeof(packet) >= 4) put_ack_packet_size = packet[3];

    put_ack_ctr++;

    switch(packet[2])
    {
        case  1: put_status = "Ok.\n"; break;
        case -3: put_status = "Write permission denied.\n"; break;
        case -1: put_status = "File path error.\n"; break;
        default: put_status = "Unknown error.\n"; break;
    }

    if(put_ack_ctr == put_ack_packet_size)
    {
        tell_object(user, sprintf("%s to %s/%s:/%s ", put_data[2], put_data[3], user->query_name(), put_data[1]) + put_status);
        map_delete(put_ids, packet[1]);
    }

}

void get_req_handler(string o_mudname, mixed array packet)
{
    string array packets = ({ });
    string fname = packet[4], contents = "";
    int success, id = packet[1];

    if(!stringp(fname) || !fname || fname == "")
        success = -1;
    else
    {
        if(fname[0..0] == "/") fname = fname[1..<1];

        fname = OOB_DIR + fname;

        if(!master()->valid_read(fname, this_object(), "read_file"))
            success = -2;
        else
        {

            if(catch(contents = read_file(fname)))
                success = -1;
            else success = 1;

        }

    }

    if(sizeof(contents) < 4000)
    {
        packets += ({ ({ "file-get-reply", id, success, contents }) });
    }
    else
    {
        string tmp1 = contents;

        while(sizeof(tmp1))
        {
            string tmp2 = "";

            while(sizeof(tmp2) < 4000 && sizeof(tmp1))
            {
                tmp2 += tmp1[0..3999] + "\n";
                tmp1 = tmp1[4000..<1];
            }

            put_ctr++;

            if(put_ctr == 1)
                packets += ({ ({ "file-get-reply", id, success, tmp2, (sizeof(contents) / 4000) + 1 }) });
            else
                packets += ({ ({ "file-get-reply", id, success, tmp2 }) });

        }

    }

    oob = find_object(I3_OOB);
    oob->send_oob_ack_packets(packet[2], packets, 1);
}

void get_reply_handler(string o_mudname, mixed array packet)
{
    object user;
    mixed array get_data;
    string fname;

    if(!arrayp(get_data = get_ids[packet[1]])) return;
    if(!objectp(user = get_data[0])) return;

    if(sizeof(packet) >= 5) get_packet_size = packet[4];

    fname = get_data[2];
    get_ctr++;

    switch(packet[2])
    {
        case  1:
        {
            int status;

            if(get_ctr == 1) catch(status = write_file(fname, packet[3], 1));
            else catch(status = write_file(fname, packet[3]));

            if(!status) get_status = "Unable to find remote file.\n";
            else get_status = "Ok.\n";

            break;
        }
        case -3: get_status = "Write permission denied.\n"; break;
        case -2: get_status = "Read permission denied.\n"; break;
        case -1: get_status = "File path error.\n"; break;
        default: get_status = "Unknown error.\n"; break;
    }

    if(get_ctr == get_packet_size)
    {
        tell_object(user, sprintf("%s:/%s to %s: ", get_data[3], get_data[1], get_data[2]) + get_status);
        map_delete(get_ids, packet[1]);
    }

}
