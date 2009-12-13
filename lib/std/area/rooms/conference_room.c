/* conference_room.c
 *
 * Tacitus @ LPUniMudlib
 * 24-July-07
 * Provides conference/meeting functionality
 *
 */

inherit ROOM;

#define FLOOR_OPEN 0
#define FLOOR_RESTRICTED 1

#define LOGGING_ENABLED 1
#define LOGGING_DISABLED 0

#define SECURE_ON 1
#define SECURE_OFF 0

class voteData
{
	int type;
	int ayeVotes;
	int nayVotes;
	object array voted;
}

void appendLog(string arg);

object chairman;
mapping floor_promoted = ([]);
string logfile;
int floor_mode = FLOOR_OPEN;
int logging = LOGGING_DISABLED;
int isSecure = SECURE_OFF;
class voteData vote;

void init()
{
	add_action("action_say", "say");
	add_action("action_chairman", "chairman");
	add_action("action_floor", "floor");
	add_action("action_grant", "grant");
	add_action("action_log", "logging");
	add_action("action_initVote", "call");
	add_action("action_vote", "vote");
	add_action("action_secure", "secure");
	add_action("action_kick", "kick");
}

int action_say(string arg)
{
	if(floor_mode == FLOOR_RESTRICTED && this_player() != chairman)
	{
		if(member_array(this_player(), keys(floor_promoted)) == -1)
		{
			tell_object(this_player(), "Error: Unable to speak at this time. Contact " + chairman->query_cap_name() + ".\n");
			return 1;
		}

		if(floor_promoted[this_player()] == 0)
		{
			tell_object(this_player(), "Error: You no longer have control of the floor.\n");
			map_delete(floor_promoted, this_player());
			return 1;
		}

		else floor_promoted[this_player()]--;
	}

	appendLog(this_player()->query_cap_name() + " : " + arg);

	return 0;
}

int action_chairman()
{
	if(chairman == this_player())
	{
		write("Notice: You release control of the floor as chairman.\n");
		tell_room(this_object(), this_player()->query_cap_name() + " has stepped down as chairman of the floor.\n", 
				({ this_player() }));
		appendLog(" --> " + this_player()->query_cap_name() + " has stepped down as chairman of the floor.");
		chairman = 0;
		return 1;
	}

	if(objectp(chairman))
		return notify_fail("Error: There is already a chairman.\n");

	write("Notice: You are now the chairman of this conference room.\n");
	tell_room(this_object(), this_player()->query_cap_name() + " is now the chairman of this conference room.\n",
		       	({ this_player() }));
	appendLog(" --> " + this_player()->query_cap_name() + " is now the chairman of this conference room.");
	chairman = this_player();
	return 1;
}

int action_floor(string arg)
{
	if(member_array(this_player(), keys(floor_promoted)) != -1)
	{
		map_delete(floor_promoted, this_player());
		write("Notice: You relinquish your control of the floor.\n");
		if(objectp(chairman)) tell_object(chairman, "Notice: " + this_player()->query_cap_name() 
				+ " has relinquished their control of the floor.\n");
		return 1;
	}

	if(this_player() != chairman) return 0;

	switch(arg)
	{
		case "open" :
		{
			if(floor_mode == FLOOR_OPEN) return notify_fail("Error: The floor is already open.\n");

			floor_mode = FLOOR_OPEN;
			tell_room(this_object(), "Notice: The floor is now open.\n");
			floor_promoted = ([]);
			appendLog(" ---> Floor opened.");
			return 1;
		}

		case "close" :
		{
			if(floor_mode == FLOOR_RESTRICTED) return notify_fail("Error: The floor is already closed.\n");

			floor_mode = FLOOR_RESTRICTED;
			tell_room(this_object(), "Notice: The floor is no longer open.\n");
			appendLog(" ---> Floor closed.");
			return 1;
		}

		default : 
		{
			return notify_fail("Syntax: floor <open/close>\n");
		}
	}
	       
}

int action_grant(string args)
{
	string name;
	int value;	
	object target;

	if(this_player() != chairman) return 0;

	if(floor_mode == FLOOR_OPEN) return notify_fail("Error: The floor is currently open.\n");

	if(!args || sscanf(args, "%s %d", name, value) != 2) return notify_fail("Syntax: grant <name> <value>\n");

	target = present(name, this_object());
	if(!target) return notify_fail("Error: " + target->query_cap_name() + " is not found.\n");

	if(target == chairman) return notify_fail("Error: You already have control on the floor.\n");

	floor_promoted += ([ target : value ]);

	write("Success: " + target->query_cap_name() + " has been granted the floor." + (value == -1 ? "\n" :
				"\n  -> " + target->query_cap_name() + " may speak " + value + " time" + 
				(value == 1 ? "" : "s") + ".\n"));
	tell_object(target, "Notice: You have been given control of the floor.\n" + (value == -1 ? "\n" :
				"\n You may speak " + value + " time" + (value == 1 ? "" : "s") + " while the floor is closed.\n"));

	return 1;
}

