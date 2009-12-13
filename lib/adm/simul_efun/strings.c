/*
 * strings.c
 *
 * Tricky @ LPUniversity
 * 27-DEC-2006
 * simul_efun
 *
 * Last edited on February 7th, 2007 by Tricky @ LPUniversity
 *
 */

string convert_from_utf8(string data)
{
    string msg = "";
    int i, sz;

    for (i = 0, sz = sizeof(data) ; i < sz ; i++)
    {
        int ud;
        int z, y, x, w, v, u;

        switch (data[i])
        {
            case 0..127:
            {
                ud = data[i];
                break;
            }
            case 128..191:
            {
                ud = data[i];
                break;
            }
            case 192..223:
            {
                z = data[i++];
                y = data[i];
                ud  = (z - 192) * 64;
                ud += (y - 128);
                break;
            }
            case 224..239:
            {
                z = data[i++];
                y = data[i++];
                x = data[i];
                ud  = (z - 224) * 4096;
                ud += (y - 128) * 64;
                ud += (x - 128);
                break;
            }
            case 240..247:
            {
                z = data[i++];
                y = data[i++];
                x = data[i++];
                w = data[i];
                ud  = (z - 240) * 262144;
                ud += (y - 128) * 4096;
                ud += (x - 128) * 64;
                ud += (w - 128);
                break;
            }
            case 248..251:
            {
                z = data[i++];
                y = data[i++];
                x = data[i++];
                w = data[i++];
                v = data[i];
                ud  = (z - 248) * 16777216;
                ud += (y - 128) * 262144;
                ud += (x - 128) * 4096;
                ud += (w - 128) * 64;
                ud += (v - 128);
                break;
            }
            case 252..253:
            {
                z = data[i++];
                y = data[i++];
                x = data[i++];
                w = data[i++];
                v = data[i++];
                u = data[i];
                ud  = (z - 252) * 1073741824;
                ud  = (y - 128) * 16777216;
                ud += (x - 128) * 262144;
                ud += (w - 128) * 4096;
                ud += (v - 128) * 64;
                ud += (u - 128);
                break;
            }
            default:
            {
                ud = "?"[0];
                break;
            }
        }

        if (ud < 192) msg += sprintf("%c", ud);
        else msg += sprintf("&#%d;", ud);
    }

    return msg;
}

string escape_string(string str)
{
    string tmp = "";

    for (int i = 0 ; i < sizeof(str) ; i++)
    {
        int ch = str[i] & 0xff;

        if ((ch < 32 && (ch != '\t' && ch != '\n' && ch != '\r')) || ch > 127)
            tmp += "ESC(" + sprintf("%d", ch) + ")";
        else tmp += sprintf("%c", ch);
    }

    return tmp;
}

/*
 * Fuzzymatch code:
 * original code by Stig Sther Bakken (Auronthas) 940715
 * Less original code: Profezzorn
 * Perlified: Richard Harrison (Tricky)
 * LPC'd: Richard Harrison (Tricky) - 2006-07-20
 */

/* The fuzziness between two strings, STRING1 and STRING2, is calculated by
 * finding the position in STRING2 of a prefix of STRING1.  The first character
 * of STRING1 is found in STRING2.  If we find it, we continue matching
 * successive characters from STRING1 at successive STRING2 positions.  When we
 * have found the longest prefix of STRING1 in STRING2, we decide whether it is
 * a match.  It is considered a match if the length of the prefix is greater or
 * equal to the offset of the beginning of the prefix of STRING1 in STRING2.
 * This means that "food" will match "barfoo" because "foo" (the prefix)
 * matches "foo" in "barfoo" with an offset and length of 3.  However, "food"
 * will not be considered to match "barfu", since the length is 1 while the
 * offset is 3.  The fuzz value of the match is the length of the prefix.  If
 * we find a match, we take the prefix off STRING1 and the string upto the end
 * of the match in STRING2.  If we do not find a match, we take off the first
 * character in STRING1.  Then we try and find the next prefix.
 *
 * So, to walk through an example:
 *
 * (FM-matchiness "pigface" "pigsfly"):
 *
 * STRING1              STRING2         MATCH LENGTH    OFFSET          FUZZ
 * pigface              pigsfly         3               0               3
 * face                 sfly            1               1               1
 * ace                  ly              0               0               0
 * ce                   ly              0               0               0
 * c                    ly              0               0               0
 *
 *      => 4
 *
 * (FM-matchiness "begining-of-l" "beginning-of-l"):
 *
 * STRING1              STRING2         MATCH LENGTH    OFFSET          FUZZ
 * begining-of-l        beginning-of-l  5               0               5
 * ing-of-l             ning-of-l       8               1               8
 *
 *      => 13
 */
