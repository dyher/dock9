/* mudlist.c
 *
 * Tricky @ Rock the Halo
 * July 22nd, 2006
 * Lists muds on the I3 network
 *
 */

#include <mudlib.h>
#include <intermud3.h>

#define WRAP_DEFAULT 79

object i3mudlist;
mapping mudlist;
string *mudlist_arr;
int up, total;
int MudnameLen, LibnameLen, DriverLen;

string buildMudlist(string, string, string, string, string, int);

string getNextArg(string *argv, int argc)
{
    string ret;

    ret = argv[argc];

    if (argv[argc][0..0] != "\"")
        return ret;

    sscanf (implode (argv[argc..<1], " "), "\"%s\"", ret);

    return ret;
}

int main(string str)
{
    object pager;
    string *argv = ({ }), *tmp_argv = ({ });
    string list = "", header = "";
    string mudname = "", libname = "", baselibname = "", driver = "", ipnum = "";
    int argc = 0;
    int all = 0, sorted = 0;
    int not = 0;
    int wrap, tmp;

    i3mudlist = find_object(I3_MUDLIST);

    if (!i3mudlist || !(mudlist = i3mudlist->get_mud_list()))
        return notify_fail("Mudlist unavailable.\n");

    wrap = WRAP_DEFAULT;

    if (this_user())
    {
        if (this_user()->query_env("wrap"))
            wrap = to_int(this_user()->query_env("wrap"));
        else if (this_user()->query("columns"))
            wrap = to_int(this_user()->query("columns"));
    }

    if (wrap > 100) wrap -= 10;

    tmp = wrap - 21;
    tmp /= 3;

    MudnameLen = tmp;
    LibnameLen = tmp;
    DriverLen = tmp - 3;

    if (str && str != "")
    {
        tmp_argv = explode(str, " ");

        foreach (string t in tmp_argv) if (t != "")
            argv += ({ t });

    }

    while (argc < sizeof (argv))
    {
        switch (argv[argc])
        {
            case "!":
                not = 1;
                break;
            case "-a":
            case "--all":
                all = 1;
                break;
            case "-s":
            case "--sorted":
                sorted = 1;
                break;
            case "-sa":
            case "-as":
                all = 1;
                sorted = 1;
                break;
            case "-m":
            case "--mud":
                argc++;

                if (argc >= sizeof(argv))
                    return notify_fail
                        ("Error [mudlist]: mudname required.\n");

                mudname = getNextArg(argv, argc);

                argc += sizeof(explode(mudname, " ")) - 1;

                if (not)
                {
                    mudname = "! " + mudname;
                    not = 0;
                }

                break;
            case "-l":
            case "--lib":
                argc++;

                if (argc >= sizeof (argv))
                    return notify_fail
                        ("Error [mudlist]: libname required.\n");

                libname = getNextArg(argv, argc);

                argc += sizeof(explode(libname, " ")) - 1;

                if (not)
                {
                    libname = "! " + libname;
                    not = 0;
                }

                break;
            case "-bl":
            case "--baselib":
                argc++;

                if (argc >= sizeof (argv))
                    return notify_fail
                        ("Error [mudlist]: baselibname required.\n");

                baselibname = getNextArg(argv, argc);

                argc += sizeof(explode(baselibname, " ")) - 1;

                if (not)
                {
                    baselibname = "! " + baselibname;
                    not = 0;
                }

                break;
            case "-d":
            case "--driver":
                argc++;

                if (argc >= sizeof(argv))
                    return notify_fail
                        ("Error [mudlist]: driver name required.\n");

                driver = getNextArg(argv, argc);
                argc += sizeof(explode(driver, " ")) - 1;

                if (not)
                {
                    driver = "! " + driver;
                    not = 0;
                }

                break;
            case "-i":
            case "--ipnum":
                argc++;

                if (argc >= sizeof(argv))
                    return notify_fail
                        ("Error [mudlist]: IP number required.\n");

                ipnum = getNextArg(argv, argc);
                argc += sizeof(explode(ipnum, " ")) - 1;

                if (not)
                {
                    ipnum = "! " + ipnum;
                    not = 0;
                }

                break;
        }

        argc++;
    }

    mudlist_arr = filter(keys(mudlist), (: arrayp(mudlist[$1]) :));

    if (sorted)
        mudlist_arr = sort_array(mudlist_arr, 1);

    total = sizeof(mudlist_arr);
    up = 0;

    list += buildMudlist(mudname, libname, baselibname, driver, ipnum, all);

    header =
        sprintf("I3 Mudlist (%i of %i)%s\n",
                up,
                total,
                !all ? "" : " - A '*' next to the name means the mud is up."
        );
    header += sprintf("%-" + wrap + "'='s\n", "");
    header +=
        sprintf(" %-:" + MudnameLen + "s %-:" + LibnameLen + "s %-:" + DriverLen + "s %-:15s %5s\n",
                "Mudname",
                "Lib",
                "Driver",
                "IP",
                "Port"
        );
    header += sprintf("%-" + wrap + "'='s\n", "");

    list = header + list;

    pager = clone_object(OBJ_PAGER);
    pager->page(list);

    return 1;
}

