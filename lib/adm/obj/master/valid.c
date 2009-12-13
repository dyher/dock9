/* valid.c
 *
 * Tacitus @ LPUniversity
 * 23-APR-05
 * Valid hooks, inherited by Master Obj
 *
 * Refractored 21-DEC-05 by Tacitus @ LPUni
 * Refractored 29-June-2008 by Tricky @ Rock the Halo
 *
 */

/* Preproccessor Statements */

/* Set this to 1 for debug messages */
#if 0
#define DEBUG(line) write_file("/log/security", line);
#else
#define DEBUG(line)
#endif

#define FILE_GROUPDATA "/adm/etc/groups"
#define FILE_ACCESSDATA "/adm/etc/access"

#define P_READ 1
#define P_WRITE 2
#define P_NETWORK 3
#define P_SHADOW 4
#define P_LINK 5
#define P_EXECUTE 6
#define P_BIND 7
#define P_OWNERSHIP 8

/* Global Variable */

mapping Access = ([ ]);
mapping Groups = ([ ]);

/* Function prototypes */

void parse_groups();
void parse_access();
string array parse(string str);
string array query_group(string group);
string array track_member(string id, string directory);
int query_access(string directory, string id, int type);
int isMember(string user, string group);

/* Functions */

void create()
{
    if (!mapp(Groups) || sizeof(Groups) < 1) parse_groups();
    if (!mapp(Access) || sizeof(Access) < 1) parse_access();
}

void parse_groups()
{
    string array arr = parse(read_file(FILE_GROUPDATA));
    int i, sz_arr = sizeof(arr);

    DEBUG("\tDebug [security]: Parsing group data file...\n");
    Groups = ([ ]);

    for (i = 0 ; i < sz_arr ; i++)
    {
        string array members;
        string group, str;
        int n, sz_members;

        if (!arr[i]) continue;

        if (sscanf(arr[i], "(%s)%s", group, str) != 2)
        {
            write("Error [security]: Invalid format of data in group data.\n");
            write("Security alert: Ignoring group on line " + (i + 1) + "\n");
            continue;
        }

        members = explode(str, ":");
        sz_members = sizeof(members);

        DEBUG(
          "Debug [security]: Adding group '" + group +
          "' with " + sz_members + " members.\n"
        );

        for (n = 0 ; n < sz_members ; n++)
        {
            if (!file_size(user_data_file(members[n])) && !sscanf(members[n], "[%*s]"))
            {
                write("Error [security]: Unknown user detected.\n");
                write(
                  "Security alert: User '" + members[n] +
                  "' ignored for group '" + group + "'.\n"
                );
                members -= ({ members[n] });
                continue;
            }

            DEBUG(
              "Debug [security]: Adding user '" + members[n] +
              "' to group '" + group + "'.\n"
            );
        }

        Groups += ([group: members]);
    }
}