private int f_fuzzymatch(string str1, string str2)
{
    int fuzz = 0;

    while(str1 != "" && str2 != "")
    {
        /* Now we will look for the first character of tmp1 in tmp2 */
        int offset = strsrch(str2, str1[0..0]);
        int str1len = strlen(str1);

        if(offset != -1 && str1len >= offset)
        {
            string *str1tmp = explode(str1, "");
            string *str2tmp = explode(str2[offset..<1], "");
            int     len = 0;
            int     str2len = strlen(str2[offset..<1]);

            /* Ok, so we have found one character, let's check how many more */
            while(str1tmp[len] == str2tmp[len])
            {
                len++;
                if(len >= str1len || len >= str2len) break;
            }

            /* We consider this a hit only if the length of the match is */
            /* bigger than or equal to the offset */
            if(len >= offset)
            {
                fuzz += len;
                str1  = str1[len..<1];
                str2  = str2[len + offset..<1];
                continue;
            }

        }

        str1 = str1[1..<1];
    }

    return fuzz;
}

/* This is how we use it. */
float fuzzymatch(string str1, string str2)
{
    float fuzz;
    int   l1 = strlen(str1);
    int   l2 = strlen(str2);
    int   t1 = l1 >= l2 ? l1 : l2;

    str1 = sprintf("%-*s", t1, str1);
    str2 = sprintf("%-*s", t1, str2);

    if(l1 == l2 && str1 == str2)
        fuzz = 100.0;
    else
    {
        fuzz  = f_fuzzymatch(str1, str2);
        fuzz += f_fuzzymatch(str2, str1);
        fuzz  = fuzz * 100.0 / (float)(l1 + l2);
    }

    return fuzz;
}

varargs string ltrim(string line, string charlist)
{
    string *whitelist;

    if(!stringp(line) || !sizeof(line)) return "";

    if(undefinedp(charlist))
        whitelist = ({ " ", "\t", "\n", "\r" });
    else
        whitelist = explode(charlist, "");

    while(1)
    {
        if(!sizeof(line)) break;

        if(member_array(line[0..0], whitelist) == -1) break;
        else line = line[1..<1];
    }

    return line;
}

varargs string rtrim(string line, string charlist)
{
    string *whitelist;

    if(!stringp(line) || !sizeof(line)) return "";

    if(undefinedp(charlist))
        whitelist = ({ " ", "\t", "\n", "\r" });
    else
        whitelist = explode(charlist, "");

    while(1)
    {
        if(!sizeof(line)) break;

        if(member_array(line[<1..<1], whitelist) == -1) break;
        else line = line[0..<2];
    }

    return line;
}

varargs string trim(string line, string charlist)
{

    if(!stringp(line) || !sizeof(line)) return "";

    line = ltrim(line, charlist);
    line = rtrim(line, charlist);

    return line;
}

