/* socket_d.c
 *
 * Tricky
 * 5-JAN-2007
 * MudOS socket daemon for LPUniLib
 *
 * Last edited on December 12th, 2007 by Tricky
 *
 */

/* Socket types and errors */
#include <socket.h>

/* Set this to a name you know will not be used as an ID */
#define SOCKET_ID "[SOCKET_D]"

/* Lower this if you want less write retries */
#define WRITERETRYLIMIT 20

/* Private vars */
privatev mapping Sockets, Ids;
privatev string ReadF, ListenF, CloseF, ReleaseF, LogFile;
privatev int SocketType;

/* Private funcs */
privatef void close_callback(int fd);
privatef void internal_close(int fd);
privatef void listen_callback(int fd);
privatef void client_read_callback(int fd, mixed data);
privatef void server_read_callback(int fd, mixed data);
privatef void write_callback(int fd);
privatef void internal_write(int fd, mixed data);
privatef void log(string sockID, string pre, string str);
privatef void error_log(string sockID, string str, int x);

/* Public funcs */
varargs int client_create(string sockID, string host, int port, int type, string read_cb, string close_cb, string log);
varargs int server_create(string sockID, int port, int type, string read_cb, string close_cb, string log);
void close(mixed fd, int delay);
void client_write(mixed fd, mixed data);
void server_write(mixed fd, mixed data, int close);
void set_socket_type(int type);
int query_socket_type(string sockID);
void set_read_cb(string func);
void set_listen_cb(string func);
void set_close_cb(string func);
void set_release_cb(string func);
void set_log_file(string file);
string query_log_file(string sockID);
mapping query_socket_info();

/*
 * Function name: create
 * Description:   Initialise the object data.
 */
void create()
{
    Sockets = ([
      SOCKET_ID: ([
        "logFile": "socket"
      ])
    ]);

    Ids = ([ ]);

    SocketType = -1;
    ReadF = 0;
    ListenF = 0;
    CloseF = 0;
    ReleaseF = 0;
    LogFile = 0;

    set_heart_beat(30);
    log(SOCKET_ID, "Success", "Created.");
}

/*
 * Function name: remove
 * Description:   Closes all sockets and destructs itself.
 */
void remove()
{
    log(SOCKET_ID, "Notice", "Removing all sockets.");

    foreach (string sockID, mapping sock in Sockets)
    {
        if (sockID == SOCKET_ID) continue;

        log(sockID, "Notice", "Removing socket.");
        internal_close(sock["fd"]);
    }

    log(SOCKET_ID, "Warning", "Destructing.");
    destruct();
}

/*
 * Function name: heart_beat
 * Description:   Cleans up orphaned file descriptors.
 */
void heart_beat()
{
    foreach (string sockID, mapping sock in Sockets)
    {
        if (sockID == SOCKET_ID) continue;
        if (objectp(sock["owner"])) continue;

        log(sockID, "Notice", "Removing orphaned socket.");
        internal_close(sock["fd"]);
    }
}

/*
 * Function name: client_create
 * Description:   Create a socket for clients and connects it to host IP and port.
 * Arguments:     sockID - Unique ID that can be used to reference the socket.
 *                host - Host IP to connect to.
 *                port - Host port number.
 *
 *                Optional arguments:-
 *                type - Socket Type.
 *                read_cb - User defined read callback function.
 *                close_cb - User defined close callback function.
 *                release_cb - User defined release callback function.
 *                log - Per-socket log file.
 * Return:        Socket creation success or error code.
 */
