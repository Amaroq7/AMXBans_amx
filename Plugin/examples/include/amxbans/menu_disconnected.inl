/*

	AMXBans, managing bans for Half-Life modifications
	Copyright (C) 2003, 2004  Ronald Renes / Jeroen de Rover
	
	Copyright (C) 2009, 2010  Thomas Kurz

*/

#if defined _menu_disconnected_included
    #endinput
#endif
#define _menu_disconnected_included

new g_hMenuid;
new g_iAddedPlayers;
new g_iChoosedPlayer[33];
new g_iPos;

public plugin_init_disconnected()
{
	g_hMenuid = register_menuid("disconnected_players");
	register_menucmd(g_hMenuid, MENU_KEY_ALL, "disconnected_players_handler")
}

public cmdBanDisconnectedMenu(id,level,cid)
{
	if (!cmd_access(id,level,cid,1))
		return PLUGIN_HANDLED
		
	DisplayMenu_Disconnected(id, 0);
	
	return PLUGIN_HANDLED
}

public DisplayMenu_Disconnected(id, page)
{
	if(!g_iAddedPlayers)
	{
		client_print(id, print_chat, "[AMXBANS] %s", _T("No Players found in List!"));
		return PLUGIN_HANDLED;
	}
	
	new keys = MENU_KEY_0;
	new b;
	new iLen;
	
	g_iPage[id] = page;
		
	if(g_coloredMenus)
		iLen += formatex(menu,charsmax(menu),"\r%s\w^n^n",_T("The last players",id))
	else
		iLen += formatex(menu,charsmax(menu),"%s^n^n",_T("The last players",id))
		
	for(new i=page*7; i < next_page(page, g_iAddedPlayers, 7)*7;i++)
	{
		keys |= (1<<b);
		b++;
		
		if(g_coloredMenus)
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "\r%d. \w%s^n", b, g_disconPLname[i]);
		else
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "%d. %s^n", b, g_disconPLname[i]);
	}
	if(is_lastpage(page, g_iAddedPlayers, 7))
	{
		keys |= MENU_KEY_8;
		
		if(g_coloredMenus)
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "^n\r8.\w %s^n\r0.\w %s", _T("Back", id), _T("Exit", id));
		else
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "^n8. %s^n0. %s", _T("Back", id), _T("Exit", id));
	}
	else if(!is_firstpage(page))
	{
		keys |= MENU_KEY_8|MENU_KEY_9;
		if(g_coloredMenus)
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "^n\r8.\w %s^n\r9.\w %s^n\r0.\w %s", _T("Back", id), _T("More", id), _T("Exit", id));
		else
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "^n8. %s^n9. %s^n0. %s", _T("Back", id), _T("More", id), _T("Exit", id));
	}
	else
	{
		keys |= MENU_KEY_9;
		if(g_coloredMenus)
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "^n\r9.\w %s^n\r0.\w %s", _T("More", id), _T("Exit", id));
		else
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "^n9. %s^n0. %s", _T("More", id), _T("Exit", id));
	}
	
	show_menu(id, keys, menu, -1, "disconnected_players");
	return PLUGIN_HANDLED;
}

