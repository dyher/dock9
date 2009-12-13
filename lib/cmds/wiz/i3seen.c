/* i3seen.c

 Tricky @ Rock the Halo
 November 14th, 2006
 Report the last known sighting of a mud user on the I3 network

*/

#include <intermud3.h>

object i3ucache;
mapping ucache, players;
string *plrs;

int sort_time(mixed a, mixed b)
{

    if(players[a] < players[b]) return -1;
    else
    if(players[a] > players[b]) return 1;
   
    return 0;
}

int main(string str)
{
    string who = 0;
    int total = 0, days = 0, seconds = 0;

    i3ucache = find_object(I3_UCACHE);

    ucache = i3ucache->getUserCache();

    if(str && str != "") sscanf(str, "%s", who);

    if(who && (who[0..1] == "-d" || who[0..4] == "--day"))
    {

        if(who[0..1] == "-d")
            days = to_int(who[2..<1]);
        else
            days = to_int(who[5..<1]);

        seconds = days * (24 * 60 * 60);

        who = 0;
    }

    plrs = ({ });
    players = ([ ]);

    foreach(string mudname, mapping usernames in ucache)
        foreach(string username, mixed data in usernames)
            if(data[I3_UC_LASTACTIVE] > 0) players[data[I3_UC_VISNAME] + "@" + mudname] = data[I3_UC_LASTACTIVE];

    foreach(string plr, int timestamp in players)
    {
        string *tmp;

        if(who == 0)
        {

            if(seconds && seconds < time() - timestamp) continue;

            tmp = plrs - ({ plr });
            plrs = tmp + ({ plr });
            total++;
        }
        else
        {
            string u, m;
            float fuzz;

            sscanf(plr, "%s@%s", u, m);

            if(who[0..0] != "@")
                fuzz = fuzzymatch(lower_case(who), lower_case(u));
            else
                fuzz = fuzzymatch(lower_case(who[1..<1]), lower_case(m));

            if(fuzz >= 66.0)
            {
                tmp = plrs - ({ plr });
                plrs = tmp + ({ plr });
            }

        }

    }

    if(sizeof(plrs) == 0) return notify_fail("Error [i3seen]: No such user found.\n");

//    if(who == 0)
//    {
        write("%^BOLD%^%^MAGENTA%^Total users seen");

        if(days != 0)
            write(" in the last " + days + " day" + (days > 1 ? "s" : ""));

        write(": " + sizeof(plrs) + "%^RESET%^\n");
//    }

    plrs = sort_array(plrs, "sort_time");

    foreach(string plr in plrs)
    {
        string tm_s;
        int yearDiff, monthDiff, weekDiff, dayDiff, hourDiff, minDiff;
        int timeDiff;
        int tm;

        tm = players[plr];

        if(tm == 0) continue;

        timeDiff = time() - tm;

        yearDiff = timeDiff / (365 * 24 * 60 * 60);
        monthDiff = (timeDiff / (28 * 24 * 60 * 60)) % 12;
        weekDiff = (timeDiff / (7 * 24 * 60 * 60)) % 4;
        dayDiff = (timeDiff / (24 * 60 * 60)) % 7;
        hourDiff = (timeDiff / (60 * 60)) % 24;
        minDiff = (timeDiff / 60) % 60;

        if(yearDiff && monthDiff) tm_s = sprintf("%i year%s %i month%s", yearDiff, yearDiff > 1 ? "s" : "", monthDiff, monthDiff > 1 ? "s" : "");
        else
        if(yearDiff) tm_s = sprintf("%i year%s", yearDiff, yearDiff > 1 ? "s" : "");
        else
        if(monthDiff && weekDiff) tm_s = sprintf("%i month%s %i week%s", monthDiff, monthDiff > 1 ? "s" : "", weekDiff, weekDiff > 1 ? "s" : "");
        else
        if(monthDiff) tm_s = sprintf("%i month%s", monthDiff, monthDiff > 1 ? "s" : "");
        else
        if(weekDiff && dayDiff) tm_s = sprintf("%i week%s %i day%s", weekDiff, weekDiff > 1 ? "s" : "", dayDiff, dayDiff > 1 ? "s" : "");
        else
        if(weekDiff) tm_s = sprintf("%i week%s", weekDiff, weekDiff > 1 ? "s" : "");
        else
        if(dayDiff && hourDiff) tm_s = sprintf("%i day%s %i hour%s", dayDiff, dayDiff > 1 ? "s" : "", hourDiff, hourDiff > 1 ? "s" : "");
        else
        if(dayDiff) tm_s = sprintf("%i day%s", dayDiff, dayDiff > 1 ? "s" : "");
        else
        if(hourDiff && minDiff) tm_s = sprintf("%i hour%s %i min%s", hourDiff, hourDiff > 1 ? "s" : "", minDiff, minDiff > 1 ? "s" : "");
        else
        if(hourDiff) tm_s = sprintf("%i hour%s", hourDiff, hourDiff > 1 ? "s" : "");
        else
        if(minDiff) tm_s = sprintf("%i minute%s", minDiff, minDiff > 1 ? "s" : "");
        else
            tm_s = "less than 1 minute";

        write("%^GREEN%^" + plr + "%^CYAN%^ was last seen on I3 %^GREEN%^" + tm_s + "%^CYAN%^ ago%^RESET%^\n");
    }

    return 1;
}

string help()
{
    return(HIW + " SYNTAX: " + NOR + "i3seen [-d<n>|user|@mud]\n");
}
