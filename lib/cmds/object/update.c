/* update.c
 *
 * Tacitus @ LPUniversity
 * 25-July-07
 * Update command
 *
 */

/* Original update command written by Byre @ LPUniversity 05-APR-05 */

#define T_NOMATCH  0
#define T_SHORTOPT 1
#define T_LONGOPT  2
#define T_STRING   3
#define T_WHITESP  4

int proccessFiles(string array files);

int flag_recursive = 0;
int flag_verbose = 0;

int main(string arg)
{
	string array files = ({});
	mixed array opts;
	int i, errorCount;

	if(!arg) arg = this_player()->query("cwf");

	opts = reg_assoc(arg || "", ({ "-[a-zA-Z]+", "--[a-zA-Z]+", "[^ \t]+", "[ \t]+"}),
                ({T_SHORTOPT, T_LONGOPT, T_STRING, T_WHITESP}), T_NOMATCH);

	flag_recursive = 0;
	flag_verbose = 0;

	for(i = 0; i < sizeof(opts[0]); i++)
	{
		switch(opts[1][i]) /* Accessing element 1 of array opts */
		{
			case T_STRING:
			case T_WHITESP:
				break;
			case T_SHORTOPT:
			{
				int flag;

				foreach(flag in opts[0][i][1..<1]) /* Accessing element 0 of array opts */
				{
					switch(flag)
					{
						case 'r' :
						{
							flag_recursive = 1;
							break;
						}

						case 'v' :
						{
							flag_verbose = 1;
							break;
						}

						default:
						{
							return notify_fail("Error: Invalid option " + opts[0][i] + "\n"); /* Accessing element 0 of array opts */
						}
					}
				}

				break;
			}

			case T_LONGOPT:
			{
				switch(opts[0][i]) /* Accessing element 0 of array opts */
				{
					case "--recursive" :
					{
						flag_recursive = 1;
						break;
					}

					case "--verbose" :
					{
						flag_verbose = 1;
						break;
					}

					default:
					{
						return notify_fail("Error: Invalid option " + opts[0][i] + "\n"); /* Accessing element 0 of array opts */
					}
				}
				
				break;
			}

			case T_STRING:
			{
				files += ({ opts[0][i] }); /* Accessing element 0 of array opts */
				break;
			}
		}
	}

	files = filter_array(glob_array(files, this_player()->query("cwd")), (: !directory_exists($1) :));

	errorCount = proccessFiles(files);

	if(errorCount)
	{
		write("%^MAGENTA%^Warning%^RESET%^ [update]: " + errorCount + (errorCount == 1 ? " error" : " errors") + " occured.\n");
		if(errorCount >= sizeof(files)) return notify_fail("Failure [update]: No files updated.\n");
	}
	
	write("%^YELLOW%^Notice%^RESET%^ [update]: Update finished. (" + (sizeof(files) - errorCount) + ")\n");
	destruct();
	return 1;
}

int proccessFiles(string array files)
{
	int errorCount = 0;

	foreach(string file in files)
	{
		object array users;
		object obj;
		string err;

		file = resolve_path(this_player()->query("cwd"), file);
		
		if(obj = find_object(file))
		{
			users = filter_array(all_inventory(obj), (: interactive :));
     			users->move(load_object(VOID_OB), 1);
		        destruct(obj);
		}

		if(file[<2..<1] != ".c") file += ".c";

		if(file == __FILE__)
		{
			write("%^YELLOW%^Warning%^RESET%^ [update]: Illegal to update self. Object destructed.\n");
			errorCount++;
			continue;
		}


		if(!file_exists(file))
		{
			write("%^RED%^Error%^RESET%^ [update]: File " + file + " not found.\n");
			errorCount++;
			continue;
		}

		this_player()->set("cwf", file);
		
		err = catch(obj = load_object(file));

		if(stringp(err))
		{
			write("%^RED%^Error%^RESET%^ [update]: File " + file + " failed to load.\n");
			write(err + "\n");
			errorCount++;
			continue;
		}

		if(flag_recursive)
		{
			if(flag_verbose) write("%^YELLOW%^Notice%^RESET%^ [update]: Recursing inheritance tree for file " + file + "\n");
			errorCount += proccessFiles(inherit_list(obj));
		}

		if(arrayp(users)) users->move(obj, 1);
		write("%^GREEN%^Success%^RESET%^ [update]: File " + file + " was updated.\n");
	}

	return errorCount;
}

string help()
{
	return " %^BOLD%^SYNTAX:%^RESET%^ update [OPTION]... [FILE]...\n\n"
		"The update command loads the specified file(s). If no argument is given,\n"
		"then it will reloads the current working file. You can also specify here\n"
		"to update the enviroment you are in. Updating a file only affects the\n"
		"master copy of the object. This means that clones of the object that were\n"
		"created before the update will NOT be affected. You will need to destroy\n"
		"those objects and re-clone them for any changes to take effect.\n\n"
		" -r, --recursive \n\t Update all objects inherited by the files being loaded.\n"
		" -v, --verbose \n\t Explain, in more detail, what is being done.\n";
}

