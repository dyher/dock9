/* channel.c
 *
 * Tacitus @ LPUniversity
 * 02-OCT-05
 * Standard Command
 *
 */

#include <ansi.h>

#ifndef ORA
#define ORA "%^ORANGE%^"
#endif
#ifndef HIK
#define HIK "%^BOLD%^%^BLACK%^"
#endif

int main(string args)
{
    string cmd, arg, argChannel;
    string array arr, multipleTune;
    int arr_sz;

    if (!args)
        return notify_fail("Sytnax: channel <list/show/tune> <argument>\n");

    sscanf(args, "%s", cmd);
    sscanf(args, "%s %s", cmd, arg);
    sscanf(args, "%s %s %s", cmd, arg, argChannel);

    switch (cmd)
    {
        case "list" :
        {
            if (!arg)
            {
                arr = CHAN_D->getMods();
                arr_sz = sizeof(arr);

                if (arr_sz == 1)
                    write("%^BOLD%^The following channel module is installed:%^YELLOW%^\n\t"
                      + arr[0] + "%^RESET%^\n");
                else
                    write("%^BOLD%^The following channel modules are installed: %^YELLOW%^\n\t"
                      + implode(arr[0..(arr_sz - 2)], ", ") + ", " + arr[arr_sz - 1]
                      + "%^RESET%^\n");

                write("\n%^MAGENTA%^You may list the channels registered by each module by typing:%^RESET%^\n");
                write("\tchannel list [network_name | all]\n");
                return 1;
            }

            if (lower_case(arg) == "all")
            {
                foreach (string mod in CHAN_D->getMods())
                {
                    arr = CHAN_D->getChls(mod);
                    arr_sz = sizeof(arr);

                    for (int l = sizeof(copy(arr)) - 1 ; l >= 0 ; l--)
                        if (member_array(this_user()->query_name(), CHAN_D->getTuned(arr[l])) == -1)
                            arr[l] = HIK + arr[l] + NOR;
                        else
                            arr[l] = HIG + arr[l] + NOR;

                    if (arr_sz > 0)
                    {
                        if (arr_sz == 1)
                            printf("The channels are as follows for '%s':\n%-:4s%-=79s\n",
                              mod,
                              "",
                              arr[0]
                            );
                        else
                        {
                            string ch_list = implode(arr[0..<2], ", ") + " and " + arr[<1];

                            printf("The channels are as follows for '%s':\n%-:4s%s\n",
                              mod,
                              "",
                              ch_list
                            );
                        }
                    }
                }

                return 1;
            }
            else
            {
                arr = CHAN_D->getChls(arg);
                arr_sz = sizeof(arr);

                if (arr_sz > 0)
                {
                    if (arr_sz == 1)
                        printf("The channels are as follows for '%s':\n%-:4s%s\n",
                          arg,
                          arr[0]
                        );
                    else
                        printf("The channels are as follows for '%s':\n%-:4s%s\n",
                          arg,
                          "",
                          implode(arr[0..arr_sz - 2], ", ") + " and " + arr[arr_sz - 1]
                        );

                    return 1;
                }
            }

            write("Channel: No channels found for group " + arg + "\n");
            return 1;
        }
        case "show" :
        {
            if (!arg)
                return notify_fail("Sytnax: channel <list/show/tune> <argument>\n");
            if (!CHAN_D->valid_ch(arg))
                return notify_fail("Channel: Channel " + arg + " does not exist.\n");

            arr = map(CHAN_D->getTuned(arg), (: capitalize($1) :) );
            arr_sz = sizeof(arr);

            if (arr_sz > 0)
            {
                if (arr_sz == 1)
                    printf("The following user is tuned into '%s':\n\t%s\n",
                      arg,
                      arr[0]
                    );
                else
                    printf("The following users are tuned into '%s':\n%-:8s%-=69s\n",
                      arg,
                      "",
                      implode(arr[0..arr_sz - 2], ", ") + " and " + arr[arr_sz - 1]
                    );

                return 1;
            }

            write("Channel: No users tuned into channel " + arg + "\n");
            return 1;
        }
        case "tune" :
        {
            if (!argChannel)
                return notify_fail("Syntax: channel tune <in/out> <channel/network>\n");

            if (arg == "in")
            {
                multipleTune = CHAN_D->getChls(argChannel);

                if (sizeof(multipleTune) > 0)
                {
                    string array tuned_in = ({ });

                    foreach (string channel in multipleTune)
                        if (CHAN_D->tune(channel, this_player()->query_name(), 1))
                            tuned_in += ({ channel });

                    arr_sz = sizeof(tuned_in);

                    if (arr_sz > 0)
                    {
                        if (arr_sz == 1)
                            printf("Tune: Tuned into channel '%s':\n",
                              tuned_in[0]
                            );
                        else
                            printf("Tune: Tuned into channels:\n%-:8s%-=69s\n",
                              "",
                              implode(tuned_in[0..arr_sz - 2], ", ") + " and "
                                + tuned_in[arr_sz - 1]
                            );
                    }

                    return 1;
                }

                if (CHAN_D->tune(argChannel, this_player()->query_name(), 1))
                    write("Tune: Tuned into channel " + argChannel + "\n");
                else
                    write("Tune: Channel " + argChannel + " does not exist.\n");

                return 1;
            }

            if(arg == "out")
            {
                multipleTune = CHAN_D->getChls(argChannel);

                if (sizeof(multipleTune) > 0)
                {
                    string array tuned_out = ({ });

                    foreach (string channel in multipleTune)
                        if (CHAN_D->tune(channel, this_player()->query_name(), 0))
                            tuned_out += ({ channel });

                    arr_sz = sizeof(tuned_out);

                    if (arr_sz > 0)
                    {
                        if (arr_sz == 1)
                            printf("Tune: Tuned out of channel '%s':\n",
                              tuned_out[0]
                            );
                        else
                            printf("Tune: Tuned out of channels:\n%-:8s%-=69s\n",
                              "",
                              implode(tuned_out[0..arr_sz - 2], ", ") + " and "
                                + tuned_out[arr_sz - 1]
                            );
                    }

                    return 1;
                }

                if (CHAN_D->tune(argChannel, this_player()->query_name(), 0))
                    write("Tune: Tuned out of channel " + argChannel + "\n");
                else
                    write("Tune: Channel " + argChannel + " does not exist.\n");

                return 1;
            }
        }
        default :
        {
            return notify_fail("Sytnax: channel <list/show/tune> <argument>\n");
        }
    }
}

string help()
{
    string *modList = CHAN_D->getMods(), mods, help;

    if (!sizeof(modList)) mods = "There is no networks currently installed.";
    if (sizeof(modList) == 1) mods = modList[0];
    else
        mods = implode(modList[0..(sizeof(modList)-2)], ", ") + ", "
          + modList[sizeof(modList) - 1];

    help  = HIW + " SYNTAX: " + NOR;
    help += @ENDHELP
channel <list/show/tune> <argument>

This command allows you to interact with the different channel networks
available here on __MUD_NAME__.

More specifically, it currently allows you to list the different channels that
you can tune into, display who is tuned into a channel, and the ability to
tune in and out of channels/networks. If you are wondering what a network is,
it is a group of channels. You may notice that different networks have
different channels, different features, and different users/muds communicating
through it. Currently, the following networks are installed on
__MUD_NAME__:
        __MODS__
ENDHELP;

    help = replace_string(help, "__MUD_NAME__", HIG + mud_name() + NOR);
    help = replace_string(help, "__MODS__", YEL + mods + NOR);

    return help;
}
