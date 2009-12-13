/* mod_ucache.c
 *
 * Tricky@Rock the Halo
 * 19-AUG-2006
 * Intermud3 ucache service
 *
 */

#include <intermud3.h>

privatev object  i3;
privatev string array local_ucacheUpdated;
privatev string array shadows;

mapping ucache;

varargs void addUserCache(string, string, string, int, int);
varargs void removeUserCache(string, string);

void create()
{
    i3 = find_object(I3_CHDMOD);
    i3->add_service_name("ucache", 1);
    i3->add_service("ucache-update", "update_handler");

    shadows = ({
      "apresence",
      "ashadow",
      "someone",
    });
    local_ucacheUpdated = 0;
    ucache = ([ ]);

    if (file_exists(I3_UCACHE_FILE + ".o")) restore_object(I3_UCACHE_FILE);

    set_heart_beat(30);
}

void remove()
{
    i3->remove_service_name("ucache");
    i3->remove_service("ucache-update");

    save_object(I3_UCACHE_FILE);
    destruct();
}

void heart_beat()
{

    if (!mapp(ucache)) ucache = ([ ]);

    foreach (string mudname, mapping usernames in ucache)
    {

        if (mudname[0..10] == "Dead_Souls_")
        {
            removeUserCache(mudname);
            continue;
        }

        foreach (string username, mixed data in usernames)
        {
            int lastActive = data[I3_UC_LASTACTIVE];
            int lastUpdate = data[I3_UC_LASTUPDATE];
            int lastActiveDiff = time() - lastActive;
            int lastUpdateDiff = time() - lastUpdate;

            if (member_array(username, shadows) != -1)
                removeUserCache(mudname, username);
            else if (member_array(lower_case(data[I3_UC_VISNAME]), shadows) != -1)
            {
                ucache[mudname][username][I3_UC_VISNAME] = capitalize(username);
                save_object(I3_UCACHE_FILE);
            }
            else if (lastActive != 0 && lastActiveDiff > 7 * 24 * 60 * 60)
                removeUserCache(mudname, username);
            else if (lastUpdate < 0)
            {
                ucache[mudname][username][I3_UC_LASTUPDATE] = lastUpdate;
                save_object(I3_UCACHE_FILE);
            }
            else if (lastUpdateDiff > 28 * 24 * 60 * 60)
            {
                I3LOG(
                  "Notice",
                  "I3_UCACHE",
                  "heart_beat",
                  sprintf(
                    "Reseting user '%s' for mud '%s'",
                      username, mudname
                  )
                );
                ucache[mudname][username][I3_UC_GENDER] = -1;
                ucache[mudname][username][I3_UC_LASTUPDATE] = time();
                save_object(I3_UCACHE_FILE);
            }

        }

    }

}

mapping getUserCache() { return copy(ucache); }

varargs void send_ucache_update(mixed o_user, int updateUser)
{
    int gender;

    if (stringp(o_user)) o_user = find_player(o_user);
    else if (!objectp(o_user)) return;

    if (updateUser == 1 && local_ucacheUpdated != 0)
    {
        if (member_array(o_user->query_name(), local_ucacheUpdated) != -1)
        {
            if (sizeof(local_ucacheUpdated) == 1) local_ucacheUpdated = 0;
            else local_ucacheUpdated -= ({ o_user->query_name() });
        }
    }

    if (local_ucacheUpdated != 0
     && member_array(o_user->query_name(), local_ucacheUpdated) != -1)
        return;

    if (local_ucacheUpdated == 0)
        local_ucacheUpdated = ({ o_user->query_name() });
    else local_ucacheUpdated += ({ o_user->query_name() });

    if (o_user->query("gender") == "male"
     || o_user->query_gender() == "male")
        gender = 0;
    else if (o_user->query("gender") == "female"
          || o_user->query_gender() == "female")
        gender = 1;
    else gender = 2;

    i3->sendToAll(
      "ucache-update",
      0,
      ({
        o_user->query_name(),
        capitalize(o_user->query_name()),
        gender
      })
    );
}

void update_handler(mixed array packet)
{
    addUserCache(packet[I3_O_MUD], packet[6], packet[7], packet[8], 1);
}

/* addUserCache(mudname, username, visname, gender)

   mudname:  Name of the mud the user is on.
   username: Name of the user that the I3 router uses.
   visname:  Visible name that the user goes by.
   gender:   0 = Male, 1 = Female, 2 = Other, 3 = Dead Souls Neutral.

 */
