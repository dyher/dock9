/* /areas/trepi/docks/room/customs.c

 emsenn@Dock 9
 091005
 The customs office, where players get a good dose of backdrop.

*/

inherit ROOM;

void create(){
    set_light(1);
    set_short("Customs Office");
    set_domains("trepi",({"docks"}));
    set_long("This is the customs office in Trepi.  Unlike your average customs "
      "office, which are focused on the monitoring of what is imported and "
      "exported form an area, this one serves to help explain the customs of "
      "both Nulens, the new continent, and Erplak.  There is a shelf full of "
      "pamphlets near the door, and a counter dividing the room in half.  "
      "Rather barren and utilitarian, this room means business.");
    set_items(([
        ({"office"}):"This is a customs office, though calling it an office is "
          "a bit of a stretch.  Except for the counter and display rack, there's "
          "very little here.",
        ({"shelf","rack","display rack"}):"There are a few pamphlet's here that "
          "you could probably look at.  It looks like there are a few about "
          "both Nulens and Erplak, as well as a few cities on each, such as "
          "Calliet and Samstone on Nulens, and Nevicia on Erplak.",
        ({"pamphlets"}):"There are several multicolored pamphlets arranged "
          "messily on the rack.",
        ({"counter"}):"There is a counter that splits the small room down the "
          "middle.",
        ({"nulens"}):"You glance over the pamphlet and read the follow about "
          "Nulens:  Nulens is a relatively recently discovered continent, quite "
          "large, bordered on the east by the Besling Ocean.  It is unknown what "
          "the western, northern, or southern borders are, as the continent is "
          "quite large and unexplored.  The empires that exist on Erplak have "
          "begun to colonize Nulens.  Oddly, due to the large technological "
          "revolution occuring, this has led to Nulens being a testing ground "
          "for many of the new technologies.",
        ({"erplak"}):"Erplak is the old country, where most of the people who "
          "live on Nulens came from, and where most of known history occured.  "
          "For many years, Erplak was stuck in a depression, however, a recent "
          "boom in trade caused by the colonization of Nulens.  This has led to "
          "a reconstruction of virtually all the cities.  However, instead of "
          "being built in a modern, technologically advanced style like the "
          "cities of Nulens, Erplakian cities are sticking to their traditional "
          "roots.",
        ({"calliet"}):"Calliet is a small village near the east coast of Nulens, "
          "originally built as a massive warehouse for the Catal Empire.  The "
          "center of the town is still the massive warehouse, which has been "
          "retrofitted to be a large indoor market.  This is the closest town "
          "to Trepi, so almost all people traveling to Nulens stop there.",
        ({"samstone"}):"Samstone is a small town far to the nroth of Nulens, "
          "nestled in a forest.  It's unsure why exactly the Talnord Empire built "
          "a town in such a remote location, but they did.",
        ({"nevicia"}):"Nevicia is somewhat of the equivalent of Trepi for "
          "Erplak.  Right on the border of the Old Country, Nevicia is constructed "
          "into the marshy shore.  There are no real roads in the city, instead "
          "the whole thing is held together by a mesh of bridges and canals.  "
          "Nevicia is the capital of the Herris Empire."
      ]));
    set_exits(([
        "west" : "/areas/trepi/docks/room/trepi_lane25.c",
      ]) );
}

string help(){
    return("This room is designed to help you learn a bit about the setting of "
      "Dock 9.  Just 'look' at the pamphlets listed in 'look display rack' and "
      "see what you can learn.");
}
