string parseTokens(string text)
{
    return replace_string(text, "%date", ctime(time()) );
}

