string parseTokens(string text)
{
    return replace_string(text, "%mud_name", mud_name());
}

