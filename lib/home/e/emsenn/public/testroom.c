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
     set_short("A test room");
     set_long(
          "This is a test area."
     );
     set_exits( ([
          "workroom" : "/home/e/emsenn/workroom.c",
     ]) );
}

