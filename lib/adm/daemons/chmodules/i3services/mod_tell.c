/* mod_tell.c

   Tricky@Rock the Halo
   18-AUG-2006
   Intermud3 tell service

   Last edited on October 6th, 2007 by Tricky@Rock the Halo

*/

#include <intermud3.h>

privatev object i3;

void create()
{
    i3 = find_object(I3_CHDMOD);
    i3->add_service_name("tell", 1);
    i3->add_service("tell", "req_handler");
}

void remove()
{
    i3 = find_object(I3_CHDMOD);
    i3->remove_service_name("tell");
    i3->remove_service("tell");

    destruct();
}

void send_tell(mixed o_user, mixed t_user, string t_mud, string msg)
{
    string vis_o_user;

    if (objectp(o_user) && o_user->is_bot())
    {
        vis_o_user = capitalize(o_user->query_bot_name());
        o_user = lower_case(vis_o_user);
    }
    else if (objectp(o_user))
    {
        vis_o_user = o_user->query_cap_name();
        o_user = lower_case(o_user->query_name());
    }
    else if (stringp(o_user))
    {
        vis_o_user = capitalize(o_user);
        o_user = lower_case(o_user);

        if (find_living(o_user) && find_living(o_user)->is_bot())
        {
            vis_o_user = capitalize(find_living(o_user)->query_bot_name(o_user));
            o_user = lower_case(find_living(o_user)->query_bot_name());
        }
        else if (find_player(o_user))
        {
            vis_o_user = capitalize(find_player(o_user)->query_name());
            o_user = lower_case(find_player(o_user)->query_name());
        }
    }
    else return;

    if (objectp(t_user)) t_user = lower_case(t_user->query_name());
    else if (stringp(t_user)) t_user = lower_case(t_user);
    else return;

    /* Intercept smileys */
    if(is_smiley(msg))
    {
        find_object(I3_EMOTE)->send_emoteto(o_user, t_user, t_mud, msg);

        return;
    }

    find_object(I3_CHDMOD)->send_packet( ({
      "tell",
      5,
      mud_name(),
      o_user,
      t_mud,
      t_user,
      vis_o_user,
      msg
    }) );
}

void req_handler(mixed *packet)
{
    object o_user;

    if(!packet[I3_T_USER] || !(o_user = find_player(packet[I3_T_USER])))
    {
        find_object(I3_ERROR)->send_error(
          0,
          packet[I3_O_MUD],
          packet[I3_O_USER],
          "unk-user",
          packet[I3_T_USER] + "@" + packet[I3_T_MUD] + " was not found!",
          packet
        );

        return;
    }

    tell_object(o_user, sprintf("%s@%s tells you: %s\n", "%^CYAN%^" + packet[6], packet[I3_O_MUD], "%^RESET%^" + packet[7]));
    o_user->set("reply", packet[6] + "@" + packet[I3_O_MUD]);

    if(o_user->query_env("away") != 0)
    {
        find_object(I3_EMOTE)->send_emoteto(o_user, packet[I3_O_USER], packet[I3_O_MUD], "is away from the keyboard!");
        send_tell(o_user, packet[I3_O_USER], packet[I3_O_MUD], o_user->query_env("away"));
    }

}
