/* /areas/trepi/npc/greeter_sailor.c

 Emsenn@Dock 9
 091207
 Greeter NPC for Trepi.  Can take newbies to the training area in Make if they
 want.

*/

inherit NPC;

void setup()
{
    ::setup();
    set_living_name("sailor");
    set_name("sailor");
    set_id(({"greeter","man"}));
    set_short("a helpful sailor");
    set_long("This sailor is here with no real purpose in life other than to "
      "help newcomers to Dock 9 get a friendly greeting.  He is much better at "
      "his job than his predecessor, who was a bit too friendly with new "
      "arrivals.");
    set_chats(({
        "say There aren't too many who come through here, but it's enough to "
          "to keep me busy.",
        "emote whistles a few cheery bars of some unfamiliar song.",
    }));
}

void catch_tell(string msg){
    string target;
    msg = TERMINAL_D->parse_pinkfish(msg, 1);
    if(sscanf(msg, "%s has entered%*s", target) || sscanf(msg, "%s has entered %*s", target)){
        command("say Hello, "+target+".  Welcome to Trepi.  If this is your first MUD, or you "
          "are just looking for a very thorough introduction, please \"say Take me to the "
          "training area.  Remember, punctuation, capitalization, and spelling count!\"");
    }
    if(sscanf(msg, "%s says: Take me to the training area.", target)){
        command("say Alright, I'll move you to the training area.");
        find_player(lower_case(target))->move("/areas/make/newbie/room/entrance.c");
        tell_room(this_object(),"In somewhat of a backward sneeze, "+target+" disappears.");
    }
}

string help(){
    return("This is a helpful sailor who will help you get the floating island "
      "of Make should you want to receive more thorough training on how to "
      "play a MUD.  He is a bit picky about how you ask, so you will have to "
      "say his sentence exactly as he does.  The command for this is: say "
      "Take me to the training area.");
}
