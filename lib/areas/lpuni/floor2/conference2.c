/* conference.
 *
 * Tacitus @ LPUniMudlib
 * 24-JULY-07
 * LPUni Cafe conference room
 *
 */

inherit "/std/area/rooms/conference_room.c";

void create()
{
	set_light(1);
	set_short("A small conference room");
	set_long(
			"A large table surronded by several chairs is the vocal point of this room. The table,\n"
			"made of oak, is strong and sturdy. In the far corner, there is a small collection of\n"
			"filing cabinets, clip boards, writing materials, white boards, and flip charts.\n"
			"The room appears to be rather new but at the same time very temporary.\n" );
	set_exits( ([ "east" :__DIR__ + "conference_lobby.c" ]) );
}

