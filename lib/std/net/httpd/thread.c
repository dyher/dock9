/* httpd/thread.c
 *
 * Tricky @ Rock the Halo
 * 13-MAY-2008
 * HTTP Thread
 *
 */

#include <driver/localtime.h>

#undef RESOLVE_REMOTE_IPS

#define HTTPLOG(s,o,f,m) log_file("http_d.log", s + " [" + o + "/" + f + "]: " + m + "\n")
#define DATEFMT "%02d/%s/%d:%02d:%02d:%02d %s"
#define LOG(fn,ip,uid,gid,ds,req,st,sz,rf,ua) \
  log_access(fn, sprintf("%s %s %s [%s] \"%s\" %d %d \"%s\" \"%s\"\n", ip, uid, gid, ds, req, st, sz, rf, ua))

privatev mapping Request, Header, Info, Vars, Cookie;
privatev mapping Services;
privatev string array ShortDays;
privatev string array ShortMonths;
privatev string ServerRoot, DocumentRoot;
privatev string IncomingData;
privatev int Fd, MSIE, Pid;

void setup(int fd);

void create(int fd)
{
    if (clonep()) setup(fd);
}

void setup(int fd)
{
    HTTPLOG("Notice", "httpd/thread", "setup", sprintf("this_object(): %O", this_object()));

    Services = ([
      "HEAD": ([
        "owner": this_object(),
        "func": "rec_head",
      ]),
      "GET": ([
        "owner": this_object(),
        "func": "rec_get",
      ]),
    ]);

    ShortDays = ({ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" });
    ShortMonths = ({
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    });

    MSIE = 0;
    Fd = fd;
    Pid = time();
    IncomingData = "";
    ServerRoot = "/www";
    DocumentRoot = ServerRoot + "/htdocs";
}

void remove()
{
    destruct();
}

void log_access(string fname, string msg)
{
    int size = file_size(fname);

    if (size == -2) return;
    if (size + strlen(msg) > 200000)
    {
        mixed *ltime = localtime(time());
        string t1, backup;
        int ret = sscanf(fname, "%s.log", t1);

        if (ret == 0)
            backup = sprintf("archive/%s-%02d%02d%02d%02d%02d%02d",
              fname,
              ltime[LT_YEAR],
              ltime[LT_MON] + 1,
              ltime[LT_MDAY],
              ltime[LT_HOUR],
              ltime[LT_MIN],
              ltime[LT_SEC],
            );
        else
            backup = sprintf("archive/%s-%02d%02d%02d%02d%02d%02d.log",
              t1,
              ltime[LT_YEAR],
              ltime[LT_MON] + 1,
              ltime[LT_MDAY],
              ltime[LT_HOUR],
              ltime[LT_MIN],
              ltime[LT_SEC],
            );
        rename(fname, fname[0..strsrch(fname, "/", -1)] + backup);
    }

    write_file(fname, msg);
}

string dirname(string path)
{
    int ptr;

    ptr = strsrch(path, "/", -1);
    return path[0..ptr - 1];
}

string basename(string path)
{
    int ptr;

    ptr = strsrch(path, "/", -1);
    return path[ptr + 1..<1];
}

varargs string array make_headers(int content_length, string content_type)
{
    mixed array ltime;
    string array headers = ({ });

    headers += ({ Info["SERVER_PROTOCOL"] + " " + Info["REDIRECT_STATUS"] });

    ltime = localtime(time());
    headers += ({
      sprintf("Date: %s, %02d %s %d %02d:%02d:%02d GMT",
        ShortDays[ltime[LT_WDAY]],
        ltime[LT_MDAY],
        ShortMonths[ltime[LT_MON]],
        ltime[LT_YEAR],
        ltime[LT_HOUR]-1,
        ltime[LT_MIN],
        ltime[LT_SEC],
      )
    });
    headers += ({ "Connection: close" });
    headers += ({ "Server: " + Info["SERVER_SOFTWARE"] });
    headers += ({ "X-Powered-By: " + baselib_name() + "/" + baselib_version() });

    if (content_length > 0)
        headers += ({ "Content-Length: " + content_length + "" });
    if (!undefinedp(content_type))
        headers += ({ "Content-Type: " + content_type });

    headers += ({
      sprintf("Last-Modified: %s, %02d %s %d %02d:%02d:%02d GMT",
        ShortDays[ltime[LT_WDAY]],
        ltime[LT_MDAY],
        ShortMonths[ltime[LT_MON]],
        ltime[LT_YEAR],
        ltime[LT_HOUR]-1,
        ltime[LT_MIN],
        ltime[LT_SEC],
      )
    });
    headers += ({
      sprintf("Expires: %s, %02d %s %d %02d:%02d:%02d GMT",
        ShortDays[ltime[LT_WDAY]],
        ltime[LT_MDAY],
        ShortMonths[ltime[LT_MON]],
        ltime[LT_YEAR],
        ltime[LT_HOUR]-1,
        ltime[LT_MIN]+1,
        ltime[LT_SEC],
      )
    });

    headers += ({ "" });

    return headers;
}

mixed eval_lpc(string arg, mapping info)
{
    object ob;
    mixed result = 0;
    string err, tmp = "", tmpname;

    if (!stringp(arg) || arg == "") return 0;

    Pid = max( ({ time(), Pid + 1 }) );

    tmpname = "httpd_thread_" + Pid;

    if (file_size(ServerRoot + "/tmp/" + tmpname + ".c") > -1)
        rm(ServerRoot + "/tmp/" + tmpname + ".c");
    if (find_object(ServerRoot + "/tmp/" + tmpname))
        destruct(find_object(ServerRoot + "/tmp/" + tmpname));

    tmp  = "mapping _SERVER = ([\n";

    foreach (mixed k, mixed v in info["_SERVER"])
        tmp += sprintf("  %O: %O,\n", k, v);

    tmp += "]);\n";
    tmp += "mapping _REQUEST = ([\n";

    foreach (mixed k, mixed v in info["_REQUEST"])
        tmp += sprintf("  %O: %O,\n", k, v);

    tmp += "]);\n";
    tmp += "mapping _LOCAL = ([\n";

    foreach (mixed k, mixed v in info["_LOCAL"])
        tmp += sprintf("  %O: %O,\n", k, v);

    tmp += "]);\n";
    tmp += "mapping _COOKIE = ([\n";

    foreach (mixed k, mixed v in info["_COOKIE"])
        tmp += sprintf("  %O: %O,\n", k, v);

    tmp += "]);\n";
    tmp += "\n";

    tmp += "void create() {}\n";
    tmp += arg + "\n";

//    write_file(ServerRoot + "/tmp/tmp_" + tmpname + ".c", tmp);
    write_file(ServerRoot + "/tmp/" + tmpname + ".c", tmp);

    err = catch(ob = load_object(ServerRoot + "/tmp/" + tmpname));

    if (!err && ob)
        err = catch(result = ob->main());

    if (err) result =
      "\n" +
      "Runtime error: " + query_privs(find_object(__FILE__)) + "\n" +
      " " + err + "\n" +
      "See logs for more details.\n";

    rm(ServerRoot + "/tmp/" + tmpname + ".c");
    destruct(ob);

//    HTTPLOG("Notice", "httpd/thread", "eval_lpc", sprintf("Result:\n%O", result));
    return result;
}

string array parse_lpc(string array data)
{
    mapping info = ([ ]);
    string array new_data = ({ });
    string eval_line = "";
    int open_lpc = 0, close_lpc = 0;

    info["_SERVER"] = Info;
    info["_REQUEST"] = Request;
    info["_LOCAL"] = Vars;
    info["_COOKIE"] = Cookie;

    foreach (string line in data)
    {
        mixed array assoc;
        string new_line;

        new_line = UNDEFINED_VALUE;
        assoc = reg_assoc(line, ({ "(<[?]lpc)+", "(lpc[?]>)+" }), ({ 1, 2 }) );

        if (sizeof(assoc[1]) == 1 && !open_lpc && trim(line) != "")
            new_data += ({ line });
        else
        {
            if (new_line == UNDEFINED_VALUE) new_line = "";

            for (int i = 0; i < sizeof(assoc[1]); i++)
            {
                if (assoc[1][i] == 1) open_lpc = 1;
                else if (assoc[1][i] == 2) close_lpc = 1;

                if (!open_lpc && !close_lpc)
                    new_line += assoc[0][i];
                else if (open_lpc == 1 && !close_lpc)
                {
//                    new_line += "<!-- LPC_OPEN -->";
                    open_lpc++;
                }
                else if (close_lpc)
                {
//                    new_line += "<!-- LPC_CLOSE -->";
                    if (eval_line != "")
                    {
                        mixed result;

//    HTTPLOG("Notice", "httpd/thread", "parse_lpc", sprintf("eval_line: %O", eval_line));
                        result = eval_lpc(eval_line, info);

                        if (result && stringp(result)) new_line += result;

                        eval_line = "";
                    }

                    open_lpc = close_lpc = 0;
                }
                else if (trim(assoc[0][i]) != "")
                {
//                    new_line += "<!-- LPC_DATA '" + assoc[0][i] + "' -->";
                    eval_line += assoc[0][i] + "\n";
                }
            }

            if (new_line != UNDEFINED_VALUE && !open_lpc) new_data += ({ new_line });
        }
    }

    return new_data;
}

void send_error(int num, string msg)
{
    object httpd = load_object(HTTP_D);
    mixed array ltime;
    string top, content, contact, bottom, datetime, logname;
    int sz;

    Info["REDIRECT_STATUS"] = num + " " + msg;
    Info["REDIRECT_HTTP_ACCEPT"] = (Info["HTTP_ACCEPT"] ? Info["HTTP_ACCEPT"] : "*/*");
    Info["REDIRECT_HTTP_USER_AGENT"] = (Info["HTTP_USER_AGENT"] ? Info["HTTP_USER_AGENT"] : Info["SERVER_SOFTWARE"]);
    Info["REDIRECT_QUERY_STRING"] = (Info["QUERY_STRING"] ? Info["QUERY_STRING"] : "");
    Info["REDIRECT_SERVER_NAME"] = Info["SERVER_NAME"];
    Info["REDIRECT_SERVER_SOFTWARE"] = Info["SERVER_SOFTWARE"];
    Info["REDIRECT_SERVER_PORT"] = Info["SERVER_PORT"];
    Info["REDIRECT_URL"] = (Info["REQUEST_URI"] ? Info["REQUEST_URI"] : "/");
    Info["REDIRECT_URI"] = Info["REDIRECT_URL"];

    Vars["TITLE"] = msg;

    top = implode(parse_lpc(explode(read_file(ServerRoot + "/error/include/top.lpc"), "\n")), "\n");
    content = implode(parse_lpc(explode(read_file(ServerRoot + "/error/" + num + ".lpc"), "\n")), "\n");
    contact = implode(parse_lpc(explode(read_file(ServerRoot + "/error/contact.lpc"), "\n")), "\n");
    bottom = implode(parse_lpc(explode(read_file(ServerRoot + "/error/include/bottom.lpc"), "\n")), "\n");

    sz = strlen(top + content + contact + bottom);

    httpd->write_data(Fd, Info["SERVER_PROTOCOL"] + " " + Info["REDIRECT_STATUS"] + "\r\n");
    ltime = localtime(time());
    httpd->write_data(Fd,
      sprintf("Date: %s, %02d %s %d %02d:%02d:%02d GMT\r\n",
        ShortDays[ltime[LT_WDAY]],
        ltime[LT_MDAY],
        ShortMonths[ltime[LT_MON]],
        ltime[LT_YEAR],
        ltime[LT_HOUR]-1,
        ltime[LT_MIN],
        ltime[LT_SEC],
      )
    );
    httpd->write_data(Fd, "Connection: close\r\n");

    if (num == 405)
        httpd->write_data(Fd, "Allow: " + implode(keys(Services), ", ") + "\r\n");

    httpd->write_data(Fd, "Content-language: en\r\n");
    httpd->write_data(Fd, "Content-type: text/html; charset=ISO-8859-1\r\n");
    httpd->write_data(Fd, "Content-Length: " + sz + "\r\n");
    httpd->write_data(Fd, "\r\n");

    httpd->write_data(Fd, top + "\n");
    httpd->write_data(Fd, content + "\n");
    httpd->write_data(Fd, contact + "\n");
    httpd->write_data(Fd, bottom + "\n");
    httpd->close_fd(Fd);

    logname = sprintf("/www/logs/access-%02d%02d%02d.log",
        ltime[LT_YEAR],
        ltime[LT_MON] + 1,
        ltime[LT_MDAY]
      );
    datetime = sprintf(DATEFMT,
      ltime[LT_MDAY],
      ShortMonths[ltime[LT_MON]],
      ltime[LT_YEAR],
      ltime[LT_HOUR],
      ltime[LT_MIN],
      ltime[LT_SEC],
      ltime[LT_ZONE]
    );
    LOG(logname, Info["REMOTE_ADDR"], "-", "-", datetime, Vars["REQUEST"], num, sz, (Info["HTTP_REFERER"] ? Info["HTTP_REFERER"] : "-"), (Info["HTTP_USER_AGENT"] ? Info["HTTP_USER_AGENT"] : "-"));

    return;
}

void parse_query(string query)
{
    Info["QUERY_STRING"] = query;

    foreach (string request in explode(query, "&"))
    {
        string array kv;

        kv = explode(request, "=");

        if (sizeof(kv) > 1)
            Request[kv[0]] = kv[1];
        else
            Request[request] = 1;
    }
}

void parse_cookies(string cookies)
{
    foreach (string cookie in explode(cookies, ";"))
    {
        string array kv;

        cookie = trim(cookie);
        kv = explode(cookie, "=");

        if (sizeof(kv) > 1)
            Cookie[kv[0]] = trim(kv[1]);
        else
            Cookie[trim(cookie, "= ")] = 0;
    }

    map_delete(Info, "HTTP_COOKIE");
}

#ifdef RESOLVE_REMOTE_IPS
void resolve_cb(string address, string resolved, int key)
{
    Info["REMOTE_HOST"] = resolved;
}
#endif

void http_read_cb(mixed data)
{
    object owner = this_object();
    mixed sockStatus;
    mixed proto_type, proto_version_major, proto_version_minor;
    string func = "rec_not_supported", lines;
    string method, uri, proto, query;
    string r_addr, r_port;
    int header_done = 0;

    data = replace_string(data, "\r", "");

    if (!IncomingData) IncomingData = "";

    IncomingData += data;

    while (IncomingData[0] == '\n') IncomingData = IncomingData[1..<1];

    if (IncomingData == "") return;

    lines = IncomingData;
    sscanf(lines, "%s %s %s\n", method, uri, proto);

    if (stringp(proto) && proto != "")
    {
        sscanf(proto, "%s/%s.%s", proto_type, proto_version_major, proto_version_minor);

        if (!undefinedp(proto_version_major)) proto_version_major = to_int(proto_version_major);
        if (!undefinedp(proto_version_minor)) proto_version_minor = to_int(proto_version_minor);
    }

//    HTTPLOG("Notice", "httpd/thread", "http_read_cb", sprintf("fd: %d\n%O", Fd, lines));

    Header = ([ ]);
    Vars = ([ ]);
    Request = ([ ]);
    Cookie = ([ ]);

    lines = lines[strsrch(lines, "\n") + 1..<1];

    while (lines != "")
    {
        string array kv;
        string header = "";
        int index = strsrch(lines, "\n");

        if (index > 0) header = lines[0..index - 1];

        lines = lines[index + 1..<1];
        header_done = 1;

        if (index == 0) break;

        header_done = 0;
        kv = explode(header, ": ");

        if (sizeof(kv) > 1)
            Header[kv[0]] = kv[1];
        else
            Header[header] = 1;
    }

//    HTTPLOG("Notice", "httpd/thread", "http_read_cb", sprintf("fd: %d\n%d/%O", Fd, header_done, lines));

    if (header_done && Header["Content-Length"])
    {
//        HTTPLOG("Notice", "httpd/thread", "http_read_cb", sprintf("fd: %d\n%O/%d/%O", Fd, Header["Content-Length"], sizeof(lines), lines));

        if (sizeof(lines) < to_int(Header["Content-Length"])) return;;
    }
    else if (!header_done) return;

    IncomingData = "";
    HTTPLOG("Notice", "httpd/thread", "http_read_cb", sprintf("fd: %d\n%O", Fd, Header));

    MSIE = 0;

    if (strsrch(lower_case(Header["User-Agent"]), "msie") != -1) MSIE = 1;

    Info = ([ ]);
    Info["DOCUMENT_ROOT"] = DocumentRoot;
    Info["SERVER_NAME"] = "ebspso.dnsalias.org";
    Info["SERVER_ADDR"] = "192.168.2.2";
    Info["SERVER_PORT"] = port() + 1;
    Info["SERVER_ADMIN"] = admin_email();
    Info["SERVER_PROTOCOL"] = "HTTP/1.1";
    Info["SERVER_SOFTWARE"] =
      replace_string(driver_version(), " ", "/") + " (" + arch() + ") " + baselib_name() + "/" + baselib_version();
    Info["SERVER_SIGNATURE"] =
      "<address>" + Info["SERVER_SOFTWARE"] + " Server at " + Info["SERVER_NAME"] + " Port " + Info["SERVER_PORT"] + "</address>";
    Info["REQUEST_METHOD"] = method;

    sockStatus = socket_status(Fd);
    r_addr = sockStatus[4][0..strsrch(sockStatus[4], ".", -1) - 1];
    r_port = sockStatus[4][strsrch(sockStatus[4], ".", -1) + 1..<1];

    Info["REMOTE_HOST"] = r_addr;

#ifdef RESOLVE_REMOTE_IPS
    resolve(r_addr, "resolve_cb");
#endif

    Info["REMOTE_ADDR"] = r_addr;
    Info["REMOTE_PORT"] = r_port;

    Vars["REQUEST"] = method + " " + uri + " " + proto;

    if (!proto || (stringp(proto) && proto == ""))
    {
        send_error(505, "HTTP Version Not Supported");
        return;
    }

    Vars["REMOTE_PROTO"] = proto;

    if (!stringp(proto_type))
    {
        send_error(505, "HTTP Version Not Supported");
        return;
    }
    else if (proto_type == "HTTP")
    {
        if (!intp(proto_version_major))
        {
            send_error(505, "HTTP Version Not Supported");
            return;
        }
        else
        {
            switch (proto_version_major)
            {
                case 1:
                {
                    if (!intp(proto_version_minor) || proto_version_minor > 1)
                    {
                        send_error(505, "HTTP Version Not Supported");
                        return;
                    }

                    break;
                }
                default:
                {
                    send_error(505, "HTTP Version Not Supported");
                    return;
                }
            }
        }
    }
    else
    {
        send_error(505, "HTTP Version Not Supported");
        return;
    }

    if (proto == "HTTP/1.1" && !Header["Host"])
    {
        send_error(400, "Bad Request");
        return;
    }

    foreach (mixed k, mixed v in Header)
    {
        k = replace_string(k, "-", "_");
        Info["HTTP_" + upper_case(k)] = v;
    }

    Info["REQUEST_URI"] = uri;
    sscanf(uri, "%s?%s", uri, query);
    Info["SCRIPT_NAME"] = uri;
    Info["QUERY_STRING"] = "";

    if (!undefinedp(query) && query != "" && query != 0)
        parse_query(query);

    if (Info["HTTP_COOKIE"])
        parse_cookies(Info["HTTP_COOKIE"]);

    if (member_array(method, keys(Services)) != -1)
    {
        owner = Services[method]["owner"];
        func = Services[method]["func"];
    }

    if (function_exists(func, owner))
        call_other(owner, func);
    else
        call_other(this_object(), "rec_not_supported");
}

void rec_head()
{
    object httpd = load_object(HTTP_D);
    mixed array ltime;
    mixed array content = ({ });
    string array headers = ({ });
    string dir, base, fname, datetime, logname;
    int sz;

    dir = dirname(Info["SCRIPT_NAME"]);
    base = basename(Info["SCRIPT_NAME"]);

    if (dir == "" && base[0] == '~')
    {
        dir = "/" + base;
        base = "";
    }

    if (dir[0..1] == "/~")
    {
        int index;

        dir = dir[2..<1];
        index = strsrch(dir, "/");

        if (index == -1) dir = user_path(dir);
        else dir = user_path(dir[0..index - 1]) + dir[index + 1..<1];

        if (dir[<1] == '/') dir = dir[0..<2];

        fname = dir;
    }
    else fname = DocumentRoot + dir;

    content = ({ 404, "Not Found" });

    if (base == "")
    {
        mixed access;

        Info["SCRIPT_FILENAME"] = fname;
        HTTPLOG("Notice", "httpd/thread", "rec_head", "fname: " + fname);

        if (file_size(fname + "/index.lpc") > -1)
        {
            access = read_file(fname + "/index.lpc");

            if (access && Request["showsource"])
                content = ({ "text/plain" }) + explode(access, "\n");
            else if (access)
                content = ({ "application/xhtml+xml; charset=utf-8" }) + parse_lpc(explode(access, "\n"));
            else
                content = ({ 403, "Forbidden" });
        }
        else if (file_size(fname + "/index.html") > -1)
        {
            if (access = read_file(fname + "/index.html"))
                content = ({ "text/html; charset=utf-8" }) + explode(access, "\n");
            else
                content = ({ 403, "Forbidden" });
        }
        else if (file_size(fname + "/index.htm") > -1)
        {
            if (access = read_file(fname + "/index.htm"))
                content = ({ "text/html; charset=utf-8" }) + explode(access, "\n");
            else
                content = ({ 403, "Forbidden" });
        }
    }
    else
    {
        mixed access;

        fname += "/" + base;
        Info["SCRIPT_FILENAME"] = fname;
        HTTPLOG("Notice", "httpd/thread", "rec_head", "fname: " + fname);

        if (base[<4..<1] == ".lpc" && file_size(fname) > -1)
        {
            if (access = read_file(fname))
                content = ({ "application/xhtml+xml; charset=utf-8" }) + parse_lpc(explode(access, "\n"));
            else
                content = ({ 403, "Forbidden" });
        }
        else if (base[<5..<1] == ".lpcs" && file_size(fname[0..<2]) > -1)
        {
            if (access = read_file(fname[0..<2]))
                content = ({ "text/plain" }) + explode(access, "\n");
            else
                content = ({ 403, "Forbidden" });
        }
        else if ((base[<5..<1] == ".html" || base[<4..<1] == ".htm") && file_size(fname) > -1)
        {
            if (access = read_file(fname))
                content = ({ "text/html; charset=utf-8" }) + explode(access, "\n");
            else
                content = ({ 403, "Forbidden" });
        }
        else if (file_size(fname) > -1)
        {
            if (access = read_file(fname))
                content = ({ "text/plain" }) + explode(access, "\n");
            else
                content = ({ 403, "Forbidden" });
        }
    }

    if (sizeof(content) == 2 && intp(content[0]))
    {
        send_error(content[0], content[1]);
        return;
    }

    Info["REDIRECT_STATUS"] = "200 OK";

    if (MSIE && content[0] != "text/plain")
        headers = make_headers(sz = sizeof(implode(content[1..<1], "\n")));
    else
        headers = make_headers(sz = sizeof(implode(content[1..<1], "\n")), content[0]);

    HTTPLOG("Notice", "httpd/thread", "rec_head", sprintf("Outgoing headers -- fd: %d\n%O", Fd, headers));

    ltime = localtime(time());
    logname = sprintf("/www/logs/access-%02d%02d%02d.log",
        ltime[LT_YEAR],
        ltime[LT_MON] + 1,
        ltime[LT_MDAY]
      );
    datetime = sprintf(DATEFMT,
      ltime[LT_MDAY],
      ShortMonths[ltime[LT_MON]],
      ltime[LT_YEAR],
      ltime[LT_HOUR],
      ltime[LT_MIN],
      ltime[LT_SEC],
      ltime[LT_ZONE]
    );
    LOG(logname, Info["REMOTE_ADDR"], "-", "-", datetime, Vars["REQUEST"], 200, sz, (Info["HTTP_REFERER"] ? Info["HTTP_REFERER"] : "-"), (Info["HTTP_USER_AGENT"] ? Info["HTTP_USER_AGENT"] : "-"));

    foreach (string line in headers)
        httpd->write_data(Fd, line + "\r\n");

    httpd->close_fd(Fd);
}

void rec_get()
{
    object httpd = load_object(HTTP_D);
    mixed array ltime;
    mixed array content = ({ });
    string array headers = ({ });
    string dir, base, fname, datetime, logname;
    int sz;

    dir = dirname(Info["SCRIPT_NAME"]);
    base = basename(Info["SCRIPT_NAME"]);

    if (dir == "" && base[0] == '~')
    {
        dir = "/" + base;
        base = "";
    }

    if (dir[0..1] == "/~")
    {
        int index;

        dir = dir[2..<1];
        index = strsrch(dir, "/");

        if (index == -1) dir = user_path(dir);
        else dir = user_path(dir[0..index - 1]) + dir[index + 1..<1];

        if (dir[<1] == '/') dir = dir[0..<2];

        fname = dir;
    }
    else fname = DocumentRoot + dir;

    content = ({ 404, "Not Found" });

    if (base == "")
    {
        mixed access;

        Info["SCRIPT_FILENAME"] = fname;
        HTTPLOG("Notice", "httpd/thread", "rec_get", "fname: " + fname);

        if (file_size(fname + "/index.lpc") > -1)
        {
            access = read_file(fname + "/index.lpc");

            if (access && Request["showsource"])
                content = ({ "text/plain" }) + explode(access, "\n");
            else if (access)
                content = ({ "application/xhtml+xml; charset=utf-8" }) + parse_lpc(explode(access, "\n"));
            else
                content = ({ 403, "Forbidden" });
        }
        else if (file_size(fname + "/index.html") > -1)
        {
            if (access = read_file(fname + "/index.html"))
                content = ({ "text/html; charset=utf-8" }) + explode(access, "\n");
            else
                content = ({ 403, "Forbidden" });
        }
        else if (file_size(fname + "/index.htm") > -1)
        {
            if (access = read_file(fname + "/index.htm"))
                content = ({ "text/html; charset=utf-8" }) + explode(access, "\n");
            else
                content = ({ 403, "Forbidden" });
        }
    }
    else
    {
        mixed access;

        fname += "/" + base;
        Info["SCRIPT_FILENAME"] = fname;
        HTTPLOG("Notice", "httpd/thread", "rec_get", "fname: " + fname);

        if (base[<4..<1] == ".lpc" && file_size(fname) > -1)
        {
            access = read_file(fname);

            if (access && Request["showsource"])
                content = ({ "text/plain" }) + explode(access, "\n");
            else if (access)
                content = ({ "application/xhtml+xml; charset=utf-8" }) + parse_lpc(explode(access, "\n"));
            else
                content = ({ 403, "Forbidden" });
        }
        else if (base[<5..<1] == ".lpcs" && file_size(fname[0..<2]) > -1)
        {
            if (access = read_file(fname[0..<2]))
                content = ({ "text/plain" }) + explode(access, "\n");
            else
                content = ({ 403, "Forbidden" });
        }
        else if ((base[<5..<1] == ".html" || base[<4..<1] == ".htm") && file_size(fname) > -1)
        {
            if (access = read_file(fname))
                content = ({ "text/html; charset=utf-8" }) + explode(access, "\n");
            else
                content = ({ 403, "Forbidden" });
        }
        else if (file_size(fname) > -1)
        {
            if (access = read_file(fname))
                content = ({ "text/plain" }) + explode(access, "\n");
            else
                content = ({ 403, "Forbidden" });
        }
    }

    if (sizeof(content) == 2 && intp(content[0]))
    {
        send_error(content[0], content[1]);
        return;
    }

    Info["REDIRECT_STATUS"] = "200 OK";

    if (MSIE && content[0] != "text/plain")
        headers = make_headers(sz = sizeof(implode(content[1..<1], "\n")));
    else
        headers = make_headers(sz = sizeof(implode(content[1..<1], "\n")), content[0]);

    HTTPLOG("Notice", "httpd/thread", "rec_get", sprintf("Outgoing headers -- fd: %d\n%O", Fd, headers));

    ltime = localtime(time());
    logname = sprintf("/www/logs/access-%02d%02d%02d.log",
        ltime[LT_YEAR],
        ltime[LT_MON] + 1,
        ltime[LT_MDAY]
      );
    datetime = sprintf(DATEFMT,
      ltime[LT_MDAY],
      ShortMonths[ltime[LT_MON]],
      ltime[LT_YEAR],
      ltime[LT_HOUR],
      ltime[LT_MIN],
      ltime[LT_SEC],
      ltime[LT_ZONE]
    );
    LOG(logname, Info["REMOTE_ADDR"], "-", "-", datetime, Vars["REQUEST"], 200, sz, (Info["HTTP_REFERER"] ? Info["HTTP_REFERER"] : "-"), (Info["HTTP_USER_AGENT"] ? Info["HTTP_USER_AGENT"] : "-"));

    foreach (string line in headers)
        httpd->write_data(Fd, line + "\r\n");

    foreach (string line in content[1..<1])
        httpd->write_data(Fd, line + "\n");

    httpd->close_fd(Fd);
}

void rec_not_supported()
{
    send_error(405, "Method Not Allowed");
    return;
}
