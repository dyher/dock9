 /* snoop.c

  Tricky
  09-MAY-2007
  Standard snoop command

 */

int main(string user)
{
    object target;

    if(!adminp(previous_object())) return notify_fail("Error [snoop]: Access denied.\n");

    if(!user)
    {

        if(!query_snoop(this_user()) && !query_snooping(this_user()))
            write("You are not snooping anyone or being snooped by anyone.\n");
        else
        {

            if(query_snoop(this_user()))
                write("You are being snooped by: '" + query_snoop(this_user())->query_cap_name() + "'\n");

            if(query_snooping(this_user()))
                write("You are snooping: '" + query_snooping(this_user())->query_cap_name() + "'\n");

        }

        return 1;
    }

    target = find_player(lower_case(user));

    if(!objectp(target) || !interactive(target))
    {
        write("Error [snoop]: User '" + user + "' is not on the mud presently.\n");
        return 1;
    }

    if(query_snooping(this_user()) == target)
    {

        if(snoop(this_user()) != this_user())
            write("Error [snoop]: Could not un-snoop user '" + user + "'\n");
        else
            write("You stop snooping '" + user + "'\n");

    }
    else
    {

        if(snoop(this_user(), target) != target)
            write("Error [snoop]: Could not snoop user '" + user + "'\n");
        else
            write("You start snooping '" + user + "'\n");

    }

    return 1;
}

string help()
{
    return (
      HIW + " SYNTAX: " + NOR + "snoop [username]\n\n" +
      "This command will allow you to snoop on the actions of others.\n\n"
      "Type 'snoop username' to start snooping that user.\n"
      "Type 'snoop username' again to stop snooping that user.\n"
      "Type 'snoop' on its own for info about who you are snooping or who\n"
      "is snooping you.\n"
    );
}
