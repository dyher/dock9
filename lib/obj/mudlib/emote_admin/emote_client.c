//emote_client.c

//Parthenon @ LPUniversity
//25-JUN-06
//LPUni emote interface client

//Last updated: July 16th, 2006 by Tacitus



#define PROMPT "Emote Editor > "
#define MARKERS ({ "$I - Emoter", "$T - Target", \
                  "$PI - Possessive emoter", "$PT - Possessive target", \
                  "$SI - Subjective emoter", "$PT - Possessive target", \
                  "$OI - Objective emoter", "$OT - Objective target", \
                  "$M - Modifier", "$V - Verb"})

inherit OBJECT;

object soul_d;
private string tmp_emote_targeted_name;
private string tmp_emote_untargeted_name;
private string *tmp_emote_untargeted_msgs;
private string *tmp_emote_targeted_msgs;

int print_all_emotes();
int alphabetize(string arg1, string arg2);

void create()
{
	set_ids( ({ "/emote_client" }) );
	soul_d = load_object(SOUL_D);
}

int main_menu()
{
	tell_object(this_player(), "\n");
	tell_object(this_player(), "1. Add new emote\n");
	tell_object(this_player(), "2. Remove existing emote\n");
	tell_object(this_player(), "3. See message syntax for specific emote\n");
	tell_object(this_player(), "4. See all available emotes\n");
	tell_object(this_player(), "5. Remove all emotes (CAREFUL)\n");
	tell_object(this_player(), "\nType 'q' to exit.\n");
	tell_object(this_player(), "\n" + PROMPT);

	input_to("menu_selection");
	return 1;
}

int menu_selection(string arg)
{
	if(lower_case(arg) == "q" || lower_case(arg) == "quit")
	{
		tell_object(this_player(), "Exiting emote editor.\n");
			destruct(this_object());
			return 1;
	}

	switch(arg)
	{
		case "1":
				tell_object(this_player(), "\n");
				tell_object(this_player(), "Please enter the command word for the new emote.\nOr type 'q' to return to the main menu.\n\n"+PROMPT);
				input_to("get_emote_name");
				break;
		case "2":
				tell_object(this_player(), "\n");
				tell_object(this_player(), "Which emote would you like to delete?\nOr type 'q' to return to the main menu.\n\n" + PROMPT);
				input_to("remove_emote");
				break;
		case "3":
				tell_object(this_player(), "\n");
				tell_object(this_player(), "Which emote would you like to see messages for?\nOr type 'q' to return to the main menu.\n"+
				"(NOTE: if you wish to see messages for the targeted version, add a '/t' to the end of the emote)\n\n"+
				PROMPT);
				input_to("get_emote_msgs");
				break;
		case "4":
				tell_object(this_player(), "\n");
				print_all_emotes();
				tell_object(this_player(), "\n");
				tell_object(this_player(), "Hit enter to go back to the main menu or type 'q' to quit.\n\n" + PROMPT);
				input_to("main_or_quit");
				break;
		case "5":
			tell_object(this_player(), "\n");
				tell_object(this_player(), "Are you sure you wish to delete all emotes from the system? (y/n)\n\n" + PROMPT);
				input_to("confirm_delete_all");
				break;
		default:
				tell_object(this_player(), "\n");
				tell_object(this_player(), "Please make a selection from the menu or type 'q' or 'quit' to exit.\n\n"+PROMPT);
				main_menu();
				break;
	}//END SWITCH

	return 1;
}

nomask void get_emote_name(string arg)
{
	if(!arg || arg == "")
	{
		tell_object(this_player(), "\nPlease enter the command word for the new emote.\n"+
		"Or type 'q' to return to the main menu.\n\n"+PROMPT);
			input_to("get_emote_name");
		return;
	}//END IF

	if(sizeof(explode(arg, " ")) > 1)
	{
		tell_object(this_player(), "\nThe emote command must be only one word. Please try again.\n"+
		"Or type 'q' to return to the main menu.\n\n"+PROMPT);
			input_to("get_emote_name");
		return;
	}//END IF

	if(arg == "q" || arg == "Q")
	{
			main_menu();
		return;
	}//END IF

	tmp_emote_untargeted_name = arg;
	tmp_emote_targeted_name = arg+"/t";

	tell_object(this_player(), "\nWill this emote be (t)argeted, (u)ntargeted or (b)oth?\n"+
		"Or type 'q' to return to the main menu.\n\n"+PROMPT);

	input_to("targeted_or_not");

	return;
}