varargs int client_create(string sockID, string host, int port, int type, string read_cb, string close_cb, string release_cb, string log)
{
    mapping sock;
    int x;

    if (undefinedp(sockID) || !stringp(sockID)) return EESOCKET;
    if (undefinedp(host) || !stringp(host)) return EESOCKET;
    if (undefinedp(port) || !intp(port)) return EESOCKET;

    if (member_array(sockID, keys(Sockets)) != -1) return EEISCONN;

    if (!undefinedp(type) && intp(type)) SocketType = type;
    if (!undefinedp(read_cb) && stringp(read_cb)) ReadF = read_cb;
    if (!undefinedp(close_cb) && stringp(close_cb)) CloseF = close_cb;
    if (!undefinedp(release_cb) && stringp(release_cb)) ReleaseF = release_cb;
    if (!undefinedp(log) && stringp(log)) LogFile = log;

    /* If we haven't set a socket type then set it to STREAM */
    if (SocketType == -1) SocketType = STREAM;

    /* Initialise client socket data */
    sock = ([
      "fd": -1,
      "owner": previous_object(),
      "type": "Client",
      "blocking": 0,
      "writeRetry": 0,
      "closing": 0,
      "closed": 0,
      "socketType": SocketType,
      "readf": ReadF,
      "closef": CloseF,
      "releasef": ReleaseF,
      "logFile": LogFile,
    ]);

    Sockets[sockID] = copy(sock);

    /* Create a socket of SocketType */
    x = socket_create(SocketType, "client_read_callback", "close_callback");

    /* Reset everything */
    SocketType = -1;
    ReadF = 0;
    CloseF = 0;
    ReleaseF = 0;
    LogFile = 0;

    /* Couldn't create a socket */
    if (x < 0)
    {
        error_log(sockID, "Client/socket_create", x);
        map_delete(Sockets, sockID);
        return x;
    }

    /* Socket created and return value is a file descriptor we use */
    sock["fd"] = x;
    Ids[sock["fd"]] = sockID;
    log(sockID, "Success", "Created client socket (" + sock["fd"] + ")");

    /* Bind the socket to a system selected port */
    /* Binding isn't necessary for clients since the system *should* do it for you */
    x = socket_bind(sock["fd"], 0);

    /* Couldn't bind it */
    if (x < 0)
    {
        error_log(sockID, "Client/socket_bind", x);
        internal_close(sock["fd"]);
        return x;
    }

    log(sockID, "Success", "Client socket bound to a port.");

    /* Now we actually connect the socket to a host and port */
    x = socket_connect(sock["fd"], host + " " + port, "client_read_callback", "write_callback");

    /* Couldn't connect to remote machine */
    if (x < 0)
    {
        error_log(sockID, "Client/socket_connect", x);
        internal_close(sock["fd"]);
        return x;
    }

    Sockets[sockID] = copy(sock);
    log(sockID, "Success", "Connected client to " + host + " " + port);
    log(
      SOCKET_ID,
      "Notice",
      sprintf("Client created - %s/%d: %O", sockID, sock["fd"], Sockets[sockID])
    );

    /* Call the user function if one is set up otherwise do nothing */
    if (stringp(sock["releasef"]))
    {
        x = socket_release(sock["fd"], sock["owner"], sock["releasef"]);

        if (x != EESUCCESS)
        {
            error_log(sockID, "Client/socket_release", x);
            return x;
        }
    }

    /* Return the file descriptor on success */
    return sock["fd"];
}

/*
 * Function name: server_create
 * Description:   Create a socket for a server and bind it to a port.
 * Arguments:     sockID - Unique ID that can be used to reference the socket.
 *                port - Server port number.
 *
 *                Optional arguments:-
 *                type - Socket Type.
 *                read_cb - User defined read callback function.
 *                listen_cb - User defined listen callback function.
 *                close_cb - User defined close callback function.
 *                release_cb - User defined release callback function.
 *                log - Per-socket log file.
 * Return:        Socket creation success or error code.
 */
