/* npc.c

 Tacitus @ LPUniversity
 28-JUNE-06
 Non-player character

*/

inherit MOBILE;
string domain, *subdomains;
int wander_interval = 0, fucked_wander_interval = 0, wander_timer = 1;

void setup();
varargs int set_domains(string d, string *sd);
int query_domain(string d);
mapping query_domains();
int set_wander(int interval);
int query_wander();
void make_wander();
int commandHook(string arg);

void create()
{
    path = ({ "/cmds/std/" });
    set("prevent_get", 1);
    if(!query("cwd")) set("cwd", "/doc");
    if(!mapp(query("env_settings"))) set("env_settings", (["colour" : "enabled"]));
    if(!query_env("news_client")) set_env("news_client", "/obj/mudlib/newsclients/std_newsclient.c");
    if(!query_env("auto_tune")) set_env("auto_tune", "localNet");
    setup();
    enable_commands();
    add_action("commandHook", "", 1);

}

varargs int set_domains(string d, string *sd) {
    domain = d;
    set("domain", d);
    if (sd) {
        subdomains = sd;
        set("subdomains", sd);
    }
    return 1;
}

int query_domain(string d) {
    if (member_array(d, domain)) {
        return 1;
    } else if (member_array(d, subdomains)) {
        return 2;
    } else {
        return 0;
    }
}

mapping query_domains() {
    string copyd = domain;
    string *copysd = subdomains;
    return (["domain" : copyd, "subdomains" : copysd]);
}

int set_wander(int interval) {
    set_heart_beat(1);
    fucked_wander_interval = interval;
    set("fucked_wander_interval", interval);
    wander_interval = interval;
    set("wander_interval", interval);
    return 1;
}

int query_wander() {
    int copy;
    copy = wander_interval;
    return wander_interval;
} 

void make_wander() {
    string *exits_v = values(environment(this_object())->query_exits());
    string *exits_k = keys(environment(this_object())->query_exits());
    for (int i = 0; i < sizeof(exits_k); i++) {
        object room = load_object(exits_v[i]);
        if (subdomains) {
            for(int j = 0; j < sizeof(subdomains); j++) {
                if(room->query_domain(subdomains[j]) == 2) {
                    exits_k -= ({exits_k[i]});
                    exits_v -= ({exits_v[i]});
                }
            }
        } else if (domain) {
            if(room->query_domain(domain) != 1) {
                exits_k -= ({exits_k[i]});
                exits_v -= ({exits_v[i]});
            }
        }
    }
    force_me(exits_k[random(sizeof(exits_k)-1)]);
}


void setup()
{
    set_name("Generic NPC");
    set_living_name(lower_case(query_name()));
    set_ids(({query_name(), "npc", "generic npc", "NPC" }));
    if(clonep(this_object())) set_heart_beat(1);
    set_long(capitalize(query_name()) + " is a generic NPC.");
    set_short(query_name());
}

void restore_user()
{
    return;
}

void save_user()
{
    return;
}

int force_me(string cmd)
{
    command(cmd);
}

void heart_beat()
{
    ::heart_beat();
    if (wander_interval) {
        if (wander_timer % fucked_wander_interval == 0) {
            if(wander_interval >= 10) fucked_wander_interval = wander_interval+(random(wander_interval*0.20)-((wander_interval*0.20)/2));
            make_wander();
            wander_timer = 1;
        } else {
            wander_timer++;
        }
    }
    return;
}

int commandHook(string arg)
{
    return ::commandHook(arg);
}