void parse_access()
{
    string array arr = parse(read_file(FILE_ACCESSDATA));
    int i, sz_arr = sizeof(arr);

    DEBUG("\tDebug [security]: Parsing access data file...\n");
    Access = ([ ]);

    for (i = 0 ; i < sz_arr ; i++)
    {
        mapping data;
        string array entries;
        string directory, str;
        int n, sz_entries;

        data = ([ ]);

        if (!arr[i]) continue;

        if (sscanf(arr[i], "(%s)%s", directory, str) != 2)
        {
            write("Error [security]: Invalid format of data in access data.\n");
            error(
              "Security alert: Fatal error parsing access data on line " +
              (i + 1) + "\n"
            );
        }

        if (str[<1] == ':')
        {
            write(
              "Error [security]: Incomplete data in access data (trailing ':').\n"
            );
            error(
              "Security alert: Fatal error parsing access data on line " +
              (i + 1) + "\n"
            );
        }

        entries = explode(str, ":");
        sz_entries = sizeof(entries);

        DEBUG(
          "Debug [security]: Parsing data for directory '" + directory + "'.\n"
        );

        for (n = 0 ; n < sz_entries ; n++)
        {
            string array permArray = allocate(8);
            string identity, permissions;

            if (sscanf(entries[n], "%s[%s]", identity, permissions) != 2)
            {
                write(
                  "Error [security]: Invalid entry(" + n +
                  ") data format in access data.\n"
                );
                error(
                  "Security alert: Fatal error parsing access data on line " +
                  (i + 1) + "\n"
                );
            }

            DEBUG(
              "Debug [security]: Adding identity '" + identity +
              "' with permission string of '" + permissions + "'.\n"
            );

            /* read, write, network, shadow, link, execute, bind, ownership */
            if (strsrch(permissions, "r") != -1) permArray[0] = "r";
            if (strsrch(permissions, "w") != -1) permArray[1] = "w";
            if (strsrch(permissions, "n") != -1) permArray[2] = "n";
            if (strsrch(permissions, "s") != -1) permArray[3] = "s";
            if (strsrch(permissions, "l") != -1) permArray[4] = "l";
            if (strsrch(permissions, "e") != -1) permArray[5] = "e";
            if (strsrch(permissions, "b") != -1) permArray[6] = "b";
            if (strsrch(permissions, "o") != -1) permArray[7] = "o";

            data += ([ identity: permArray ]);
        }

        Access += ([directory: data]);
    }
}

string array parse(string str)
{
    string array arr;
    int i, sz_arr;

    if (!stringp(str) || str == "") return ({ });

    arr = explode(str, "\n");
    sz_arr = sizeof(arr);

    for (i = 0 ; i < sz_arr ; i++)
    {
        arr[i] = replace_string(arr[i], " ", "");
        arr[i] = replace_string(arr[i], "\t", "");
    }

    /* Filter out all comments and empty lines. */
    arr = filter_array(arr - ({ "" }), (: $1[0] != '#' && $1[0..0] != "" && sizeof($1) :) );
    return arr;
}

int valid_shadow(object ob)
{
    string location, name;

    location = get_base(ob);
    name = query_privs(ob);

    if (ob == this_object() || ob == master()) return 0;
    if (query_access(location, name, P_SHADOW) && !ob->disallow_shadow(ob)) return 1;

    return 0;
}

int valid_bind(object obj, object owner, object victim)
{
    string name;

    name = query_privs(previous_object());

    if (query_access(get_base(owner), name, P_BIND)
     && query_access(get_base(victim), name, P_BIND))
        return 1;

    return 0;
}

int valid_compile_to_c()
{
    return 0;
}

int valid_hide(object ob)
{
    return 0;
}

int valid_link(string from, string to)
{
    string name;

    if (this_interactive()) name = query_privs(this_interactive());
    else name = query_privs(previous_object());

    if (query_access(from, name, P_LINK) && query_access(to, name, P_LINK))
        return 1;

    return 0;
}

int valid_object(object ob)
{
    string location, name;

    location = file_name(ob);

    if (this_interactive()) name = query_privs(this_interactive());
    else name = query_privs(previous_object());

    if (!name) name = "<noname>";
    if (location == "/adm/daemons/login") return 1;
    if (query_access(location, name, P_EXECUTE)) return 1;

    return 0;
}

int valid_override(string file, string efun_name, string mainfile)
{
    DEBUG(
      "Debug [valid_override]: File: " + file +
      " Efun: " + efun_name +
      " Mainfile: " + mainfile + "\n"
    );

    if (mainfile == SIMUL_OB + ".c") return 1;
    if (efun_name == "destruct" && mainfile == OBJECT + ".c") return 1;
    if (efun_name == "ed") return 1;

    return 0;
}