varargs int server_create(string sockID, int port, int type, string read_cb, string listen_cb, string close_cb, string release_cb, string log)
{
    mapping sock;
    int x;

    if (undefinedp(sockID) || !stringp(sockID)) return EESOCKET;
    if (undefinedp(port) || !intp(port)) return EESOCKET;

    if (member_array(sockID, keys(Sockets)) != -1) return EEISCONN;

    if (!undefinedp(type) && intp(type)) SocketType = type;
    if (!undefinedp(read_cb) && stringp(read_cb)) ReadF = read_cb;
    if (!undefinedp(listen_cb) && stringp(listen_cb)) ListenF = listen_cb;
    if (!undefinedp(close_cb) && stringp(close_cb)) CloseF = close_cb;
    if (!undefinedp(release_cb) && stringp(release_cb)) ReleaseF = release_cb;
    if (!undefinedp(log) && stringp(log)) LogFile = log;

    /* If we haven't set a socket type then set it to STREAM */
    if (SocketType == -1) SocketType = STREAM;

    /* Initialise server socket data */
    sock = ([
      "fd": -1,
      "owner": previous_object(),
      "type": "Server",
      "blocking": 0,
      "writeRetry": 0,
      "buffer": 0,
      "closing": 0,
      "closed": 0,
      "socketType": SocketType,
      "readf": ReadF,
      "listenf": ListenF,
      "closef": CloseF,
      "releasef": ReleaseF,
      "logFile": LogFile,
    ]);

    Sockets[sockID] = copy(sock);

    /* Create a socket of SocketType */
    x = socket_create(SocketType, "server_read_callback", "close_callback");

    /* Reset everything */
    SocketType = -1;
    ReadF = 0;
    ListenF = 0;
    CloseF = 0;
    ReleaseF = 0;
    LogFile = 0;

    /* Couldn't create a socket */
    if (x < 0)
    {
        error_log(sockID, "Server/socket_create", x);
        map_delete(Sockets, sockID);
        return x;
    }

    /* Socket created and return value is a file descriptor we use */
    sock["fd"] = x;
    Ids[sock["fd"]] = sockID;
    log(sockID, "Success", "Created server socket (" + sock["fd"] + ")");

    /* Bind the socket to the supplied port */
    x = socket_bind(sock["fd"], port);

    /* Couldn't bind it */
    if (x < 0)
    {
        error_log(sockID, "Server/socket_bind", x);
        internal_close(sock["fd"]);
        return x;
    }

    log(sockID, "Success", "Server socket bound to port " + port);

    /* Now we set up the listen callback */
    x = socket_listen(sock["fd"], "listen_callback");

    /* Couldn't set up the socket to listen */
    if (x < 0)
    {
        error_log(sockID, "Server/socket_listen", x);
        internal_close(sock["fd"]);
        return x;
    }

    Sockets[sockID] = copy(sock);
    log(sockID, "Success", "Server listen callback set up.");
    log(
      SOCKET_ID,
      "Notice",
      sprintf("Server created - %s/%d: %O", sockID, sock["fd"], Sockets[sockID])
    );

    /* Call the user function if one is set up otherwise do nothing */
    if (stringp(sock["releasef"]))
    {
        x = socket_release(sock["fd"], sock["owner"], sock["releasef"]);

        if (x != EESUCCESS)
        {
            error_log(sockID, "Server/socket_release", x);
            return x;
        }
    }

    /* Return the file descriptor on success */
    return sock["fd"];
}

/*
 * Function name: close_callback
 * Description:   Called when the connection terminates unexpectably.
 * Arguments:     fd - Socket file descriptor.
 */
privatef void close_callback(int fd)
{
    object o;
    string f;
    string sockID, type;

    if (member_array(fd, keys(Ids)) == -1) return;

    sockID = Ids[fd];
    type = Sockets[sockID]["type"];
    log(sockID, "Warning", type + " connection (" + fd + ") terminated.");

    /* Indicate that the socket is closed */
    Sockets[sockID]["closed"] = 1;

    o = Sockets[sockID]["owner"];
    f = Sockets[sockID]["closef"];

    /* Call the user function if one is set up */
    if (stringp(f)) call_other(o, f, fd);

    internal_close(fd);
}

/*
 * Function name: internal_close
 * Description:   Internal socket close.
 * Arguments:     fd - Socket file descriptor.
 */
privatef void internal_close(int fd)
{
    mapping sock;
    string sockID;
    int x;

    if (member_array(fd, keys(Ids)) == -1) return;

    sockID = Ids[fd];
    sock = copy(Sockets[sockID]);

    if (!sock)
    {
        /* Remove the socket data */
        map_delete(Sockets, sockID);
        map_delete(Ids, fd);
        return;
    }

    log(SOCKET_ID, "Warning", sock["type"] + " connection (" + sockID + "/" + sock["fd"] + ") closing.");

    if (!sock["closed"] && sock["buffer"])
    {
        call_out("internal_close", 2, fd);
        return;
    }

    if ((!sock["closed"] || sock["closing"]) && ((x = socket_close(sock["fd"])) != EESUCCESS))
        error_log(sockID, sock["type"] + "/internal_close", x);
    else
        log(sockID, "Success", sock["type"] + " connection (" + sock["fd"] + ") closed.");

    /* Remove the socket data */
    map_delete(Sockets, sockID);
    map_delete(Ids, fd);
}

/*
 * Function name: close
 * Description:   Public socket close.
 * Arguments:     fd - Socket file descriptor or ID.
 *
 *                Optional argument:-
 *                delay - Delays the closing of the socket for 'delay' seconds.
 */