mapping pluralExceptions = ([
    "*": ([ "is": "are" ]),
    "b": ([ "lues": "lues", "uses": "us", "onuses": "onus", ]),
    "c": ([ "hildren": "hild", ]),
    "d": ([ "ata": "atum", "ice": "ie", "ies": "ie", ]),
    "f": ([ "eet": "oot", "ora": "orum", ]),
    "g": ([ "eese": "oose", ]),
    "h": ([ "as": "ave", ]),
    "i": ([ "ndices": "ndex", ]),
    "l": ([ "ice": "ouse", "otuses": "otus", ]),
    "m": ([ "onies": "oney", "ice": "ouse", "atrices": "atrix", ]),
    "o": ([ "pus": "pera", "xen": "x", ]),
    "p": ([ "ants": "ants", ]),
    "r": ([ "emains": "emains", ]),
    "s": ([ "phinges": "phinx", "taves": "taff", ]),
    "t": ([ "hieves": "hief", "eeth": "ooth", ]),
    "v": ([ "ertices": "ertex", "axen": "ax", "iruses": "irus", "irii": "irus", ]),
    "w": ([ "ere": "as", ]),
]);

private int pe_map_initialized = 0;

string singularize(string str)
{
    mapping exceptions;
    string array tmp_arr;
    string array vowels = ({ "a", "e", "i", "o", "u" });
    string tmp = "", pre = "", plural = "", rest = "", single = "";
    int found = 0;

    str = trim(str);

    if(str == "") return "";

    if(sscanf(str, "%s of %s", plural, rest) != 2)
        plural = str;
    else
        rest = " of " + rest;

    tmp_arr = explode(plural, " ");
    plural = tmp_arr[<1];
    tmp = lower_case(plural);

    if(sizeof(tmp_arr) > 1) pre = implode(tmp_arr[0..<2], " ");

    if(file_size("/adm/simul_efun/plural_exceptions") > 0 && !pe_map_initialized)
    {
        string array lines;
        string pe;

        pluralExceptions = ([ ]);
        pe_map_initialized = 1;

        pe = read_file("/adm/simul_efun/plural_exceptions");
        lines = explode(pe, "\n");
        lines = filter(lines, (: $1[0..0] != "#" && sizeof(trim($1)) :) );

        foreach(string line in lines)
        {
            string plural_word, singular_word;

            sscanf(line, "%s%*([ \t]+)%s", plural_word, singular_word);

            if(plural_word[0] == '*')
            {

                if(!mapp(pluralExceptions["*"])) pluralExceptions["*"] = ([ ]);

                pluralExceptions["*"] += ([ plural_word[1..<1]: singular_word ]);
            }
            else
            {

                if(!mapp(pluralExceptions[plural_word[0..0]])) pluralExceptions[plural_word[0..0]] = ([ ]);

                pluralExceptions[plural_word[0..0]] += ([ plural_word[1..<1]: singular_word[1..<1] ]);
            }

        }

    }

    exceptions = pluralExceptions["*"];

    if(mapp(exceptions) && stringp(exceptions[tmp]))
    {
        single = exceptions[tmp];
        found = 1;
    }
    else
    {
        exceptions = pluralExceptions[tmp[0..0]];

        if(mapp(exceptions) && stringp(exceptions[tmp[1..<1]]))
        {
            single = tmp[0..0] + exceptions[tmp[1..<1]];
            found = 1;
        }

    }

    if(!found)
    {

        if(tmp[<2..<1] == "s'")
        {
            single = tmp[0..<2];
        }
        else
        if(tmp[<3..<1] == "s's")
        {
            single = tmp[0..<3];
        }
        else
        if(tmp[<2..<1] == "ss" && member_array(tmp[<3..<3], vowels) != -1)
        {
            single = tmp;
        }
        else
        if(tmp[<3..<1] == "ies")
        {
            single = tmp[0..<4] + "y";
        }
        else
        if(tmp[<3..<1] == "ves")
        {

            if(member_array(tmp[<4..<4], vowels) != -1)
                single = tmp[0..<4] + "ve";
            else
                single = tmp[0..<4] + "f";

        }
        else
        if(tmp[<3..<1] == "oes")
        {
            single = tmp[0..<3];
        }
        else
        if(tmp[<2..<1] == "ae" ||
           tmp[<3..<1] == "pes" ||
           tmp[<3..<1] == "ges" ||
           tmp[<3..<1] == "tes" ||
           tmp[<3..<1] == "ces" ||
           tmp[<3..<1] == "les")
        {
            single = tmp[0..<2];
        }
        else
        if(tmp[<3..<1] == "ses")
        {

            if(member_array(tmp[<4..<4], vowels) != -1)
                single = tmp[0..<2];
            else
                single = tmp[0..<3];

        }
        else
        if(tmp[<2..<1] == "es")
        {

            if(member_array(tmp[0..0], vowels) != -1 || member_array(tmp[<4..<4], vowels) != -1 || member_array(tmp[<3..<3], vowels) != -1)
                single = tmp[0..<2];
            else
                single = tmp[0..<3];

        }
        else
        if(tmp[<3..<1] == "men")
        {
            single = tmp[0..<3] + "an";
        }
        else
        if(tmp[<1..<1] == "s")
        {
            single = tmp[0..<2];
        }
        else single = tmp;

    }

    if(pre != "") pre += " ";
    if(plural != tmp) single = capitalize(single);

    return pre + single + rest;
}

