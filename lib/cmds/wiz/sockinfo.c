/* sockinfo.c

 Tacitus @ LPUniversity
 05-MAY-06
 Socket info command
 
*/

#include <mudlib.h>

int main() 
{
    write(dump_socket_status());
    write("\n");

    if (find_object(SOCKET_D))
    {
        mapping sockets = find_object(SOCKET_D)->query_socket_info();

        foreach (string sockID, mapping sock in sockets)
        {
            printf("%s:\n", sockID);
            printf(" * fd:      %d\n", sock["fd"]);
            printf(" * type:    %s\n", sock["type"]);
            printf(" * owner:   %O\n", sock["owner"]);
            printf(" * logFile: %s\n", sock["logFile"]);
        }
    }

    write("\n");
    return 1; 
}

string help() {
    return(" %^BOLD%^SYNTAX:%^RESET%^ sockinfo\n\n"
      "The sockinfo command will tell you what sockets are loaded\n"
      "and what they are currently being used for.\n");
}
