string parseTokens(string text)
{
    return replace_string(text, "%lib_version", lib_version());
}

