/* http_d.c
 *
 * Tricky @ Rock the Halo
 * 7-MAY-2008
 * HTTP Daemon
 *
 */

#include <driver/localtime.h>
#include <socket.h>

#define HTTPLOG(s,o,f,m) log_file("http_d.log", s + " [" + o + "/" + f + "]: " + m + "\n")

privatev mapping Fds;

privatef int setup_http_socket();

void create()
{
    int fd;

    if ((fd = setup_http_socket()) < 0)
    {
        if (fd == EEISCONN)
        {
            HTTPLOG("Warning", "HTTP_D", "create", "Already connected, maybe still closing. Will try again.");
            call_out("create", 2);

            return;
        }

        HTTPLOG("Error", "HTTP_D", "create", "Unable to setup socket - " + socket_error(fd));
        /* Don't need to call remove() so we call destruct() this way. */
        destruct();
        return;
    }

    Fds = ([ ]);
    set_heart_beat(30);
}

void remove()
{
    object sock = find_object(SOCKET_D);

    sock->close("HTTP_D");
    destruct();
}

void heart_beat()
{
    mixed array ltime;
    string logname;
    int size;

    ltime = localtime(time());
    logname = sprintf("/www/logs/access-%02d%02d%02d.log",
        ltime[LT_YEAR],
        ltime[LT_MON] + 1,
        ltime[LT_MDAY]
      );

    size = file_size(logname);

    if (size == -1) write_file(logname, "", 1);
}

privatef int setup_http_socket()
{
    object sock = find_object(SOCKET_D);

    sock->set_socket_type(STREAM);
    sock->set_read_cb("server_read_cb");
    sock->set_listen_cb("listen_cb");
    sock->set_close_cb("close_cb");
    sock->set_log_file("http_d.log");

    return sock->server_create("HTTP_D", port() + 1);
}

void close_cb(int fd)
{
    HTTPLOG("Notice", "HTTP_D", "close_cb", "Closing connection to " + fd);

    map_delete(Fds, fd);
}

void listen_cb(int l_fd, int r_fd)
{
    object http_thread;

    HTTPLOG("Notice", "HTTP_D", "listen_cb", sprintf("l_fd: %d, r_fd: %d", l_fd, r_fd));

    http_thread = new("/std/net/httpd/thread", r_fd);
    set_privs(http_thread, "[httpd]");
    Fds = ([ r_fd: http_thread ]);

    HTTPLOG("Notice", "HTTP_D", "listen_cb", sprintf("http_thread: %O", http_thread));
}

void server_read_cb(int fd, mixed data)
{
    Fds[fd]->http_read_cb(data);
}

void write_data(int fd, mixed data)
{
    SOCKET_D->client_write(fd, data);
}

void close_fd(int fd)
{
    Fds[fd]->remove();
    map_delete(Fds, fd);
    SOCKET_D->close(fd);
}

int file_rm(string fname)
{
    return rm(fname);
}

int file_write(string fname, string str, int flag)
{
    return write_file(fname, str, flag);
}
