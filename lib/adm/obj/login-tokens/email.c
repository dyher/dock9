string parseTokens(string text)
{
    return replace_string(text, "%email", admin_email() );
}

