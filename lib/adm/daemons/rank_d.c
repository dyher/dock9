/* rank_d.c

 Tacitus @ LPuniversity
 06-FEB-07
 Access Control List Daemon

*/

/*
    Last edited on 06-FEB-07 by Tacitus@LPUniversity
    QA has never occured on this file.
*/

#define FILE_DATA "/data/daemons/rank_d"

/* Prototypes */

int createRank(string groupValue, int weightValue);
int removeRank(string groupValue);
int setRankProperty(string rank, mixed key, mixed value);
int clearRankProperty(string rank, mixed key);

string array getRanks();
int queryQuantativeRank(string user);
string queryQualativeRank(string user);
varargs int isQuanativeRank(mixed user, int rank, int exclusiveFlag);
varargs int isQualativeRank(mixed user, string rank, int exclusiveFlag);
mixed switchRankContext(mixed rank);

varargs object array getInteractives(mixed rank, int disableExclusiveFlag);
varargs int compare(string user1, string user2, int flag);
mixed sort(mixed data);

int commit_state();
int restore_state();
void print_state();

/* Global Variables */

string array ranks;
mapping rankProperties;

void create()
{
    if(!file_exists(FILE_DATA + ".o"))
    {
        write("\n No rank configuration detected.\n");
        write("    ... Populating with default data.\n");

        ranks = ({"user", "developer", "sage", "admin" });
        
        rankProperties = ([
            "user"      : ([
                "short"  : "user",
                "long"   : "user",
                "colour" : "%^GREEN%^", ]),
            "developer" : ([
                "short"  : "dev",
                "long"   : "developer",
                "colour" : "%^YELLOW%^", ]),
             "sage"     : ([
                "short"  : "sage",
                "long"   : "senior developer",
                "colour" : "%^MAGENTA%^", ]),
             "admin"    : ([
                "short"  : "admin",
                "long"   : "administrator",
                "colour" : "%^BOLD%^%^RED%^", ]),
        ]);
                
        commit_state();
    }

    else restore_state();
}

int createRank(string groupValue, int weightValue)
{
    if(weightValue > sizeof(ranks)) weightValue = sizeof(ranks);
    if(weightValue < 0 || member_array(groupValue, ranks) != -1) return 0;

    if(weightValue == 0) ranks = ({ groupValue }) + ranks;
    else ranks = ranks[0..(weightValue-1)] + ({ groupValue }) + ranks[weightValue..];
    
    rankProperties += ([ 
        groupValue : ([
            "short"  : (sizeof(groupValue) > 5 ? groupValue[0..5] : groupValue),
            "long"   : groupValue,
            "colour" : "" ]),
        ]);
    commit_state();
    return 1;
}

int removeRank(string groupValue)
{
    if(member_array(groupValue, ranks) == -1) return 0;

    ranks -= ({ groupValue });
    map_delete(rankProperties, groupValue);
    commit_state();
    return 1;
}

int setRankProperty(string rank, mixed key, mixed value)
{
    if(!mapp(rankProperties[rank])) return 0;
    rankProperties[rank] += ([ key : value ]);
    commit_state();
    return 1;
}

mixed queryRankProperty(string rank, string key)
{
    if(!mapp(rankProperties[rank])) return 0;
    return rankProperties[rank][key];
}

int clearRankProperty(string rank, mixed key)
{
    if(nullp(rankProperties[rank][key])) return 0;
    map_delete(rankProperties[rank], key);
    commit_state();
    return 1;
}

string array getRanks()
{
     return ranks + ({});
}

int queryQuantativeRank(mixed user)
{
    int rank = sizeof(ranks);

    if(objectp(user)) user = user->query_name();

    while(rank--)
         if(master()->isMember(user, ranks[rank]))
            return rank;

    return 0;
}

string queryQualativeRank(mixed user)
{
    int rank = sizeof(ranks);

    if(objectp(user)) user = user->query_name();

    while(rank--)
         if(master()->isMember(user, ranks[rank]))
            return ranks[rank];

    return ranks[0];
}

varargs int isQuanativeRank(mixed user, int rank, int exclusiveFlag)
{
    if(objectp(user)) user = user->query_name();

    if(rank == 0)
    {
        if(exclusiveFlag && queryQuantativeRank(user) >= 0) return 0;
        else return 1;
    }
    
    else if(master()->isMember(user, ranks[rank]))
    {
        if(exclusiveFlag && queryQuantativeRank(user) != rank) return 0;
        else return 1;
    }
    
    return 0;
}

varargs int isQualativeRank(mixed user, string rank, int exclusiveFlag)
{
    if(objectp(user)) user = user->query_name();

    if(member_array(rank, ranks) != -1 && master()->isMember(user, rank))
    {
        if(exclusiveFlag && queryQualativeRank(user) != rank) return 0;
        else return 1;
    }
    else if(rank == "user")
    {
        if(exclusiveFlag && queryQuantativeRank(user) > 0) return 0;
        return 1;
    }
    return 0;
}

mixed switchRankContext(mixed rank)
{
    if(intp(rank))
    {
        if(rank < 0 || rank > sizeof(ranks)) return -1;
        else return ranks[rank];
    }
    
    else return member_array(rank, ranks);
}

varargs int compare(string user1, string user2, int flag)
{
    if(flag) return queryQuantativeRank(user1) - queryQuantativeRank(user2);

    if(queryQuantativeRank(user1) > queryQuantativeRank(user2)) return 1;
    if(queryQuantativeRank(user1) == queryQuantativeRank(user2)) return 0;
    else return -1;
}

varargs object array getInteractives(mixed rank, int disableExclusiveFlag)
{
    if(intp(rank))
    {
        if(disableExclusiveFlag) return sort_array(filter_array(users(),
            (: isQuanativeRank :), rank), (: strcmp($2->query_name(), 
            $1->query_name()) :));
        else return sort_array(filter_array(users(), (: isQuanativeRank :), 
            rank, 1), (: strcmp($2->query_name(), $1->query_name()) :));
    }
    else if(stringp(rank))
    {
        if(disableExclusiveFlag) return sort_array(filter_array(users(),
            (: isQualativeRank :), rank), (: strcmp($2->query_name(), 
            $1->query_name()) :));
        else return sort_array(filter_array(users(), (: isQualativeRank :), 
            rank, 1), (: strcmp($2->query_name(), $1->query_name()) :));
    }
    
    else return ({});
}

mixed sort(mixed data)
{
    if(!arrayp(data) || sizeof(data) <= 0) return ({});
    if(objectp(data[0])) data = data->query_name();
    return sort_array(data, "compare");
}

int commit_state()
{
    return save_object(FILE_DATA);
}

int restore_state()
{
    return restore_object(FILE_DATA);
}

void print_state()
{
    printf("Size of Index: %d\n%O\n", sizeof(ranks), ranks);
}
