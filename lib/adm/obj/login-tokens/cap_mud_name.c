string parseTokens(string text)
{
    return replace_string(text, "%cap_mud_name", implode(explode(mud_name(),
    	""), (: capitalize($1) + capitalize($2) :)));
}

