/* /areas/trepi/docks/room/cargo3.c

 emsenn@Dock 9
 091005
 Part of the shipping area.

*/

inherit ROOM;

void create(){
    set_light(1);
    set_short("Shipping Area");
    set_domains("trepi",({"docks"}));
    set_long("This is where cargo is sorted before being loaded on ships.  It "
      "continues to the west, but ends abruptly to the east, being cut off by "
      "the ocean.  There aren't many crates in this part of the loading area, "
      "and what few there are look well used.  Trepi Lane is to the northwest "
      "and north, while the Besling Ocean is to the south.");
    set_items(([
        ({"area"}):"This is the western end of the shipping area in Trepi.",
        ({"cargo","crates"}):"There aren't too many crates here, but those that "
          "are show their age; their wood has turned the color of pitch and "
          "looks generally abused.",
        ({"ocean","besling ocean"}):"The Besling Ocean, where it turns into "
          "Trepi Strait, is to the south, southeast, and southwest.  And to the "
          "north, and east, too, but there's too much island in the way to see "
          "that.",
        ({"island"}):"I Visited Trepi Island And I Didn't Even Get a T-Shirt.",
        ({"trepi lane","lane","road","street"}):"Trepi Lane, which encompasses "
          "Old Trepi is to all sides of the north.",
      ]));
    set_chats(({
        "The town of Trepi, to the north, does something.  Not something "
          "interesting, but something nonetheless.",
        "A pebble rolls down off the shipping area and falls with a soft "
          "\"Plink\" into the ocean."
      }));
    set_exits(([
        "north"     : "/areas/trepi/docks/room/trepi_lane3.c",
	    "west"      : "/areas/trepi/docks/room/cargo2.c",
        "northwest" : "/areas/trepi/docks/room/trepi_lane2.c",
      ]) );
}
