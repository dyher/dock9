/* messages.c

   Tricky
   14-MAY-2007
   simul_efun

*/

int is_smiley(string msg)
{
    string *smilies = ({
      /* Some smilies from MSN Messenger */
      ":-)", ":)",
      ":-D", ":D",
      ":-O", ":O", ":-o", ":o",
      ":-P", ":P", ":-p", ":p",
      ";-)", ";)",
      ":-(", ":(",
      ":-S", ":S", ":-s", ":s",
      ":-|", ":|",
      ":'(",
      ":-$", ":$",
      ":-@", ":@",
      ":-#", ":#",
      "8o|",
      "8-|", "8|",
      "^o)",
      ":-*", ":*",
      "+o(",
      ":^)",
      "*-)", "*)",
      "<:o)",
      "8-)", "8)",
      "|-)", "|)",
      ":-[", ":[",
      ":-b", ":b",
      "o:-)", "o:)",

      /* Some smilies from Yahoo! Messenger */
      "=;",
      ";;)",
      "i-)", "i)",
      ">:D<",
      "l-)", "l)",
      ":-/", ":/"
      "o/",
      ":-&", ":&",
      ":X", ":x",
      ":\">",
      "[-(", "[(",
      ":o)",
      "8-}", "8}",
      "=((",
      "<:-p", "<:p",
      "(:|",
      "X(", "x(",
      "=P~", "=p~",
      ":>",
      ":-?", ":?",
      "B-)", "B)", "b-)", "b)",
      "#-O", "#O", "#-o", "#o",
      "=D>",
      "#:-S", "#:S", "#:-s", "#:s",
      ":-SS", ":SS", ":-ss", ":ss",
      ">:)",
      "@-)", "@)",
      ":-((", ":((",
      ":^O", ":^o",
      ":-))", ":))",
      ":-W", ":W", ":-w", ":w",
      ":-<", ":<",
      "/:)",
      ">:-P", ">:P", ">:-p", ">:p",
      "=))",
      "<):-)", "<):)",

      /* Miscellaneous */
      "><>", "<><",
      ":*)",
      "8//=/\\=",
    });

    foreach(string emoteicon in smilies)
    {

        if(sizeof(msg) < sizeof(emoteicon)) continue;
        else
        if(sizeof(msg) > sizeof(emoteicon))
        {

            if(msg[0..sizeof(emoteicon)] == emoteicon + " ")
                return 1;

        }
        else
        if(msg == emoteicon)
            return 1;

    }

    return 0;
}

int is_emote(string msg)
{
    return (msg[0..0] == ":" || is_smiley(msg));
}

int is_morse(string msg)
{
    int status;

    msg = trim(msg);

    if(member_array(" ", explode(msg, "")) == -1 && member_array("/", explode(msg, "")) == -1)
        return 0;

    status = sizeof(msg);

    for(int i = 0; i < sizeof(msg); i++)
        if(msg[i] != '-' && msg[i] != '.' && msg[i] != ' ' && msg[i] != '/')
            status--;

    return ((status * 100) / sizeof(msg) > 85);
}

varargs string translate_morse(string msg, int morse)
{
    mapping morse_to_text = ([
      ".-": "A", "-...": "B", "-.-.": "C", "-..": "D", ".": "E", "..-.": "F",
      "--.": "G", "....": "H", "..": "I", ".---": "J", "-.-": "K", ".-..": "L",
      "--": "M", "-.": "N", "---": "O", ".--.": "P", "--.-": "Q", ".-.": "R",
      "...": "S", "-": "T", "..-": "U", "...-": "V", ".--": "W", "-..-": "X",
      "-.--": "Y", "--..": "Z",
      "-----": "0", ".----": "1", "..---": "2", "...--": "3", "....-": "4",
      ".....": "5", "-....": "6", "--...": "7", "---..": "8", "----.": "9",
      ".-.-.-": ".", "--..--": ",", "---...": ":", "..--..": "?", ".----.": "'", "-....-": "-",
      "-..-.": "/", "-.--.": "(", "-.--.-": ")", ".-..-.": "\"", ".--.-.": "@", "-...-": "=",
    ]);
    mapping text_to_morse = ([ ]);
    string array translated = ({ });
    string tmp = "";

    for(int i = 0; i < sizeof(msg); i++)
    {
        int j = i + 1;

        tmp += msg[i..i];

        if(msg[i..j] == "  " || msg[i..j] == " /")
        {
            tmp += " ";

            while(msg[j] == ' ' || msg[j] == '/') j++;

            i = j - 1;
        }

    }

    msg = tmp;

    if(!undefinedp(morse) && morse != 0)
    {

        foreach(string morse_letter in explode(msg, " "))
            if(stringp(morse_to_text[morse_letter]))
                translated += ({ morse_to_text[morse_letter] });
            else
            if(morse_letter == "")
                translated += ({ " " });
            else
                translated += ({ "?unknown?" });

    }
    else
    if(undefinedp(morse) || morse == 0)
    {

        foreach(string morse_letter, string letter in morse_to_text)
            text_to_morse += ([ letter: morse_letter ]);

        foreach(string letter in explode(upper_case(msg), ""))
            if(stringp(text_to_morse[letter]))
                translated += ({ text_to_morse[letter], " " });
            else
            if(letter == " ")
                translated += ({ " " });
            else
                translated += ({ "" });

    }

    return trim(implode(translated, ""));
}

varargs string translate_iglatinpay(string msg, int iglatinpay)
{
    string array translated = ({ });

    if(!undefinedp(iglatinpay) && iglatinpay != 0)
    {
        translated = explode(msg, "");
    }
    else
    if(undefinedp(iglatinpay) || iglatinpay == 0)
    {
        string array vowels = ({ "a", "e", "i", "o", "u" });

        foreach(string word in explode(msg, " "))
        {
            string first, rest;
            int capital = (word[0..0] >= "A" && word[0..0] <= "Z"? 1 : 0);

            if(member_array(lower_case(word[0..0]), vowels) != -1)
                translated += ({ word });
            else
            {
                int j = 0, k = sizeof(word);

                for(int i = 0; i < k; i++)
                {

                    if(member_array(lower_case(word[i..i]), vowels) == -1) j++;
                    else break;

                }

                first = word[j..(k - 1) + j];
                rest = word[0..j - 1];

                if(capital)
                {
                    first = capitalize(first);
                    rest = lower_case(rest);
                }

                translated += ({ first, rest });
            }

            translated += ({ "ay", " " });
        }

    }

    return trim(implode(translated, ""));
}
