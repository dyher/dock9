/* /areas/trepi/room/dock9.c

 emsenn@Dock 9
 091005
 First room new players see, a dock in Trepi.

*/

inherit ROOM;

void create(){
    set_light(1);
    set_short("Training Ground");
    set_long("This is the training location for people who have never played "
      "a MUD before.");
    set_items( ([
      ]) );
    set_exits(([
      ]) );
    set_chats(({
    }));
}
