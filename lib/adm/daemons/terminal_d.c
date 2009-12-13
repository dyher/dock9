/* terminal_d.c
 *
 * Tacitus @ LPUniversity
 * 17-JUNE-06
 * Mudlib object
 *
 * Contributors: Tacitus, Tricky
 *
 * Definitions:
 *
 *     <colour>    foreground colour
 *     B_<colour>  background colour
 *     RESET       return to the normal font -- not a terminal reset.
 *     BOLD        bold or bright letters
 *     UNDERLINE   underline mode
 *     FLASH       flashing or blinking letters
 *     REVERSE     reverse video mode
 *     NORMAL      normal intensity (not bold)
 *     STATUS      unknown
 *     WINDOW      unknown
 *     INITTERM    unknown
 *     ENDTERM     unknown
 *
 */

#include <terminal.h>

#define CHAR(p) sprintf("%c", (p))
#define ESC(p)  "\e"+(p)
/* CSI == Control Sequence Introducer */
#define CSI     ESC("[")
#define ANSI(p) CSI+(p)
/* SGR == Select Graphic Rendition */
#define SGR(p)  ANSI(p)+"m"

#define WRAP_DEFAULT     80 /* Column size of 80 */
#define INDENT_DEFAULT   0  /* How much to indent the wrapped text */

/* Terminal aliases */
/* Your terminal name: Real terminal name */
private mapping term_aliases = ([
  "xterm-debian": "xterm",
  "zmud": "vt100",
  "amiga": "vt220",
  "linux": "vt220",
  "xterm-color": "xterm",
  "vt102": "vt100",
  "dec-vt100": "vt100",
  "vt300": "vt220",
  "vt100a": "vt100",
  "vt100-ansi": "vt100",
  "dec-vt220": "vt220",
  "vt400": "vt220",
  "vs100": "xterm",
  "sun-cmd": "vt220-nc",
  "unknown": TERMINAL_DUMB,
  "ibm": TERMINAL_DUMB,
  "iris-ansi-net": "vt220",
  "network": TERMINAL_DUMB,
  "vt320": "vt220",
  "dtterm": "vt220",
  "screen": "vt220",
  "dec-vt320": "vt220",
  "dec-vt100i": "vt100",
  "ansii": "ansi",
  "vt100i": "vt100",
  "vt200-80": "vt220",
  "ibm-3279-2-e": TERMINAL_DUMB,
  "ibm-3279-2": TERMINAL_DUMB,
  "vt200": "vt220",
  "dec-vt200": "vt220",
  "hft": "vt220",
  "mac": "vt220-nc",
  "vt340": "vt220",
  "vt340-am": "vt220",
  "ibm-3278-2": TERMINAL_DUMB,
  "ibm-3278-2-e": TERMINAL_DUMB,
  "kterm": "xterm",
  "sun": "vt220-nc",
  "sun1": "vt220-nc",
  "xterms": "xterm",
  "vs100s": "xterm",
  "vt52": TERMINAL_DUMB,
  "vt420": "vt220",
  "dec-vt420": "vt220",
  "aixterm-m": "aixterm",
  "ibm-3151": "ibm3151",
  "ibm-3278-5": TERMINAL_DUMB,
  "du": TERMINAL_DUMB,
  "dialup": TERMINAL_DUMB,
  "dec-vt52": TERMINAL_DUMB,
  "vt100-pro": "vt100",
  "tinyfugue": "vt220",
  "cygwin": "ansi",
  "cygwinB19": "ansi",
  "cygwinDBG": "ansi",
  "gmud": "ansi",
  "mushclient": "ansi",
  "tintin++": "vt100",
  "kildclient": "ansi",
  "kmuddy": "ansi",
]);