void close(mixed fd, int delay)
{
    if (undefinedp(delay)) delay = 0;

    if (intp(fd))
    {
        if (member_array(fd, keys(Ids)) == -1) return;
        if (Sockets[Ids[fd]]["owner"] != previous_object())
        {
            log(Ids[fd], "Warning", "Illegal close by sockID '" + Ids[fd] + "' in object '" + Sockets[Ids[fd]]["owner"] + "'\n");
            return;
        }

        call_out("internal_close", delay, fd);
    }
    else if (stringp(fd))
    {
        if (member_array(fd, keys(Sockets)) == -1) return;
        if (Sockets[fd]["owner"] != previous_object())
        {
            log(fd, "Warning", "Illegal close by sockID '" + fd + "' in object '" + Sockets[fd]["owner"] + "'\n");
            return;
        }

        call_out("internal_close", delay, Sockets[fd]["fd"]);
    }
}

/*
 * Function name: listen_callback
 * Description:   Called when the socket receives an incoming connection.
 * Arguments:     fd - Socket file descriptor.
 */
privatef void listen_callback(int fd)
{
    object o;
    mapping sock;
    string sockID, f;
    int x;

    sockID = Ids[fd];

    /* Accept the incoming connection */
    x = socket_accept(Sockets[sockID]["fd"], "server_read_callback", "write_callback");

    /* Couldn't accept the incoming connection */
    if (x < 0)
    {
        error_log(sockID, "Listen/socket_accept", x);
        internal_close(fd);
        return;
    }

    /* Initialise remote socket data */
    sock = ([
      "fd": x,
      "owner": Sockets[sockID]["owner"],
      "type": "Remote",
      "blocking": 0,
      "buffer": 0,
      "closing": 0,
      "closed": 0,
      "readf": Sockets[sockID]["readf"],
      "closef": Sockets[sockID]["closef"],
      "logFile": Sockets[sockID]["logFile"],
    ]);

    Sockets[sockID + "." + sock["fd"]] = copy(sock);
    Ids[sock["fd"]] = sockID + "." + sock["fd"];
    log(sockID, "Success", "Accepted the incoming connection from " + socket_address(sock["fd"]));

    o = Sockets[sockID]["owner"];
    f = Sockets[sockID]["listenf"];

    /* Call the user function if one is set up otherwise do nothing */
    if (stringp(f)) call_other(o, f, fd, sock["fd"]);
}

/*
 * Function name: client_read_callback
 * Description:   Called when data arrives on the client socket.
 * Arguments:     fd - Socket file descriptor.
 *                data - Incoming data.
 */
privatef void client_read_callback(int fd, mixed data)
{
    string sockID = Ids[fd];
    object o = Sockets[sockID]["owner"];
    string f = Sockets[sockID]["readf"];

    /* Call the user function if one is set up otherwise do nothing */
    if (stringp(f)) call_other(o, f, fd, data);
}

/*
 * Function name: server_read_callback
 * Description:   Called when data arrives on the server socket.
 * Arguments:     fd - Socket file descriptor.
 *                data - Incoming data.
 */
privatef void server_read_callback(int fd, mixed data)
{
    string sockID = Ids[fd];
    object o = Sockets[sockID]["owner"];
    string f = Sockets[sockID]["readf"];

    /* Call the user function if one is set up otherwise do nothing */
    if (stringp(f)) call_other(o, f, fd, data);
}

/*
 * Function name: write_callback
 * Description:   Called when data is ready to be written to the socket.
 * Arguments:     fd - Socket file descriptor.
 */
