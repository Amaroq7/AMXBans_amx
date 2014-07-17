/*

	AMXBans, managing bans for Half-Life modifications
	Copyright (C) 2003, 2004  Ronald Renes / Jeroen de Rover
	
	Copyright (C) 2009, 2010  Thomas Kurz

*/

#if defined _menu_flag_included
    #endinput
#endif
#define _menu_flag_included

new g_hFlagMenu;
new g_hUnflagMenu;
new g_hFlagtimeMenu;
new g_hFlagReasonMenu;

public plugin_init_flagging()
{
	g_hFlagMenu = register_menuid("menu_flagplayer");
	g_hUnflagMenu = register_menuid("menu_unflagplayer");
	g_hFlagtimeMenu = register_menuid("menu_flagtime");
	g_hFlagReasonMenu = register_menuid("menu_flagreason");
	register_menucmd(g_hFlagMenu, MENU_KEY_ALL, "actionFlaggingMenu");
	register_menucmd(g_hUnflagMenu, MENU_KEY_0|MENU_KEY_1|MENU_KEY_2, "actionUnflagMenu");
	register_menucmd(g_hFlagtimeMenu, MENU_KEY_ALL, "actionFlagtimeMenu");
	register_menucmd(g_hFlagReasonMenu, MENU_KEY_ALL, "actionFlagReasonMenu");
}

