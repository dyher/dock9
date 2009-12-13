/* /areas/make/management/towns.c

 emsenn@Dock 9
 091212
 One of the creator resources rooms in Make.

*/

inherit ROOM;

void create(){
    set_light(1);
    set_short("Control Center");
    set_domains("make",({"management"}));
    set_long("This is a room that has a large map of the world in the center.  "
      "Around the map are reconstructions of buildings.  While most look "
      "generic, others are clearly recognizable landmarks.");
    set_items( ([
        ({"map","center"}):"There is a large map of the world here.  Oddly, "
          "it is unfinished, as much of Nulens has yet to have been painted.",
      ]) );
    set_chats(({
        "A voice comes over the loudspeaker, \"Construct additional pylons!\""
      }});
    set_exits(([
	"south" : __DIR__ +"pier2.c",
      ]) );
}