public disconnected_players_handler(id,key)
{
	if(key == 7)
	{
		DisplayMenu_Disconnected(id, --g_iPage[id])
		return PLUGIN_HANDLED;
	}
	else if(key == 8)
	{
		DisplayMenu_Disconnected(id, ++g_iPage[id])
		return PLUGIN_HANDLED;
	}
	
	g_iChoosedPlayer[id]=g_iPage[id]*7+key;
	
	formatex(g_choicePlayerName[id], charsmax(g_choicePlayerName[]), g_disconPLname[g_iChoosedPlayer[id]])
	formatex(g_choicePlayerAuthid[id], charsmax(g_choicePlayerAuthid[]), g_disconPLauthid[g_iChoosedPlayer[id]])
	formatex(g_choicePlayerIp[id], charsmax(g_choicePlayerIp[]), g_disconPLip[g_iChoosedPlayer[id]]);
	
	g_choicePlayerId[id]=-1
	
	if(get_cvarptr_num(pcvar_debug) >= 2)
		log_amx("[AMXBans PlayerDiscMenu %d] %d choice: %s | %s | %s | %d",menu,id,g_choicePlayerName[id],g_choicePlayerAuthid[id],g_choicePlayerIp[id],g_choicePlayerId[id])
	
	g_iPage[id] = 0;
	if(g_iAdminUseStaticBantime[id])
	{
		cmdReasonMenu(id, 0)
	}
	else
	{
		cmdBantimeMenu(id, 0)
	}
	
	return PLUGIN_HANDLED
}
/*************************************************************************************/
public cmdMenuBanDisc(id)
{
	if(!id)
		return PLUGIN_HANDLED
	
	if(!get_ban_type(g_ban_type[id],charsmax(g_ban_type),g_choicePlayerAuthid[id],g_choicePlayerIp[id])) {
		log_amx("[AMXBans Disc ERROR] Steamid / IP Invalid! Bantype: <%s> | Authid: <%s> | IP: <%s>",g_ban_type[id],g_choicePlayerAuthid[id],g_choicePlayerIp[id])
		return PLUGIN_HANDLED
	}
	
	if(get_cvarptr_num(pcvar_debug) >= 2) {
		log_amx("[AMXBans cmdMenuBanDisc %d] %d | %s | %s | %s | %s (%d min)",id,\
		g_choicePlayerId[id],g_choicePlayerName[id],g_choicePlayerAuthid[id],g_choicePlayerIp[id],g_choiceReason[id],g_choiceTime[id])
	}
	
	if (equal(g_ban_type[id], "S"))
	{
		formatex(pquery, charsmax(pquery),"SELECT player_id FROM %s%s WHERE player_id='%s' and expired=0", g_dbPrefix, tbl_bans, g_choicePlayerAuthid[id])
		if ( get_cvarptr_num(pcvar_debug) >= 2 )
			log_amx("[AMXBans cmdMenuBanDisc] Banned a player by SteamID")
	}
	else
	{
		formatex(pquery, charsmax(pquery),"SELECT player_ip FROM %s%s WHERE player_ip='%s' and expired=0", g_dbPrefix, tbl_bans, g_choicePlayerIp[id])
		if ( get_cvarptr_num(pcvar_debug) >= 2 )
			log_amx("[AMXBans cmdMenuBanDisc] Banned a player by IP/steamID")
	}
	
	_cmdMenuBanDisc(id);
	
	return PLUGIN_HANDLED	
}
public _cmdMenuBanDisc(id)
{
	if ( get_cvarptr_num(pcvar_debug) >= 1 )
		log_amx("[cmdMenuBanDisc function 2]")
	
	if (mysql_num_rows(g_SqlX)) {
		client_print(id,print_console,"[AMXBANS] %s",_T("Player is already banned."))
		g_being_banned[id] = false
		return PLUGIN_HANDLED
	}
	
	new admin_nick[64], admin_steamid[35], admin_ip[22]
	mysql_get_username_safe(id, admin_nick, charsmax(admin_nick))
	get_user_ip(id, admin_ip, charsmax(admin_ip), 1)
	get_user_authid(id, admin_steamid, charsmax(admin_steamid))
	
	new server_name[256]
	get_cvar_string("hostname", server_name, charsmax(server_name))
	
	if ( get_cvarptr_num(pcvar_add_mapname) == 1 ) {
		new mapname[32]
		get_mapname(mapname,31)
		format(server_name,charsmax(server_name),"%s (%s)",server_name,mapname)
	}
	new servername_safe[256]
	mysql_escape_string(server_name,servername_safe,charsmax(servername_safe))
	
	new player_name[64]
	mysql_escape_string(g_choicePlayerName[id],player_name,charsmax(player_name))
	
	formatex(g_disconPLauthid[g_iChoosedPlayer[id]], charsmax(g_disconPLauthid[]), "");
	formatex(g_disconPLip[g_iChoosedPlayer[id]], charsmax(g_disconPLip[]), "");
	formatex(g_disconPLname[g_iChoosedPlayer[id]], charsmax(g_disconPLname[]), "");
	
	if(--g_iAddedPlayers != 0) //Move entries if array is not clear
	{
		move_array(g_disconPLauthid, charsmax(g_disconPLauthid[]), MAX_DISCONNECTED_PLAYERS, g_iPos);
		move_array(g_disconPLip, charsmax(g_disconPLip[]), MAX_DISCONNECTED_PLAYERS, g_iPos);
		move_array(g_disconPLname, charsmax(g_disconPLname[]), MAX_DISCONNECTED_PLAYERS, g_iPos);
	}
	
	mysql_query(g_SqlX, "INSERT INTO `%s%s` (player_id,player_ip,player_nick,admin_ip,admin_id,admin_nick,ban_type,ban_reason,ban_created,ban_length,server_name,server_ip,expired) \
			VALUES('%s','%s','%s','%s','%s','%s','%s','%s',UNIX_TIMESTAMP(NOW()),%d,'%s','%s:%s',0)", \
			g_dbPrefix, tbl_bans, g_choicePlayerAuthid[id],g_choicePlayerIp[id],player_name,admin_ip,admin_steamid,admin_nick,g_ban_type[id],g_choiceReason[id],g_choiceTime[id],servername_safe,g_ip,g_port)
	
	__cmdMenuBanDisc(id);
	
	return PLUGIN_HANDLED
}
public __cmdMenuBanDisc(id)
{
	if ( get_cvarptr_num(pcvar_debug) >= 1 )
		log_amx("[AMXBans cmdMenuBanDisc function 3] %d: %s",id,g_choicePlayerName[id])
	
	if(mysql_insert_id(g_SqlX))
	{
		client_print(id,print_console,"[AMXBANS] %s",_T("Player has been banned!"))
	}
	else
	{
		client_print(id,print_console,"[AMXBANS] %s", _T("Banning failed!"))
	}
	return PLUGIN_HANDLED
}
/*************************************************************************************/
disconnect_remove_player(id)
{
	if(is_user_bot(id) || is_user_hltv(id))
		return PLUGIN_CONTINUE;
		
	new name[32];
	get_user_name(id,name,charsmax(name))
	
	for(new i=0;i < MAX_DISCONNECTED_PLAYERS;i++)
	{
		if(!equal(name,g_disconPLname[i]))
			continue;
		
		formatex(g_disconPLname[i], charsmax(g_disconPLname[]), "");
		formatex(g_disconPLip[i], charsmax(g_disconPLip[]), "");
		formatex(g_disconPLauthid[i], charsmax(g_disconPLauthid[]), "");
		
		if(--g_iAddedPlayers != 0)
		{
			move_array(g_disconPLauthid, charsmax(g_disconPLauthid[]), MAX_DISCONNECTED_PLAYERS, g_iPos);
			move_array(g_disconPLip, charsmax(g_disconPLip[]), MAX_DISCONNECTED_PLAYERS, g_iPos);
			move_array(g_disconPLname, charsmax(g_disconPLname[]), MAX_DISCONNECTED_PLAYERS, g_iPos);
		}
		
		break;
	}
	
	return PLUGIN_CONTINUE
}
disconnected_add_player(id)
{
	if(is_user_bot(id) || is_user_hltv(id))
		return PLUGIN_CONTINUE;
	
	new name[32],authid[35],ip[22]
	get_user_name(id,name,charsmax(name))
	get_user_authid(id,authid,charsmax(authid))
	get_user_ip(id,ip,charsmax(ip),1)
	
	ArrayPushString(g_disconPLname, charsmax(g_disconPLname[]), name, 1)
	ArrayPushString(g_disconPLauthid, charsmax(g_disconPLip[]), authid, 0)
	ArrayPushString(g_disconPLip, charsmax(g_disconPLname[]), ip, 0)
	
	move_array(g_disconPLauthid, charsmax(g_disconPLauthid[]), MAX_DISCONNECTED_PLAYERS, g_iPos);
	move_array(g_disconPLip, charsmax(g_disconPLip[]), MAX_DISCONNECTED_PLAYERS, g_iPos);
	move_array(g_disconPLname, charsmax(g_disconPLname[]), MAX_DISCONNECTED_PLAYERS, g_iPos);
	
	g_iAddedPlayers++;
	
	return PLUGIN_CONTINUE
}

ArrayPushString(array[][], array_size, string[], inc)
{
	if(g_iPos >= MAX_DISCONNECTED_PLAYERS)
	{
		g_iPos = 0;
	}
	formatex(array[g_iPos], array_size-1, string);
	
	if(inc)
		g_iPos++;
}