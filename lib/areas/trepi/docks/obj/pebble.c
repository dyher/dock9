inherit OBJECT;

void create()
{
    set_id(({"pebble","stone","rock"}));
    set_short("white pebble");
    set_long("A small white pebble.");
}

string help(){
    return("It's just a pebble, there's nothing you can do with it.");
}
