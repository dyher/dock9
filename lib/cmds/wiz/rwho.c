/* rwho.c

 Tricky @ RtH
 July 22nd, 2006
 Lists rwho information about an active mud on the I3 network

 Last edited on July 8th, 2007 by Tricky @ RtH

*/

#include <mudlib.h>
#include <intermud3.h>

object i3, i3who;

int main(string str)
{
    i3 = find_object(I3_CHDMOD);
    i3who = find_object(I3_WHO);

    if(!objectp(i3))
        return(notify_fail("The Intermud 3 module is currently not loaded. Please try again later.\n"));

    if(!objectp(i3who))
        return(notify_fail("The Intermud 3 'who' module is currently not loaded. Please try again later.\n"));

    if(!str || str == "")
        return(notify_fail("You must specify a flag and/or a partial or full mudname.\n"));

    if(str == "-all")
    {

        if(adminp(this_player()))
            i3who->remote_who(this_player(), (string)0);
        else
            return(notify_fail("You do not have enough priveledges to send a remote who to all muds.\n"));

    }
    else
    {
        mapping mudlist = i3->get_mudlist();
        string array mudlist_arr;
        string array muds = ({ });
        string flag = "", search = "";

        if(!mudlist) return notify_fail("Mudlist unavailable.\n");

        mudlist_arr = keys(mudlist);

        if(sscanf(str, "-%s %s", flag, search) == 0)
            search = str;

        sscanf(search, "\"%s\"", search);

        search = lower_case(search);

        foreach(string name in mudlist_arr)
        {
            string fuzz_name;
            float fuzz;

            switch(flag)
            {
                case "l": fuzz_name = mudlist[name][5]; break;
                case "bl": fuzz_name = mudlist[name][6]; break;
                default: fuzz_name = name; break;
            }

            fuzz = fuzzymatch(lower_case(fuzz_name), search);

            // write("-" + flag + ": " + name + "(" + fuzz_name + ") == " + fuzz + "\n");

            if(fuzz == 100.0 && fuzz_name == name)
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

        if(sizeof(muds) < 1)
            return(notify_fail("No such mud available.\n"));

        if(sizeof(muds) == 1)
            write("Checking: %^BOLD%^" + muds[0] + "%^RESET%^\n");
        else
            write("Checking: %^BOLD%^" + implode(muds[0..<2], "%^RESET%^, %^BOLD%^") + "%^RESET%^ and %^BOLD%^" + muds[<1] + "%^RESET%^\n");

        foreach(string name in muds)
            if(mudlist[name][0] == -1)
                i3who->remote_who(this_player(), name);

    }

    return 1;
}

string help()
{
    return(HIW + " SYNTAX: " + NOR + "rwho [-l|-bl] [name]\n"
      "Lists rwho information about an active mud on the I3 network.\n");
}