privatef void write_callback(int fd)
{
    string sockID;
    int x, times;

    if (member_array(fd, keys(Ids)) == -1) return;

    sockID = Ids[fd];

    if (member_array(sockID, keys(Sockets)) == -1 && !Sockets[sockID]) return;

    /* Not blocking at the moment */
    Sockets[sockID]["blocking"] = 0;

    /* If the socket is closed (close_callback called) then clean up */
    /* If we are closing and there is no data in the buffer then close the socket */
    if (Sockets[sockID]["closed"] || (!Sockets[sockID]["buffer"] && Sockets[sockID]["closing"]))
    {
        internal_close(fd);
        return;
    }

    /* Stop trying to write data for a bit */
    if (Sockets[sockID]["writeRetry"] == WRITERETRYLIMIT)
    {
        Sockets[sockID]["blocking"] = 1;
        Sockets[sockID]["writeRetry"] = 0;
        call_out("write_callback", 5, fd);
        return;
    }

    /* Pre-set the socket_write error to EESUCCESS */
    x = EESUCCESS;

    times = 10;

    /* Process when:
         * there is data in the buffer
         * socket_write is successful
         * looped less than 10 times (limit spam)
     */
    while (Sockets[sockID]["buffer"] && x == EESUCCESS && times > 0)
    {
        times--;
#if 0
        /* Turn this on for lots of log messages. */
        log(
          SOCKET_ID,
          "Notice",
          sprintf("Writing on %s/%d: %O",
            sockID,
            Sockets[sockID]["fd"],
            Sockets[sockID]["buffer"][0]
          )
        );
#endif

        /* Write first set of data in buffer to the socket */
        switch (x = socket_write(fd, Sockets[sockID]["buffer"][0]))
        {

            /* Break out of the switch if successful */
            case EESUCCESS: break;

            /* Data has been buffered and we need to suspend further writes until it is cleared */
            case EECALLBACK:
            {
                Sockets[sockID]["blocking"] = 1;
                break;
            }

            /* The socket is blocked so we need to call write_callback again */
            case EEWOULDBLOCK:
            {
                call_out("write_callback", 1, fd);
                return;
            }

            /* Problem with send */
            case EESEND:
            {
                if (!Sockets[sockID]) return;
                if (member_array("writeRetry", keys(Sockets[sockID])) == -1)
                    Sockets[sockID]["writeRetry"] = 0;

                Sockets[sockID]["writeRetry"] = Sockets[sockID]["writeRetry"] + 1;
                call_out("write_callback", 2, fd);
                return;
            }

            /* Flow control has been violated, shouldn't see this */
            case EEALREADY:
            {
                Sockets[sockID]["blocking"] = 1;
                return;
            }

            /* Something went really wrong so we close the socket */
            default:
            {
                error_log(sockID, Sockets[sockID]["type"] + "/socket_write", x);
                internal_close(fd);
                return;
            }

        }

        Sockets[sockID]["writeRetry"] = 0;

        /* Remove the data we have written from the buffer */
        if (sizeof(Sockets[sockID]["buffer"]) == 1)
        {
            Sockets[sockID]["buffer"] = 0;

            /* If the socket is closed (close_callback called) then clean up */
            /* or if we are closing then close the socket */
            if (Sockets[sockID]["closed"] || Sockets[sockID]["closing"])
            {
                internal_close(fd);
                return;
            }
        }
        else Sockets[sockID]["buffer"] = Sockets[sockID]["buffer"][1..<1];
    }
}

/*
 * Function name: internal_write
 * Description:   Internal socket write.
 * Arguments:     fd - Socket file descriptor.
 *                data - Outgoing data.
 */
privatef void internal_write(int fd, mixed data)
{
    string sockID;

    sockID = Ids[fd];

    /* Add data to the buffer */
    if (Sockets[sockID]["buffer"]) Sockets[sockID]["buffer"] += ({ data });
    else Sockets[sockID]["buffer"] = ({ data });

    /* If we are blocking then return otherwise write it */
    if (Sockets[sockID]["blocking"]) return;
    else
    {
        Sockets[sockID]["writeRetry"] = 0;
        write_callback(fd);
    }
}

/*
 * Function name: client_write
 * Description:   Public client socket write.
 * Arguments:     fd - Socket file descriptor.
 *                data - Outgoing data.
 */
void client_write(mixed fd, mixed data)
{
    if (intp(fd))
    {
        if (member_array(fd, keys(Ids)) == -1) return;
        if (Sockets[Ids[fd]]["owner"] != previous_object())
        {
            log(Ids[fd], "Warning", "Illegal client_write by sockID '" + Ids[fd] + "' in object '" + Sockets[Ids[fd]]["owner"] + "'\n");
            return;
        }

        internal_write(fd, data);
    }
    else if (stringp(fd))
    {
        if (member_array(fd, keys(Sockets)) == -1) return;
        if (Sockets[fd]["owner"] != previous_object())
        {
            log(fd, "Warning", "Illegal client_write by sockID '" + fd + "' in object '" + Sockets[fd]["owner"] + "'\n");
            return;
        }

        internal_write(Sockets[fd]["fd"], data);
    }
}

/*
 * Function name: server_write
 * Description:   Public server socket write.
 * Arguments:     fd - Socket file descriptor.
 *                data - Outgoing data.
 *                close - If set to '0' the connection is kept open after writing the data.
 *                        If set to '1' the connection is closed after writing the data.
 */
