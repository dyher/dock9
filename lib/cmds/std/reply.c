/* reply.c

 Tacitus @ LPUniversity
 05-MAY-06
 Std reply command

 Last Updated: January 12th, 2007 by Tricky @ RtH
 
*/

#include <intermud3.h>

int main(string message)
{
    object user;
    string d_user, d_mud;
    string who = this_player()->query("reply");

    if(!message) return(notify_fail("Error [tell]: You must provide an argument. Syntax: reply <message>\n"));

    /* Reply to I3 user */
    if(sscanf(who, "%s@%s", d_user, d_mud))
    {
        object I3, I3_tell, I3_emote;

        I3 = find_object(I3_CHDMOD);
        I3_tell = find_object(I3_TELL);
        I3_emote = find_object(I3_EMOTE);

        if(!objectp(I3))
            return(notify_fail("The Intermud 3 module is currently not loaded. Please try again later.\n"));

        if(!objectp(I3_emote))
            return(notify_fail("The Intermud 3 'emoteto' module is currently not loaded. Please try again later.\n"));

        if(lower_case(d_user) == lower_case(query_privs(this_player())) &&
           lower_case(d_mud) == lower_case(mud_name()))
        {

            if(I3_emote->isSmiley(message) == 0 && message[0] == ':')
                message = message[1..];

            write(CYN + "You reply to yourself: " + NOR + message + "\n" + NOR);
            tell_room(environment(this_player()), this_player()->query_cap_name() + " starts talking to themselves.\n" + NOR, this_player());

            return 1;
        }

        if(!I3_emote->isSmiley(message) && message[0] == ':')
        {
            I3_emote->send_emoteto(this_player(), d_user, d_mud, " " + message[1..]);
            write(CYN + "You reply to " + capitalize(d_user) + "@" + d_mud + ": " + NOR + this_player()->query_cap_name() + " " + message[1..] + "\n" + NOR);
        }
        else
        {

            if(!objectp(I3_tell))
                return(notify_fail("The Intermud 3 'tell' module is currently not loaded. Please try again later.\n"));

            I3_tell->send_tell(this_player(), d_user, d_mud, message);
            write(CYN + "You reply to " + capitalize(d_user) + "@" + d_mud + ": " + NOR + message + "\n" + NOR);
        }

        return 1;
    }

    user = find_player(who);
    
    if(!objectp(user)) return notify_fail("Error [tell]: User " + who + " is not found.\n");
    
    if(user == this_player())
    {

        if(message[0] == ':') message = message[1..];

        write(CYN + "You reply to yourself: " + NOR + message + "\n" + NOR);
        tell_room(environment(this_player()), this_player()->query_cap_name() + " starts talking to themselves.\n" + NOR, this_player());

        return 1;
    }

    if(message[0] == ':')
    {
        tell_object(user, CYN + this_player()->query_cap_name() + " " + message[1..] + "\n" + NOR);
        write(CYN + "You reply to " + capitalize(who) + ": " + NOR + this_player()->query_cap_name() + " " + message[1..] + "\n" + NOR);
    }
    else
    {
        tell_object(user, CYN + this_player()->query_cap_name() + " tells you: " + NOR + message + "\n" + NOR);
        write(CYN + "You reply to " + capitalize(who) + ": " + NOR + message + "\n" + NOR);
    }

    user->set("reply", query_privs(this_player()));

    return 1;
}

string help()
{
    return(HIW + " SYNTAX:" + NOR + " reply <message>\n\n"
    "This command will send a message to the last user that sent\n"
    "you a message if they are still online.\n\n"
    HIW + "See also: " + NOR + "say, channel\n");
}
