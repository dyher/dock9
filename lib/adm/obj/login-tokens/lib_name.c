string parseTokens(string text)
{
    return replace_string(text, "%lib_name", lib_name() );
}

