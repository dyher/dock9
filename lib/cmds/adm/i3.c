/* i3.c

 Tricky @ RtH
 9-JAN-07
 Executes an I3 function.

*/

int main(string str)
{
    string err, tmp;

    if (!adminp(previous_object()))
        return notify_fail("Error [i3]: Access denied.\n");
    if (!find_object(I3_CHDMOD))
        return notify_fail("Error [i3]: I3 daemon is not loaded.\n");
    if (!stringp(str))
    {
        write("Error [i3]: You must supply an argument. Syntax: i3 <function([args, ...])>\n");
        return 1;
    }
    if (!directory_exists(user_path(previous_object()->query_name()))) 
        return notify_fail("Error [i3]: You must have a home directory to use i3.\n");

    write("%^BOLD%^Evaluating:%^RESET%^ I3 function " + str + "\n\n");

    if (file_size(user_path(previous_object()->query_name()) + "tmp_eval_file.c") != -1)
        rm (user_path(previous_object()->query_name()) + "tmp_eval_file.c");
    if (find_object(user_path(previous_object()->query_name()) + "tmp_eval_file"))
        destruct(find_object(user_path(previous_object()->query_name()) + "tmp_eval_file"));

    tmp  = "#include <intermud3.h>\n";
    tmp += "\n";
    tmp += "mixed eval()\n";
    tmp += "{ return find_object(I3_CHDMOD)->" + str + "; }\n";

    write_file(user_path(previous_object()->query_name()) + "tmp_eval_file.c", tmp);
    err = catch(printf("Result = %O\n", load_object(user_path(previous_object()->query_name()) + "tmp_eval_file.c")->eval()));

    if (err) write("\n%^RED%^Runtime error:%^RESET%^\n " + err + "\n%^MAGENTA%^See logs for more details.%^RESET%^\n");

    rm(user_path(previous_object()->query_name()) + "tmp_eval_file.c");
    return 1;
}

string help(){
    return (HIW + " SYNTAX:" + NOR + " i3 <function>([args, ...])\n\n"
      + "This command will execute an I3 function with any supplied args.\n");
}

