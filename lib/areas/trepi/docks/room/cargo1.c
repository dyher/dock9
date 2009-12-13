/* /areas/trepi/docks/room/cargo1.c

 emsenn@Dock 9
 091005
 Part of the cargo area off the south pier of Trepi.

*/

inherit ROOM;

void create(){
    object mob = clone_object("/areas/trepi/docks/npc/high_sailor.c");
    set_light(1);
    set_short("Shipping Area");
    set_domains("trepi",({"docks"}));
    set_long("This is where cargo is sorted before being loaded on the various "
      "ships that pull into Trepi.  There are crates of all sizes stacked "
      "precariously on gravel covered clearing, as well as tarps and ropes and "
      "other implements of transportation.  Immediately to the west is the "
      "pier, while to the north and northeast is Trepi Lane, "
      "an old cobbled road that surrounds Old Trepi, the original settlement "
      "on the island.");
    set_items(([
        ({"cargo","crates","crate"}):"There are many crates here, forming small "
          "skyscrapers of imports and exports.",
        ({"imports","exports"}):"The crates here are stamped with many different "
          "things, from proclamations of their contents to pledges of allegiance "
          "to a throne.",
        ({"pledge"}):"\"For Beetroot!\"",
        ({"ships"}):"There oddly aren't any ships at any of the docks that you "
          "can see.",
        ({"clearing"}):"This is a narrow band of beach that's been leveled off "
          "near the southwestern corner of town, and is used as a public area "
          "to keep the flow of goods going.  It has been paved with loosely "
          "packed white gravel, most likely gathered from the floor of Trepi "
          "Strait.",
        ({"gravel","pebbles","rocks","ground","stones"}):"The gravel is "
          "shockingly white and smooth, well on its way to becoming sand.",
        ({"sand"}):"Try looking for sand here in another few hundred years, you "
          "may have more luck.",
        ({"tarps"}):"There are several tarps here, some folded up, others rolled, "
          "and others spread over crates to protect them from the tropical "
          "storms that Trepi sometimes gets.",
        ({"ropes","rope"}):"They're long and brown and if you had very poor "
          "vision you may mistake them for giant licorice.",
        ({"trepi lane","lane","road","street","cobble","cobbles"}):"Trepi Lane "
          "is to the north of here, and circles around all of Old Trepi.",
        ({"trepi","old trepi"}):"Old Trepi is the original settlement created "
          "on the island, and makes up the western half of the town.  This is "
          "reflected in the architecture of the neighborhood; the houses are "
          "constructed of faded brick, and the Lane is uneven cobble.",
        ({"pier"}):"Jutting out to the west is the pier, which has docks seven "
          "through twelve attached to it.  Docks one to six were off another "
          "pier to the north, however, a freak accident involving walnuts "
          "destroyed them.",
      ]));
    set_chats(({
        "A sharp rapping comes from one of the crates, but stops abruptly.",
        "The ropes may or may not look delicious.",
        "One of the tarps rustles slightly in the soft ocean breeze.",
      }));
    set_exits(([
        "north"     : "/areas/trepi/docks/room/trepi_lane1.c",
        "northeast" : "/areas/trepi/docks/room/trepi_lane2.c",
        "east"      : "/areas/trepi/docks/room/cargo2.c",
	    "west"      : "/areas/trepi/docks/room/pier1.c",
      ]) );
    mob->move(this_object());
}
