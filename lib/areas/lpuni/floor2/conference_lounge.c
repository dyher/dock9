/* converence_lobby.c
 *
 * Tacitus @ LPUniversity
 * 25-July-07
 * Lpuni Cafe Area
 *
 */

#define FLOOR1 "/areas/lpuni/floor1/"

inherit ROOM;

void create()
{
	set_light(1);
	set_short("The conference lounge");
	set_long("An ad-hoc assortment of comfy-looking chairs orbit randomly displaced tables.\n"
			"On the far corner, there is a help-yourself bar and other refreshments.\n");
	set_exits( ([   "north" : __DIR__ + "conference_lobby.c", ]) );

}

void init()
{
	add_action("action_pour", "pour");
}

int action_pour(string arg)
{
	object drink;

	if(!arg) return notify_fail("Syntax: pour <drink name>\n");

	drink = new(OBJECT);
	drink->set_short(arg);
	drink->set_long("A freshly poured drink of " + arg + ".\n");
	drink->set_id( explode(arg, " ") + ({ arg }) );
	drink->move(this_player());

	write("You pour yourself a drink of " + arg + ".\n");
	tell_room(this_object(), this_player()->query_cap_name() + " poured themself a drink of " + arg + ".\n", ({ this_player() }) );

	return 1;
}