public cmdFlaggingMenu(id,level,cid)
{
	if (!cmd_access(id,level,cid,1))
		return PLUGIN_HANDLED
		
	g_iPage[id] = 0;
	cmdFlaggingMenu2(id, 0);
	return PLUGIN_HANDLED
}
public cmdFlaggingMenu2(id, page)
{
	new iLen, b, keys = MENU_KEY_0;
	
	if(g_coloredMenus)
		iLen += formatex(menu, charsmax(menu), "\r%s\w^n^n", _T("Flag player",id));
	else
		iLen += formatex(menu, charsmax(menu), "%s^n^n", _T("Flag player",id));
	
	new iFlags, iFlags_admin = get_user_flags(id);
	get_players(g_iPlayers[id], g_iNum[id], "ch");
	
	for(new i=page*7;i<next_page(page, g_iNum[id], 7)*7;i++)
	{
		get_user_name(g_iPlayers[id][i], g_PlayerName[i],charsmax(g_PlayerName[]));
		
		iFlags = get_user_flags(g_iPlayers[id][i]);
		if((iFlags & ADMIN_IMMUNITY) && !(iFlags_admin & ADMIN_SUPREME))
		{
			if(g_coloredMenus)
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "\d#.\w %s\r *\w^n", g_PlayerName[i]);
			else
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "#. %s *^n", g_PlayerName[i]);
		}
		else if((iFlags & ADMIN_IMMUNITY) && (iFlags_admin & ADMIN_SUPREME))
		{
			keys |= (1<<b)
			b++;
			
			if(g_coloredMenus)
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "\r%d.\w %s\r *\w^n", b, g_PlayerName[i]);
			else
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "%d. %s *^n", b, g_PlayerName[i]);
				
		}
		
		else if(!iFlags || iFlags & ADMIN_USER)
		{
			keys |= (1<<b)
			b++;
			
			if(g_coloredMenus)
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "\r%d.\w %s^n", b, g_PlayerName[i]);
			else
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "%d. %s^n", b, g_PlayerName[i]);
				
		}
		else if((iFlags & ADMIN_SUPREME) && (iFlags_admin && ADMIN_SUPREME))
		{
			keys |= (1<<b)
			b++;
			
			if(g_coloredMenus)
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "\r%d.\w %s\r *\w^n", b, g_PlayerName[i]);
			else
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "%d. %s *^n", b, g_PlayerName[i]);
		}
	}
	
	if(is_lastpage(page, g_iNum[id], 7))
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
	
	show_menu(id, keys, menu, -1, "menu_flagplayer");
	return PLUGIN_HANDLED
}
public actionFlaggingMenu(id,key)
{
	if(key == 7)
	{
		cmdFlaggingMenu2(id, --g_iPage[id]);
		return PLUGIN_HANDLED;
	}
	else if(key == 8)
	{
		cmdFlaggingMenu2(id, ++g_iPage[id]);
		return PLUGIN_HANDLED;
	}
	else if(key == 9)
		return PLUGIN_HANDLED;
	
	new pid=g_iPlayers[id][g_iPage[id]*7+key];
	
	copy(g_choicePlayerName[id],charsmax(g_choicePlayerName[]),g_PlayerName[pid])
	get_user_authid(pid,g_choicePlayerAuthid[id],charsmax(g_choicePlayerAuthid[]))
	get_user_ip(pid,g_choicePlayerIp[id],charsmax(g_choicePlayerIp[]),1)
	g_choicePlayerId[id]=pid
	
	if(get_cvarptr_num(pcvar_debug) >= 2)
		log_amx("[AMXBans FlagPlayerMenu %d] %d choice: %d | %s | %s | %d",g_hFlagMenu,id,g_choicePlayerName[id],g_choicePlayerAuthid[id],g_choicePlayerIp[id],g_choicePlayerId[id])
	
	g_iPage[id] = 0;
	if(g_being_flagged[pid])
		cmdUnflagMenu2(id, 0)
	else
		cmdFlagtimeMenu(id, 0)
		
	return PLUGIN_HANDLED
}
public cmdUnflagMenu(id,level,cid)
{
	if(!cmd_access(id,level,cid,1))
		return PLUGIN_HANDLED
	
	g_iPage[id] = 0;
	cmdUnflagMenu2(id, 0);
	return PLUGIN_HANDLED;
}
public cmdUnflagMenu2(id, page)
{
	new iLen;
	new szTime[64]
	
	if(g_coloredMenus)
		iLen += formatex(menu, charsmax(menu), "\r%s\w^n^n", _T("Player already flagged! Options:",id));
	else
		iLen += formatex(menu, charsmax(menu), "%s^n^n", _T("Player already flagged! Options:",id));
	
	get_flagtime_string(id,g_flaggedTime[g_choicePlayerId[id]],szTime,charsmax(szTime),1)
	if(g_coloredMenus)
		format(szTime,charsmax(szTime),"\y(%s: %s)\w",szTime,g_flaggedReason[g_choicePlayerId[id]])
	else
		format(szTime,charsmax(szTime),"(%s: %s)",szTime,g_flaggedReason[g_choicePlayerId[id]])
		
	if(g_coloredMenus)
		iLen += formatex(menu[iLen], charsmax(menu)-iLen, "\r1.\w %s %s^n\r2.\w %s^n^n\r0.\w %s", _T("Remove flag",id), szTime, _T("Set new flag",id), _T("Exit",id));
	else
		iLen += formatex(menu[iLen], charsmax(menu)-iLen, "1. %s %s^n2. %s^n^n0. %s", _T("Remove flag",id), szTime, _T("Set new flag",id), _T("Exit",id));
	
	show_menu(id, MENU_KEY_0|MENU_KEY_1|MENU_KEY_2, menu, -1, "menu_unflagplayer");
	return PLUGIN_HANDLED
}
public actionUnflagMenu(id,key)
{
	g_iPage[id] = 0;
	if(key == 9)
		return PLUGIN_HANDLED;
	
	else if(!key)
	{
		UnflagPlayer(id,1)
	}
	else if(key == 1)
	{
		UnflagPlayer(id,0)
		cmdFlagtimeMenu(id, 0)
	}
	
	return PLUGIN_HANDLED
}
public cmdFlagtimeMenu(id, page)
{
	new iLen, b, keys = MENU_KEY_0;
	
	if(g_coloredMenus)
		iLen += formatex(menu, charsmax(menu), "\r%s\w^n^n", _T("Flagtime menu",id));
	else
		iLen += formatex(menu, charsmax(menu), "%s^n^n", _T("Flagtime menu",id));
		
	if(!g_flagtimesnum)
	{
		log_amx("[AMXBans Notice] Flagtimes empty, loading defaults")
		loadDefaultBantimes(3)
	}
	
	new szDisplay[128];
	for(new i=page*7;i < next_page(page, g_flagtimesnum, 7)*7;i++)
	{
		get_flagtime_string(id,g_FlagMenuValues[i],szDisplay,charsmax(szDisplay))
		
		keys |= (1<<b)
		b++;
		
		if(g_coloredMenus)
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "\r%d.\w %s^n", b, szDisplay);
		else
			iLen += formatex(menu[iLen], charsmax(menu)-iLen, "%d. %s^n", b, szDisplay);
	}
	
	if(is_lastpage(page, g_flagtimesnum, 7))
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
	show_menu(id, keys, menu, -1, "menu_flagtime");
	return PLUGIN_HANDLED
}
public actionFlagtimeMenu(id,key)
{
	if(key == 9)
		return PLUGIN_HANDLED;
		
	else if(key == 8)
	{
		cmdFlagtimeMenu(id, ++g_iPage[id]);
		return PLUGIN_HANDLED;
	}
	else if(key == 7)
	{
		cmdFlagtimeMenu(id, --g_iPage[id]);
		return PLUGIN_HANDLED;
	}
	
	g_choiceTime[id]=g_FlagMenuValues[g_iPage[id]*7+key];
	
	if(get_cvarptr_num(pcvar_debug) >= 2)
		log_amx("[AMXBans FlagtimeMenu %d] %d choice: %d min",g_hFlagtimeMenu,id,g_choiceTime[id])
	
	g_iPage[id] = 0;
	cmdFlagReasonMenu(id, 0)
	
	return PLUGIN_HANDLED
}
public cmdFlagReasonMenu(id, page)
{
	new iLen, b, keys = MENU_KEY_0, szDisplay[128], szTime[64];
	new custom_static_time = get_cvarptr_num(pcvar_custom_statictime)
	
	if(g_coloredMenus)
		iLen += formatex(menu, charsmax(menu), "\r%s\w^n^n", _T("Flagging-Reason Menu",id));
	else
		iLen += formatex(menu, charsmax(menu), "%s^n^n", _T("Flagging-Reason Menu",id));
	
	for(new i=page*7;i < next_page(page, g_iLoadedReasons, 7)*7;i++)
	{
		if(is_firstpage(page))
		{
			if(custom_static_time >= 0)
			{
				keys |= (1<<b);
				b++;
				
				if(g_coloredMenus)
					formatex(szDisplay, charsmax(szDisplay), "\r%d.\w %s^n", b, _T("Userdefined reason",id))
				else
					formatex(szDisplay, charsmax(szDisplay), "%d. %s^n", b, _T("Userdefined reason",id))
					
				if(g_iAdminUseStaticBantime[id])
				{
					get_bantime_string(id,custom_static_time,szTime,charsmax(szTime))
					format(szDisplay,charsmax(szDisplay),"%s (%s)",szDisplay,szTime)
				}
			}
		}
		else
		{
			keys |= (1<<b);
			b++;
			
			if(g_coloredMenus)
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "\r%d.\w %s^n^n", b, g_banReasons[i]);
			else
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "%d. %s^n^n", b, g_banReasons[i]);
				
			if(g_iAdminUseStaticBantime[id])
			{
				get_bantime_string(id,g_banReasons_Bantime[i],szTime,charsmax(szTime))
				format(szDisplay,charsmax(szDisplay),"%s (%s)",szDisplay,szTime)
			} 
		}
	}
	if(is_lastpage(page, g_iLoadedReasons, 7))
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
	show_menu(id, keys, menu, -1, "menu_flagreason");
	return PLUGIN_HANDLED
}
public actionFlagReasonMenu(id,key)
{
	if(key == 9)
		return PLUGIN_HANDLED;
	else if(key == 8)
	{
		cmdFlagReasonMenu(id, ++g_iPage[id]);
		return PLUGIN_HANDLED;
	}
	else if(key == 7)
	{
		cmdFlagReasonMenu(id, --g_iPage[id]);
		return PLUGIN_HANDLED;
	}
	
	if(is_firstpage(g_iPage[id]) && !key)
	{
		if(g_iAdminUseStaticBantime[id]) g_choiceTime[id]=get_cvarptr_num(pcvar_custom_statictime)
		g_in_flagging[id]=true
		set_custom_reason[id]=true
		client_cmd(id,"messagemode amxbans_custombanreason")
		return PLUGIN_HANDLED
	}
	else
	{
		new aid = g_iPage[id]*7+key;
		formatex(g_choiceReason[id],charsmax(g_choiceReason[]), g_banReasons[aid])
		if(g_iAdminUseStaticBantime[id]) g_choiceTime[id]=g_banReasons_Bantime[aid]
	}
	
	if(get_cvarptr_num(pcvar_debug) >= 2)
		log_amx("[AMXBans FlagReasonMenu %d] %d choice: %s (%d min)",g_hFlagReasonMenu,id,g_choiceReason[id],g_choiceTime[id])
	
	FlagPlayer(id)
	return PLUGIN_HANDLED
}
/*******************************************************************************************************************/
FlagPlayer(id)
{
	if(get_cvarptr_num(pcvar_debug) >= 2)
		log_amx("[AMXBans FlagPlayer %d] %d | %s | %s | %s | %s | %d min ",id,\
			g_choicePlayerId[id],g_choicePlayerName[id],g_choicePlayerAuthid[id],g_choicePlayerIp[id],\
			g_choiceReason[id],g_choiceTime[id])
			
	new anick[64],aauthid[35],aip[22],pname[64]
	mysql_get_username_safe(id,anick,charsmax(anick))
	get_user_authid(id,aauthid,charsmax(aauthid))
	get_user_ip(id,aip,charsmax(aip),1)
	
	mysql_escape_string(g_choicePlayerName[id],pname,charsmax(pname))
	
	mysql_query(g_SqlX, "INSERT INTO `%s%s` (`player_ip`,`player_id`,`player_nick`,\
		`admin_ip`,`admin_id`,`admin_nick`,`reason`,`created`,`length`,`server_ip`) VALUES \
		('%s','%s','%s','%s','%s','%s','%s',UNIX_TIMESTAMP(NOW()),'%d','%s:%s')",g_dbPrefix, tbl_flagged, \
		g_choicePlayerIp[id],g_choicePlayerAuthid[id],pname,aip,aauthid,anick,\
		g_choiceReason[id],g_choiceTime[id],g_ip,g_port)
	
	g_in_flagging[id]=false
	
	_FlagPlayer(id);
	
	return PLUGIN_HANDLED
	
}
UnflagPlayer(id,announce=0)
{
	if(get_cvarptr_num(pcvar_debug) >= 2)
		log_amx("[AMXBans UnflagPlayer %d] %d | %s",id,g_choicePlayerId[id],g_choicePlayerName[id])
	
	formatex(pquery, charsmax(pquery), "DELETE FROM `%s%s` WHERE `player_id`='%s' OR `player_ip`='%s'",g_dbPrefix, tbl_flagged, \
		g_choicePlayerAuthid[id],g_choicePlayerIp[id])
	
	if(!get_cvarptr_num(pcvar_flagged_all))
		format(pquery, charsmax(pquery),"%s AND `server_ip`='%s:%s'",pquery,g_ip,g_port)
	
	mysql_query(g_SqlX, pquery)
	
	g_in_flagging[id]=false
	
	_UnflagPlayer(id, announce);
	
	return PLUGIN_HANDLED
}
/*******************************************************************************************************************/
public _FlagPlayer(id)
{
	if(mysql_affected_rows(g_SqlX))
	{
		client_print(id,print_chat,_T("[AMXBans] You have flagged player %s",id),g_choicePlayerName[id])
		g_being_flagged[g_choicePlayerId[id]]=true
		g_flaggedTime[g_choicePlayerId[id]]=g_choiceTime[id]
		copy(g_flaggedReason[g_choicePlayerId[id]],charsmax(g_flaggedReason[]),g_choiceReason[id])
		
		amxbans_player_flagged(g_choicePlayerId[id], (g_choiceTime[id]*60), g_choiceReason[id]);
	}
	else
	{ 
		client_print(id,print_chat,_T("[AMXBans] Flagging of player %s failed",id),g_choicePlayerName[id])
		g_being_flagged[g_choicePlayerId[id]]=false
	}
	return PLUGIN_HANDLED
}
public _UnflagPlayer(id, announce)
{
	if(mysql_affected_rows(g_SqlX))
	{
		if(announce)
		{
			client_print(id,print_chat,_T("[AMXBans] You removed the flag of player %s",id),g_choicePlayerName[id])
		}
		g_being_flagged[g_choicePlayerId[id]]=false
		amxbans_player_unflagged(g_choicePlayerId[id]);
	}
	else
	{ 
		client_print(id,print_chat,_T("[AMXBans] Flag removing from player %s failed",id),g_choicePlayerName[id])
	}
	return PLUGIN_HANDLED
}
