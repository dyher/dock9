/* /areas/trepi/docks/room/dock10.c

 emsenn@Dock 9
 091005
 One of the docks in Trepi

*/

inherit ROOM;

void create(){
    set_light(1);
    set_short("Dock 10");
    set_domains("trepi",({"docks"}));
    set_long("This dock seems a little different than the others.  Maybe it's the way it looked across you from the pier.  Maybe it's the way to waves lap gently at its long wooden shafts, driven deeply into the bed of the Strait.  Or maybe it's the fact that there's a giant stone monkey head on it.");
    set_items( ([
        ({"dock"}):"The dock here seems strangely attractive.  With it's long wooden columns thrust far into the gravel below, and the smooth dark planks of wood, weathered well by years of experience, the dock exudes almost an erotic aura.  However, it doesn't, because that would be silly.  And a bit unhygenic; all the dock really exudes is a briney crust.",
        ({"pier"}):"The pier to the north looks so... homely... in comparison with Dock 10.",
        ({"waves"}):"Calling the slight rocking motion in the water below \"waves\" may be a bit much.  Slow and rhythmic, the waters of Trepi Strait surge and wane, distorting the white gravel below.",
        ({"shafts","columns","pillars"}):"The dock is supported by several long wooden pillars.  So long, and firm... sturdy... hmm?  Oh yes.  They support the dock.  Right.",
        ({"wood"}):"The wood of the dock is dark and weathered.  It has obviously serviced a great number of ships in its lifetime, and certainly knows how to treat them right.",
        ({"bed","floor","below","ground"}):"The white pebbles that make up the bed of the Trepi Strait somehow look as though they're moving around quite a bit under the water, but it's just an optical illusion caused by the slightly wavy water.",
        ({"monkey","head","monkey head","stone","stone head","stone monkey head"}):"That's the second biggest monkey head you've ever seen!"
      ]) );
    set_exits(([
	"north" : __DIR__ +"pier1.c",
      ]) );
}
