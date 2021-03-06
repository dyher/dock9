/* who.c

 Tacitus @ LPUniversity
 06-FEB-07
 Standard command
 
*/

string renderTag(string rank);
string array sortRankIndex(string array ranks);
string make_wholist();

int main(string arg)
{
    string mudname, who_list;

    if (arg && sscanf(arg, "@%s", mudname) == 1)
    {
        "/adm/daemons/chmodules/i3.c"->send_packet(
            ({ "who-req", 5, mud_name(), this_player()->query_name(), mudname, 0 })
        );
        return 1;
    }

    who_list = make_wholist();
    printf("%s\n", who_list);

    return 1;
}

string make_wholist()
{
    string output;
    
    output =     
    "\tDock 9\n\n";
    
    output += sprintf("%10s  %10s\n\n", "Username [* editing, + in input]", "Idle");
    
    foreach(string rank in sortRankIndex(RANK_D->getRanks()))
    {
        foreach(object user in RANK_D->getInteractives(rank))
        {
            output += sprintf(" %-s   %-15s %15s\n", renderTag(rank),
        	  capitalize(user->query_name()) +
        	  (user->query_env("away") ? " (afk)" : "") +
        	  (in_edit( user ) ? "*" : "") +
        	  (in_input( user ) && !user->query_env("away") ? "+" : ""),
        	  query_idle( user ) / 60 + "m");
        }
    }
    
    return output;
}

string renderTag(string rank)
{
    string tag;
    
    tag = "[ " + capitalize(RANK_D->queryRankProperty(rank, "short"));
    
    while(sizeof(tag) < 10)
    {
        if(sizeof(tag) == 8) tag += "%^RESET%^]";
        else tag += " ";
    }
    
    tag = replace_string(tag, "[", "[" + RANK_D->queryRankProperty(rank,
        "colour"));
        
    return tag;
}

string array sortRankIndex(string array ranks)
{
    string array returnValue = ({});
    int i = sizeof(ranks);
    
    while(i--) returnValue += ({ ranks[i] });
        
    return returnValue;    
}

string help()
{
    return(HIW + " SYNTAX: " + NOR + "who [@<mud>]\n\n" +
      "This command will display all the users who are currently logged\n" +
      "into " + mud_name() + ". It also lets you know if they are currently\n" +
      "editing, in input, and/or idle. You may also find out who is on a mud\n" +
      "on the I3 network by using the 'who @<mud>' syntax\n");
}
