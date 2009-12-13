/* help.c

* Tacitus @ LPUniversity
* 08-OCT-05
* Standard Command

* -> First attempt at a help system <-

*/

//Last Edited on October 6th, 2006 by Tacitus

#define HELP_PATH ({"/doc/general/", "/doc/game/"})
#define DEV_PATH ({"/doc/wiz/", "/doc/driver/efun/all/", "/doc/driver/applies/", "/doc/driver/lpc/constructs/", "/doc/driver/lpc/types/",})
#define BORDER HIW + "[=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=]" + NOR + "\n"

#include <logs.h>

int main(string str)
{
    string file, *path, err, output = "";
    object cmd, pager, *roominv, *playerinv;
    int i;

    pager = clone_object(OBJ_PAGER);
    if(!str) str = "help";
    path = this_player()->query_path();
    if(str == "here"){
        if(!environment(this_player())->help()){
            notify_fail("Error [help]: There is no help available for this "
              "room.\n\n");
            return 0;
        }else{
            output += BORDER+"\t%^BOLD%^YELLOW%^Help file for room "
              "%^RESET%^MAGENTA%^"+file_name(environment(this_player()))+
              "%^RESET%^BOLD%^YELLOW%^\n"+BORDER+"\n";
            output += (environment(this_player())->help() + "\n");
            pager->page(output);
            return 1;
        }
    }
    roominv = all_inventory(environment(this_player()));
    for(i=0;i < sizeof(roominv); i++){
        if(roominv[i]->id(str)){
            if(roominv[i]->help()){
                output += BORDER+"\t%^BOLD%^YELLOW%^Help file for object "
                  "%^RESET%^MAGENTA%^"+file_name(roominv[i])+
                  "%^RESET%^BOLD%^YELLOW%^\n"+BORDER+"\n";
                output += (roominv[i]->help()+"\n");
                pager->page(output);
                return 1;
            }
            else{
                notify_fail("Error [help]: Object \""+roominv[i]->query_short()+
                  "\" does not have any help documenation.");
                return 0;
            }
        }
    }
    playerinv = all_inventory(this_player());
    for(i=0;i < sizeof(playerinv); i++){
        if(playerinv[i]->id(str)){
            if(playerinv[i]->help()){
                output += BORDER+"\t%^BOLD%^YELLOW%^Help file for object "
                  "%^RESET%^MAGENTA%^"+file_name(playerinv[i])+
                  "%^RESET%^BOLD%^YELLOW%^\n"+BORDER+"\n";
                output += (playerinv[i]->help()+"\n");
                pager->page(output);
                return 1;
            }
            else{
                notify_fail("Error [help]: Object \""+roominv[i]->query_short()+
                  "\" does not have any help documenation.\n");
                return 0;
            }
        }
    }
    for(i = 0; i < sizeof(path); i++){        
        if(file_exists(path[i] + str + ".c")){
            err = catch(cmd = load_object(path[i] + str));
            if(!err) file = cmd->help();
            if(err) return notify_fail("Error [help]: This is a problem with "
            + str + "\n Please inform an admin.\n\n");
            if(!file) return notify_fail("Error [help]: The command " + str +
              " exists but there is no help file for it.\n"
              " Please inform an admin.\n\n");
            output += BORDER;
            output += ("\t\t %^BOLD%^YELLOW%^ Help file for command '%^RESET%^MAGENTA%^"
              +  capitalize(str) + "%^RESET%^BOLD%^YELLOW%^'\n");
            output += BORDER + "\n";
            output += (file + "\n");
            pager->page(output);
            return 1;
        }
    }
    path = HELP_PATH;
    if(devp(this_player())) path += DEV_PATH;
    if(adminp(this_player())) path += ({"/doc/admin/"});
    for(i = 0; i < sizeof(path); i++){
        if(file_exists(path[i] + str)){
            file = read_file(path[i] + str);
            output += BORDER;
            output += ("\t\t %^BOLD%^YELLOW%^ Help file for topic '%^RESET%^MAGENTA%^"
              +  capitalize(str) + "%^RESET%^BOLD%^YELLOW%^'\n");
            output += BORDER + "\n";
            output += (file + "\n");
            pager->page(output);
            return 1;
        }
    }
    log_file(LOG_HELP, "Not found: " + str + "\n");
    return notify_fail("Error [help]: Unable to find helpfile for: " + str + "\n");
}

string help(){
     return(HIW + " SYNTAX: " + NOR + "help <topic>\n\n" +
     "Whenever you need help or information regarding something in\n"
     "the mud, this is the place come. This command gives you instant\n"
     "access to a wealth of information that will be vital to your\n"
     "stay here on " + mud_name() + ". Help that you want not\n"
     "written yet? Let us know and we'll get right on it!\n");
}




