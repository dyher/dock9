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
	set_short("A small lobby");
	set_long(
			"This is the lobby for the LPUniMudlib conference centre.\n"
			"There is a sign that hang from the ceiling:\n\n"
			"\t\t                Conference #3\n"
			"\t\t                     |\n"
			"\t\t Conference #2 <- -LOBBY-  -> Conference #1\n"
			"\t\t                     |\n"
			"\t\t                   Lounge\n"
			);
	set_exits( ([   "east" : __DIR__ + "conference.c",
			"west" : __DIR__ + "conference2.c",
			"north" : __DIR__ + "conference3.c",
			"south" : __DIR__ + "conference_lounge.c",
			"down" : FLOOR1 + "backr.c" ]) );
}
