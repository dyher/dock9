/* ls.c
 *
 * Kenon @ LPUniversity
 * 10-OCT-06
 * File listing.
 *
 * Contributors: Kenon, Tacitus, Tricky
 */

#include <terminal.h>

#define SYNTAX "Syntax: ls [-l] [-a] [-S] [-t] [-r] [-F] [-P] [file1...]\n"

#define SIZE_STOP 12
#define DATE_STOP 13
#define NAME_STOP 40

#define SPACESBETWEENFILES 3

/* Space before and space after. From filename_prefix, filename_suffix */

#define SPACEADDEDBYFORMAT 2

/* Command line argument tokens */

#define T_NOMATCH  0
#define T_SHORTOPT 1
#define T_LONGOPT  2
#define T_STRING   3
#define T_WHITESP  4

/* I would like to move arrange_string, no_colours and strip_colours out of this file */

string arrange_string(string, int);
string strip_colours(string);
int numeric_sort(int, int, mixed, mixed);
int main(string);
string help();
string filename_prefix(mixed array);
string filename_suffix(mixed array, int);

string __Path;

string arrange_string(string str, int width)
{
   int i, j, maxi, len;
   string stripped_colours;

   if (!width) return "";

   len = strlen(stripped_colours = strip_colours(str));

   if (len < width)
       return str + repeat_string(" ", width - len);
   if (len == width) return str;

   /* Full str indexed by i. Visible chars in stripped_colours indexed by j */
   j = 0;
   maxi = strlen(str);

   for (i = 0 ; i < maxi ; i++)
   {
       if (str[i] == stripped_colours[j])
       {
           j++;
           if(j >= width) break;
       }
   }

   return str[0..i];
}

string strip_colours(string str)
{
    return TERMINAL_D->parse_pinkfish(str, TERMINAL_DUMB);
}

int numeric_sort(int field, int sort_order, mixed a, mixed b)
{
    if (a[field] == b[field]) return 0;
    if (a[field] > b[field]) return sort_order;
    return -sort_order;
}