int action_log(string logpath)
{

	if(!logging)
	{
 		if(!logpath) return notify_fail("You must specify a logfile to enable logging.\n");
		logging = LOGGING_ENABLED;
		logfile = logpath;
		tell_room(this_object(), "Notice: Logging is enabled and available at: " + logpath + "\n");
		appendLog("\n\t  *** CONFERENCE LOGGING STARTED ***");
		return 1;
	}

	if(chairman && chairman != this_player() || !logpath) 
	{
		write("Current logfile: " + logfile + "\n");
		return 1;
	}

	if(logpath == "disable")
	{
		tell_room(this_object(), "Notice: Logging has been disabled by " + this_player()->query_cap_name() + "\n");
		appendLog("\n\t  *** CONFERENCE LOGGING TERMINATED ***");
		logging = LOGGING_DISABLED;
		return 1;
	}

	else
	{
		string oldlog = logfile;

		tell_room(this_object(), "Notice: Conference will now be logged to " + logfile + "\n");
		appendLog("\n\t *** CONFERENCE LOGFILE REASSIGNMENT (new log: " + logpath + " ) ***");
		logfile = logpath;
		appendLog("\n\t  *** CONFERENCE LOGGING STARTED (old log: " + oldlog + " ) ***");
		return 1;
	}
}

int action_initVote(string args)
{
	string type, topic;
	int time;

	if(objectp(chairman) && this_player() != chairman) return 0;

	if(classp(vote)) return notify_fail("Error: There is already a vote in progress.\n");
	
	if(!args || sscanf(args, "%d %s %s", time, type, topic) != 3) return notify_fail("Syntax: call <minutes> <[rollcall | ballot]> <topic>\n");
	if(time < 1) return notify_fail("Error: value minutes (time) must be greater then 0.\n");
	
	vote = new(class voteData);
	if(type == "rollcall") vote->type = 0;
	else if (type == "ballot") vote->type = 1;
	else return notify_fail("Error: Vote type " + type + " invalid.\n");

	vote->nayVotes = 0;
	vote->ayeVotes = 0;
	vote->voted = ({});

	tell_room(this_object(),
		" --> *** " + time + " minute " + type + " vote called by " + this_player()->query_cap_name() + " on topic: *** <--\n" +
		"\t " + topic + "\n");
	appendLog("--> *** " + capitalize(type) + " vote called by " + this_player()->query_cap_name() + " on topic: " + topic);
	call_out("voteCompleted", time * 60);

	return 1;

}

int action_vote(string args)
{
	if(!vote) return notify_fail("Error: There is currently no vote in progress.\n");
	if(member_array(this_player(), vote->voted) != -1) return notify_fail("Error: You have already voted.\n");

	args = lower_case(args);

	switch(args)
	{
		case "aye" :
		case "yes" :
		case "affirmitive" :
		{
			tell_object(this_player(), "You have voted aye.\n");

			if(vote->type == 0)
			{
				tell_room(this_object(), this_player()->query_cap_name() + " votes %^BOLD%^GREEN%^" + args + "%^RESET%^.\n",
					({ this_player() }) );
				appendLog(" ** (+) " + this_player()->query_cap_name() + " voted affirmitive.");
			}

			vote->ayeVotes++;
			break;
		}

		case "nay" :
		case "no" :
		case "negative" :
		{
			tell_object(this_player(), "You have voted nay.\n");

			if(vote->type == 0)
			{
				tell_room(this_object(), this_player()->query_cap_name() + " votes %^BOLD%^BLUE%^" + args + "%^RESET%^.\n",
					({ this_player() }) );
				appendLog(" ** (-) " + this_player()->query_cap_name() + " voted negative.");
			}

			vote->nayVotes++;
			break;
		}

		case "abstain" :
		{
			tell_object(this_player(), "Notice: You have thrown away your vote.\n");

			if(vote->type == 0)
			{
				tell_room(this_object(), this_player()->query_cap_name() + " has %^YELLOW%^abstained%^RESET%^ from voting.\n",
					({ this_player() }) );
				appendLog(" ** (_) " + this_player()->query_cap_name() + " publically abstained.");
			}

			break;

		}

		default : return notify_fail("Error: That argument is not valid.\n");
	}

	vote->voted += ({ this_player() });
	return 1;			
}