/* How to format Pinkfish using the named terminal */
private mapping terms = ([
  TERMINAL_DUMB: ([
    "RESET": CHAR(8),
    "BOLD": "",
    "UNDERLINE": "",
    "FLASH": "",
    "BLINK": "",
    "REVERSE": "",
    "NORMAL": "",

    "BLACK": "",
    "RED": "",
    "GREEN": "",
    "ORANGE": "",
    "YELLOW": "",
    "BLUE": "",
    "MAGENTA": "",
    "CYAN": "",
    "WHITE": "",
    "DEFAULT": "",

    "B_BLACK": "",
    "B_RED": "",
    "B_GREEN": "",
    "B_ORANGE": "",
    "B_YELLOW": "",
    "B_BLUE": "",
    "B_MAGENTA": "",
    "B_CYAN": "",
    "B_WHITE": "",
    "B_DEFAULT": "",

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "ansi": ([
    "RESET": SGR("40;37")+SGR("0;10"),          /* Set defaults + all attributes off */
    "BOLD": SGR(1),
    "UNDERLINE": SGR(4),                        /* Not widely supported */
    "FLASH": SGR(5),
    "BLINK": SGR(5),
    "REVERSE": SGR(7),                          /* Swap fg and bg */
    "NORMAL": SGR(22),                          /* Normal intensity */

    "BLACK": SGR(30),
    "RED": SGR(31),
    "GREEN": SGR(32),
    "ORANGE": SGR(33),                          /* Low intensity yellow */
    "YELLOW": SGR("1;33"),
    "BLUE": SGR(34),
    "MAGENTA": SGR(35),
    "CYAN": SGR(36),
    "WHITE": SGR(37),
    "DEFAULT": SGR(39),

    "B_BLACK": SGR(40),
    "B_RED": SGR(41),
    "B_GREEN": SGR(42),
    "B_ORANGE": SGR(43),
    "B_YELLOW": SGR("1;43"),
    "B_BLUE": SGR(44),
    "B_MAGENTA": SGR(45),
    "B_CYAN": SGR(46),
    "B_WHITE": SGR(47),
    "B_DEFAULT": SGR(49),

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "ansi-no-flash": ([
    "RESET": SGR("40;37")+SGR("0;10"),          /* Set defaults + all attributes off */
    "BOLD": SGR(1),
    "UNDERLINE": SGR(4),                        /* Not widely supported */
    "FLASH": "",
    "BLINK": "",
    "REVERSE": SGR(7),                          /* Swap fg and bg */
    "NORMAL": SGR(22),                          /* Normal intensity */

    "BLACK": SGR(30),
    "RED": SGR(31),
    "GREEN": SGR(32),
    "ORANGE": SGR(33),                          /* Low intensity yellow */
    "YELLOW": SGR("1;33"),
    "BLUE": SGR(34),
    "MAGENTA": SGR(35),
    "CYAN": SGR(36),
    "WHITE": SGR(37),
    "DEFAULT": SGR(39),

    "B_BLACK": SGR(40),
    "B_RED": SGR(41),
    "B_GREEN": SGR(42),
    "B_ORANGE": SGR(43),
    "B_YELLOW": SGR("1;43"),
    "B_BLUE": SGR(44),
    "B_MAGENTA": SGR(45),
    "B_CYAN": SGR(46),
    "B_WHITE": SGR(47),
    "B_DEFAULT": SGR(49),

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "ansi-status": ([
    "RESET": SGR("40;37")+SGR("0;10"),          /* Set defaults + all attributes off */
    "BOLD": SGR(1),
    "UNDERLINE": SGR(4),                        /* Not widely supported */
    "FLASH": SGR(5),
    "BLINK": SGR(5),
    "REVERSE": SGR(7),                          /* Swap fg and bg */
    "NORMAL": SGR(22),                          /* Normal intensity */

    "BLACK": SGR(30),
    "RED": SGR(31),
    "GREEN": SGR(32),
    "ORANGE": SGR(33),                          /* Low intensity yellow */
    "YELLOW": SGR("1;33"),
    "BLUE": SGR(34),
    "MAGENTA": SGR(35),
    "CYAN": SGR(36),
    "WHITE": SGR(37),
    "DEFAULT": SGR(39),

    "B_BLACK": SGR(40),
    "B_RED": SGR(41),
    "B_GREEN": SGR(42),
    "B_ORANGE": SGR(43),
    "B_YELLOW": SGR("1;43"),
    "B_BLUE": SGR(44),
    "B_MAGENTA": SGR(45),
    "B_CYAN": SGR(46),
    "B_WHITE": SGR(47),
    "B_DEFAULT": SGR(49),

    "STATUS": ANSI("23;24r")+ESC(8),
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "xterm": ([
    "RESET": SGR("40;37")+ESC("(B")+ANSI("m"),  /* Set defaults + all attributes off */
    "BOLD": SGR(1),
    "UNDERLINE": SGR(4),                        /* Not widely supported */
    "FLASH": SGR(5),
    "BLINK": SGR(5),
    "REVERSE": SGR(7),                          /* Swap fg and bg */
    "NORMAL": SGR(22),                          /* Normal intensity */

    "BLACK": SGR(30),
    "RED": SGR(31),
    "GREEN": SGR(32),
    "ORANGE": SGR(33),                          /* Low intensity yellow */
    "YELLOW": SGR("1;33"),
    "BLUE": SGR(34),
    "MAGENTA": SGR(35),
    "CYAN": SGR(36),
    "WHITE": SGR(37),
    "DEFAULT": SGR(39),

    "B_BLACK": SGR(40),
    "B_RED": SGR(41),
    "B_GREEN": SGR(42),
    "B_ORANGE": SGR(43),
    "B_YELLOW": SGR("1;43"),
    "B_BLUE": SGR(44),
    "B_MAGENTA": SGR(45),
    "B_CYAN": SGR(46),
    "B_WHITE": SGR(47),
    "B_DEFAULT": SGR(49),

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "vt100": ([
    "RESET": SGR("40;37")+ANSI("m")+CHAR(15),   /* Set defaults + all attributes off */
    "BOLD": SGR(1),
    "UNDERLINE": SGR(4),                        /* Not widely supported */
    "FLASH": SGR(5),
    "BLINK": SGR(5),
    "REVERSE": SGR(7),                          /* Swap fg and bg */
    "NORMAL": SGR(22),                          /* Normal intensity */

    "BLACK": SGR(30),
    "RED": SGR(31),
    "GREEN": SGR(32),
    "ORANGE": SGR(33),                          /* Low intensity yellow */
    "YELLOW": SGR("1;33"),
    "BLUE": SGR(34),
    "MAGENTA": SGR(35),
    "CYAN": SGR(36),
    "WHITE": SGR(37),
    "DEFAULT": SGR(39),

    "B_BLACK": SGR(40),
    "B_RED": SGR(41),
    "B_GREEN": SGR(42),
    "B_ORANGE": SGR(43),
    "B_YELLOW": SGR("1;43"),
    "B_BLUE": SGR(44),
    "B_MAGENTA": SGR(45),
    "B_CYAN": SGR(46),
    "B_WHITE": SGR(47),
    "B_DEFAULT": SGR(49),

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "vt220": ([
    "RESET": SGR("40;37")+ANSI("m")+ESC(")B"),  /* Set defaults + all attributes off */
    "BOLD": SGR(1),
    "UNDERLINE": SGR(4),                        /* Not widely supported */
    "FLASH": SGR(5),
    "BLINK": SGR(5),
    "REVERSE": SGR(7),                          /* Swap fg and bg */
    "NORMAL": SGR(22),                          /* Normal intensity */

    "BLACK": SGR(30),
    "RED": SGR(31),
    "GREEN": SGR(32),
    "ORANGE": SGR(33),                          /* Low intensity yellow */
    "YELLOW": SGR("1;33"),
    "BLUE": SGR(34),
    "MAGENTA": SGR(35),
    "CYAN": SGR(36),
    "WHITE": SGR(37),
    "DEFAULT": SGR(39),

    "B_BLACK": SGR(40),
    "B_RED": SGR(41),
    "B_GREEN": SGR(42),
    "B_ORANGE": SGR(43),
    "B_YELLOW": SGR("1;43"),
    "B_BLUE": SGR(44),
    "B_MAGENTA": SGR(45),
    "B_CYAN": SGR(46),
    "B_WHITE": SGR(47),
    "B_DEFAULT": SGR(49),

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "vt100-nc": ([
    "RESET": ANSI("m")+CHAR(15),                /* Set defaults + all attributes off */
    "BOLD": SGR(1),
    "UNDERLINE": SGR(4),                        /* Not widely supported */
    "FLASH": SGR(5),
    "BLINK": SGR(5),
    "REVERSE": SGR(7),                          /* Swap fg and bg */
    "NORMAL": SGR(22),                          /* Normal intensity */

    "BLACK": SGR(30),
    "RED": SGR("37;7"),
    "GREEN": SGR(1),
    "ORANGE": SGR(37),
    "YELLOW": SGR("1;4"),
    "BLUE": SGR(37),
    "MAGENTA": SGR(37),
    "CYAN": SGR(4),
    "WHITE": SGR(37),
    "DEFAULT": SGR(37),

    "B_BLACK": SGR(40),
    "B_RED": SGR(41),
    "B_GREEN": SGR(42),
    "B_ORANGE": SGR(43),
    "B_YELLOW": SGR("1;43"),
    "B_BLUE": SGR(44),
    "B_MAGENTA": SGR(45),
    "B_CYAN": SGR(46),
    "B_WHITE": SGR(47),
    "B_DEFAULT": SGR(49),

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "vt220-nc": ([
    "RESET": ANSI("m")+ESC("(B"),               /* Set defaults + all attributes off */
    "BOLD": SGR(1),
    "UNDERLINE": SGR(4),                        /* Not widely supported */
    "FLASH": SGR(5),
    "BLINK": SGR(5),
    "REVERSE": SGR(7),                          /* Swap fg and bg */
    "NORMAL": SGR(22),                          /* Normal intensity */

    "BLACK": SGR(30),
    "RED": SGR("37;7"),
    "GREEN": SGR(1),
    "ORANGE": SGR(37),
    "YELLOW": SGR("1;4"),
    "BLUE": SGR(37),
    "MAGENTA": SGR("7;4"),
    "CYAN": SGR(4),
    "WHITE": SGR(37),
    "DEFAULT": SGR(37),

    "B_BLACK": SGR(40),
    "B_RED": SGR(41),
    "B_GREEN": SGR(42),
    "B_ORANGE": SGR(43),
    "B_YELLOW": SGR("1;43"),
    "B_BLUE": SGR(44),
    "B_MAGENTA": SGR(45),
    "B_CYAN": SGR(46),
    "B_WHITE": SGR(47),
    "B_DEFAULT": SGR(49),

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "hpterm": ([
    "RESET": ESC("&v0S")+ESC("&d@"),
    "BOLD": ESC("&dB"),
    "UNDERLINE": ESC("&dD"),
    "FLASH": "",
    "BLINK": "",
    "REVERSE": ESC("&dB"),
    "NORMAL": "",

    "BLACK": "",
    "RED": ESC("&dB"),
    "GREEN": ESC("&dD"),
    "ORANGE": "",
    "YELLOW": ESC("&dB"),
    "BLUE": "",
    "MAGENTA": "",
    "CYAN": "",
    "WHITE": "",
    "DEFAULT": "",

    "B_BLACK": "",
    "B_RED": "",
    "B_GREEN": "",
    "B_ORANGE": "",
    "B_YELLOW": "",
    "B_BLUE": "",
    "B_MAGENTA": "",
    "B_CYAN": "",
    "B_WHITE": "",
    "B_DEFAULT": "",

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "aixterm": ([
    "RESET": SGR("32")+SGR("40")+SGR("0;10")+ESC("(B"),
    "BOLD": SGR(1),
    "UNDERLINE": ESC('W'),
    "FLASH": "",
    "BLINK": "",
    "REVERSE": ESC('p'),
    "NORMAL": "",

    "BLACK": "",
    "RED": SGR(1),
    "GREEN": ESC('W'),
    "ORANGE": "",
    "YELLOW": ESC('p'),
    "BLUE": "",
    "MAGENTA": "",
    "CYAN": "",
    "WHITE": "",
    "DEFAULT": "",

    "B_BLACK": "",
    "B_RED": "",
    "B_GREEN": "",
    "B_ORANGE": "",
    "B_YELLOW": "",
    "B_BLUE": "",
    "B_MAGENTA": "",
    "B_CYAN": "",
    "B_WHITE": "",
    "B_DEFAULT": "",

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "ibm3151": ([
    "RESET": ESC("4@")+ESC(">B"),
    "BOLD": ESC("4H"),
    "UNDERLINE": ESC("4B"),
    "FLASH": ESC("4D"),
    "BLINK": ESC("4D"),
    "REVERSE": ESC("4A"),
    "NORMAL": "",

    "BLACK": "",
    "RED": ESC("4H"),
    "GREEN": ESC("4B"),
    "ORANGE": "",
    "YELLOW": ESC("4A"),
    "BLUE": "",
    "MAGENTA": "",
    "CYAN": "",
    "WHITE": "",
    "DEFAULT": "",

    "B_BLACK": "",
    "B_RED": "",
    "B_GREEN": "",
    "B_ORANGE": "",
    "B_YELLOW": "",
    "B_BLUE": "",
    "B_MAGENTA": "",
    "B_CYAN": "",
    "B_WHITE": "",
    "B_DEFAULT": "",

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
  "freedom": ([
    "RESET": ESC("G0"),
    "BOLD": ESC("G@"),
    "UNDERLINE": ESC("G8"),
    "FLASH": ESC("G2"),
    "BLINK": ESC("G2"),
    "REVERSE": ESC("G4"),
    "NORMAL": "",

    "BLACK": "",
    "RED": "",
    "GREEN": "",
    "ORANGE": "",
    "YELLOW": "",
    "BLUE": "",
    "MAGENTA": "",
    "CYAN": "",
    "WHITE": "",
    "DEFAULT": "",

    "B_BLACK": ESC("GD"),
    "B_RED": ESC("GD"),
    "B_GREEN": ESC("GD"),
    "B_ORANGE": ESC("G4"),
    "B_YELLOW": ESC("G4"),
    "B_BLUE": ESC("G4"),
    "B_MAGENTA": ESC("G4"),
    "B_CYAN": ESC("GD"),
    "B_WHITE": ESC("G4"),
    "B_DEFAULT": ESC("G4"),

    "STATUS": "",
    "WINDOW": "",
    "INITTERM": "",
    "ENDTERM": "",

    "%": "%^",
  ]),
]);

string array query_terms()
{
    return copy(keys(terms));
}

mapping query_term_info(string terminal)
{
    string term = term_aliases[terminal];

    if (!stringp(term)) term = terminal;
    if (mapp(terms[term])) return copy(terms[term]);

    return UNDEFINED_VALUE; /* Defined as ([ ])[0] */
}

string query_user_term(object user)
{
    if (!objectp(user)) return TERMINAL_DEFAULT;
    if (user->query_env("term")) return user->query_env("term");
    else if (user->query("term")) return user->query("term");

    return TERMINAL_DEFAULT;
}

int query_user_width(object user)
{
    if (!objectp(user)) return 0;
    if (user->query_env("wrap")) return to_int(user->query_env("wrap"));
    else if (user->query("columns")) return to_int(user->query("columns"));

    return WRAP_DEFAULT;
}

int query_user_indent(object user)
{
    if (!objectp(user)) return 0;
    if (user->query_env("indent")) return to_int(user->query_env("indent"));

    return INDENT_DEFAULT;
}

/* For the moment these next 2 functions use simple logic. In the future
 * I will update them to use mappings so that you can just call
 *  parse_to_imc2(string str)
 *  parse_from_imc2(string str)
 *
 * Could probably do with going in a separate file. Same as parse_pinkfish().
 */
string pinkfish_to_imc2(string str)
{
    // Foreground
    str = replace_string(str, "%^BLACK%^", "~x"); // Black
    str = replace_string(str, "%^RED%^", "~R"); // Red
    str = replace_string(str, "%^GREEN%^", "~G"); // Green
    str = replace_string(str, "%^BLUE%^", "~B"); // Blue
    str = replace_string(str, "%^WHITE%^", "~W"); // White
    str = replace_string(str, "%^ORANGE%^", "~y"); // Orange
    str = replace_string(str, "%^CYAN%^", "~c"); // Cyan
    str = replace_string(str, "%^YELLOW%^", "~Y"); // Yellow
    str = replace_string(str, "%^MAGENTA%^", "~p"); // Magenta -> purple sounds closest
    str = replace_string(str, "%^GRAY%^", "~w"); // Gray doesn't display on my MUD, bah :(
    // Background
    str = replace_string(str, "%^B_BLACK%^", "^x");
    str = replace_string(str, "%^B_RED%^", "^R"); // Red
    str = replace_string(str, "%^B_GREEN%^", "^G"); // Green
    str = replace_string(str, "%^B_BLUE%^", "^b"); // Blue
    str = replace_string(str, "%^B_WHITE%^", "^W"); // White
    str = replace_string(str, "%^B_ORANGE%^", "^O"); // Orange
    str = replace_string(str, "%^B_CYAN%^", "^c"); // Cyan
    str = replace_string(str, "%^B_YELLOW%^", "^Y"); // Yellow
    str = replace_string(str, "%^B_MAGENTA%^", "^p"); // Magenta -> purple sounds closest
    // Misc.
    str = replace_string(str, "%^FLASH%^", "~$"); // Flash -> Blink
    str = replace_string(str, "%^BOLD%^", "~L"); // Bold
    str = replace_string(str, "%^RESET%^", "~!");
    //Replace anything that was done in %^BOLD%^RED%^ format and wasn't already caught
    str = replace_string(str, "%^FLASH", "~$"); // Flash -> Blink
    str = replace_string(str, "%^BOLD", "~L"); // Bold
    str = replace_string(str, "%^RESET", "~!");
    str = replace_string(str, "FLASH%^", "~$"); // Flash -> Blink
    str = replace_string(str, "BOLD%^", "~L"); // Bold
    str = replace_string(str, "RESET%^", "~!");
    return str;
}

string imc2_to_pinkfish(string str)
{
    string output = "";
    int sz;
    /*
     * For colors explanation, refer to IMC Packet Documentation by Xorith.
     * Thanks very much for putting that out, by the way. :)
     * Found at http://hub00.muddomain.com/imc2_protocol_doc.txt
     */
    sz = sizeof(str) - 1;

    while (sizeof(str) > 1)
    {
        switch (str[0])
        {
            case '~': // Foreground
                switch (str[1])
                {
                    case 'Z': break; // Random
                    case 'x': output += "%^BLACK%^"; break; // Black
                    case 'r': output += "%^RED%^"; break; // Dark Red
                    case 'g': output += "%^GREEN%^"; break; // Dark Green
                    case 'y': output += "%^ORANGE%^"; break; // Orange
                    case 'b': output += "%^BLUE%^"; break; // Dark Blue
                    case 'p': output += "%^MAGENTA%^"; break; // Purple
                    case 'c': output += "%^CYAN%^"; break; // Cyan
                    case 'w': output += "%^WHITE%^"; break; // Grey
                    case 'D': output += "%^BOLD%^BLACK%^"; break; // Dark Grey
                    case 'z': output += "%^BOLD%^BLACK%^"; break; // Same as ~D
                    case 'R': output += "%^BOLD%^RED%^"; break; // Red
                    case 'G': output += "%^BOLD%^GREEN%^"; break; // Green
                    case 'Y': output += "%^BOLD%^YELLOW%^"; break; // Yellow
                    case 'B': output += "%^BOLD%^BLUE%^"; break; // Blue
                    case 'P': output += "%^BOLD%^MAGENTA%^"; break; // Pink
                    case 'C': output += "%^BOLD%^CYAN%^"; break; // Light Blue
                    case 'W': output += "%^BOLD%^WHITE%^"; break; // White

                    case 'm': output += "%^MAGENTA%^"; break; // same as p
                    case 'd': output += "%^WHITE%^"; break; // same as w
                    case 'M': output += "%^BOLD%^MAGENTA%^"; break; // same as P
                      // Misc.
                    case '!': output += "%^RESET%^"; break; // Reset
                    case 'L': output += "%^BOLD%^"; break; // Bold
                    case 'u': break; // Underline
                    case '$': output += "%^FLASH%^"; break; // Blink
                    case 'i': break; // Italic
                    case 'v': break; // Reverse
                    case 's': break; // Strike-thru

                    case '~': output += "~"; break; // ~~ prints as ~
                    default : output += "~"; // Don't skip over this
                        // (cheap hack is to add a character in front so the [2..] thing still works)
                        str = " " + str;
                        break;
                }
                str = str[2..];
                break;
            case '^':  // Background
                switch (str[1])
                {
                    case 'Z': break; // Random
                    case 'x': output += "%^B_BLACK%^"; break; // Black
                    case 'r': output += "%^B_RED%^"; break; // Dark Red
                    case 'g': output += "%^B_GREEN%^"; break; // Dark Green
                    case 'O': output += "%^B_ORANGE%^"; break; // Orange
                    case 'B': output += "%^B_BLUE%^"; break; // Dark Blue
                    case 'p': output += "%^B_MAGENTA%^"; break; // Purple
                    case 'c': output += "%^B_CYAN%^"; break; // Cyan
                    case 'w': output += "%^B_WHITE%^"; break; // Grey
                    case 'z': output += "%^B_BLACK%^"; break; // Dark Grey
                    case 'R': output += "%^B_RED%^"; break; // Red
                    case 'G': output += "%^B_GREEN%^"; break; // Green
                    case 'Y': output += "%^B_YELLOW%^"; break; // Yellow
                    case 'b': output += "%^B_BLUE%^"; break; // Blue
                    case 'P': output += "%^B_MAGENTA%^"; break; // Pink
                    case 'C': output += "%^B_BLUE%^"; break; // Light Blue
                    case 'W': output += "%^B_WHITE%^"; break; // White
                    case '^': output += "^"; break; // ^^ prints as ^
                    default : output += "^"; // Don't skip over this
                        // (cheap hack is to add a character in front so the [2..] thing still works)
                        str = " " + str;
                        break;
                }
                str = str[2..];
                break;
            case '`': // Blinking Foreground
                switch (str[1])
                {
                    case 'Z': output += "%^FLASH%^"; break; // Random
                    case 'x': output += "%^FLASH%^BLACK%^"; break; // Black
                    case 'r': output += "%^FLASH%^RED%^"; break; // Dark Red
                    case 'g': output += "%^FLASH%^GREEN%^"; break; // Dark Green
                    case 'O': output += "%^FLASH%^ORANGE%^"; break; // Orange
                    case 'b': output += "%^FLASH%^BLUE%^"; break; // Dark Blue
                    case 'p': output += "%^FLASH%^MAGENTA%^"; break; // Purple
                    case 'c': output += "%^FLASH%^CYAN%^"; break; // Cyan
                    case 'w': output += "%^FLASH%^WHITE%^"; break; // Grey
                    case 'z': output += "%^FLASH%^BOLD%^BLACK%^"; break; // Dark Grey
                    case 'R': output += "%^FLASH%^BOLD%^RED%^"; break; // Red
                    case 'G': output += "%^FLASH%^BOLD%^GREEN%^"; break; // Green
                    case 'Y': output += "%^FLASH%^BOLD%^YELLOW%^"; break; // Yellow
                    case 'B': output += "%^FLASH%^BOLD%^BLUE%^"; break; // Blue
                    case 'P': output += "%^FLASH%^BOLD%^MAGENTA%^"; break; // Pink
                    case 'C': output += "%^FLASH%^BOLD%^CYAN%^"; break; // Light Blue
                    case 'W': output += "%^FLASH%^BOLD%^WHITE%^"; break; // White
                    case '`': output += "`"; break; // `` prints as `
                    default : output += "`"; // Don't skip over this
                        // (cheap hack is to add a character in front so the [2..] thing still works)
                        str = " " + str;
                        break;
                }
                str = str[2..];
                break;
            default:
                output += str[0..0];
                str = str[1..];
                break;
        }
    }
    output += str;

    output = replace_string(output, "\\n", "\n");
    output = replace_string(output, "\\r", "\r");
    output = replace_string(output, "\\\"", "'");
    output = replace_string(output, "\"", "");
    output = replace_string(output, "\\\\", "\\");

    return output;
}

varargs string parse_pinkfish(string msg, mixed terminal, int _wrap, int _indent)
{
    string user_term;
    int wrap, indent;

    if (!stringp(msg)) return "";

    /* Get the term name of previous_object(), usually this_player() */
    if (stringp(terminal) && member_array(terminal, keys(term_aliases)))
        user_term = terminal;
    else user_term = query_user_term(previous_object());

    user_term = lower_case(user_term);

    /* Get the wrap and indent of previous_object(), usually this_player() */
    if (!undefinedp(_wrap) && intp(_wrap)) wrap = _wrap;
    else wrap = query_user_width(previous_object());

    if (!undefinedp(_indent) && intp(_indent)) indent = _indent;
    else indent = query_user_indent(previous_object());

    /* If the flag is set then strip all Pinkfish codes */
    if (undefinedp(terminal) || stringp(terminal) || (intp(terminal) && !terminal))
    {
        string out, term = term_aliases[user_term];

        if (!stringp(term)) term = user_term;
        if (mapp(terms[term]))
        {
            out = terminal_colour(msg, terms[term], wrap, indent);

            /* Used to stop FluffOS inserting a TELNET_GA */
            if (term == TERMINAL_DUMB) out = replace_string(out, CHAR(8), "");

            return out;
        }
    }

    /*
     * If terminal is not known or 'terminal' is set to non-zero then strip
     * all Pinkfish codes.
     */
    /* Used to stop FluffOS inserting a TELNET_GA */
    return replace_string(
      terminal_colour(msg, terms[TERMINAL_DUMB], wrap, indent),
      CHAR(8),
      ""
    );
}

/* About the only thing left of Tacitus' code */
mapping unsafe_ansi = ([
  "HOMEC": "",
  "CLEAR": "",
  "RESETC": "",
  "SAVEC": "",
  "UNFR": "",
  "FRBOTTOM": "",
  "FRTOP": "",
  "ENDTERM": "",
  "INITTERM": "",
  "WINDOW": "",
  "STATUS": "",
  "CLEAR": "",
  "ER_SOL": "",
  "ER_EOL": "",
  "ER_LINE": "",
  "ER_DOWN": "",
  "ER_UP": "",
  "ER_LINE": "",
]);

string strip_unsafeAnsi(string msg)
{
    int i;

    for(i = 0; i < sizeof(unsafe_ansi); i++)
        msg = replace_string(msg, "%^" + unsafe_ansi[i] + "%^", "");

    return msg;
}