string buildMudlist(string mudname, string libname, string baselibname,
                    string driver, string ipnum, int all)
{
    string list = "";

    foreach (string name in mudlist_arr)
    {
        mixed val;

        if (mudname != "")
        {

            if (mudname[0..0] == "!")
            {
                if (lower_case(name[0..strlen(mudname[2..<1]) - 1]) ==
                    lower_case(mudname[2..<1]))
                    continue;
            }
            else if (lower_case(name[0..strlen(mudname) - 1]) !=
                     lower_case(mudname))
                continue;
        }

        if (libname != "")
        {
            val = mudlist[name];

            if (libname[0..0] == "!")
            {
                if (lower_case(val[5][0..strlen(libname[2..<1]) - 1]) ==
                    lower_case(libname[2..<1]))
                    continue;
            }
            else if (lower_case(val[5][0..strlen(libname) - 1]) !=
                     lower_case(libname))
                continue;
        }

        if (baselibname != "")
        {
            val = mudlist[name];

            if (baselibname[0..0] == "!")
            {
                if (lower_case(val[6][0..strlen(baselibname[2..<1]) - 1]) ==
                    lower_case(baselibname[2..<1]))
                    continue;
            }
            else if (lower_case(val[6][0..strlen(baselibname) - 1]) !=
                     lower_case(baselibname))
                continue;
        }

        if (driver != "")
        {
            val = mudlist[name];

            if (driver[0..0] == "!")
            {
                if (lower_case(val[7][0..strlen(driver[2..<1]) - 1]) ==
                    lower_case(driver[2..]))
                    continue;
            }
            else if (lower_case(val[7][0..strlen(driver) - 1]) !=
                     lower_case(driver))
                continue;
        }

        if (ipnum != "")
        {
            val = mudlist[name];

            if (ipnum[0..0] == "!")
            {
                if (lower_case(val[1][0..strlen(ipnum[2..<1]) - 1]) ==
                    lower_case(ipnum[2..]))
                    continue;
            }
            else if (lower_case(val[1][0..strlen(ipnum) - 1]) !=
                     lower_case(ipnum))
                continue;
        }

        val = mudlist[name];

        if (val[0] == 0 && !all) continue;

        if (val[0] > 0)
        {
            list += sprintf("%s is rebooting\n", name);
            continue;
        }

        if (val[0] != 0) up++;

        list += sprintf("%s%-:" + MudnameLen + "s %-:" + LibnameLen + "s %-:" + DriverLen + "s %-:15s %5i\n",
                        val[0] != 0 && all ? "*" : " ",
                        TERMINAL_D->parse_pinkfish(name, "dumb"),
                        TERMINAL_D->parse_pinkfish(val[5], "dumb"),
                        TERMINAL_D->parse_pinkfish(val[7], "dumb"),
                        val[1],
                        val[2]
                );
    }

    return list;
}

string help()
{
    string help = HIW + " SYNTAX: " + NOR;

    help += @ENDHELP
mudlist [-a | -s] [!] [-m <mudname>] [-l <libname>] [-d <drivername>]

Lists the muds connected to the Intermud 3 network.
The options available are:
-a              List every mud up or down
-s              List muds sorted alphabetically (by name)
-m <mudname>    Filter muds with this name
-l <libname>    Filter muds using this library
-d <drivername> Filter muds using this driver
-i <ipnumber>   Filter muds using this IP number
! -m, -l, -d or -i Negates the filter

-a and -s can be combined into either -sa or -as
-m, -l and -d can be combined (with '!') to list a partial selection of muds.
(The names are case-insensitive and can be just the first few letters. If
there are spaces in a name, use quotes such as "dead souls".)

Long options are:
-a: --all
-s: --sorted
-m: --mud
-l: --lib
-d: --driver
-i: --ipnum

ENDHELP;

    help += HIW + " EXAMPLES:" + NOR + @ENDHELP

%^GREEN%^mudlist%^RESET%^
  Lists muds online
  unsorted

%^GREEN%^mudlist -s -l lpuni%^RESET%^
  Lists muds online
  using a lib starting with 'lpuni'
  sorted

%^GREEN%^mudlist -a ! -l dead -m l%^RESET%^
  Lists all muds (up or down)
  not using a lib starting with 'dead'
  name starting with a 'l'
  unsorted

%^GREEN%^mudlist -a ! -l dead -s ! -m l -d dgd%^RESET%^
  Lists all muds (up or down)
  not using a lib starting with 'dead'
  name not starting with a 'l'
  driver name starting with 'dgd'
  sorted
ENDHELP;

    return help;
}
