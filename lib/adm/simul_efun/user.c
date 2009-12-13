//user.c

//Tacitus @ LPUniversity
//Grouped on October 22nd, 2005

string user_data_file(string name)
{
     if(!name ||!stringp(name)) return 0;
     name = lower_case((string)name);
     return("/data/users/" + name[0..0] + "/" + name + "/" + name);
}

string user_mob_data(string name)
{
	if(!name ||!stringp(name)) return 0;
     name = lower_case((string)name);
     return("/data/users/" + name[0..0] + "/" + name + "/" + name + "_mob");
}


string user_data_directory(string priv)
{
	if(!priv ||!stringp(priv)) return 0;
	priv = lower_case((string)priv);
	return("/data/users/" + priv[0..0] + "/" + priv + "/");
}

string user_core_data_directory(string priv)
{
	if(!priv ||!stringp(priv)) return 0;
	priv = lower_case((string)priv);
	return("/data/users/" + priv[0..0] + "/" + priv + "/");
}

string user_path(string name)
{
     return ("/home/" + name[0..0] + "/" + name + "/");
}

string debug_info(int operation, object ob)
{
    string new_debug_info;

    if (objectp(this_player()) && !isMember(query_privs(this_player()), "admin"))
        return "Admin only!";

    if (catch { new_debug_info = efun::debug_info(operation, ob); })
        return "***Error";

    return new_debug_info;
}

object array users()
{
    return filter(livings(), (: $1->query_link() :) );
}

/* find_player() is supposed to return an object for players that
   were once interactive. This enforces that. */
object find_player(string player)
{
    object array plr = filter(users(), (: $1->id($(player)) :));

    if (sizeof(plr) < 1) return 0;

    return plr[0];
}

/* A fix for objects that don't check if a player has gone link-dead. */
int query_idle(object ob)
{
    if (interactive(ob)) return efun::query_idle(ob);

    return time() - ob->query("last_login");
}
