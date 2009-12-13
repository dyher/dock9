/* emotes command

   Parthenon @ LPUniversity
   28-JUN-06
   General Command

   Gutted on October 10th, 2007 by Trick@Rock the Halo
*/

mapping emote_types;

void print_emotes(string array emotes);

int main(string arg)
{
    string array emotes = ({ });

    emotes = sort_array(SOUL_D->query_emotes(), 1);

    if(!emotes)
	return notify_fail("Error [emotes]: There are no emotes available\n");

    emote_types = ([
      "untargeted": ({ }),
      "targeted": ({ }),
      "both": ({ })
    ]);

    foreach(string emote in copy(emotes))
    {

        if(emote[<2..<1] == "/t")
        {
            string untargeted = emote[0..<3];

            if(member_array(untargeted, emote_types["untargeted"]) != -1)
            {
                emote_types["untargeted"] -= ({ untargeted });
                emote_types["both"] += ({ untargeted });
                emotes -= ({ emote });
            }
            else
            {
                emote_types["targeted"] += ({ untargeted });
                emotes -= ({ emote });
                emotes += ({ untargeted });
            }

        }
        else
            emote_types["untargeted"] += ({ emote });

    }

    write(HIW + "\nAvailable emotes:\n" + NOR);

    print_emotes(sort_array(emotes, 1));

    return 1;
}

void print_emotes(string *emotes)
{
    int sz, rows, extra;

    sz = sizeof(emotes);
    rows = sz / 4;
    extra = sz % 4;

    if(extra) rows++;

    foreach(string s in emote_types["untargeted"])
    {
        int pos = member_array(s, emotes);

        if(pos == -1) continue;

        emotes[pos] = sprintf("%s%-19s%s", HIC, emotes[pos] + "+", NOR);
    }

    foreach(string s in emote_types["targeted"])
    {
        int pos = member_array(s, emotes);

        if(pos == -1) continue;

        emotes[pos] = sprintf("%s%-19s%s", HIB, emotes[pos] + "*", NOR);
    }

    foreach(string s in emote_types["both"])
    {
        int pos = member_array(s, emotes);

        if(pos == -1) continue;

        emotes[pos] = sprintf("%s%-19s%s", HIG, emotes[pos], NOR);
    }

    for(int i = 0; i < rows; i++)
    {
        write(emotes[i + rows * 0]);

        if(i + rows * 1 < sz)
            write(emotes[i + rows * 1]);

        if(i + rows * 2 < sz)
            write(emotes[i + rows * 2]);

        if(i + rows * 3 < sz)
            write(emotes[i + rows * 3]);

        write("\n");
    }

    write("\n");
    write(HIC + "+ - untargeted only." + NOR + "\n");
    write(HIB + "* - targeted only." + NOR + "\n\n");
    write(HIG + "All others, targeted and untargeted." + NOR + "\n\n");
}

string help()
{
    return(HIW + " SYNTAX: " + NOR + "emotes\n\n" +
      "This command allows you to see all of the emotes available for you\n"+
      "to use.\n\n"+
      HIW + "See also: " + NOR + "emote\n");
}

