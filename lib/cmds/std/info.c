/* /cmds/std/info.c

    emsenn@Dock 9
    091208
    Command to view basic information about a room

*/
string help();

int main(string args)
{
    int i;
    object targetob, *roominv, *playerinv;
    playerinv = all_inventory(this_player());
    roominv = all_inventory(environment(this_player()));

    if(!args) return(notify_fail("SYNTAX: info <thing>\n"));
    if(args == ("here"||"room")) targetob = environment(this_player());
    if(args == ("me"||"self")) targetob = this_player();
    else{
        for(i=0;i < sizeof(roominv); i++) if(roominv[i]->id(args))
          targetob = roominv[i];
        for(i=0;i < sizeof(playerinv); i++) if(playerinv[i]->id(args))
          targetob = playerinv[i];
    }
    if(targetob){
        write(
          "Path: "+file_name(targetob)+"\n"
          "Launchpad: http://bazaar.launchpad.net/~dock9/dock9/trunk/annotate/head%3A/lib"
             +file_name(targetob)+".c\n" //This should try to see the extension, not assume .c
          "Short name: "+targetob->query_short()+"\n"
          "Heartbeat "+query_heart_beat(targetob)+"\n"
        );
        return 1;
    }
    else return(notify_fail("SYNTAX: info <thing>\n\n"+args+" is not a valid thing.\n"));
}

string help()
{
    return
    HIW + " SYNTAX:" + NOR + " info [thing]\n\n" +
    "This command shows you some basic information about the argument, if "
    "available.\n\nExamples:\n\tinfo here\n\tinfo me\n\tinfo sailor";
}