int valid_socket(object caller, string func, mixed array info)
{
    DEBUG(
      "Debug [valid_socket]: Previous: " + sprintf("%O", all_previous_objects()) +
      " Caller: " + sprintf("%O", caller) +
      " Func: " + func +
      " Info: " + sprintf("%O", info) + "\n"
    );

    /* We might code a daemon or something that allows
       us to ban connections to certain ports/connections */

    /* string name;
    name = query_privs(caller);
    if(query_access(get_base(info[1]), name, P_NETWORK)) return 1;
    return 0;  */

    return 1;
}

int valid_save_binary(string file)
{
    return 1;
}

int valid_read(string file, object user, string func)
{
    object array all_previous = all_previous_objects();

    if (func == "file_size") return 1;
    if (func == "restore_object"
     && member_array(find_object("/adm/daemons/finger_d.c"), all_previous) != -1)
        return 1;

    if (this_interactive() && query_privs(user) != "[daemon]")
    {
        string name, ud_dir;

        name = query_privs(this_interactive());
        ud_dir = user_data_directory(name);

        if (strlen(file) > strlen(ud_dir))
            if (file[0..(strlen(ud_dir) - 1)] == ud_dir)
                return 1;
    }

    foreach (object obj in all_previous)
    {
        string name, true_name, tmp, tmp2;

        if (this_interactive() && query_privs(obj) != "[daemon]")
            name = query_privs(this_interactive());
        else name = query_privs(obj);

        if (!name) name = "<noname>";

        true_name = query_privs(obj);
        if (!true_name) true_name = "<noname>";

        DEBUG(
          "Debug [valid_read]: Object: " + sprintf("%O", obj) +
          " File: " + file +
          " Name: " + name +
          " TName: " + true_name + "\n"
        );

        if (obj == this_object()) continue;
        if (obj == master()) continue;
        if (obj == find_object(SIMUL_OB)) continue;
        if (file && sscanf(file, "/home/%*s/%s/%s", tmp, tmp2))
        {
            if (true_name == tmp || true_name == "[home_" + tmp + "]") continue;
            if (tmp2 && tmp2[0..5] == "public") continue;
            if (tmp2 && tmp2[0..6] == "private" && true_name == tmp) continue;
            if (tmp2 && tmp2[0..6] == "private"
             && true_name != tmp && !isMember(true_name, "admin"))
                return 0;
        }
        if (query_access(file, name, P_READ)) continue;

        return 0;
    }

    return 1;
}

int valid_write(string file, object user, string func)
{
    string name, tmp, tmp2;

    if (this_interactive() && query_privs(user) != "[daemon]")
        name = query_privs(this_interactive());
    else name = query_privs(user);

    if (!name) name = "<noname>";
    if (user == this_object() || user == master()) return 1;
    if (strlen(file) > strlen(user_data_directory(name)))
        if (file[0..(strlen(user_data_directory(name))-1)] == user_data_directory(name))
            return 1;

    if (file && sscanf(file, "/home/%*s/%s/%s", tmp, tmp2))
    {
        if (name == tmp || name == "[home_" + tmp + "]") return 1;
        if (tmp2 && tmp2[0..6] == "public/" && tmp2 != "public/") return 1;
        if (tmp2 && tmp2[0..6] == "private" && name == tmp) return 1;
    }

    DEBUG("Debug [valid_write]: File: " + file + " Name: " + name + "\n");

    if (query_access(file, name, P_WRITE)) return 1;

    return 0;
}

