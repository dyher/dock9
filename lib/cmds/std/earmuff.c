/* earmuff.c
 *
 * Tricky @ Rock the Halo
 * 23-MAY-08
 * Earmuff command
 *
 */

int main(string args)
{
    if (!stringp(args) || args == "")
    {
        string array victims = this_user()->query_earmuffed();

        if (sizeof(victims) == 1)
            printf("You are ignoring the following:\n%-:8s%-=69s\n",
              "",
              victims[0]
            );
        else if (sizeof(victims) > 1)
        {
            printf("You are ignoring the following:\n%-:8s%-=69s\n",
              "",
              implode(victims[0..<2], ", ") + " and " + victims[<1]
            );
        }
        else
            write("You are ignoring no-one.\n");
    }
    else
    {
        args = trim(args);

        while (stringp(args) && args != "")
        {
            string arg1 = "", argn = "";

            if (sscanf(args, "\"%s\"%s", arg1, argn) == 0)
                if (sscanf(args, "%s %s", arg1, argn) == 0)
                    sscanf(args, "%s", arg1);

            this_user()->add_earmuffed(arg1);
            write("Ignoring: " + arg1 + "\n");
            args = trim(argn);
        }
    }

    return 1;
} 

string help()
{
    string str = "";

    if (!devp(this_user()))
    {
        str += @HELP
%^BOLD%^SYNTAX:%^RESET%^ earmuff [name]

This command allows you to ignore channel messages from the name specified.

NB: If a name has spaces in it then place it in quotes.
HELP;
    }
    else
    {
        str += @HELP
%^BOLD%^SYNTAX:%^RESET%^ earmuff [name | name@mud]

This command allows you to ignore channel messages from the name specified.
You can also earmuff all channel messages coming from a user at a specific
mud, for example: earmuff "luser@Spammy Mud II"

NB: If a name has spaces in it then place it in quotes.
HELP;
    }

    str += @END

    See also: unmuff
END;

    return str;
}
