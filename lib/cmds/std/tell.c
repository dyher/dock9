/* tell.c

   Gwegster @ LPUniversity
   30-JUNE-05
   a tell command

   Last edited on July 8th, 2007 by Tricky @ RtH

*/

#include <intermud3.h>

int main(string str)
{
    string who, message, where;
    object user;

    if(!stringp(str)) return notify_fail("Syntax: tell <player>[@<mud>: | . | :]<message>\n");

    if(!sscanf(str, "%s@%s: %s", who, where, message))
    {

        if(sscanf(str, ". %s", message))
        {
            who = this_player()->query("retell");
            if(!who) return notify_fail("Error [tell]: No Previous tells sent.\n");
            sscanf(who, "%s@%s", who, where);
        }
        else if(!sscanf(str, "%s %s", who, message))
            return(notify_fail("Usage: tell <player> <message>\n"));

    }

    if(!message)
        return(notify_fail("Incorrect syntax, please check the help.\n"));

    if(where && devp(this_player()))
    {
        object I3, I3_mudlist, I3_tell, I3_emote;
        mapping mudlist;
        string array muds = ({ });

        I3 = find_object(I3_CHDMOD);
        I3_mudlist = find_object(I3_MUDLIST);
        I3_tell = find_object(I3_TELL);
        I3_emote = find_object(I3_EMOTE);

        if(!objectp(I3))
            return(notify_fail("The Intermud 3 module is currently not loaded. Please try again later.\n"));

        if(!objectp(I3_mudlist))
            return(notify_fail("The Intermud 3 'mudlist' module is currently not loaded. Please try again later.\n"));

        if(!(mudlist = I3_mudlist->get_mud_list())) return notify_fail("Mudlist unavailable.\n");

        foreach(string name in keys(mudlist))
        {
            float fuzz = fuzzymatch(lower_case(name), lower_case(where));

            if(fuzz == 100.0)
            {
                muds = ({ name });
                break;
            }
            else if(fuzz > 50.0) muds += ({ name });

        }

        switch(sizeof(muds))
        {
            case 0: return(notify_fail("Target mud does not exist.\n"));
            case 1: break;
            default:
            {
                write("Target mud is ambiguous. Which do you mean?\n");

                foreach(string name in muds) write(" * '" + name + "'\n");

                return 1;
            }
        }

        where = muds[0];

        if(!objectp(I3_emote))
            return(notify_fail("The Intermud 3 'emoteto' module is currently not loaded. Please try again later.\n"));

        if(!I3_emote->isSmiley(message) && message[0] == ':')
        {
            I3_emote->send_emoteto(this_player(), who, where, message[1..<1]);
            write(CYN + "You emote to " + capitalize(who) + "@" + where + ": " 
                + NOR + capitalize(this_player()->query_name()) 
                + " " + message[1..<1] + "\n");
        }
        else if(I3_emote->isSmiley(message))
        {
            I3_emote->send_emoteto(this_player(), who, where, message);
            write(CYN + "You emote to " + capitalize(who) + "@" + where + ": "
                + NOR + capitalize(this_player()->query_name())
                + " " + "\n");
        }
        else
        {

            if(!objectp(I3_tell))
                return(notify_fail("The Intermud 3 'tell' module is currently not loaded. Please try again later.\n"));

            I3_tell->send_tell(this_player(), who, where, message);
            write(CYN + "You tell " + capitalize(who) + "@" + where 
                + ": " + NOR + message + "\n");
        }

        this_player()->set("retell", who + "@" + where);

        return 1;
    }
    else if(where && !devp(this_player()))
        return 0;

    user = find_living(lower_case(who));
    if (user && environment(user) != environment(this_player())) user = 0;
    if (!user) user = find_player(lower_case(who));
    
    if(!objectp(user)) return notify_fail("Error [tell]: User " + who 
        + " is not found.\n");

    if(user->query_env("away"))
    {
        write("That user is currently away" + (user->query_env("away") != "" ?
            ": (" + user->query_env("away") + ")\n" : ".\n"));

        return 1;
    }

    if(user == this_player())
    {

        if(message[0] == ':')
            write(CYN + "You emote to yourself: " + NOR + user->query_cap_name() 
                + " " + message[1..<1] + "\n" + NOR);
        else
            write(CYN + "You tell yourself: " + NOR + message + "\n" + NOR);

        tell_room(environment(this_player()), 
            capitalize(this_player()->query_name()) 
            + " starts talking to themselves.\n" + NOR, this_player());
        this_player()->set("retell", query_privs(user));

        return 1;
    }

    if(message[0] == ':')
    {
        tell_object(user, CYN + this_player()->query_cap_name() + " " + NOR 
            + message[1..<1] + "\n" + NOR);
        write(CYN + "You emote to " + capitalize(who) + ": " + NOR 
            + this_player()->query_cap_name() + " " + message[1..<1] 
            + "\n" + NOR);
    }
    
    else
    {
        tell_object(user, CYN + capitalize((string)this_player()->query_name()) 
            + " tells you: " + NOR + message + "\n" + NOR);
        write(CYN + "You tell " + capitalize(who) + ": " + NOR + message 
            + "\n" + NOR);
    }

    user->set("reply", query_privs(this_player()));
    this_player()->set("retell", query_privs(user));

    return 1;
}

string help()
{
    return(HIW + " SYNTAX:" + NOR + " tell <player>[@<mud>: | . | :]<message>\n\n"
      "This command will send a message to the specified player if they\n"
      "are online. For example, if you type 'tell tacitus hey' then\n"
      "he'll see '" + CYN + capitalize(this_player()->query_name()) +
      " tells you:" + NOR + " hey'. If you use 'tell . <message>\n" +
      "it will send the message to the last person that you used tell to talk to.\n" +
      "You may also talk to a user on another mud in the I3 network using the\n" +
      "'tell <user>@<mud>: <message>' syntax. You may also emote to another playter\n" +
      "using the tell <user> :<message> syntax.\n\n" +
      HIW + "See also: " + NOR + "say, channel\n");
}


