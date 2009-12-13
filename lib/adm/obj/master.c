/* master.c

 Tacitus @ LPUniversity
 April 15th, 2005
 master object

*/

/* Preproccessor Statement */

#include <config.h>
#include <localtime.h>

#pragma show_error_context

#define ERRORLEVEL 1

/* inherits */

inherit "/adm/obj/master/valid";

/* Functions */

void flag(string str)
{
    write("Flags disabled.\n");
}

privatef object connect(int port)
{
    object login_ob;
    mixed err;

    err = catch(login_ob = new(LOGIN_OB));

    if (err)
    {
        write("I'm sorry, but it appears that mud is not functional at the moment.\n");
        write(err);
        return 0;
    }

    return login_ob;
}

privatef void preload(string filename)
{
    mixed err;
#ifdef __HAS_RUSAGE__
    mapping before, after;
#endif
    int cmd_time = 0;

    write("Preloading: " + filename + "...");

#ifdef __HAS_RUSAGE__
    before = rusage();
#else
    cmd_time = time_expression
    {
#endif
        err = catch (load_object(filename));
#ifndef __HAS_RUSAGE__
    };
#else
    after = rusage();

    if (sizeof(before) && sizeof(after))
        cmd_time = after["utime"] - before["utime"];

    cmd_time *= 1000;
#endif

    if (stringp(err))
        write("\n\tError " + err + " when loading " + filename + "\n");
    else
    {
        if (cmd_time == 0)
            write("\n");
        else if (cmd_time >= 1000000)
            write(sprintf("(%.3f s)\n", cmd_time / 1000000.0));
        else if (cmd_time >= 1000)
            write(sprintf("(%.3f ms)\n", cmd_time / 1000.0));
        else
            write(sprintf("(%.3f us)\n", cmd_time * 1.0));
    }
}

privatef string array epilog(int load_empty)
{
    string array pre_load;
    string file;

    set_privs(this_object(), "[master]");

    write(@END_COPY
 Sapidlib, Copyright (C) 2005-2008 Sapidlib Developers
 Sapidlib comes with ABSOLUTELY NO WARRANTY; for details type `license w'.
 This is free software, and you are welcome to redistribute it under
 certain conditions; type `license c' for details.
END_COPY);

    file = read_file("/adm/etc/preload");

    if (!stringp(file) || file == "") return ({ });

    pre_load = explode(file, "\n");
    return filter_array(pre_load - ({ "" }), (: $1[0] != '#' && $1[0..0] != "" && sizeof($1) :) );
}


privatef void log_error(string file, string message)
{
    string username;

    if(this_player()) username = this_player()->query_name();
    if(stringp(username) && user_exists(username))
    {
	write_file(user_path(username) + "log", "\n" + message);
	if(this_player()->query_env("error_output") != "disabled")
	    tell_object(this_player(), message);
    }

    log_file("compile", message);
}

//error_handler needs to be rewritten
privatef void error_handler(mapping error, int caught)
{
    object ob;
    string logContent = "---\n";

    ob = this_interactive() || this_user();

    if(caught) logContent += "**Error Caught\n";

    logContent += sprintf("Error: %s\n", error["error"]);

    if(error["object"])
        logContent += sprintf("Current object: %O\n", error["object"]);
    else
        logContent += "No current object.\n";

    if(error["program"])
        logContent += sprintf("Current Program: %O\n", error["program"]);
    else
        logContent += "No current program.\n";

    logContent += sprintf("Line: %5d, File: %s\n\n", error["line"], error["file"]);

    logContent += sprintf("Traceback:\n%s\n\n",
      implode(map_array(
        error["trace"],
        (:
          sprintf(
            "Line: %5d, File: %s\n Object: %O\n Program: %s" +
#if ERRORLEVEL > 1
            "\n Arguments: '%s'" +
#elif ERRORLEVEL > 2
            "\n Locals: '%s'" +
#endif
            "",
              $1["line"],
              $1["file"],
              $1["object"] || "No object.",
              $1["program"] || "No program."
#if ERRORLEVEL > 1
              , ($1["arguments"] ? implode($1["arguments"], ", ") : "No arguments.")
#elif ERRORLEVEL > 2
              , ($1["locals"] ? implode($1["locals"], ", ") : "No locals.")
#endif
          )
        :)
      ), "\n")
    );

    if(ob)
    {
        if(sscanf(error["file"], "/home/%*s/" + ob->query_name() + "/%*s"))
            write_file(user_path(ob->query_name()) + "log", logContent[0..<2]);

        if(!caught) tell_object(ob, logContent);
    }

    log_file("log", logContent);
}

