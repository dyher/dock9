/* people.c

 Abbot @ LPUniversity
 14-JULY-2006
 Developer People Command

*/

/*  Last editted by Tacitus on October 4 */

#define WRAP_DEFAULT 80

string renderTag(string rank, string colour);
string array sortRankIndex(object array ranks);

int main()
{
    mapping sorted_users = ([ ]);
    object array tmp_users;
    int ctr = 0, wrap;
    int ip_len, loc_len;

    if (this_user()->query_env("wrap"))
        wrap = to_int(this_user()->query_env("wrap"));
    else if (this_user()->query("columns"))
        wrap = to_int(this_user()->query("columns"));
    else wrap = WRAP_DEFAULT;

    if (wrap > 100) wrap -= 10;

    wrap -= 26;
    wrap /= 2;

    ip_len = wrap - 4;
    loc_len = wrap + 4;

    printf("%-12s%-10s%-" + ip_len + "s%-" + loc_len + "s%-4s\n", "Name", "Rank", "IP", "Location", "Idle");
    printf("%-12s%-10s%-" + ip_len + "s%-" + loc_len + "s%-4s\n", "----", "----", "--", "--------", "----");

    foreach(string rank in sortRankIndex(RANK_D->getRanks()))
    {
        tmp_users = sort_array(
          RANK_D->getInteractives(rank),
          (: strcmp($1->query_name(), $2->query_name()) :)
        );

        foreach (object u in tmp_users)
        {
            if (!environment(u)) continue;

            sorted_users += ([
              ctr++: ([
                "object": u,
                "name": u->query_name(),
                "colour": RANK_D->queryRankProperty(rank, "colour"),
                "rank": RANK_D->queryRankProperty(rank, "short"),
              ]),
            ]);
        }

    }

    for(int i = 0; i < ctr; i++)
    {
    	object thisUser;
    	string thisName, thisRank, thisIP, thisEnv, thisIdle;
    	string a, b;
        int idleTime, weeks, days, hours, mins;

        thisUser = sorted_users[i]["object"];
        thisName = sorted_users[i]["name"];
        thisRank = renderTag(sorted_users[i]["rank"], sorted_users[i]["colour"]);
        thisIP   = query_ip_name(thisUser);

        if (!stringp(thisIP)) thisIP = "<none>";

        if (strlen(thisIP) >= ip_len)
        {
            string array parts = explode(thisIP, ".");
            string tmp_thisIP;
            int ptr = sizeof(parts) - 2;

            tmp_thisIP = implode(parts[0..ptr], ".") + "..." + parts[<1];

            while(strlen(tmp_thisIP) >= ip_len && ptr > 0)
            {
                ptr--;
                tmp_thisIP = implode(parts[0..ptr], ".") + "..." + parts[<1];
            }

            thisIP = tmp_thisIP;
        }

        thisEnv  = file_name(environment(thisUser));

        sscanf(thisEnv, "/home/%s/%s/%*s", a, b);

        if(strsrch(thisEnv, "/home/") == 0)
            thisEnv = replace_string(thisEnv, "/home/" + a + "/" + b, "~" + b);

        if(strlen(thisEnv) >= loc_len)
        {
            thisEnv += " ";
/*
            string array parts = explode(thisEnv, "/");
            string tmp_thisEnv;
            int ptr = sizeof(parts) - 2;

            tmp_thisEnv = "/" + implode(parts[0..ptr], "/") + "/.../" + parts[<1];

            while(strlen(tmp_thisEnv) >= loc_len)
            {
                ptr--;
                tmp_thisEnv = "/" + implode(parts[0..ptr], "/") + "/.../" + parts[<1];
            }

            thisEnv = tmp_thisEnv;
*/
        }

        if (interactive(thisUser)) idleTime = query_idle(thisUser);
        else idleTime = time() - thisUser->query("last_login");

        weeks = idleTime / (86400 * 7);
        days = (idleTime / 86400) % 7;
        hours = (idleTime / 3600) % 24;
        mins = (idleTime / 60) % 60;

        thisIdle = (weeks > 0 ? weeks + "w" : (days > 0 ? days + "d" : (hours > 0 ? hours + "h" : mins > 0 ? mins + "m" : "-")));

        printf("%-12s%-10s%-" + ip_len + "s%-" + loc_len + "s%-4s\n", capitalize(thisName), thisRank, thisIP, thisEnv, thisIdle);
    }

    return 1;
}

string renderTag(string rank, string colour)
{
    string tag;

    tag = sprintf("%-10s", rank);
    tag = replace_string(tag, rank, colour + capitalize(rank) + NOR);

    return tag;
}

string array sortRankIndex(string array ranks)
{
    string array returnValue = ({});
    int i = sizeof(ranks);

    while(i) { i--; returnValue += ({ ranks[i] }); }

    return returnValue;
}

string help()
{
    return(HIW + " SYNTAX: " + NOR + "people\n\n"
      "This command allows you to see all users logged in. They are\n"
      "displayed with their name, rank, ip, location, and minutes of idle\n"
      "time.\n");
}