int query_access(string directory, string id, int type)
{
    mapping data;
    string array permissions;

    DEBUG(
      "Debug [security]: Permission query for '" + id +
      "' in '" + directory + "'.\n"
    );
    data = Access[directory];

    if (!mapp(data))
    {
        string array exp;
        int sz_exp, i;

        DEBUG(
          "Debug [query_access]: " + directory +
          " not in mapping. Searching for parent permissions.\n"
        );
        exp = explode(directory, "/");
        sz_exp = sizeof(exp);

        while (!Access[directory] && sz_exp--)
        {
            directory = "/";

            for (i = 0 ; i < sz_exp ; i ++)
                directory += exp[i] + "/";
        }

        if (!Access[directory]) directory = "/";
    }

    if (!Access[directory]) return 0;
    else data = Access[directory];

    DEBUG(
      "Debug [query_access]: Final directory set to '" + directory + "'.\n"
    );
    permissions = data[id];

    if (!permissions || !arrayp(permissions) || sizeof(permissions) < 1)
        permissions = track_member(id, directory);

    if (sizeof(permissions) < 1)
    {
        DEBUG(
          "Debug [security]: Using defalt permissions for '" + id +
          "' in '" + directory + "'.\n"
        );
        permissions = data["(all)"];
    }


    if (sizeof(permissions) < 1 || !arrayp(permissions))
    {
        DEBUG(
          "Debug [security]: No permissions found for '" + id +
          "' in '" + directory + "'.\n"
        );
        return 0;
    }

    switch (type)
    {
        /* read, write, network, shadow, link, execute, bind, ownership */
        case P_READ:
            if (member_array("r", permissions) != -1)
            {
                DEBUG(
                  "Debug [query_access]: Permission granted to read for " +
                  directory + "\n"
                );
                return 1;
            }
            break;
        case P_WRITE:
            if (member_array("w", permissions) != -1)
            {
                DEBUG(
                  "Debug [query_access]: Permission granted to write for " +
                  directory + "\n"
                );
                return 1;
            }
            break;
        case P_NETWORK:
            if (member_array("n", permissions) != -1)
            {
                DEBUG(
                  "Debug [query_access]: Permission granted to access network for " +
                  directory + "\n"
                );
                return 1;
            }
            break;
        case P_SHADOW:
            if (member_array("s", permissions) != -1)
            {
                DEBUG(
                  "Debug [query_access]: Permission granted to shadow for " +
                  directory + "\n"
                );
                return 1;
            }
            break;
        case P_LINK:
            if (member_array("l", permissions) != -1)
            {
                DEBUG(
                  "Debug [query_access]: Permission granted to link for " +
                  directory + "\n"
                );
                return 1;
            }
            break;
        case P_EXECUTE:
            if (member_array("e", permissions) != -1)
            {
                DEBUG(
                  "Debug [query_access]: Permission granted to execute for " +
                  directory + "\n"
                );
                return 1;
            }
            break;
        case P_BIND:
            if (member_array("b", permissions) != -1)
            {
                DEBUG(
                  "Debug [query_access]: Permission granted to bind for " +
                  directory + "\n"
                );
                return 1;
            }
            break;
        case P_OWNERSHIP:
            if (member_array("o", permissions) != -1)
            {
                DEBUG(
                  "Debug [query_access]: Permission granted as ownership for " +
                  directory + "\n"
                );
                return 1;
            }
            break;
    }

    DEBUG("Debug [query_access]: Permission denied (" + type + ").\n");
    return 0;
}

string array track_member(string id, string directory)
{
    mapping data = Access[directory];
    string array ks = keys(data);
    string array groupData;
    int i, sz_ks = sizeof(ks);

    DEBUG(
      "Debug [security]: Tracking member '" + id +
      "' for '" + directory + "'.\n"
    );

    for (i = 0 ; i < sz_ks ; i++)
    {
        groupData = query_group(ks[i]);

        if (!arrayp(groupData) || sizeof(groupData) < 1) continue;
        if (member_array(id, groupData) != -1) return data[ks[i]];
    }

    return ({ });
}

string array query_group(string group)
{
    if (!Groups[group]) sscanf(group, "(%s)", group);

    return Groups[group];
}

int isMember(string user, string group)
{
    string array data;
    int i;

    if (!user || !group) return 0;

    data = Groups[group];

    if (!arrayp(data)) return 0;

    for (i = 0 ; i < sizeof(data) ; i++)
    {
        string group_name;

        if (!stringp(data[i])) continue;
        if (sscanf(data[i], "(%s)", group_name))
            data += Groups[group_name];
    }

    if (member_array(user, data) != -1) return 1;
    else return 0;
}
