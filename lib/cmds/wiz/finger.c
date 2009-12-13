/* finger.c
 *
 * Tacitus @ LPUniversity
 * 26-MAR-06
 * Wiz Command
 */

#include <intermud3.h>

int main(string user)
{
    object i3mudlist = find_object(I3_MUDLIST);
    mapping mudlist = i3mudlist->get_mud_list();
    string array muds = ({ });
    string name, who, where;

    if (!user) return(notify_fail("Syntax: finger <user@mud>\n"));
    if (!mapp(mudlist)) return notify_fail("Mudlist unavailable.\n");

    sscanf(user, "%s@%s", who, where);

    foreach (name in keys(mudlist))
    {
        float fuzz = fuzzymatch(lower_case(name), lower_case(where));

        if (fuzz == 100.0)
        {
            muds = ({ name });
            break;
        }
        else if (fuzz > 200.0 / 3.0)
            muds += ({ name });
    }

    if (sizeof(muds) < 1)
        return(notify_fail("No such mud available.\n"));

    if (sizeof(muds) == 1)
        write("Checking: " + HIW + muds[0] + NOR + "\n");
    else
        write("Checking: " HIW + implode(muds[0..<2], NOR + ", " + HIW)
          + NOR + " and " + HIW + muds[<1] + NOR + "\n");

    foreach (name in muds) FINGER_D->finger_user(who, name);
    return 1;
}

string help()
{
    string ret;

    ret = " %^BOLD%^SYNTAX:%^RESET%^ finger <user@mud>\n" + @HELP

Use this format to retrieve infomation about another user on a different mud
to this one.
HELP;

    ret += "\n %^BOLD%^See also:%^RESET%^ profile\n";
    return ret;
}