int action_secure(string args)
{
	if(!args) return notify_fail("Syntax: secure <[on | off]>\n");

	if(objectp(chairman) && this_player() != chairman) return 0;

	if(args == "off")
	{
		if(isSecure == SECURE_OFF) return notify_fail("Error: Room security is already disabled.\n");
		isSecure = SECURE_OFF;
		write("You have disabled room security.\n");
		tell_room(this_object(), this_player()->query_cap_name() + " has disabled room security.\n", ({ this_player() }) );
		appendLog(" * Room security was disabled by " + this_player()->query_cap_name() + "\n");
	}

	else if(args == "on")
	{
		if(isSecure == SECURE_ON) return notify_fail("Error: Room security is already enabled.\n");
		isSecure = SECURE_ON;
		write("You have enabled room security.\n");
		tell_room(this_object(), this_player()->query_cap_name() + " has enabled room security.\n", ({ this_player() }) );
		appendLog(" * Room security was enabled by " + this_player()->query_cap_name() + "\n");
	}

	else return notify_fail("Syntax: secure <[on | off]>\n");

	return 1;	
}

int action_kick(string args)
{
	string targetName, direction;
	object target;

	if(chairman != this_player()) return 0;
	if(!args || sscanf(args, "%s %s", targetName, direction) != 2) return notify_fail("Syntax: kick <user> <exit>\n");
	if(!valid_exit(direction)) return notify_fail("Error: " + direction + " is not a valid exit.\n");

	target = present(targetName, this_object());
	if(!objectp(target)) return notify_fail("Error: " + targetName + " is not present.\n");

	tell_object(target, "%^BOLD%^%^RED%^ " + this_player()->query_cap_name() + " has kicked you out of the conference room.%^RESET%^\n\n");

	target->move(query_exit(direction));

	tell_object(this_player(), "You have kicked " + target->query_cap_name() + " from the room.\n");
	tell_room(this_object(), this_player()->query_cap_name() + " kicked " + target->query_cap_name()
		+ " from the room.\n", ({ this_player() }) );
	appendLog(" * (!) " + this_player()->query_cap_name() + " kicked " + target->query_ca_name() + 
		" from the room.");

	return 1;
}

void voteCompleted()
{
	tell_room(this_object(), "\t\t %^BOLD%^YELLOW%^--> Vote Results <--%^RESET%^\n"
		" %^BOLD%^GREEN%^ + Affirmitive (Aye) votes%^RESET%^: " + vote->ayeVotes + "\n"
		" %^BOLD%^BLUE%^ - Negative (Nay) votes%^RESET%^: " + vote->nayVotes + "\n"
		" %^YELLOW%^ / Abstained%^RESET%^: " + (sizeof(filter_array(all_inventory(), (: interactive($1) :)))
			- (vote->ayeVotes + vote->nayVotes)) + "\n");

	if(vote->ayeVotes == vote->nayVotes)
	{
		tell_room(this_object(), "\n %^BOLD%^** Affirmitive and Negative have tied **%^RESET%^\n");
		appendLog(" ** --> Vote Results: " + vote->ayeVotes + " vs. " + vote->nayVotes + " - Tie.");
	}
	
	else if(vote->ayeVotes > vote->nayVotes)
	{
		tell_room(this_object(), "\n %^BOLD%^** Affirmitive is in the majority **%^RESET%^\n");
		appendLog(" ** --> Vote Results: " + vote->ayeVotes + " vs. " + vote->nayVotes + " - Affirmitive wins.");
	}
	
	else
	{
		tell_room(this_object(), "\n  %^BOLD%^**Negative is in the majority **%^RESET%^\n");
		appendLog(" ** --> Vote Results: " + vote->ayeVotes + " vs. " + vote->nayVotes + " - Negative wins.");
	}

	vote = 0;
}

void appendLog(string arg)
{
	if(logging != LOGGING_ENABLED) return;

	if(arg[<2..] != "\n") arg += "\n";
	if(!write_file(logfile, ctime(time()) + " # " + arg))
		if(objectp(chairman))
			tell_object(chairman, "Warning: Unable to write to specified logfile.\n");
}

int receive_object(object ob)
{
	if(isSecure == SECURE_ON && ob != chairman)
	{
		tell_object(ob, "The door seems to be locked. A \"meeting is in progress\" sign is hung.\n");
		return 0;
	}

	return 1;
}

int release_object(object ob)
{
	if(sizeof(filter_array(all_inventory(), (: interactive($1) :))) - 1 <= 0) isSecure = SECURE_OFF;
	return 1;
}