void server_write(mixed fd, mixed data, int close)
{
    if (undefinedp(close)) close = 0;

    if (intp(fd))
    {
        if (member_array(fd, keys(Ids)) == -1) return;
        if (Sockets[Ids[fd]]["owner"] != previous_object())
        {
            log(Ids[fd], "Warning", "Illegal server_write by sockID '" + Ids[fd] + "' in object '" + Sockets[Ids[fd]]["owner"] + "'\n");
            return;
        }

        Sockets[Ids[fd]]["closing"] = close;
        internal_write(fd, data);
    }
    else if (stringp(fd))
    {
        if (member_array(fd, keys(Sockets)) == -1) return;
        if (Sockets[fd]["owner"] != previous_object())
        {
            log(fd, "Warning", "Illegal server_write by sockID '" + fd + "' in object '" + Sockets[fd]["owner"] + "'\n");
            return;
        }

        Sockets[fd]["closing"] = close;
        internal_write(Sockets[fd]["fd"], data);
    }
}

/*
 * Function name: log
 * Description:   Internal socket log.
 * Arguments:     sockID - Socket ID.
 *                pre - Pre-log message.
 *                str - Log message.
 */
privatef void log(string sockID, string pre, string str)
{
    /* If we have set a log file then append the message to it */
    if (member_array(sockID, keys(Sockets)) != -1)
        log_file(
          Sockets[sockID]["logFile"],
          sprintf("%s [SOCKET_D%s]: %s\n",
            pre,
            (sockID == SOCKET_ID ? "" : "/" + sockID),
            str
          )
        );
}

/*
 * Function name: error_log
 * Description:   Internal socket error log.
 * Arguments:     sockID - Socket ID.
 *                str - Log message.
 *                x - Socket error code.
 */
privatef void error_log(string sockID, string str, int x)
{
    log(sockID, "Error", sprintf("%s - %s", str, socket_error(x)));
}

/*
 * Function name: set_socket_type
 * Description:   Sets the user supplied socket type.
 * Arguments:     type - Socket type.
 */
void set_socket_type(int type)
{
    SocketType = type;
}

/*
 * Function name: query_socket_type
 * Description:   Queries the user supplied socket type.
 * Arguments:     Socket ID.
 * Return:        Socket type or '-1' if the socket does not exist.
 */
int query_socket_type(string sockID)
{
    if (member_array(sockID, keys(Sockets)) == -1) return -1;

    return Sockets[sockID]["socketType"];
}

/*
 * Function name: set_read_cb
 * Description:   Sets the user supplied socket read callback function.
 * Arguments:     func - User function (as a string).
 */
void set_read_cb(string func)
{
    ReadF = func;
}

/*
 * Function name: set_listen_cb
 * Description:   Sets the user supplied socket listen callback function.
 * Arguments:     func - User function (as a string).
 */
void set_listen_cb(string func)
{
    ListenF = func;
}

/*
 * Function name: set_close_cb
 * Description:   Sets the user supplied socket close callback function.
 * Arguments:     func - User function (as a string).
 */
void set_close_cb(string func)
{
    CloseF = func;
}

/*
 * Function name: set_release_cb
 * Description:   Sets the user supplied socket close callback function.
 * Arguments:     func - User function (as a string).
 */
void set_release_cb(string func)
{
    ReleaseF = func;
}

/*
 * Function name: set_log_file
 * Description:   Sets the user supplied socket log file.
 * Arguments:     file - Log file.
 */
void set_log_file(string file)
{
    LogFile = file;
}

/*
 * Function name: query_log_file
 * Description:   Queries the user supplied socket log file.
 * Arguments:     Socket ID.
 * Return:        Log file or '0' if the socket does not exist.
 */
string query_log_file(string sockID)
{
    if (member_array(sockID, keys(Sockets)) == -1) return 0;

    return Sockets[sockID]["logFile"];
}

/*
 * Function name: query_socket_info
 * Description:   Queries the internal Sockets mapping.
 * Return:        All connected sockets.
 */
mapping query_socket_info()
{
    mapping tmp = ([ ]);

    foreach (string sockID, mapping sock in Sockets)
    {
        if(sockID == SOCKET_ID) continue;

        tmp[sockID] = sock;
    }

    return copy(tmp);
}
