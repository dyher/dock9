/* /areas/trepi/docks/room/cargo2.c

 emsenn@Dock 9
 091005
 Part of the shipping area in Trepi.

*/

inherit ROOM;

void create(){
    set_light(1);
    set_short("Shipping Area");
    set_domains("trepi",({"docks"}));
    set_long("The shipping area continues to the east and west from here, while "
      "to the northwest, north, and northeast, is Trepi Lane.  A level "
      "gravel clearing, with crates set up like towers, forming a tight little "
      "maze in the center.");
    set_items( ([
        ({"area","clearing"}):"This is where merchants and other men of the "
          "world stash their contraba... er, their goods.",
        ({"trepi lane","lane","street","road"}):"Trepi Lane is to all sides of "
          "the north.",
        ({"gravel","stones","rocks"}):"Lightly packed smooth round pebbles make "
          "up this quasi-beach.",
        ({"beach","quasi-beach"}):"The shipping area may qualify for a beach, if "
          "the closest thing you know to a beach is a city sandbox.  There's no "
          "sand and a distinct lack of surf, but it is nature, and it is near "
          "the ocean.",
        ({"maze"}):"The crates in the center hear are arranged in sharply angling "
          "alleys.",
        ({"crates","towers"}):"There are many crates here, stacked into columns "
          "in the center of the area.",
      ]) );
    set_chats(({
        "One of the towers leans heavily in a gust of breeze, before righting "
          "itself as a breeze comes in from the opposite direction.",
        "The white gravel becomes slightly closer to sand.",
        "A haggard looking sailor comes over and peers at some crates, grunting, "
          "and leaving again.",
      }));
    set_exits(([
        "north"     : "/areas/trepi/docks/room/trepi_lane2.c",
        "northeast" : "/areas/trepi/docks/room/trepi_lane3.c",
        "east"      : "/areas/trepi/docks/room/cargo3.c",
	    "west"      : "/areas/trepi/docks/room/cargo1.c",
        "northwest" : "/areas/trepi/docks/room/trepi_lane1.c",
      ]) );
}
