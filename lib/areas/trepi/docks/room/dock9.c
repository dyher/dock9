/* /areas/trepi/docks/room/dock9.c

 emsenn@Dock 9
 091005
 First room new players see, a dock in Trepi.

*/

inherit ROOM;

void create(){
    set_light(1);
    set_short("Dock 9");
    set_domains("trepi",({"docks"}));
    set_long("This is the infamous Dock 9, where, for as long as anyone can "
      "recall, people have appeared, lost and confused.  Off the pier in the "
      "town of Trepi, Dock 9 has a reputation as being one of the more "
      "magical places on the world. There are a few crates here, though the "
      "area does seem fairly boring, considering the high quantity magical "
      "energy that are said to reside here.  To the west and south you can "
      "see the the Trepi Strait, which separates Trepi, the island and city, "
      "from Nulens, a large mostly empty continent just over the horizon to "
      "the west.  To the east is the town of Trepi itself.");
    set_items( ([
        ({"dock 9","dock"}):"You're standing on Dock 9.  A bit to south is Dock "
          "12, and to the east is Dock 8.  The docks are made of plain wood, "
          "well worn and weathered from its years of service, allowing the "
          "nations of Erplak to ship goods to and from Nulens.",
        ({"pier"}):"Dock 9 juts off the north side of the "
          "pier.  Made of some wood, made unrecognizable from "
          "years of exposure to the salty waters of Trepi Strait, the pier "
          "appears to be very old, yet sturdy.",
        ({"nulens"}):"Nulens is across the Strait to the west, just out of "
          "sight even on the clearest days.",
        ({"strait","trepi strait"}):"To the west of Dock 9 is the Trepi "
          "Strait, a long but narrow body of water that separates Trepi from "
          "Nulens, the relatively recently discovered continent, now being "
          "colonized by the empires of Erplak, an old and heavily populated "
          "continent on the other side of the Besling Ocean.",
        ({"straight","trepi straight"}):"While the Trepi Strait is indeed "
          "fairly straight, running almost due north and south between Trepi "
          "and Nulens, that does not mean you should all it a straight.  If "
          "having your English corrected offends you, it's probably best for "
          "everyone if you just go ahead and log off now.",
        ({"crates"}):"There are several crates stacked here, all painted with "
          "the stenciled lettering DOCK 9.",
        ({"water"}):"There is water to the north, south, and east.  It is "
          "shockingly clear, letting you see the small white pebbles that "
          "lie a few meters under the water.  Occasionally a fish flits past.",
        ({"pebbles","rocks","stones"}):"Under and around the docks are smooth white pebbles.  "
          "Presumably they were once white rocks, and are progressing along "
          "with their transformation into glittering white sand beaches.",
        ({"magic","energy"}):"You can't see magic, but if you could, there'd "
          "probably be quite a bit of it here.",
        ({"island","trepi"}):"Trepi Island is just off the to the east.  From "
          "here you can see the main section of town, as well as some jungle "
          "a bit to the south of it.",
        ({"jungle"}):"A bit south of the town of Trepi is a small dense "
          "jungle.",
        ({"dock 12"}):"Dock 12 is across the pier from here, to the south.",
	({"dock 8"}):"Dock 8 is a bit to the east of here, separated by a "
          "clear expanse of water."
      ]) );
    set_exits(([
	"south" : __DIR__ +"pier3.c",
      ]) );
    set_chats(({
        "A single butterfly, coloured brilliant blue and sharp cyan, appears, "
          "flaps its wings twice, and vanishes again.",
        "A large wave shakes the legs of the dock, and you detect a soft citrus "
          "scent in the air.",
        "You can hear the quite murmur of a sleepy town just off to the east."
    }));
    set_domains("trepi",({"docks"}));
}
