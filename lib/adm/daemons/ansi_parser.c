/* ansi_parser.c

 Tacitus @ LPUniversity
 17-JUNE-06
 Mudlib object

*/

string strip_unsafeAnsi(string arg)
{
    return load_object(TERMINAL_D)->strip_unsafeAnsi(arg);
}

varargs string parse_pinkfish(string msg, mixed flag)
{
    return load_object(TERMINAL_D)->parse_pinkfish(msg, flag);
}
