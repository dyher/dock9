/* object.c
 *
 * Tacitus
 * Grouped on October 22nd, 2005
 */

string get_base(mixed ob)
{
    string temp;

    if (stringp(ob)) return ob;

    sscanf(file_name(ob), "%s#%*s", temp);

    if (!temp) temp = file_name(ob);

    return temp;
}

int getoid(object ob)
{
	int id;

	sscanf(file_name(ob || previous_object()), "%*s#%d", id);
	return id;
}
