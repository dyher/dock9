string parseTokens(string text)
{
    return replace_string(text, "%open_status", open_status() );
}