privatef void crash(string crash_message, object command_giver, object current_object)
{
    foreach (object ob in users())
    {
	tell_object(ob, "Master object shouts: Damn!\nMaster object tells you: The game is crashing.\n");
	catch(ob->save_user());
    }

    log_file("shutdown", MUD_NAME + " crashed on: " + ctime(time()) + ", error: " + crash_message + "\n");
    log_file("crashes", MUD_NAME + " crashed on: " + ctime(time()) + ", error: " + crash_message + "\n");

    if (command_giver) {
	log_file("crashes", "this_player: " + file_name(command_giver) + " :: " + command_giver->query_name() + "\n");
    }

    if (current_object) {
	log_file("crashes", "this_object: " + file_name(current_object) + "\n");
    }

}

string get_save_file_name(string file, object who)
{
    return "/tmp/ed_SAVE_" + who->query_name() + "#" + file + random(1000);
}

string privs_file(string filename)
{
    string temp;

    if(sscanf(filename, "/adm/daemons/%s", temp)) return "[daemon]";
    if(sscanf(filename, "/adm/obj/%s", temp)) return "[adm_obj]";
    if(sscanf(filename, "/adm/%s", temp)) return "[admin]";
    if(sscanf(filename, "/cmds/adm/%s", temp)) return "[cmd_admin]";
    if(sscanf(filename, "/cmds/file/%s", temp)) return "[cmd_file]";
    if(sscanf(filename, "/cmds/object/%s", temp)) return "[cmd_object]";
    if(sscanf(filename, "/cmds/wiz/%s", temp))return "[cmd_wiz]";
    if(sscanf(filename, "/cmds/%s", temp)) return "[cmd]";
    if(sscanf(filename, "/home/%*s/%s/%*s", temp)) return "[home_" + temp + "]";
    if(sscanf(filename, "/open/%s", temp)) return "[open]";
    if(sscanf(filename, "/std/%s", temp)) return "[std_object]";
    if(sscanf(filename, "/obj/mudlib/%s", temp)) return "[mudlib_object]";
    if(sscanf(filename, "/obj/%s", temp)) return "[gen_obj]";
    else return "object";
}

string object_name(object ob)
{
    if(ob->query_name()) return ob->query_name();

    return file_name(ob);
}

mixed compile_object(string file)
{
    string *pathExploded, serverPath;
    object server, virtualObject;
    int index;

    write_file("/log/virtual_compile", "Request: " + file + "\n");

    pathExploded = explode(file, "/");
    index = member_array("virtual_area", pathExploded);

    if(index != -1)
    {
	serverPath = "/" + implode(pathExploded[0..index], "/") + "/virtual_server.c";
	if(file_exists(serverPath))
	{
	    write_file("/log/virtual_compile", "Server: " + serverPath + "\n");

	    catch(server = load_object(serverPath));
	    if(objectp(server))
	    {
		virtualObject = server->compile_object(file);

		if(objectp(virtualObject))
		{
		    write_file("/log/virtual_compile",
		      "Virtual object loaded for " + file + "\n");
		    return virtualObject;
		}

		else
		{
		    write_file("/log/virtual_compile",
		      "No Virtual object loaded for " + file + "\n");
		    return 0;
		}
	    }

	}

	write("No functional server found at " + serverPath + "\n");
    }

    return 0;
}

string make_path_absolute(string file)
{
    file = resolve_path((string)this_player()->query_cwd(), file);
    return file;
}

void log_file(string file, string msg)
{
    string fname;
    int size;

    if (query_privs(previous_object()) == "[open]") return;

    msg = ctime(time()) + ":" + msg;
    fname = LOG_DIR + "/" + file;
    size = file_size(fname);

    if (size == -2) return;
    if (size + strlen(msg) >= 200000)
    {
	mixed array ltime = localtime(time());
	string t1;
	string backup;
	int ret = sscanf(file, "%s.log", t1);

	if (ret == 0)
	    backup =
              sprintf("archive/%s-%04d%02d%02d%02d%02d%02d",
	        file,
                ltime[LT_YEAR],
                ltime[LT_MON] + 1,
	        ltime[LT_MDAY],
	        ltime[LT_HOUR],
	        ltime[LT_MIN],
                ltime[LT_SEC],
	      );
	else
	    backup =
	      sprintf("archive/%s-%02d%02d%02d%02d%02d%02d.log",
	        t1,
	        ltime[LT_YEAR],
	        ltime[LT_MON] + 1,
	        ltime[LT_MDAY],
	        ltime[LT_HOUR],
	        ltime[LT_MIN],
	        ltime[LT_SEC],
	    );
	rename(fname, LOG_DIR + "/" + backup);
    }

    write_file(fname, msg);
}

int save_ed_setup(object user, mixed config)
{
    user->set("ed_setup", config);
    return 1;
}

int retrieve_ed_setup(object user)
{
    return user->query("ed_setup");
}
