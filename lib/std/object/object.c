/* object.c
 *
 * Tacitus @ LPUniversity
 * 04-APR-05
 * Base Object
 *
 */

#include <config.h>

inherit OB_E;

private string *ids, name, short, long, *chats; 
private int chat_interval, elapsed_chat_time;

void remove();
int move(mixed dest);
int moveAllowed(mixed dest);
int heart_beat();
void set_chats(string *chats, int interval);
void set_id(mixed str);
void set_ids(string *arg);
int id(string arg);
int set_name(string str);
string query_name();
string query_cap_name();
int set_short(string str);
int set_long(string str);
string query_short();
string query_long();
int receive_object(object ob);
int release_object(object ob);

void remove()
{
    load_object(VOID_OB);

    foreach (object ob in all_inventory())
    {
        if (interactive(ob))
        {
            tell_object(ob, "You watch as the environment around you disappears.\n");
            ob->move(VOID_OB);
        }
    }

    efun::destruct(this_object());
}

int move(mixed dest)
{
    object ob;

    if (stringp(dest)) ob = load_object(dest);
    if (objectp(dest)) ob = (object)dest;
    if (!objectp(ob)) return 0;
    if (!ob->receive_object(this_object())) return 0;
    if (environment(this_object()))
        if (!environment(this_object())->release_object(this_object()))
            return 0;

    move_object(dest);
    return 1;
}

int moveAllowed(mixed dest)
{
    object ob;

    if (stringp(dest)) ob = load_object(dest);
    if (objectp(dest)) ob = (object)dest;
    if (!objectp(ob)) return 0;
    if (!ob->receive_object(this_object())) return 0;
    if (environment(this_object()))
        if (!environment(this_object())->release_object(this_object()))
            return 0;

    return 1;
}

int heart_beat() {
    if (chats) {
        if (elapsed_chat_time % chat_interval == 0) {
            string msg = chats[random(sizeof(chats))];
            if (living(this_object())) {
                command(msg);
            } else {
                say(msg + "\n");
            }
        }
        elapsed_chat_time++;
        return 1;
    }
    return 0;
}

varargs void set_chats(string *arg, int ival) {
    chats = arg;
    write(this_object());
    if(!ival) chat_interval = 60;
    else chat_interval = ival;
    if(chat_interval >= 10) chat_interval = chat_interval+(random(chat_interval*0.20)-((chat_interval*0.20)/2));
    set_heart_beat(1);
    elapsed_chat_time = random(chat_interval);
}

void set_id(mixed str)
{
    if (arrayp(str)) set_ids(str);
    else set_ids( ({ str }) );
}

void set_ids(string *arg)
{
    if (stringp(arg)) arg = ({arg});

    ids = arg;
    set("ids", arg);

    if (query_name() && member_array(query_name(), ids) == -1)
    {
        ids += ({ query_name() });
        set("ids", ids);
    }
}

int id(string arg)
{
    if (!arg || !stringp(arg)) return 0;
    if (sizeof(ids) < 1) ids = ({query_name()});
    if (member_array(arg, ids) != -1) return 1;
    if (sizeof(query("ids")) > 0)
        if (member_array(arg, query("ids")) != -1) return 1;

    return 0;
}

int set_name(string str)
{
    if (interactive(this_object())
      && !isMember(query_privs(previous_object()), "admin")
      && previous_object() != this_object()) return 0;
    else
    {
        name = str;
        set("name", str);
        return 1;
    }
}

string query_name()
{
    if (!name) return query("name");

    return name;
}

string query_cap_name()
{
    if (!name && !stringp(query("name"))) return "";
    else if (!name) return capitalize(query("name"));
    else return capitalize(name);
}

int set_short(string str)
{
    short = str;
    set("short", str);
}

string query_short()
{
    if (!short)
    {
        if (!query("short")) return name;

        return query("short");
    }

    return short;
}

int set_long(string str)
{
    long = str;
    set("long", str);
}

string query_long()
{
    if (!long) return query("long");

    return long;
}

int receive_object(object ob)
{
    return 1;
}

int release_object(object ob)
{
    return 1;
}
