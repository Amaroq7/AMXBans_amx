/*

	AMXBans, managing bans for Half-Life modifications
	Copyright (C) 2003, 2004  Ronald Renes / Jeroen de Rover
	
	Copyright (C) 2009, 2010  Thomas Kurz

*/

#if defined _menu_ban_included
    #endinput
#endif
#define _menu_ban_included

new g_hMenuBan;
new g_hMenuTime;
new g_hMenuBanReason;
new g_iPage[33];
new g_iPlayers[33][32];
new g_iNum[33];

public plugin_init_banmenu()
{
	g_hMenuBan = register_menuid("menu_player");
	g_hMenuTime = register_menuid("menu_bantime");
	g_hMenuBanReason = register_menuid("menu_banreason");
	register_menucmd(g_hMenuBan, MENU_KEY_ALL, "actionBanMenu")
	register_menucmd(g_hMenuTime, MENU_KEY_ALL, "actionBantimeMenu")
	register_menucmd(g_hMenuBanReason, MENU_KEY_ALL, "actionReasonMenu");
}

public cmdBanMenu(id,level,cid)
{
	if (!cmd_access(id,level,cid,1))
		return PLUGIN_HANDLED
	
	g_iPage[id] = 0;
	cmdBanMenu2(id, 0)
	return PLUGIN_HANDLED
}
	
public cmdBanMenu2(id, page)
{
	new iLen, b = 1, keys = MENU_KEY_0|MENU_KEY_1;
	
	if(g_coloredMenus)
		iLen += formatex(menu, charsmax(menu), "\r%s\w^n^n", _T("Ban menu", id))
	else
		iLen += formatex(menu, charsmax(menu), "%s^n^n", _T("Ban menu", id))
	
	if(g_coloredMenus)
		iLen += formatex(menu[iLen], charsmax(menu)-iLen, "\r%d.\y %s\w^n^n", b, (!g_menuban_type[id]) ? _T("Ban instantly", id) : (g_menuban_type[id] == 1) ? _T("Ban after this round", id) : _T("Ban after this map", id))
	else
		iLen += formatex(menu[iLen], charsmax(menu)-iLen, "%d. %s^n^n", b, (!g_menuban_type[id]) ? _T("Ban instantly", id) : (g_menuban_type[id] == 1) ? _T("Ban after this round", id) : _T("Ban after this map", id))
	
	new iFlags, iFlags_admin = get_user_flags(id);
	get_players(g_iPlayers[id], g_iNum[id], "ch");
	
	for(new i=page*6;i<next_page(page, g_iNum[id], 6)*6;i++)
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
	
	if(is_lastpage(page, g_iNum[id], 6))
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
	
	show_menu(id, keys, menu, -1, "menu_player");
	return PLUGIN_HANDLED
}

