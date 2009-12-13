/* file.c
 *
 * Various authors.
 * Filesystem simuls.
 *
 */

#include <config.h>

//file_owner(string file);

//Tacitus @ LPUniversity
//02-APR-05
//Simul-Efun

//Last edited on July 1st, 2005 by Tacitus

string file_owner(string file)
{
    string temp;
    
    if (file[0] != '/') file = "/" + file;

    if (sscanf(file, "/home/%s/%s/%*s", temp, temp) == 2) {
        return temp;
    }
    if(sscanf(file, "/adm/%s/%*s", temp) == 2) {
     return "Admin/" + temp;
     }
          
    return 0;
}

//file_lines(string file);

//Gwegster @ LPUniversity
//30-JUNE-05
// A simul_efun that takes the specified file and checks for
//      the amount of "\n"s in the file, thereby getting the
//      total amount of lines in the file.

int file_lines(string file)
{
     string str;
     string *lines;

     if(file_size(file) < 1)
     return 0;

     str = read_file(file);
     lines = explode(str + "", "\n");
     return sizeof(lines);
}

// tail(string file)

// Gwegster @ LPUniversity
// 30-JULY-05
// A simple tail command

/* Changes:

   Gwegster, 06/30/05 - changed default morelines to 20
   Gwegster, 06/30/05 - made tail a simul_efun
*/ 

// function: tail
// returns: -1 for no file, 0 if nothing entered, and 1 if it all worked

int tail(string file)
{
  if(!file)
  {
    return 0;
  }

  file = resolve_path(this_player()->query("cwd"),file);
  
  if(!file_exists(file))
  {
     return -1;
  }

  if(!this_player()->query("morelines"))
  {
    this_player()->set("morelines", 20);
  }
  
  write(read_file(file, (file_lines(file) - this_player()->query("morelines")), (file_lines(file))));
  write("\n");
  return 1;
}

//log_file(string file, string str)

//Tacitus @ LPUniversity
//15-APR-05
//Simul-efuns

int log_file(string file, string str)
{
     if(!file || !str) return 0;
     master()->log_file(file, str);
     return 1;
}

/*
 * Copyright 2006 Doug Swarin (Zac@StarMUD)
 *   http://www.starmud.com/
 *   telnet://home.starmud.com:4000
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

varargs string *glob_array (string *paths, string cwd) {
  int i;
  string root, *path, *globbed;

  if (!stringp(cwd))
    cwd = "/";

  paths = map(paths - ({ "" }), (: resolve_path($2, $1) :), cwd);

  for (i = 0; i < sizeof(paths); i++) {
    if (sizeof(path = explode(paths[i], "/") - ({ "" })) < 1) {
      paths[i] = "/";
      continue;
    }

    if (strsrch(path[<1], "?") >= 0 || strsrch(path[<1], "*") >= 0) {
      if (pointerp(globbed = get_dir("/" + implode(path, "/"))))
        globbed -= ({ ".", ".." });
      else
        globbed = ({ });

      root = replace_string("/" + implode(path[0..<2], "/") + "/", "//", "/");

      paths = paths[0..(i - 1)] +
              map(globbed, (: $2 + $1 :), root) +
              paths[(i + 1)..];

      i += sizeof(globbed) - 1;
    }
  }

  return paths;
}
