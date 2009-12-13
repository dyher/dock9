string parseTokens(string text)
{
    /* We subtract one to not count the logging in user. */
    int num_users = sizeof(users()) - 1;

    if (num_users <= 0)
        return replace_string(text, "%user_count", "no");
    else
        /* "" + var is quicker/cleaner than sprintf("%i", var) */
        return replace_string(text, "%user_count", "" + num_users);
}