int main(string arg)
{
    mixed array output_files;
    mixed array output_file;
    string output_str = "";
    int num_files;
    mixed array tokens;
    int max, i;
    int show_all = 0, sort_order = 1, size_sort = 0, time_sort = 0,
        long_list = 0, classify = 0, always_show_path = 0;
    string array paths=({ });
    int c;

    tokens = reg_assoc(
      arg || "",
      ({ "-[a-zA-Z]+", "--[a-zA-Z]+", "[^ \t]+", "[ \t]+" }),
      ({ T_SHORTOPT, T_LONGOPT, T_STRING, T_WHITESP }),
      T_NOMATCH
    );
    max = sizeof(tokens[0]);

    for (i = 0 ; i < max ; i++)
    {
        switch (tokens[1][i])
        {
            case T_NOMATCH:
            case T_WHITESP:
                break;
            case T_SHORTOPT:
                foreach (c in tokens[0][i][1..<1])
                {
                    switch (c)
                    {
                        case 'a':
                            show_all = 1;
                            break;
                        case 'r':
                            sort_order = -1;
                            break;
                        case 'S':
                            size_sort = 1;
                            break;
                        case 't':
                            time_sort = 1;
                            break;
                        case 'l':
                            long_list = 1;
                            break;
                        case 'F':
                            classify = 1;
                            break;
                        case 'P':
                            always_show_path = 1;
                            break;
                        default:
                            write("\nBad option: " + tokens[0][i] + "\n");
                            return 1;
                    }
                }
                break;
            case T_LONGOPT:
                switch (tokens[0][i])
                {
                    case "--all":
                        show_all = 1;
                        break;
                    case "--reverse":
                        sort_order = -1;
                        break;
                    case "--classify":
                        classify = 1;
                        break;
                    default:
                        write("\nBad option: " + tokens[0][i] + "\n");
                        return 1;
                }
                break;
            case T_STRING:
                paths += ({ tokens[0][i] });
                break;
        }
    }

    if (!sizeof(paths)) paths = ({ "." });

    /* Parse done, we have a list of paths and some boolean flags. */
    foreach (__Path in paths)
    {
        __Path = resolve_path(this_player()->query("cwd"), __Path);

        if (always_show_path || sizeof(paths) > 1)
            output_str += __Path + ":\n";

        if (__Path == "")
            __Path = this_player()->query("cwd");

        switch (file_size(__Path))
        {
            case -2://Dir
                if(__Path[<1] != '/')
                    __Path += "/";

                output_files = get_dir(__Path, -1);
                break;
            case -1:
                write("\nRead error\n");
                return 1;
            default:
                output_files = get_dir(__Path, -1);
                //ls on a file. we will change __Path so it contains the containing dir.
                __Path = __Path[0..strsrch(__Path, "/", 1)];
                break;
        }

        if (!output_files)
        {
            write("Invalid path: " + __Path + "\n");
            return 1;
        }

        if (!show_all)
            output_files = filter(output_files, (: ($1[0][0] != '.') :));

        num_files = sizeof(output_files);

        if (size_sort)
            output_files = sort_array(output_files, (: numeric_sort, 1, sort_order :));
        else if (time_sort)
            output_files = sort_array(output_files, (: numeric_sort, 2, sort_order :));
        else if (sort_order == -1)
            output_files = sort_array(output_files, sort_order);

        if (long_list)
        {
            //Long listing.
            string file_detail_str = "", temp, this_dir = "";
            int file_sizes = 0;

            foreach (output_file in output_files)
            {
                if (output_file[1] == -2)
                    file_detail_str = "\nd";
                else
                {
                    file_sizes += output_file[1];
                    file_detail_str = "\n-";
                    temp = sprintf("%d", output_file[1]);
                    file_detail_str = arrange_string(file_detail_str,
                      SIZE_STOP - strlen(temp));
                    file_detail_str += temp;
                }

                file_detail_str = arrange_string(file_detail_str, DATE_STOP);
                file_detail_str += ctime(output_file[2]);
                this_dir += sprintf("%s%s%s%%^RESET%%^%s",
                arrange_string(file_detail_str, NAME_STOP),
                filename_prefix(output_file),output_file[0],
                  /* Note that suffix is printed after RESET */
                  filename_suffix(output_file, classify)
                );
            }

            output_str = sprintf("%s%dK (%d bytes) in %d file(s)%s\n",
              output_str, file_sizes/1024, file_sizes, num_files, this_dir);
        }
        else
        {
            //Short listing.
            int largest_file_name = 0, screen_width, files_per_line;

            foreach (output_file in output_files)
                if (largest_file_name < i = strlen(output_file[0]))
                    largest_file_name = i;

            screen_width = TERMINAL_D->query_user_width(this_user());

            if (largest_file_name >= screen_width)
                files_per_line = 1;
            else
                files_per_line = (screen_width - 2)
                  / (largest_file_name + SPACESBETWEENFILES + SPACEADDEDBYFORMAT);

            i = 0;
            largest_file_name += SPACESBETWEENFILES;

            foreach (output_file in output_files)
            {
                if (++i == files_per_line)
                {
                    i = 0;
                    output_str = sprintf("%s%s%s%%^RESET%%^%s\n",
                      output_str,
                      filename_prefix(output_file),
                      output_file[0],
                      filename_suffix(output_file, classify)
                    );
                }
                else
                {
                    output_str=sprintf("%s%s%s",
                      output_str,
                      filename_prefix(output_file),
                      arrange_string(
                        sprintf("%s%%^RESET%%^%s",
                          output_file[0],
                          filename_suffix(output_file, classify)
                        ),
                        largest_file_name
                      )
                    );
                }
            }
        }

        output_str += "\n";
    }//End foreach around paths

    write(output_str);
    return 1;
}

string help()
{
    return HIW + " SYNTAX: " + NOR + SYNTAX + @BODY
        
This command lists the contents of a directory (current directory if none 
is specified). '*' and '?' may be used as wildcards. '*' indicates any number 
of characters, while '?' indicates a single character.

        -l, --list
                Shows extra information about files.
        -a, --all
                Include hidden files (name starts with period character) in the listing.
        -S
                Size sort files.
        -t
                Date sort files.
        -r, --reverse
                Reverse sort direction.
        -F, --classify
                Append an indicator to names.
        -P,
                Display path for all dirs
 %^BOLD%^See also:%^RESET%^ cd
BODY;
}

/* Return a single visible character as well as any colour */
string filename_prefix(mixed array file_details)
{
    switch (file_details[1])
    {
        case -2:
            return "%^BLUE%^%^BOLD%^ ";
        default:
            switch (file_details[0][<2..<1]) //Will hit default if size > 2
            {
                case ".c":
                    if (stat(__Path+file_details[0])[2])
                        return "%^GREEN%^%^BOLD%^*";
                    return "%^GREEN%^ ";
                case ".h":
                    return "%^RED%^ ";
                case __SAVE_EXTENSION__:
                    return "%^CYAN%^ ";
                default:
                    return "%^WHITE%^ ";
            }
    }
}

/* Return a single visible character */
string filename_suffix(mixed array file_details, int classify)
{
    if (classify && file_details[1] == -2)
        return "/";

    return " ";
}
