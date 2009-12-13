/*
 * mod_mudlist.c
 *
 * Tricky
 * 18-AUG-2006
 * Intermud3 MudList service
 *
 * Last edited on December 17th, 2007 by Tricky
 *
 */

#include <intermud3.h>

privatev object I3;

mapping MudList, MudUpdate;

void remove_mud_from_list(string mud);
void remove_mud_from_update(string mud);
mapping get_mud_update();
mapping get_mud_list();
string array get_muds();
string array get_lc_muds();

void create()
{
    I3_CHDMOD->add_service("mudlist", "req_handler");

    if (file_exists(I3_MUDLIST_FILE + ".o"))
        restore_object(I3_MUDLIST_FILE);

    if (!MudList) MudList = ([ ]);
    if (!MudUpdate) MudUpdate = ([ ]);

    set_heart_beat(150);
    save_object(I3_MUDLIST_FILE);
}

void remove()
{
    I3_CHDMOD->remove_service("mudlist");
    save_object(I3_MUDLIST_FILE);
    destruct();
}

void heart_beat()
{
    string array muds = ({ });
    string muds_deleted;

    foreach (string mudname, int timestamp in copy(MudUpdate))
    {
        if (MudList[mudname][0] != -1 && time() - timestamp > 5 * 60 * 60)
        {
            muds += ({ mudname });
            remove_mud_from_list(mudname);
            remove_mud_from_update(mudname);
        }
    }

    if (sizeof(muds) > 1)
    {
        muds_deleted = implode(muds[0..<2], "', '") + "' and '" + muds[<1];
        I3LOG("Notice", "I3_MUDLIST", "heart_beat", "Deleted muds '" + muds_deleted + "' from the MudList.");
    }
    else if (sizeof(muds) == 1)
    {
        muds_deleted = muds[0];
        I3LOG("Notice", "I3_MUDLIST", "heart_beat", "Deleting mud '" + muds_deleted + "' from the MudList.");
    }
}

void remove_mud_from_list(string mud)
{
    map_delete(MudList, mud);
    save_object(I3_MUDLIST_FILE);
}

void remove_mud_from_update(string mud)
{
    map_delete(MudUpdate, mud);
    save_object(I3_MUDLIST_FILE);
}

mapping get_mud_update()
{
    return copy(MudUpdate);
}

mapping get_mud_list()
{
    return copy(MudList);
}

string array get_muds()
{
    return keys(MudList);
}

string array get_lc_muds()
{
    string array orig_arr = get_muds();
    string array new_arr = ({ });

    foreach (string name in orig_arr) new_arr += ({ lower_case(name) });

    return new_arr;
}

void req_handler(mixed array packet)
{
    mapping info_mapping = packet[7];
    string log_msg = "";
    int mudlist_id = packet[6];

    if (sizeof(packet) != 8)
    {
        I3LOG("Error", "I3_MUDLIST", "req_handler", "We don't like MudList packet size. Should be 8 but is " + sizeof(packet));
        return;
    }

    I3 = find_object(I3_CHDMOD);

    if (packet[2] != I3->getData("router_list")[0][0])
    {
        I3LOG("Error", "I3_MUDLIST", "req_handler", "We don't like packet element 2 (" + packet[2] + "). It should be " + I3->getData("router_list")[0][0]);
        return;
    }

    if (mudlist_id == I3->getData("mudlist_id"))
    {
        I3LOG(
          "Warning",
          "I3_MUDLIST",
          "req_handler",
          sprintf("%s (%d). %s",
            "We don't like packet element 6",
            mudlist_id,
            "It is the same as the current one. Continuing anyway."
          )
        );
    }
    else
    {
        log_msg = "mudlist_id: '" + mudlist_id + "'";
        I3LOG("Notice", "I3_MUDLIST", "req_handler", log_msg);
        I3->setData("mudlist_id", mudlist_id);
    }

    foreach (mixed mudname, mixed data in info_mapping)
    {
        log_msg = "";

        if (!arrayp(data))
        {
            log_msg = RED + "Removing mud '" + mudname + "'" + NOR + ", reason: Router has purged it from the listing.";

            if (MudList[mudname])
            {
                map_delete(MudList, mudname);
                map_delete(MudUpdate, mudname);
            }
        }
        else if (data[0] != -1)
        {
            if (data[0] > 7 * 24 * 60 * 60)
            {
                log_msg = RED + "Removing mud '" + mudname + "'" + NOR + ", reason: Shutdown delay is too long.";

                if (MudList[mudname])
                {
                    map_delete(MudList, mudname);
                    map_delete(MudUpdate, mudname);
                }
            }
            else
            {
                log_msg = YEL + "Mud '" + mudname + "' is down." + NOR + " Restart time: ";

                if (data[0] == 0)
                    log_msg += YEL + "unknown.";
                else if (data[0] == 1)
                    log_msg += GRN + "now.";
                else if (data[0] <= 5 * 60)
                    log_msg += GRN + data[0] + " seconds.";
                else if (data[0] > 5 * 60)
                    log_msg += RED + "indefinate.";

                log_msg += NOR;
            }

            if (MudList[mudname])
            {
                MudList[mudname] = data;
                MudUpdate[mudname] = time();
            }
        }
        else
        {
            string extra = "", libname = "", driver = "";

            if (stringp(data[5])) data[5] = trim(data[5]);
            if (stringp(data[7])) data[7] = trim(data[7]);
            if (stringp(data[5]) && data[5] != "") libname = "Libname: " + data[5];
            if (stringp(data[7]) && data[7] != "") driver = "Driver: " + data[7];

            if (libname != "" && driver == "")
                extra = " (" + libname + ")";
            else
            if (libname == "" && driver != "")
                extra = " (" + driver + ")";
            else
            if (libname != "" && driver != "")
                extra = " (" + libname + ", " + driver + ")";

            if (MudList[mudname])
                log_msg = GRN + "Mud '" + mudname + "' is up." + NOR + extra;
            else log_msg = HIG + "Adding mud '" + mudname + "' to the MudList." + NOR + extra;

            MudList[mudname] = data;
            MudUpdate[mudname] = time();
        }

        if (!log_msg || log_msg == "") continue;

        ANNOUNCE_CHDMOD->rec_m_msg(
          "intermud3",
          "I3_MUDLIST",
          log_msg
        );
        I3LOG("Notice", "I3_MUDLIST", "req_handler", TERMINAL_D->parse_pinkfish(log_msg, "dumb"));
    }

    save_object(I3_MUDLIST_FILE);
}
