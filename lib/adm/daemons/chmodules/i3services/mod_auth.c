/* mod_auth.c
 *
 * Tricky @ Rock the Halo
 * 18-AUG-2006
 * Intermud3 auth service
 *
 * Last edited on April 30th, 2008 by Tricky @ Rock the Halo
 *
 */

#include <intermud3.h>

privatev object  I3;
privatev mapping SessionKeys;

/*
 * Function name: create
 * Description:   Hook into I3 and initialise the object data.
 */
void create()
{
    I3 = find_object(I3_CHDMOD);
    I3->add_service_name("auth", 1);
    I3->add_service("auth-mud-req", "req_handler");
    I3->add_service("auth-mud-reply", "reply_handler");

    SessionKeys = ([ ]);
    set_heart_beat(30);
}

/*
 * Function name: remove
 * Description:   Removes the hooks and destructs itself.
 */
void remove()
{
    I3 = find_object(I3_CHDMOD);
    I3->remove_service_name("auth");
    I3->remove_service("auth-mud-req");
    I3->remove_service("auth-mud-reply");

    destruct();
}

/*
 * Function name: heart_beat
 * Description:   Retire old session keys.
 */
void heart_beat()
{
    if (sizeof(keys(SessionKeys)))
    {
        foreach (string t_mudname, mixed data in SessionKeys)
        {
            /* Remove the authorization session keys after 10 minutes */
            if (time() - data["registered"] > 10 * 60)
            {
                I3LOG(
                  "Notice",
                  "I3_AUTH",
                  "heart_beat",
                  "Removing session key for '" + t_mudname + "'"
                );
                map_delete(SessionKeys, t_mudname);
            }
        }
    }
}

/*
 * Function name: set_session_key
 * Description:   Sets a mud's session key.
 * Arguments:     t_mudname - Target mudname.
 *                session_key - Key to register the session with.
 */
void set_session_key(string t_mudname, int session_key)
{
    SessionKeys[t_mudname] = ([
      "registered": time(),
      "session_key": session_key,
    ]);
}

/*
 * Function name: remove_session_key
 * Description:   Removes a mud's session key.
 * Arguments:     t_mudname - Target mudname.
 */
void remove_session_key(string t_mudname)
{
    I3LOG(
      "Notice",
      "I3_AUTH",
      "remove_session_key",
      "Removing session key for '" + t_mudname + "'"
    );
    map_delete(SessionKeys, t_mudname);
}

/*
 * Function name: query_session_keys
 * Description:   Queries the session keys.
 * Return:        Copy of the session keys mapping or 0 (zero) if none.
 */
mixed query_session_keys()
{
    if (sizeof(SessionKeys)) return copy(SessionKeys);
    else return 0;
}

/*
 * Function name: query_session_key
 * Description:   Queries a specified mud's session key.
 * Arguments:     t_mudname - Target mudname.
 * Return:        Session key of the mud or 0 (zero) if none.
 */
mixed query_session_key(string t_mudname)
{
    if (SessionKeys[t_mudname]) return SessionKeys[t_mudname]["session_key"];
    else return 0;
}

/*
 * Function name: send_auth_mud_req
 * Description:   Sends an auth-mud-req packet to the specified mud.
 * Arguments:     t_mudname - Target mudname.
 */
void send_auth_mud_req(string t_mudname)
{
    find_object(I3_CHDMOD)->sendToMud("auth-mud-req", 0, t_mudname, 0);
}

/*
 * Function name: req_handler
 * Description:   Handler for a auth-mud-req packet.
 * Arguments:     packet - Incoming packet.
 */
void req_handler(mixed *packet)
{
    string o_mud = packet[I3_O_MUD];
    int session_key = 0, sz_o_mud = strlen(o_mud);

    /* Simple hash */
    for (int i = 0; i < sz_o_mud; i++)
        session_key += (o_mud[i] + ((i / 2) + 1));

    session_key = (session_key * 1000) + random(470831)
      + ((time() & 0x00ffffff) ^ 0x005a5a5a);

    I3LOG(
      "Notice",
      "I3_AUTH",
      "req_handler",
      "Registering '" + o_mud + "' with the session key: " + session_key
    );
    set_session_key(o_mud, session_key);
    find_object(I3_CHDMOD)->sendToMud("auth-mud-reply", 0, o_mud, session_key);
}

/*
 * Function name: reply_handler
 * Description:   Handler for a auth-mud-reply packet.
 * Arguments:     packet - Incoming packet.
 */
void reply_handler(mixed *packet)
{
    string o_mud = packet[I3_O_MUD];
    int session_key = packet[6];

    I3LOG(
      "Notice",
      "I3_AUTH",
      "reply_handler",
      "'" + o_mud + "' registered your mud with the session_key: " + session_key
    );
    set_session_key(o_mud, session_key);
    find_object(I3_OOB)->oob_authorized(o_mud);
}