varargs void test_singularize(string str)
{
    string char = "a";

    if(!undefinedp(str)) char = str[0..0];

    if(file_size("/adm/simul_efun/dict_" + char + ".txt") > 0)
    {
        string array words;
        int num_resets = 0, num_words = 0;

        words = explode(read_file("/adm/simul_efun/dict_" + char + ".txt"), "\n");
        num_words = sizeof(words);

        write_file("/tmp/singular_" + char + ".txt", "/* Singularize output. */\n", 1);

        while(sizeof(words) > 0)
        {
            string word, singular, plural, result;

            word = words[0];
            singular = singularize(word);
            plural = pluralize(singular);

            if(singular != word && plural != word)
            {
                result = sprintf("Word: %-20s Singular: %-20s Plural: %-20s\n", word, singular, plural);
                write_file("/tmp/singular_" + char + ".txt", result);
            }

            words = words[1..<1];

            if(num_resets > num_words) break;

            num_resets++;
            reset_eval_cost();
        }

        write_file("/tmp/singular_" + char + ".txt", "/* Num Resets: " + num_resets + " */\n");
    }

}

string a_an(string s1, string s2)
{
    string sep = " ";

    s1 = trim(s1);
    s2 = trim(s2);

    if(s1 == "") sep = "";
    if(s2 == "") return s1;

    if(s2[0..1] == "a " || s2[0..2] == "an ") return s1 + sep + s2;

    switch(s2[0..0])
    {
        case "a": case "e": case "i": case "o": case "u":
        case "A": case "E": case "I": case "O": case "U":
            return s1 + sep + "an " + s2;
        default:
            return s1 + sep + "a " + s2;
    }

}

string is_are(string s1, string s2, string s3, string s4)
{
    int plural = 0;

    s1 = trim(s1);
    s2 = trim(s2);
    s3 = trim(s3);
    s4 = trim(s4);

    if(s3[<2..<1] == "ys" || s3[<3..<1] == "es" ||
       s3[<2..<1] == "s'" || s3[<3..<1] == "s's" ||
       (s3[<1..<1] == "s" && s3[<2..<1] != "'s")) plural = 1;
        plural = 1;

    if(s2 == "")
    {
        if(!plural) return s1 + " is " + a_an("", s3) + " " + s4;
        else return s1 + " are some " + s3 + " " + s4;
    }
    else
    {
        if(!plural) return s1 + " is " + s2 + " " + s3 + " " + s4;
        else return s1 + " are " + s2 + " " + s3 + " " + s4;
    }

}