varargs void addUserCache(string mudname, string username, string visname, int gender, int newuser)
{
    string gender_str;

    if (!stringp(mudname) || mudname == "") return;
    if (!stringp(username) || username == "") return;
    if (!stringp(visname) || visname == "") return;

    switch (gender)
    {
        case 0:
        {
            gender_str = "male";
            break;
        }
        case 1:
        {
            gender_str = "female";
            break;
        }
        case 2:
        {
            gender_str = "neuter";
            break;
        }
        case 3:
        {
            gender_str = "neutral";
            break;
        }
        default:
        {
            gender_str = "other(" + gender + ")";
            break;
        }
    }

    I3LOG(
      "Notice",
      "I3_UCACHE",
      "addUserCache",
      "Adding '" + username + "@" + mudname + "' " + "[" +
        (stringp(visname) ? visname : "0") + "/" + gender_str + "]"
    );

    if (!ucache[mudname]) ucache[mudname] = ([ ]);
    if (!ucache[mudname][username]) ucache[mudname][username] = allocate(4);
    if (!stringp(visname) || !visname || visname == "")
        ucache[mudname][username][I3_UC_VISNAME] = username;
    else ucache[mudname][username][I3_UC_VISNAME] = visname;

    ucache[mudname][username][I3_UC_GENDER]  = gender;

    if (!undefinedp(newuser))
        ucache[mudname][username][I3_UC_LASTUPDATE] = -time();
    else ucache[mudname][username][I3_UC_LASTUPDATE] = time();

    ucache[mudname][username][I3_UC_LASTACTIVE] = time();
    save_object(I3_UCACHE_FILE);
}

varargs void removeUserCache(string mudname, string username)
{
    string array muds = ({ });

    if (undefinedp(mudname) || mudname == 0)
        muds = keys(ucache);
    else if (undefinedp(username) || username == 0)
        muds = ({ mudname });
    else
    {
        I3LOG(
          "Notice",
          "I3_UCACHE",
          "removeUserCache",
          sprintf("Deleting user '%s' from mud '%s'", username, mudname)
        );
        map_delete(ucache[mudname], username);

        if (sizeof(ucache[mudname]) == 0) muds = ({ mudname });
    }

    if (sizeof(muds) > 0)
    {
        foreach (string mud in muds)
        {
            I3LOG(
              "Notice",
              "I3_UCACHE",
              "removeUserCache",
              sprintf("Deleting mud '%s'", mud)
            );
            map_delete(ucache, mud);
        }
    }

    save_object(I3_UCACHE_FILE);
}

string getVisname(string mudname, string username)
{
    mixed user;

    if (member_array(username, shadows) != -1) return "A Shadow";
    if (!ucache[mudname]) return 0;

    user = ucache[mudname][username];

    if (!arrayp(user) || sizeof(user) == 0) return 0;

    return user[I3_UC_VISNAME];
}

int getGender(string mudname, string username)
{
    mixed user;

    if (member_array(username, shadows) != -1) return 2;
    if (!ucache[mudname]) return -1;

    user = ucache[mudname][username];

    if (!arrayp(user) || sizeof(user) == 0) return -1;

    return user[I3_UC_GENDER];
}

varargs void checkUser(string mudname, string username, string visname, int stamp)
{
    object i3_channel = find_object(I3_CHANNEL);
    string gender_str;
    int gender;

    if (find_living(username) && find_living(username)->is_bot())
        username = lower_case(visname);

    if (member_array(username, shadows) != -1)
    {
        visname = "A Shadow";
        gender = 2;
        stamp = 0;
    }
    else gender = getGender(mudname, username);

    if ((!undefinedp(visname) && visname == 0) || gender == -1)
    {
        if (visname == 0)
        {
            object user = find_living(username);

            if (!objectp(user)) visname = capitalize(username);
            else if (user->is_bot())
            {
                visname = user->query_bot_name(username);
                username = lower_case(visname);
            }
            else visname = user->query_cap_name();
        }

        gender = 2;
        addUserCache(mudname, username, visname, gender, 1);
        i3_channel->send_chan_user_req(mudname, username);

        save_object(I3_UCACHE_FILE);
    }
    else if (!undefinedp(stamp) && stamp != 0)
    {
        mixed user = ucache[mudname][username];

        if (time() - user[I3_UC_LASTUPDATE] > 28 * 24 * 60 * 60)
        {
            if (visname == 0)
            {
                user = find_living(username);

                if (!objectp(user)) visname = capitalize(username);
                else if (user->is_bot())
                {
                    visname = user->query_bot_name(username);
                    username = lower_case(visname);
                }
                else visname = user->query_cap_name();
            }

            gender = 2;
            addUserCache(mudname, username, visname, gender);
            i3_channel->send_chan_user_req(mudname, username);
        }
        else
        {
            user[I3_UC_LASTACTIVE] = time();
            ucache[mudname][username] = user;
        }

        save_object(I3_UCACHE_FILE);
    }

    switch (gender)
    {
        case 0:
        {
            gender_str = "male";
            break;
        }
        case 1:
        {
            gender_str = "female";
            break;
        }
        case 2:
        {
            gender_str = "neuter";
            break;
        }
        case 3:
        {
            gender_str = "neutral";
            break;
        }
        default:
        {
            gender_str = "other(" + gender + ")";
            break;
        }
    }

    I3LOG(
      "Notice",
      "I3_UCACHE",
      "checkUser",
      "Checked '" + username + "@" + mudname + "' " + "[" +
        (stringp(visname) ? visname : "0") + "/" + gender_str + "]"
    );
}