nomask void targeted_or_not(string arg)
{
	int i;
	string tmp = "";
	string existing_emotes = soul_d->query_emotes();

	if(!arg || arg == "")
	{
			tell_object(this_player(), "\nPlease enter 't', 'u' or 'b'.\n"+
		"Will this emote be (t)argeted, (u)ntargeted or (b)oth?\n"+
		"Or type 'q' to return to the main menu.\n\n"+PROMPT);
			input_to("targeted_or_not");
		return;
	}//END IF

	if(arg == "q" || arg == "Q")
	{
		main_menu();
		return;
	}//END IF

	if(member_array(lower_case(arg), ({ "b", "t", "u" }) ) == -1)
	{
			tell_object(this_player(), "\nPlease enter 't', 'u' or 'b'.\n"+
		"Will this emote be (t)argeted, (u)ntargeted or (b)oth?\n"+
			"Or type 'q' to return to the main menu.\n\n"+PROMPT);
		input_to("targeted_or_not");
			return;
	}//END IF

	if(((lower_case(arg) == "t") || (lower_case(arg) == "b")) && (member_array(tmp_emote_targeted_name, existing_emotes) != -1))
	{
			tell_object(this_player(), "\nThere is already a targeted emote of that name. Please try another.\n"+
		"Or type 'q' to return to the main menu.\n\n"+PROMPT);
		input_to("get_emote_name");
			return;
	}//END IF
	else if(((lower_case(arg) == "u") || (lower_case(arg) == "b")) && (member_array(tmp_emote_untargeted_name, existing_emotes) != -1))
	{
		tell_object(this_player(), "\nThere is already an untargeted emote of that name. Please try another.\n"+
			"Or type 'q' to return to the main menu.\n\n"+PROMPT);
		input_to("get_emote_name");
		return;
	}//END IF

	if(lower_case(arg) == "u")
	{
			if(tmp_emote_targeted_msgs)
			tmp_emote_targeted_msgs = ({});

		tell_object(this_player(), "\nPlease enter the message printed to the emoter if there is no target.\n"+
		"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

			for(i = 0; i < sizeof(MARKERS); i++)
			tmp += MARKERS[i]+"\n";

			tell_object(this_player(), tmp + "\n"+PROMPT);

		input_to("get_player_msg", 0, 1);
		return;
	}//END IF
	else if(lower_case(arg) == "t")
	{
		if(tmp_emote_untargeted_msgs)
			tmp_emote_untargeted_msgs = ({});

			tell_object(this_player(), "\nPlease enter the message printed to the emoter if there is a target.\n"+
		"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

			for(i = 0; i < sizeof(MARKERS); i++)
				tmp += MARKERS[i]+"\n";

		tell_object(this_player(), tmp + "\n"+PROMPT);

			input_to("get_player_msg", 0, 2);
		return;
	}//END ELSE IF
	else
	{
			if(tmp_emote_untargeted_msgs)
			tmp_emote_untargeted_msgs = ({});

			if(tmp_emote_targeted_msgs)
				tmp_emote_targeted_msgs = ({});

		tell_object(this_player(), "\nPlease enter the message printed to the emoter if there is no target.\n"+
			"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

		for(i = 0; i < sizeof(MARKERS); i++)
				tmp += MARKERS[i]+"\n";

		tell_object(this_player(), tmp + "\n"+PROMPT);

			input_to("get_player_msg", 0, 3);
		return;
	}//END ELSE
}

nomask void get_player_msg(string arg, int flag)
{
	int i;
	string tmp = "";

	if(arg == "q" || arg == "Q")
	{
			main_menu();
		return;
	}//END IF

	switch(flag)
	{
		case 1:
				if(!arg || arg == "")
				{
				tell_object(this_player(), "\nPlease enter the message printed to the emoter if there is no target.\n"+
					"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

				for(i = 0; i < sizeof(MARKERS); i++)
					tmp += MARKERS[i]+"\n";

				tell_object(this_player(), tmp + "\n"+PROMPT);

				input_to("get_player_msg", 0, 1);
				return;
				break;
				}//END IF

				tmp_emote_untargeted_msgs = ({ arg });

				tell_object(this_player(), "\nPlease enter the message printed to the room if there is no target.\n"+
				"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

				for(i = 0; i < sizeof(MARKERS); i++)
				tmp += MARKERS[i]+"\n";

				tell_object(this_player(), tmp + "\n"+PROMPT);

				input_to("get_room_msg", 0, 1);
				return;
				break;
		case 2:
				if(!arg || arg == "")
				{
				tell_object(this_player(), "\nPlease enter the message printed to the emoter if there is a target.\n"+
				"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

				for(i = 0; i < sizeof(MARKERS); i++)
					tmp += MARKERS[i]+"\n";

				tell_object(this_player(), tmp + "\n"+PROMPT);

				input_to("get_player_msg", 0, 2);
				return;
				break;
				}//END IF

				tmp_emote_targeted_msgs = ({ arg });

				tell_object(this_player(), "\nPlease enter the message printed to the room if there is a target.\n"+
				"Or type 'q' to return to the main menu.\n\nSubstitute the folowing wherever you like in the string:\n");

				for(i = 0; i < sizeof(MARKERS); i++)
				tmp += MARKERS[i]+"\n";

				tell_object(this_player(), tmp + "\n"+PROMPT);

				input_to("get_room_msg", 0, 2);
				return;
				break;
		case 3:
				if(!arg || arg == "")
				{
				tell_object(this_player(), "\nPlease enter the message printed to the emoter if there is no target.\n"+
					"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

				for(i = 0; i < sizeof(MARKERS); i++)
					tmp += MARKERS[i]+"\n";

				tell_object(this_player(), tmp + "\n"+PROMPT);

				input_to("get_player_msg", 0, 3);
				return;
				break;
				}//END IF

				tmp_emote_untargeted_msgs = ({ arg });

				tell_object(this_player(), "\nPlease enter the message printed to the room if there is no target.\n"+
				"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

				for (i = 0; i < sizeof(MARKERS); i++ )
				tmp += MARKERS[i]+"\n";

				tell_object(this_player(), tmp + "\n"+PROMPT);

				input_to("get_room_msg", 0, 3);
				return;
				break;
	}//END SWITCH
}


nomask void get_room_msg(string arg, int flag)
{
	int i;
	string tmp = "";

	if(arg == "q" || arg == "Q")
	{
		main_menu();
		return;
	}//END IF

	switch(flag)
	{
		case 1:
				if(!arg || arg == "")
				{
				tell_object(this_player(), "\nPlease enter the message printed to the room if there is no target.\n"+
					"Or type 'q' to return to the main menu.\n\nSubstitue the following wherever you like in the string:\n");


				for(i = 0; i < sizeof(MARKERS); i++)
					tmp += MARKERS[i]+"\n";

				tell_object(this_player(), tmp + "\n"+PROMPT);

				input_to("get_room_msg", 0, 1);
				return;
				break;
				}//END IF

				tmp_emote_untargeted_msgs += ({ arg });

				tell_object(this_player(), "\nPlease enter the default mod to the UNTARGETED emote. Such as 'happily',\n"+
				"or 'haphazardly'. If you do not wish to have a default, then press enter.\nOr press 'q'"+
				"to return to the main menu.\n\n"+PROMPT);

				input_to("get_mods", 0, 1);
				return;
				break;
		case 2:
				if(!arg || arg == "")
				{
				tell_object(this_player(), "\nPlease enter the message printed to the room if there is a target.\n"+
					"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

				for(i = 0; i < sizeof(MARKERS); i++)
					tmp += MARKERS[i]+"\n";

				tell_object(this_player(), tmp + "\n"+PROMPT);

				input_to("get_room_msg", 0, 2);
				return;
				break;
				}//END IF

				tmp_emote_targeted_msgs += ({ arg });

				tell_object(this_player(), "\nPlease enter the message to print to the target.\n"+
				"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

				for(i = 0; i < sizeof(MARKERS); i++)
				tmp += MARKERS[i]+"\n";

				tell_object(this_player(), tmp + "\n"+PROMPT);

				input_to("get_target_msg");
				return;
				break;
		case 3:
				if(!arg || arg == "")
				{
				tell_object(this_player(), "\nPlease enter the message printed to the room if there is no target.\n"+
					"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

				for(i = 0; i < sizeof(MARKERS); i++)
					tmp += MARKERS[i]+"\n";

				tell_object(this_player(), tmp + "\n"+PROMPT);

				input_to("get_room_msg", 0, 3);
				return;
				break;
				}//END IF

				tmp_emote_untargeted_msgs += ({ arg });

				tell_object(this_player(), "\nPlease enter the default mod to the UNTARGETED emote. Such as 'happily',\n"+
				"or 'haphazardly'. If you do not wish to have a default, then press enter.\nOr press 'q'"+
				"to return to the main menu.\n\n"+PROMPT);

				input_to("get_mods", 0, 3);
				return;
				break;
	}//END SWITCH
}


nomask void get_target_msg(string arg)
{
	int i;
	string tmp = "";

	if(arg == "q" || arg == "Q")
	{
		main_menu();
		return;
	}

	if(!arg || arg == "")
	{
		tell_object(this_player(), "\nPlease enter the message printed to the target.\n"+
			"Or type 'q' to return to the main menu.\n\nSubstitue the following wherever you like in the string:\n");

		for(i = 0; i < sizeof(MARKERS); i++)
				tmp += MARKERS[i]+"\n";

		tell_object(this_player(), tmp + "\n"+PROMPT);

			input_to("get_target_msg");
		return;
	}


	tmp_emote_targeted_msgs += ({ arg });

	tell_object(this_player(), "\nPlease enter the default mod to the TARGETED emote. Such as 'happily',\n"+
		"or 'haphazardly'. If you do not wish to have a default, then press enter.\nOr press 'q' "+
		"to return to the main menu.\n\n"+PROMPT);

	input_to("get_mods", 0, 2);

	return;
}

nomask void get_mods(string arg, int flag)
{
	int i;
	string tmp = "";

	switch(flag)
	{
		case 1:
				if(!arg || arg == "")
				tmp_emote_untargeted_msgs += ({ "" });
				else
				tmp_emote_untargeted_msgs += ({ arg });

				if(!soul_d->query_emotes())
				soul_d->reset_emotes();

				soul_d->set_emote(tmp_emote_untargeted_name, tmp_emote_untargeted_msgs);

				tell_object(this_player(), "\nNew untargeted emote '"+tmp_emote_untargeted_name+"' added.\n");

			tell_object(this_player(), "\n");
				tell_object(this_player(), "Hit enter to go back to the main menu or type 'q' to quit.\n\n" + PROMPT);
				input_to("main_or_quit");
				return;
				break;
		case 2:
				if(!arg || arg == "")
				tmp_emote_targeted_msgs += ({ "" });
				else
				tmp_emote_targeted_msgs += ({ arg });

				soul_d->set_emote(tmp_emote_targeted_name, tmp_emote_targeted_msgs);

				tell_object(this_player(), "\nNew targeted emote '"+tmp_emote_targeted_name[0..<3]+"' added.\n");

				tell_object(this_player(), "\n");
				tell_object(this_player(), "Hit enter to go back to the main menu or type 'q' to quit.\n\n" + PROMPT);
				input_to("main_or_quit");
				return;
				break;
		case 3:
				if(!arg || arg == "")
				tmp_emote_untargeted_msgs += ({ "" });
				else
				tmp_emote_untargeted_msgs += ({ arg });

				soul_d->set_emote(tmp_emote_untargeted_name, tmp_emote_untargeted_msgs);

				tell_object(this_player(), "\nNew untargeted emote '"+tmp_emote_untargeted_name+"' added.\n");

				tell_object(this_player(), "\nPlease enter the message printed to the emoter if there is a target.\n"+
				"Or type 'q' to return to the main menu.\n\nSubstitute the following wherever you like in the string:\n");

				for(i = 0; i < sizeof(MARKERS); i++)
				tmp += MARKERS[i]+"\n";

				tell_object(this_player(), tmp + "\n"+PROMPT);

				input_to("get_player_msg", 0, 2);
				return;
				break;
	}//END SWITCH
}

nomask void remove_emote(string emote)
{
	string *existing_emotes;

	if(emote == "q" || emote == "quit")
	{
		main_menu();
		return;
	}

	if(!(existing_emotes = soul_d->query_emotes()))
	{
		tell_object(this_player(), "There is no emote by that name. Please try another.\nOr type 'q' to return to the main menu.\n\n"+PROMPT);
			input_to("remove_emote");
		return;
	}//END IF

	if((member_array(emote, existing_emotes) == -1) && (member_array(emote+"/t", existing_emotes) == -1))
	{
			tell_object(this_player(), "There is no emote by that name. Please try another.\nOr type 'q' to return to the main menu.\n\n"+PROMPT);
		input_to("remove_emote");
			return;
	}//END IF

	if((member_array(emote, existing_emotes) != -1) && (member_array(emote+"/t", existing_emotes) != -1))
	{
		tell_object(this_player(), "\nBoth a targeted version and an untargeted version of this emote exists\n"+
		"exists. Would you like to delete the (t)argeted version, the (u)ntargeted\n"+
		"version or (b)oth?\nOr type 'q' to return to the main menu.\n\n"+PROMPT);

			input_to("which_to_delete", 0, emote, emote+"/t");
		return;
	}

	if(member_array(emote, existing_emotes) != -1)
			soul_d->delete_emote(emote);
	else
		soul_d->delete_emote(emote+"/t");

	tell_object(this_player(), "\n");
	tell_object(this_player(), "Emote '"+emote+"' deleted.\n");

	tell_object(this_player(), "\n");
	tell_object(this_player(), "Hit enter to go back to the main menu or type 'q' to quit.\n\n" + PROMPT);
	input_to("main_or_quit");

	return;
}

nomask void which_to_delete(string arg, string emote, string targeted_emote)
{
	if(arg == "q" || arg == "Q")
	{
		main_menu();
		return;
	}

	if(!arg || arg == "")
	{
		tell_object(this_player(), "\nBoth a targeted version and an untargeted version of "
			"this emote exists exists.\n"
			"Would you like to delete the (t)argeted version, the (u)ntargeted\n" +
			"version or (b)oth?\nOr type 'q' to return to the main menu.\n\n"+PROMPT);

		input_to("which_to_delete", 0, emote, targeted_emote);
		return;
	}

	switch(arg)
	{
		case "t":
				soul_d->delete_emote(targeted_emote);
				tell_object(this_player(), "\nTargeted emote '"+emote+"' deleted.\n");
				break;
		case "u":
				soul_d->delete_emote(emote);
				tell_object(this_player(), "\nUntargeted emote '"+emote+"' deleted.\n");
				break;
		case "b":
				soul_d->delete_emote(targeted_emote);
				soul_d->delete_emote(emote);
				tell_object(this_player(), "\nTargeted emote '"+emote+"' deleted.\n");
				tell_object(this_player(), "Untargeted emote '"+emote+"' deleted.\n");
				break;
	}//END SWITCH

	tell_object(this_player(), "\n");
	tell_object(this_player(), "Hit enter to go back to the main menu or type 'q' to quit.\n\n" + PROMPT);
	input_to("main_or_quit");

	return;
}

int confirm_delete_all(string arg)
{
	if(!arg || arg == "") tell_object(this_player(), "\nDeletion of all emotes cancelled.\n");


	else if(lower_case(arg) == "y" || lower_case(arg) == "yes")
	{
		soul_d->reset_emotes();
		tell_object(this_player(), "\nAll emotes have been deleted.\n");
	}

	else if(lower_case(arg) == "n" || lower_case(arg) == "no")
	{
		tell_object(this_player(), "\n");
		tell_object(this_player(), "Deletion of all emotes cancelled.\n");
	}

	else tell_object(this_player(), "\nDeletion of all emotes cancelled.\n");


	tell_object(this_player(), "\n");
	tell_object(this_player(), "Hit enter to go back to the main menu or type 'q' to quit.\n\n"
		+ PROMPT);
	input_to("main_or_quit");
	return 1;
}

int get_emote_msgs(string arg)
{
	string *msgs;
	int i;

	if(arg == "q" || arg == "Q")
	{
		main_menu();
		return 1;
	}//END IF

	msgs = soul_d->query_emote_msgs(arg);

	if(!msgs)	tell_object(this_player(), "That emote does not exist, or has no output.\n");
	else
	{
		for(i = 0; i < sizeof(msgs); i++)
		{
			if(i == 0) tell_object(this_player(), "Message to emoter: " + msgs[i] + "\n");
			if(i == 1) tell_object(this_player(), "Message to room: " + msgs[i] + "\n");
			if(i == 2 && sizeof(msgs) == 3) tell_object(this_player(), "Default modifier: "
				+ msgs[i] + "\n");
			else if(i == 2 && sizeof(msgs) == 4) tell_object(this_player(),
				"Message to target: " + msgs[i] + "\n");
			if(i == 3) tell_object(this_player(), "Default modifier: " + msgs[i] + "\n");

		}
	}

	tell_object(this_player(), "\n");
	tell_object(this_player(), "Hit enter to go back to the main menu or type 'q' to quit.\n\n" + PROMPT);
	input_to("main_or_quit");
	return 1;
}

int main_or_quit(string arg)
{
	if(lower_case(arg) == "q" || lower_case(arg) == "quit")
	{
		tell_object(this_player(), "Exiting emote editor.\n");
		destruct(this_object());
		return 1;
	}

	main_menu();
	return 1;
}

int print_all_emotes()
{
	string *emotes;
	int i;

	emotes = soul_d->query_emotes();

	if(!emotes) tell_object(this_player(), "There are no emotes to list.\n");

	else
	{
		emotes = sort_array(emotes, "alphabetize");

		for(i = 0; i < sizeof(emotes); i++)
			tell_object(this_player(), emotes[i]+"\n");
	}

	return 1;
}

int alphabetize(string arg1, string arg2)
{
	return strcmp(arg1, arg2);
}