/* finger.c
 *
 * Tacitus @ LPUniversity
 * 26-MAR-06
 * Std Command
 */

#include <intermud3.h>

int main(string user)
{
    if (!user) return(notify_fail("Syntax: finger <user>\n"));
    if (sscanf(user, "%*s@%*s") == 2)
        return 0;

    write(FINGER_D->finger_user(user));
    return 1;
}

string help()
{
    string ret;

    ret = HIW + " SYNTAX:" + NOR + " finger <user>\n" + @HELP

This command will display information regarding an existing user such as their
e-mail address, last login rank, plan, etc. This command will be able to
retrieve the information even if they are offline.
HELP;

    return ret;
}


