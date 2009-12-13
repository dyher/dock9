/* history_d.c
 *
 * Tricky
 * 8-OCT-2007
 * History daemon
 *
 */

#define DEFAULT_HISTORY_DIR  "/data/daemons/history_d"
#define HISTORY_DATA_FILE    "/data/daemons/history_d/history"
#define DEFAULT_HISTORY_SIZE 500

/* Private vars */
privatev mapping History, FileHistory;

/* Public vars */
mapping HistoryDir, HistorySize, ChannelIndex;

/* Private funcs */
privatef int save_daemon();
privatef int restore_daemon();

/* Public funcs */
mapping fix_chan_index();

/*
 * Function name: create
 * Description:   Initialise the object data.
 */
void create()
{
    History = ([ ]);
    FileHistory = ([ ]);

    restore_daemon();

    if (!HistoryDir) HistoryDir  = ([ ]);
    if (!HistorySize) HistorySize = ([ ]);
    if (!ChannelIndex) ChannelIndex = ([ ]);

    fix_chan_index();
    save_daemon();
}

/*
 * Function name: remove
 * Description:   Saves the object data and destructs itself.
 */
void remove()
{
    save_daemon();
    destruct();
}

/*
 * Function name: set_history_dir
 * Description:   Set the history directory of the specified channel.
 * Arguments:     channel - Channel name.
 *
 *                Optional arguments:-
 *                dir - Directory path.
 */
varargs void set_history_dir(string channel, string dir)
{
    if (undefinedp(dir) || !stringp(dir) || dir == "")
        HistoryDir[channel] = DEFAULT_HISTORY_DIR;
    else HistoryDir[channel] = dir;
}

/*
 * Function name: query_history_dir
 * Description:   Queries the history directory of the specified channel.
 * Arguments:     channel - Channel name.
 * Return:        History directory path.
 */
string query_history_dir(string channel)
{
    if (member_array(channel, keys(HistoryDir)) == -1)
        return DEFAULT_HISTORY_DIR;

    return HistoryDir[channel];
}

/*
 * Function name: set_history_size
 * Description:   Set the history size of the specified channel.
 * Arguments:     channel - Channel name.
 *
 *                Optional arguments:-
 *                size - Size of history buffer.
 */
varargs void set_history_size(string channel, int size)
{
    if (undefinedp(size) || !intp(size) || size == 0)
        HistorySize[channel] = DEFAULT_HISTORY_SIZE;
    else HistorySize[channel] = size;
}

/*
 * Function name: query_history_size
 * Description:   Queries the history size of the specified channel.
 * Arguments:     channel - Channel name.
 * Return:        History buffer size.
 */
int query_history_size(string channel)
{
    if (member_array(channel, keys(HistorySize)) == -1)
        return DEFAULT_HISTORY_SIZE;

    return HistorySize[channel];
}

/*
 * Function name: set_chan_index
 * Description:   Set the index number of the specified channel.
 * Arguments:     channel - Channel name.
 *                index - Channel index.
 */
void set_chan_index(string channel, int index)
{
    ChannelIndex[channel] = index;
    save_daemon();
}

/*
 * Function name: remove_chan_index
 * Description:   Removes the index number of the specified channel.
 * Arguments:     channel - Channel name.
 */
void remove_chan_index(string channel)
{
    map_delete(ChannelIndex, channel);
    save_daemon();
}

/*
 * Function name: query_chan_index
 * Description:   Queries the index number of the specified channel.
 * Arguments:     channel - Channel name.
 * Return:        Channel index or -1 if it doesn't exist.
 */
int query_chan_index(string channel)
{
    if (ChannelIndex && member_array(channel, keys(ChannelIndex)) != -1)
        return ChannelIndex[channel];

    return -1;
}

/*
 * Function name: set_chan_indexes
 * Description:   Set the index number of all channels.
 * Arguments:     index - Channel index.
 */
void set_chan_indexes(int index)
{
    foreach (string channel in keys(ChannelIndex))
        set_chan_index(channel, index);
}

/*
 * Function name: fix_chan_index
 * Description:   Fix the channel index mapping.
 * Return:        Fixed channel index mapping.
 */
mapping fix_chan_index()
{
    mapping new_channel_index = ([ ]);

    if (!mapp(ChannelIndex) || sizeof(ChannelIndex) < 1) return 0;

    foreach (string channel, int index in ChannelIndex)
    {
        mixed array files;
        string basedir;
        int tmp;

        basedir = query_history_dir(channel) + "/" + channel + "-";
        files = get_dir(basedir + "*");

        if (!arrayp(files) || sizeof(files) < 1) continue;

        files = sort_array(files, 1);

        if (sizeof(files) < 1) continue;

        tmp = to_int(files[<1][strsrch(files[<1], "-", -1) + 1..<1]);
        new_channel_index[channel] = index;

        if (tmp == index) continue;

        new_channel_index[channel] = tmp;
    }

    ChannelIndex = copy(new_channel_index);
    save_daemon();
    return copy(ChannelIndex);
}

/*
 * Function name: set_history
 * Description:   Initialises the history buffers.
 * Arguments:     channel - History channel.
 *                message - Array of text messages.
 *
 *                Optional arguments:-
 *                append - If set then append the messages to the end of the buffer.
 */
