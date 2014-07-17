/*

	AMXBans, managing bans for Half-Life modifications
	Copyright (C) 2003, 2004  Ronald Renes / Jeroen de Rover
	
	Copyright (C) 2009, 2010  Thomas Kurz

*/

#if defined _menu_stocks_included
    #endinput
#endif
#define _menu_stocks_included

stock get_pages(var, ratio)
{
	new iPages = (var / ratio)
	iPages += (var % ratio) ? 1 : 0;
	
	return iPages;
}

stock is_lastpage(page, var, ratio)
{
	return (page + 1 > get_pages(var, ratio)) ? 1 : 0;
}

stock next_page(page, var, ratio)
{
	return (page + 1 > get_pages(var, ratio)) ? page+1 : get_pages(var, ratio);
}

stock is_firstpage(page)
{
	return (!page) ? 1 : 0;
}

move_array(array[][], array_size, size, &position)
{
	new bool:empty, pos;
	for(new i=0;i<size;i++)
	{
		if(empty && strlen(array[i]))
		{
			pos = i;
			formatex(array[--pos], array_size-1, array[i]);
			formatex(array[i], array_size-1, "");
			empty = true;
		}
		else if(!strlen(array[i]))
		{
			empty = true;
		}
		else
			empty = false;
	}
	reset_pos(array, size, position);
}

reset_pos(array[][], size, &var)
{
	var = 0;
	for(new i=0;i<size;i++)
	{
		if(strlen(array[i]))
			var++;
	}
}

/*******************************************************************************************************************/
/*stock MenuGetPlayers(menu,callback) {
	new plnum = get_maxplayers()
	new szID[3],count
	
	for(new i=1;i <= plnum;i++) {
		if(!is_user_connected(i)) continue
		count++
		get_user_name(i,g_PlayerName[i],charsmax(g_PlayerName[]))
		num_to_str(i,szID,charsmax(szID))
		menu_additem(menu,g_PlayerName[i],szID,0,callback)
	}
}

stock MenuGetBantime(id,menu) {
	if(!g_highbantimesnum || !g_lowbantimesnum) {
		log_amx("[AMXBans Notice] High or Low Bantimes empty, loading defaults")
		loadDefaultBantimes(0)
	}
	
	new szDisplay[128],szTime[11]
	// Admins with flag n or what HIGHER_BAN_TIME_ADMIN is set to, will get the higher ban times
	if (get_user_flags(id) & get_higher_ban_time_admin_flag()) {
		for(new i;i < g_highbantimesnum;i++) {
			get_bantime_string(id,g_HighBanMenuValues[i],szDisplay,charsmax(szDisplay))
			num_to_str(g_HighBanMenuValues[i],szTime,charsmax(szTime))
			menu_additem(menu,szDisplay,szTime)
		}
	} else {
		for(new i;i < g_lowbantimesnum;i++) {
			get_bantime_string(id,g_LowBanMenuValues[i],szDisplay,charsmax(szDisplay))
			num_to_str(g_LowBanMenuValues[i],szTime,charsmax(szTime))
			menu_additem(menu,szDisplay,szTime)
		}
	}
}
stock MenuGetFlagtime(id,menu) {
	if(!g_flagtimesnum) {
		log_amx("[AMXBans Notice] Flagtimes empty, loading defaults")
		loadDefaultBantimes(3)
	}
	
	new szDisplay[128],szTime[11]
	for(new i;i < g_flagtimesnum;i++) {
		get_flagtime_string(id,g_FlagMenuValues[i],szDisplay,charsmax(szDisplay))
		num_to_str(g_FlagMenuValues[i],szTime,charsmax(szTime))
		menu_additem(menu,szDisplay,szTime)
	}
}*/
/*******************************************************************************************************************/
stock get_bantime_string(id,btime,text[],len)
{
	if(btime <=0)
	{
		formatex(text,len,_T("Permanent ban", id))
	}
	else
	{
		new szTime[64]
		get_time_length(id,btime,timeunit_minutes,szTime,charsmax(szTime))
		formatex(text,len,_T("Ban for %s",id),szTime)
	}
}
/*stock get_flagtime_string(id,btime,text[],len,without=0) {
	if(btime <=0 ) {
		if(!without) {
			formatex(text,len,"%L",id,"FLAG_PERMANENT")
		} else {
			formatex(text,len,"%L",id,"PERMANENT")
		}
	} else {
		if(!without) {
			new szText[128]
			get_time_length(id,btime,timeunit_minutes,szText,charsmax(szText))
			formatex(text,len,"%L",id,"FLAG_FOR_MINUTES",szText)
		} else {
			get_time_length(id,btime,timeunit_minutes,text,len)
		}
	}
}*/
/*******************************************************************************************************************/
/*
user_viewing_menu() {
	new menu,newmenu,menupage
	new pnum=get_maxplayers()
	for(new i=1;i<=pnum;i++) {
		if(!is_user_connected(i) || is_user_bot(i) || is_user_hltv(i)) continue
		
		if(player_menu_info(i,menu,newmenu,menupage)) {
			if(newmenu != -1) {
				client_print(i,print_chat,"[AMXBans] %L", LANG_PLAYER, "UPDATE_MENU", newmenu,menupage)
				menu_destroy(newmenu)
				menu_display(i,newmenu,menupage)
			} 
		}else {
			client_print(i,print_chat,"[AMXBans] %L", LANG_PLAYER, "NO_MENU_OPENED")
			
		}
	}
}
*/
/*******************************************************************************************************************/
/*public setCustomBanReason(id,level,cid)
{
	if (!cmd_access(id,level,cid,1)) {
		return PLUGIN_HANDLED
	}

	if(!set_custom_reason[id]) return PLUGIN_HANDLED
	
	new szReason[128]
	read_argv(1,szReason,127)
	copy(g_choiceReason[id],127,szReason)
	
	set_custom_reason[id]=false
	
	if(get_pcvar_num(pcvar_debug) >= 2)
		log_amx("[AMXBans CustomReason] %d choice: %s (%d min)",id,g_choiceReason[id],g_choiceTime[id])
	
	if(g_in_flagging[id]){
		g_in_flagging[id]=false
		FlagPlayer(id)
	} else if(g_choicePlayerId[id] == -1) {
		//disconnected ban
		cmdMenuBanDisc(id)
	} else {
		cmdMenuBan(id)
	}
	return PLUGIN_HANDLED
}*/