public actionBanMenu(id,key)
{
	if(!key)
	{
		if(g_supported_game)
		{
			if(++g_menuban_type[id] > 2)
				g_menuban_type[id] = 0;
		}
		else
		{
			if(++g_menuban_type[id] > 1)
				g_menuban_type[id] = 0;
		}
			
		cmdBanMenu2(id, g_iPage[id]);
		
		return PLUGIN_HANDLED;
	}
	else if(key == 8)
	{
		cmdBanMenu2(id, --g_iPage[id]);
		return PLUGIN_HANDLED;
	}
	else if(key == 9)
	{
		cmdBanMenu2(id, ++g_iPage[id]);
		return PLUGIN_HANDLED;
	}
		
	g_choicePlayerId[id]=g_iPlayers[id][g_iPage[id]*6+key-1];
	
	if(!is_user_connected(g_choicePlayerId[id])) {
		client_print(id,print_chat,_T("Player has left the server and can't be banned."))
		client_cmd(id,"amx_bandisconnectedmenu")
		return PLUGIN_HANDLED
	}
	
	copy(g_choicePlayerName[id],charsmax(g_choicePlayerName[]),g_PlayerName[g_choicePlayerId[id]])
	get_user_authid(g_choicePlayerId[id],g_choicePlayerAuthid[id],charsmax(g_choicePlayerAuthid[]))
	get_user_ip(g_choicePlayerId[id],g_choicePlayerIp[id],charsmax(g_choicePlayerIp[]),1)
	
	if(get_cvarptr_num(pcvar_debug) >= 2)
		log_amx("[AMXBans PlayerMenu %d] %d choice: %d | %s | %s | %d",menu,id,g_choicePlayerName[id],g_choicePlayerAuthid[id],g_choicePlayerIp[id],g_choicePlayerId[id])
	
	//see if the admin can choose the bantime
	g_iPage[id] = 0;
	if(g_iAdminUseStaticBantime[id])
	{
		cmdReasonMenu(id, 0);
	}
	else
	{
		cmdBantimeMenu(id, 0);
	}
	return PLUGIN_HANDLED
}
public cmdBantimeMenu(id, page)
{
	new iLen, b, keys = MENU_KEY_0, szDisplay[128];
	
	if(g_coloredMenus)
		iLen += formatex(menu, charsmax(menu), "\r%s\w^n^n", _T("Bantime Menu", id))
	else
		iLen += formatex(menu, charsmax(menu), "%s^n^n", _T("Bantime Menu", id))
	
	if(!g_highbantimesnum || !g_lowbantimesnum)
	{
		log_amx("[AMXBans Notice] High or Low Bantimes empty, loading defaults")
		loadDefaultBantimes(0)
	}
	
	new szFlags[2];
	get_cvarptr_string(pcvar_higher_ban_time_admin, szFlags, charsmax(szFlags));
	
	if(get_user_flags(id) & read_flags(szFlags))
	{
		for(new i=page*7;i < next_page(page, g_highbantimesnum, 7)*7;i++)
		{
			get_bantime_string(id,g_HighBanMenuValues[i],szDisplay,charsmax(szDisplay))
			
			keys |= (1<<b);
			b++;
			
			if(g_coloredMenus)
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "\r%d.\w %s^n", b, szDisplay);
			else
				iLen += formatex(menu[iLen], charsmax(menu)-iLen, "%d. %s^n", b, szDisplay);
		}
	}
	if(is_lastpage(page, g_highbantimesnum, 7))
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
	show_menu(id, keys, menu, -1, "menu_bantime");
	return PLUGIN_HANDLED
}
public actionBantimeMenu(id,key)
{
	if(key == 8)
	{
		cmdBantimeMenu(id, --g_iPage[id]);
		return PLUGIN_HANDLED;
	}
	else if(key == 9)
	{
		cmdBantimeMenu(id, ++g_iPage[id]);
		return PLUGIN_HANDLED;
	}
	g_choiceTime[id]=g_HighBanMenuValues[g_iPage[id]*7+key]
	
	if(get_cvarptr_num(pcvar_debug) >= 2)
		log_amx("[AMXBans BantimeMenu %d] %d choice: %d min",menu,id,g_choiceTime[id])
	
	g_iPage[id] = 0;
	cmdReasonMenu(id, 0)
	
	return PLUGIN_HANDLED
}
public cmdReasonMenu(id, page)
{
	new iLen, b, keys = MENU_KEY_0, szDisplay[128], szTime[64]
	new custom_static_time = get_cvarptr_num(pcvar_custom_statictime)
	
	if(g_coloredMenus)
		iLen += formatex(menu, charsmax(menu), "\r%s\w^n^n", _T("Banreason Menu",id))
	else
		iLen += formatex(menu, charsmax(menu), "%s^n^n", _T("Banreason Menu",id))
	
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
	show_menu(id, keys, menu, -1, "menu_banreason");
	return PLUGIN_HANDLED
}
public actionReasonMenu(id,key)
{
	if(key == 8)
	{
		cmdReasonMenu(id, --g_iPage[id]);
		return PLUGIN_HANDLED;
	}
	else if(key == 9)
	{
		cmdReasonMenu(id, ++g_iPage[id]);
		return PLUGIN_HANDLED;
	}
	
	if(is_firstpage(g_iPage[id]) && !key)
	{
		if(g_iAdminUseStaticBantime[id])
			g_choiceTime[id]=get_cvarptr_num(pcvar_custom_statictime)
			
		set_custom_reason[id]=true
		client_cmd(id,"messagemode amxbans_custombanreason")
		return PLUGIN_HANDLED
	}
	else
	{
		new aid = g_iPage[id]*7+key;
		formatex(g_choiceReason[id],charsmax(g_choiceReason[]),g_banReasons[aid])
		if(g_iAdminUseStaticBantime[id]) g_choiceTime[id]=g_banReasons_Bantime[aid]
	}
	
	if(get_cvarptr_num(pcvar_debug) >= 2)
		log_amx("[AMXBans ReasonMenu %d] %d choice: %s (%d min)",menu,id,g_choiceReason[id],g_choiceTime[id])
	
	if(g_choicePlayerId[id] == -1)
	{
		//disconnected ban
		cmdMenuBanDisc(id)
	}
	else
	{
		cmdMenuBan(id)
	}
	return PLUGIN_HANDLED
}
