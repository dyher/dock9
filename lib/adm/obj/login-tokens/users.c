string parseTokens(string text)
{
    string array list = filter_array(users()->query_cap_name(), 
    	(: stringp($1) :));
    string userList;
    
    switch(sizeof(list))
    {
    	case 0:  userList = "";
    		 break;
    	case 1: userList = list[0] + "\n"; 
    		break;
    	default: userList = implode(list[0..<2], ", ") + " and " + list[<1] 
    			+ "\n";
    		break;
    }
    
    text = replace_string(text, "%users", "\n" + userList);
    return text;
}
