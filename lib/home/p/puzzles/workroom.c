/* workroom.c

Tacitus @ LPUniversity
23-OCT-05
Generic Workroom

This is file demonstates the mudlib's room inheritable
basic capabilities and mudlib coding standards. If you
do not know how to edit this file to produce your own
personalized workroom, then please refer to the mudlib,
lpc, and education documentation or contact a more senior
developer.

Regarding the room inheritable, it is compatable with
the two most common used methods used in today's mudlib.
 a) set_<prop>(<value);
 b) set(<prop>, <value>);

The recommended and supported method is set_<prop>(<value>);
if there is one available. The second method is ment for
non-standard values and backwards area support.

*/

/* Last edited on July 11th, 2006 by Tacitus */

inherit ROOM;

void create()
{
     set_light(1);
     set_short("Puzzles' Workroom");
     set_long(
          "This is Puzzles' workroom. The room is cube shaped, and\n"
          "each wall appeard to be made up of various coloured square\n"
          "tiles. There is a desk in the middle of the room.\n"
     );
     set_exits( ([
          "cafe" : "/areas/lpuni/entrance.c",
          "void" : "/areas/std/void.c"
     ]) );
     set_items( ([
          ({"wall","walls"}) : 
          "The walls are made of different coloured square tiles. There\n"
          "appears to be thin cracks between each tile.\n",
          "desk" :
          "The desk is made of steel, and has a mirrored surface. There\n"
          "is an assortment of puzzles scattered across it.\n", 
     ]) );
     set_chats( ({
          "Suddenly, one of the walls rotates, clicking into a new position.\n"
          "Some of the adjacent coloured tiles now match.\n",
          }));
}

