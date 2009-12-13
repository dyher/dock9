/* mudinfo.c

 Tricky @ RtH
 July 22nd, 2006
 List information about a mud on the I3 network

 Last Updated: March 29th, 2007 by Tricky @ RtH

*/

#include <intermud3.h>

int main(string str)
{
    object i3mudlist;
    mapping mudlist;
    mixed *data;
    string *muds = ({ });
    string list = "", name;

    i3mudlist = find_object(I3_MUDLIST);

    if(!(mudlist = i3mudlist->get_mud_list())) return notify_fail("Mudlist unavailable.\n");

    if(!str || str == "") return notify_fail("Usage: mudinfo <mudname>\n");

    sscanf(str, "\"%s\"", str);

    foreach(name in keys(mudlist))
    {
        float fuzz = fuzzymatch(lower_case(name), lower_case(str));

        if(fuzz == 100.0)
        {
            muds = ({ name });
            break;
        }
        else
        if(fuzz > 50.0)
        {
            muds += ({ name });
        }

    }

    foreach(name in muds)
    {
        string *services = ({ }), *others = ({ });

        list = sprintf("Intermud 3 Information for %s\n%78'='s\n", name, "");

        data = mudlist[name];

        if(data[0] == -1)
            list += sprintf("%-:25s %s is UP\n", "State:", name);

        if(data[0] == 0)
            list += sprintf("%-:25s %s is DOWN\n", "State:", name);

        if(data[0] > 0)
            list += sprintf("%-:25s %s is REBOOTING in %i second%s\n", "State:", name, data[0], data[0] == 1 ? "" : "s");

        list += sprintf("%-:25s %s (%i)\n", "IP Address and port:", data[1], data[2]);

        list += sprintf("%-:25s %s\n", "Mudlib:", data[5]);

        list += sprintf("%-:25s %s\n", "Base Mudlib:", data[6]);

        list += sprintf("%-:25s %s\n", "Driver:", data[7]);

        list += sprintf("%-:25s %s\n", "Mud Type:", data[8]);

        list += sprintf("%-:25s %s\n", "Status:", data[9]);

        list += sprintf("%-:25s %s\n", "Admin Email:", data[10]);

        if(mapp(data[11])) services = keys(data[11]);
        if(mapp(data[12])) others = keys(data[12]);

        if(sizeof(services) == 0)
        {
            list += sprintf("%-:25s %-=50s\n", "Services:", "None");
        }
        else
        if(sizeof(services) == 1)
        {
            string service = services[0];

            if(data[11][service] == 1)
                list += sprintf("%-:25s %-=50s\n", "Services:", service);
            else
                list += sprintf("%-:25s %-=50s(%s)\n", "Services:", service, "" + data[11][service]);

        }
        else
        {
            string services_info = "";

            services = sort_array(filter(services, (: stringp($1) :) ), 1);

            foreach(string service in services[0..<2])
                services_info += service + (data[11][service] == 1 ? "" : "(" + data[11][service] + ")") + ", ";

            services_info  = services_info[0..<3] + " ";
            services_info += "and " + services[<1] + (data[11][services[<1]] == 1 ? "" : "(" + data[11][services[<1]] + ")");

            list += sprintf("%-:25s %-=50s\n", "Services:", services_info);
        }

        if(sizeof(others) == 0)
        {
            list += sprintf("%-:25s %-=50s\n", "Other Services:", "None");
        }
        else
        if(sizeof(others) >= 1)
        {

            if(sizeof(others) == 1)
            {
                list += sprintf("%-:25s %-=50s\n", "Other Services:", others[0]);
            }
            else
            {
                list += sprintf("%-:25s %-=50s\n",
                  "Other Services:",
                  implode(others[0..<2], ", ") + " and " + others[<1]
                );
            }

            list += "\n";

            foreach(string other in others)
                list += sprintf("%s%s%s\n",
                  other,
                  other == "banner" ? "\n" : ": ",
                  stringp(data[12][other]) ? data[12][other] : sprintf("%O", data[12][other])
                );

        }

        foreach(string line in explode(list, "\n")) write(line + "\n");

        write("\n");
    }

    return 1;
}

string help()
{
    return(HIW + " SYNTAX: " + NOR + "mudinfo <mudname>\n"
    "Lists information about a mud connected to the Intermud 3 network.\n");
}
