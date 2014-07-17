/*

	AMXBans, managing bans for Half-Life modifications
	Copyright (C) 2003, 2004  Ronald Renes / Jeroen de Rover
	
	Copyright (C) 2009, 2010  Thomas Kurz

*/

#if defined _menu_history_included
    #endinput
#endif
#define _menu_history_included

new g_hMenuHistory;

public plugin_init_hostory()
{
	g_hMenuHistory = register_menuid("menu_history");
	register_menucmd(g_hMenuHistory, MENU_KEY_ALL, "actionHistoryMenu")
}

public cmdBanhistoryMenu(id,level,cid)
{
	if(!cmd_access(id,level,cid,1))
		return PLUGIN_HANDLED
	
	g_iPage[id] = 0;
	cmdBanhistoryMenu2(id,0)
	
	return PLUGIN_HANDLED
}

public cmdBanhistoryMenu2(id,page)
{
	new iLen, b, keys = MENU_KEY_0, szName[32];
	
	get_players(g_iPlayers[id], g_iNum[id], "ch");
	
	for(new i=page*7;i<next_page(page, g_iNum[id], 7)*7;i++)
	{
		get_user_name(g_iPlayers[id][i], szName,charsmax(szName));
		
		keys |= (1<<b)
		b++;
			
		if(g_coloredMenus)
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "\r%d.\w %s^n", b, szName);
		else
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "%d. %s^n", b, szName);
				
	}
	show_menu(id, keys, menu, -1, "menu_history");
	return PLUGIN_HANDLED
}
public actionHistoryMenu(id,key)
{
	new pid=g_iPlayers[id][g_iPage[id]*7+key];
	
	mysql_query(g_SqlX, "SELECT amxban_motd FROM `%s%s` WHERE address = '%s:%s'", g_dbPrefix, tbl_serverinfo, g_ip, g_port)
	
	select_motd_history(id, pid)
	
	return PLUGIN_HANDLED
}
public select_motd_history(id, pid)
{
	new authid[35],name[32]
	get_user_authid(pid,authid,charsmax(authid))
	get_user_name(pid,name,charsmax(name))
	
	new motd_url[256]
	if (!mysql_num_rows(g_SqlX)) {
		return PLUGIN_HANDLED	
	}
	
	mysql_nextrow(g_SqlX);
	mysql_getfield(g_SqlX, 0, motd_url, 256)
	
	//http://URL/motd.php?sid=%s&adm=%d&lang=%s
	if(contain(motd_url,"?sid=%s&adm=%d&lang=%s") != -1)
	{
		new url[256],lang[5],title[128]
		
		formatex(title,charsmax(title),_T("Banhistory of %s",id),name)
		
		get_user_info(id,"lang",lang,charsmax(lang))
		if(equal(lang,""))
			get_cvar_string("amx_language",lang,charsmax(lang))
		
		//copy(url,charsmax(url),g_motdurl)
		formatex(url,charsmax(url),motd_url,authid,1,lang)
		if(get_cvarptr_num(pcvar_debug) >= 2)
			log_amx("[AMXBans BanHistory Motd] %s",url)
		
		show_motd(id,url,title)
	}
	else
	{
		log_amx("[AMXBans ERROR BanHistory] %s",_T("No MOTD-Entry has been found. Set it in the webGUI -> Servers!"))
		client_print(id,print_chat,"[AMXBans] %s",_T("No MOTD-Entry has been found. Set it in the webGUI -> Servers!"))
	}
	return PLUGIN_HANDLED
}