varargs void set_history(string channel, string array message, int append)
{
    int size;

    if (member_array(channel, keys(History)) == -1)
        History[channel] = ({ });
    if (member_array(channel, keys(FileHistory)) == -1)
        FileHistory[channel] = ({ });

    if (!undefinedp(append))
    {
        History[channel] += message;
        FileHistory[channel] += message;
    }
    else
    {
        History[channel] = message;
        FileHistory[channel] = message;
    }

    if (member_array(channel, keys(HistorySize)) == -1)
        size = DEFAULT_HISTORY_SIZE;
    else size = HistorySize[channel];

    if (sizeof(History[channel]) > size)
        History[channel] = History[channel][sizeof(History[channel]) - size..<1];
    if (sizeof(FileHistory[channel]) > size)
        FileHistory[channel] = FileHistory[channel][size..<1];

    save_daemon();
}

/*
 * Function name: add_history
 * Description:   Add messages to the history buffer.
 * Arguments:     channel - History channel.
 *                message - Array of text messages.
 */
void add_history(string channel, string array message)
{
    string array msg = ({ });
    string saveVar = "", dir;
    int size;

    if (member_array(channel, keys(History)) == -1)
        History[channel] = ({ });
    if (member_array(channel, keys(FileHistory)) == -1)
        FileHistory[channel] = ({ });

    foreach (string line in message)
        msg += ({ sprintf("%s %s", ctime(time()), line) });

    History[channel] += msg;
    FileHistory[channel] += msg;

    if (member_array(channel, keys(HistoryDir)) == -1)
        dir = DEFAULT_HISTORY_DIR;
    else dir = HistoryDir[channel];

    if (member_array(channel, keys(HistorySize)) == -1)
        size = DEFAULT_HISTORY_SIZE;
    else size = HistorySize[channel];

    if (sizeof(History[channel]) > size)
        History[channel] = History[channel][sizeof(History[channel]) - size..<1];

    if (sizeof(FileHistory[channel]) > size)
    {
        foreach (string line in FileHistory[channel][0..size - 1])
            saveVar += line;

        write_file(dir + "/" + channel + "-" + sprintf("%08i", ChannelIndex[channel]), saveVar, 1);
        ChannelIndex[channel]++;
        saveVar = "";
        FileHistory[channel] = FileHistory[channel][size..<1];

        foreach (string line in FileHistory[channel]) saveVar += line;
    }
    else foreach (string line in msg) saveVar += line;

    write_file(dir + "/" + channel + "-" + sprintf("%08i", ChannelIndex[channel]), saveVar);
    save_daemon();
}

/*
 * Function name: rem_history
 * Description:   Remove a history buffer from the mapping.
 * Arguments:     channel - History channel.
 */
void rem_history(string channel)
{
    if (member_array(channel, keys(History)) != -1)
        map_delete(History, channel);
}

/*
 * Function name: rem_file_history
 * Description:   Remove a file history buffer from the mapping.
 * Arguments:     channel - History channel.
 */
void rem_file_history(string channel)
{
    if (member_array(channel, keys(FileHistory)) != -1)
        map_delete(FileHistory, channel);
}

/*
 * Function name: get_history
 * Description:   Gets the history mapping.
 * Return:        A copy of the history mapping.
 */
mapping get_history()
{
    return copy(History);
}

/*
 * Function name: get_file_history
 * Description:   Gets the file history mapping.
 * Return:        A copy of the file history mapping.
 */
mapping get_file_history()
{
    return copy(FileHistory);
}

/*
 * Function name: load_channel_history
 * Description:   Loads a channel's history and initializes the mappings.
 * Arguments:     channel - History channel.
 */
void load_channel_history(string channel)
{
    string indexStr;
    int index;

    if (!ChannelIndex) ChannelIndex = ([ channel: 0 ]);
    else if (member_array(channel, keys(ChannelIndex)) == -1)
        ChannelIndex[channel] = 0;

    index = ChannelIndex[channel];

    if (index > 0)
    {
        indexStr = sprintf("%08i", index - 1);

        if (file_exists(query_history_dir(channel) + "/" + channel + "-" + indexStr))
        {
            string array arr_lines = ({ });
            string lines = read_file(query_history_dir(channel) + "/" + channel + "-" + indexStr);

            foreach (string line in explode(lines, "\n"))
                arr_lines += ({ line + "\n" });

            set_history(channel, arr_lines);
        }
        else
            set_history(channel, ({ }) );
    }

    indexStr = sprintf("%08i", index);

    if (file_exists(query_history_dir(channel) + "/" + channel + "-" + indexStr))
    {
        string array arr_lines = ({ });
        string lines = read_file(query_history_dir(channel) + "/" + channel + "-" + indexStr);

        foreach (string line in explode(lines, "\n"))
            arr_lines += ({ line + "\n" });

        set_history(channel, arr_lines, 1);
    }
    else
        set_history(channel, ({ }), 1);

    save_daemon();
}

/*
 * Function name: save_daemon
 * Description:   Saves the object data.
 * Return:        Success or failure.
 */
privatef int save_daemon()
{
    if (!directory_exists(implode(explode(HISTORY_DATA_FILE, "/")[0..<2], "/")))
        mkdir(implode(explode(HISTORY_DATA_FILE, "/")[0..<2], "/"));
    return save_object(HISTORY_DATA_FILE);
}

/*
 * Function name: restore_daemon
 * Description:   Restores the object data.
 * Return:        Success or failure.
 */
privatef int restore_daemon()
{
    if (file_exists(HISTORY_DATA_FILE + ".o"))
        return restore_object(HISTORY_DATA_FILE);

    return 0;
}
