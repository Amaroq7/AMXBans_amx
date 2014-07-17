/*

	AMXBans, managing bans for Half-Life modifications
	Copyright (C) 2003, 2004  Ronald Renes / Jeroen de Rover
	
	Copyright (C) 2009, 2010  Thomas Kurz

*/

#if defined _cmdban_included
    #endinput
#endif
#define _cmdban_included

public cmdMenuBan(id) {
	if(!id) return PLUGIN_HANDLED
	
	if(g_being_banned[g_choicePlayerId[id]]) {
		client_print(id,print_chat,_T("[AMXBans] Blocking doubleban from <%s>"), g_choicePlayerName[id])
	}
	g_being_banned[g_choicePlayerId[id]]=true
	
	if(!get_ban_type(g_ban_type[id],charsmax(g_ban_type),g_choicePlayerAuthid[id],g_choicePlayerIp[id])) {
		log_amx("[AMXBans ERROR cmdMenuBan] Steamid / IP Invalid! Bantype: <%s> | Authid: <%s> | IP: <%s>",g_ban_type[id],g_choicePlayerAuthid[id],g_choicePlayerIp[id])
		g_being_banned[g_choicePlayerId[id]]=false
		return PLUGIN_HANDLED
	}
	
	if(get_cvarptr_num(pcvar_debug) >= 2) {
		log_amx("[AMXBans cmdMenuBan %d] %d | %s | %s | %s | %s (%d min)",id,\
		g_choicePlayerId[id],g_choicePlayerName[id],g_choicePlayerAuthid[id],g_choicePlayerIp[id],g_choiceReason[id],g_choiceTime[id])
	}
	
	if (equal(g_ban_type[id], "S")) {
		formatex(pquery, charsmax(pquery),"SELECT player_id FROM %s%s WHERE player_id='%s' and expired=0", g_dbPrefix, tbl_bans, g_choicePlayerAuthid[id])
		if ( get_cvarptr_num(pcvar_debug) >= 2 )
			log_amx("[AMXBans cmdMenuBan] Banned a player by SteamID")
	} else {
		formatex(pquery, charsmax(pquery),"SELECT player_ip FROM %s%s WHERE player_ip='%s' and expired=0", g_dbPrefix, tbl_bans, g_choicePlayerIp[id])
		if ( get_cvarptr_num(pcvar_debug) >= 2 )
			log_amx("[AMXBans cmdMenuBan] Banned a player by IP/steamID")
	}
	
	mysql_query(g_SqlX, pquery);
	
	_cmdMenuBan(id, g_choicePlayerId[id]);
	
	return PLUGIN_HANDLED	
}
public _cmdMenuBan(id, player)
{
	if ( get_cvarptr_num(pcvar_debug) >= 1 )
		log_amx("[AMXBans cmdMenuBan function 2]Playerid: %d", player)
	
	if (mysql_num_rows(g_SqlX)) {
		client_print(id,print_console,"[AMXBANS] %s", _T("Player is already banned."))
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
	new player_nick[64]
	mysql_escape_string(g_choicePlayerName[id],player_nick,charsmax(player_nick))
	
	mysql_query(g_SqlX, "INSERT INTO `%s%s` (player_id,player_ip,player_nick,admin_ip,admin_id,admin_nick,ban_type,ban_reason,ban_created,ban_length,server_name,server_ip,expired) \
			VALUES('%s','%s','%s','%s','%s','%s','%s','%s',UNIX_TIMESTAMP(NOW()),%d,'%s','%s:%s',0)", \
			g_dbPrefix, tbl_bans, g_choicePlayerAuthid[id],g_choicePlayerIp[id],player_nick,admin_ip,admin_steamid,admin_nick,g_ban_type[id],g_choiceReason[id],g_choiceTime[id],servername_safe,g_ip,g_port)
	
	insert_bandetails(id, g_choicePlayerId[id])
	
	return PLUGIN_HANDLED
}
/*******************************************************************************************************************/
/*******************************************************************************************************************/
public cmdBan(id, level, cid)
{
	/* Checking if the admin has the right access */
	if (!cmd_access(id,level,cid,3))
		return PLUGIN_HANDLED
		
	g_menuban_type[id] = 0
	
	new text[128], iBanDisconnected;
	read_args(text, 127)
	
	// get player ident and bantime depending on the ban cmd format (old or new)
	new ban_length[50]
	parse(text, ban_length, 49, g_ident, 49, g_choiceReason[id], charsmax(g_choiceReason[]))
	
	trim(g_ident)
	trim(ban_length)
	trim(g_choiceReason[id])
	remove_quotes(g_choiceReason[id])
	
	// Check so the ban command has the right format
	if( !is_str_num(ban_length) || read_argc() < 3 )
	{
		client_print(id,print_console,"[AMXBans] %s", _T("amx_ban <Time in Minutes> <SteamID | Nickname> <Reason>"))
		return PLUGIN_HANDLED
	}
	
	//if the reason is empty use the default ban reason from cvar
	if(!strlen(g_choiceReason[id])) {
		get_cvarptr_string(pcvar_default_banreason,g_choiceReason[id],charsmax(g_choiceReason[]))
	}
	
	g_choiceTime[id] = abs(str_to_num(ban_length))
	new cTimeLength[128]
	if (g_choiceTime[id] > 0)
		get_time_length(id, g_choiceTime[id], timeunit_minutes, cTimeLength, 127)
	else
		formatex(cTimeLength, 127, _T("permanently", id))

	// This stops admins from banning perm in console if not adminflag n
	if(!admin_high_bantime_values(id) && g_choiceTime[id] == 0)
	{
		client_print(id,print_console,"[AMXBans] %s", _T("You are not authorized to ban via Console."))
		return PLUGIN_HANDLED
	}

	// Try to find the player that should be banned
	g_choicePlayerId[id] = CmdTargetExtra(id, g_ident, 11, true) //obey immunity|allow yourself|can't be bot
	
	if(!g_choicePlayerId[id]) // no valid player found among the specified/connected (or action can't be performed on him [immunity, etc.])
	{
		g_being_banned[0]=false
		return PLUGIN_HANDLED
	}
		
	else if(g_choicePlayerId[id] == -1)
	{
		iBanDisconnected = 1;
		g_being_banned[0]=false
		
		new szIP[25], szAuthid[35];
		
		read_argv(4, szAuthid, charsmax(szAuthid));
		read_argv(5, szIP, charsmax(szIP));
		
		if(!strlen(szAuthid) && !strlen(szIP))
		{
			console_print(id, _T("You must pass Authid or IP."));
			return PLUGIN_HANDLED;
		}
		
		formatex(g_choicePlayerName[id], charsmax(g_choicePlayerName[]), g_ident);
		formatex(g_choicePlayerIp[id], charsmax(g_choicePlayerIp[]), szIP);
		formatex(g_choicePlayerAuthid[id], charsmax(g_choicePlayerAuthid[]), szAuthid);
	}
	else
	{
		if(g_being_banned[g_choicePlayerId[id]])
		{
			if ( get_cvarptr_num(pcvar_debug) >= 1 )
				log_amx("[AMXBans Blocking doubleban(g_being_banned)] Playerid: %d BanLenght: %s Reason: %s", g_choicePlayerId[id], g_choiceTime[id], g_choiceReason[id])
				
			return PLUGIN_HANDLED
		}
	
		g_being_banned[g_choicePlayerId[id]] = true
	
		if ( get_cvarptr_num(pcvar_debug) >= 1 )
			log_amx("[AMXBans cmdBan function 1]Playerid: %d", g_choicePlayerId[id])

		if (g_choicePlayerId[id])
		{
			get_user_authid(g_choicePlayerId[id], g_choicePlayerAuthid[id], 49)
			get_user_ip(g_choicePlayerId[id], g_choicePlayerIp[id], 29, 1)
		}
	}

	if(!get_ban_type(g_ban_type[id],charsmax(g_ban_type[]),g_choicePlayerAuthid[id],g_choicePlayerIp[id])) {
		log_amx("[AMXBans ERROR cmdBan] Steamid / IP Invalid! Bantype: <%s> | Authid: <%s> | IP: <%s>",g_ban_type[id],g_choicePlayerAuthid[id],g_choicePlayerIp[id])
		g_being_banned[g_choicePlayerId[id]]=false
		return PLUGIN_HANDLED
	}
	
	if (equal(g_ban_type[id], "S"))
	{
		formatex(pquery, charsmax(pquery),"SELECT player_id FROM %s%s WHERE player_id='%s' AND expired=0", g_dbPrefix, tbl_bans, g_choicePlayerAuthid[id])
		
		if ( get_cvarptr_num(pcvar_debug) >= 1 )
			log_amx("[AMXBans cmdBan] Banned a player by SteamID: %s",g_choicePlayerAuthid[id])
	}
	else
	{
		formatex(pquery, charsmax(pquery),"SELECT player_ip FROM %s%s WHERE player_ip='%s' AND expired=0", g_dbPrefix, tbl_bans, g_choicePlayerIp[id])
		
		if ( get_cvarptr_num(pcvar_debug) >= 1 )
			log_amx("[AMXBans cmdBan] Banned a player by IP/steamID: %s",g_choicePlayerIp[id])
	}
	
	mysql_query(g_SqlX, pquery);
	cmd_ban_(id, iBanDisconnected);
	
	return PLUGIN_HANDLED
}
public cmd_ban_(id, ban_disconnected)
{
	if ( get_cvarptr_num(pcvar_debug) >= 1 )
		log_amx("[AMXBans cmd_ban_ function 2]Playerid: %d", g_choicePlayerId[id])
	
	new bool:serverCmd = false
	/* Determine if this was a server command or a command issued by a player in the game */
	if ( id == 0 )
		serverCmd = true;
	
	else
	{
		if (!mysql_num_rows(g_SqlX))
		{
			if (g_choicePlayerId[id])
			{
				get_user_name(g_choicePlayerId[id], g_choicePlayerName[id], charsmax(g_choicePlayerName[]))
			}
			else if(g_choicePlayerId[id] && !ban_disconnected) /* The player was not found in server */
			{
				// Must make that false to be able to ban another player not on the server
				// Players that aren't in the server always get id = 0
				g_being_banned[0] = false
			
				if (serverCmd)
					server_print(_T("[AMXBans] Player %s could not be found"),g_ident)
				else
					console_print(id, _T("[AMXBans] Player %s could not be found"),g_ident)
	
				if ( get_cvarptr_num(pcvar_debug) >= 1 )
					log_amx("[AMXBans] Player %s could not be found",g_ident)
	
				return PLUGIN_HANDLED
			}
			
			new admin_nick[100], admin_steamid[50], admin_ip[20]
			get_user_ip(id, admin_ip, 19, 1)
			
			if (!serverCmd)
			{
				get_user_authid(id, admin_steamid, 49)
				mysql_get_username_safe(id, admin_nick, 99)
	
				if ( get_cvarptr_num(pcvar_debug) >= 1 )
					log_amx("[AMXBans cmdBan] Adminsteamid: %s, Servercmd: %s", admin_steamid, (serverCmd)?"Yes":"No")
			}
			else
			{
				/* If the server does the ban you cant get any steam_ID or team */
				admin_steamid = ""
		
				/* This is so you can have a shorter name for the servers hostname.
				Some servers hostname can be very long b/c of sponsors and that will make the ban list on the web bad */
				new servernick[100]
				get_cvarptr_string(pcvar_server_nick, servernick, 99)
				if (strlen(servernick))
					copy(admin_nick,charsmax(admin_nick),servernick)
			}
		
			/* If HLGUARD ban, the admin nick will be set to [HLGUARD] */
			if ( contain(g_choiceReason[id], "[HLGUARD]") != -1 )
				copy(admin_nick,charsmax(admin_nick),"[HLGUARD]")
		
			/* If ATAC ban, the admin nick will be set to [ATAC] */
			if ( contain(g_choiceReason[id], "Max Team Kill Violation") != -1 )
				copy(admin_nick,charsmax(admin_nick),"[ATAC]")
				
			if ( get_cvarptr_num(pcvar_debug) >= 1 )
				log_amx("[AMXBans cmdBan] Admin nick: %s, Admin userid: %d", admin_nick, get_user_userid(id))
			
			new server_name[200]
			mysql_get_servername_safe(server_name, charsmax(server_name))

			if ( get_cvarptr_num(pcvar_add_mapname) ) {
				new mapname[32]//, pre[4],post[4]
				get_mapname(mapname,31)
				format(server_name,charsmax(server_name),"%s (%s)",server_name,mapname)
			}

			new player_nick[64]
			mysql_escape_string(g_choicePlayerName[id],player_nick,charsmax(player_nick))
			new admin_nick_safe[200]
			mysql_escape_string(admin_nick,admin_nick_safe,charsmax(admin_nick_safe))
		
			mysql_query(g_SqlX, "INSERT INTO `%s%s` \
				(player_id,player_ip,player_nick,admin_ip,admin_id,admin_nick,ban_type,ban_reason,ban_created,ban_length,server_name,server_ip,expired) \
				VALUES('%s','%s','%s','%s','%s','%s','%s','%s',UNIX_TIMESTAMP(NOW()),%d,'%s','%s:%s',0)", \
				g_dbPrefix, tbl_bans, g_choicePlayerAuthid[id], g_choicePlayerIp[id], player_nick, admin_ip, admin_steamid, admin_nick, g_ban_type[id], \
				g_choiceReason[id], g_choiceTime[id], server_name, g_ip, g_port);
		
			if(!ban_disconnected)
				insert_bandetails(id, g_choicePlayerId[id]);
			
		}
		else
		{
			if ( serverCmd )
				log_message("[AMXBans] %s",_T("Player is already banned."))
			else
				client_print(id,print_console,"[AMXBans] %s",_T("Player is already banned."))
			// Must make that false to be able to ban another player not on the server
			// Players that aren't in the server always get id = 0
			g_being_banned[g_choicePlayerId[id]] = false

		}
	}
	
	return PLUGIN_HANDLED
}
/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*******************************************************************************************************************/
public insert_bandetails(id, player)
{
	if ( get_cvarptr_num(pcvar_debug) >= 1 )
		log_amx("[AMXBans cmdBan function 5]Playerid: %d",g_choicePlayerId[id])
	
	new bid = mysql_insert_id(g_SqlX)
	
	//break if the banned player should not be kicked at the moment
	if(g_menuban_type[id]==1) return PLUGIN_HANDLED
	if(g_menuban_type[id]==2) {
		g_nextround_kick[g_choicePlayerId[id]]=true
		//g_nextround_kick_time[g_choicePlayerId[id]]=g_choiceTime[id]
		g_nextround_kick_bid[g_choicePlayerId[id]]=bid
		//copy(g_nextround_kick_Reason[g_choicePlayerId[id]],charsmax(g_nextround_kick_Reason[]),g_choiceReason[id])
		
		return PLUGIN_HANDLED
	}
	
	select_amxbans_motd(id,g_choicePlayerId[id],bid)
	return PLUGIN_HANDLED
}
/*******************************************************************************************************************/
public select_amxbans_motd(id,player,bid) {
	if ( get_cvarptr_num(pcvar_debug) >= 1 )
		log_amx("[AMXBans cmdBan function 5]Bid: %d", bid)

	//get ban details from db
	
	mysql_query(g_SqlX, "SELECT si.amxban_motd,ba.player_nick,ba.player_id,ba.player_ip, \
		ba.admin_nick,ba.admin_id,ba.ban_type,ba.ban_reason,ba.ban_length FROM `%s%s` as si,`%s%s` as ba \
		WHERE ba.bid=%d AND si.address = '%s:%s'", g_dbPrefix, tbl_serverinfo, g_dbPrefix, tbl_bans, bid,g_ip, g_port)
		
	_select_amxbans_motd(id, player, bid);
	
	return PLUGIN_HANDLED
}
public _select_amxbans_motd(id, player, bid) {
	if ( get_cvarptr_num(pcvar_debug) >= 1 )
		log_amx("[AMXBans cmdBan function 6]Playerid: %d, Bid: %d", player, bid)

	new bool:serverCmd = false
	/* Determine if this was a server command or a command issued by a player in the game */
	if ( id == 0 )
		serverCmd = true;
	
	new amxban_motd_url[256]
	new admin_steamid[35], admin_nick[100],pl_steamid[35],pl_nick[100],pl_ip[22]
	new ban_type[32],ban_reason[128],iBanLength
	
	if (!mysql_num_rows(g_SqlX)) {
		amxban_motd_url[0]='^0'
		log_amx("[AMXBans cmdBan function 6.1] select_motd without result: %d, Bid: %d", player, bid)
		
		if(player && is_user_connected(player)) {
			new pid=player+200
			set_task(kick_delay, "delayed_kick", pid)
		}
		return PLUGIN_HANDLED
		//copy(amxban_motd_url,256, "0")	
	} else {
		mysql_nextrow(g_SqlX);
		mysql_getfield(g_SqlX, 1, amxban_motd_url, 256)
		mysql_getfield(g_SqlX, 2, pl_nick, 99)
		mysql_getfield(g_SqlX, 3, pl_steamid, 34)
		mysql_getfield(g_SqlX, 4, pl_ip, 21)
		mysql_getfield(g_SqlX, 5, admin_nick, 99)
		mysql_getfield(g_SqlX, 6, admin_steamid, 34)
		mysql_getfield(g_SqlX, 7, ban_type, 31)
		mysql_getfield(g_SqlX, 8, ban_reason, 127)
		iBanLength=mysql_getfield(g_SqlX,9)
	}
	
	new admin_team[11]
	
	get_user_team(id, admin_team, 10)
	
	new cTimeLengthPlayer[128]
	new cTimeLengthServer[128]
		
	if (iBanLength > 0) {
		get_time_length(player, iBanLength, timeunit_minutes, cTimeLengthPlayer, 127)
		get_time_length(0, iBanLength, timeunit_minutes, cTimeLengthServer, 127)
	} else { //Permanent Ban
		formatex(cTimeLengthPlayer, 127, _T("permanently", player))
		formatex(cTimeLengthServer, 127, _T("permanently", 0))
	}
	
	new show_activity = get_cvar_num("amx_show_activity")
	if( (get_user_flags(id)&get_admin_mole_access_flag() || id == 0) && (get_cvarptr_num(pcvar_show_name_evenif_mole) == 0) )
		show_activity = 1
	
	if (player) {
		new complain_url[256]
		get_cvarptr_string(pcvar_complainurl ,complain_url, 255)
			
		client_print(player,print_console,"[AMXBans] ===============================================")
		
		new ban_motd[1400]
		switch(show_activity)
		{
			case 1:
			{
				client_print(player,print_console,_T("[AMXBans] You have been banned from this Server!"))
				client_print(player,print_console,_T("[AMXBans] You can complain about your ban @ %s"), complain_url)
				formatex(ban_motd, charsmax(ban_motd), _T("<body bgcolor=#9CB3B3><font size=^"18^" color=^"red^">You have been banned.</font><br><br><font color=^"black^">Reason: %s<br>Duration: %s<br>SteamID: %s</font>", player), ban_reason, cTimeLengthPlayer, pl_steamid)
			}
			case 2:
			{
				client_print(player,print_console,_T("[AMXBans] Banned by Admin: %s"), admin_nick)
				client_print(player,print_console,_T("[AMXBans] You can complain about your ban @ %s"), complain_url)
				formatex(ban_motd, charsmax(ban_motd), _T("<body bgcolor=#9CB3B3><font size=^"18^" color=^"red^">You have been banned.</font><br><br><font color=^"black^">Reason: %s<br>Duration: %s<br>SteamID: %s<br>Admin: %s</font>", player), ban_reason, cTimeLengthPlayer, pl_steamid, admin_nick)
			}
			case 3:
			{
				if (is_user_admin(id))
				{
					client_print(player,print_console,_T("[AMXBans] Banned by Admin: %s"), admin_nick)
					client_print(player,print_console,_T("[AMXBans] You can complain about your ban @ %s"), complain_url)
					formatex(ban_motd, charsmax(ban_motd), _T("<body bgcolor=#9CB3B3><font size=^"18^" color=^"red^">You have been banned.</font><br><br><font color=^"black^">Reason: %s<br>Duration: %s<br>SteamID: %s<br>Admin: %s</font>", player), ban_reason, cTimeLengthPlayer, pl_steamid, admin_nick)
				}
				else
				{
					client_print(player,print_console,_T("[AMXBans] You have been banned from this Server!"))
					client_print(player,print_console,_T("[AMXBans] You can complain about your ban @ %s"), complain_url)
					formatex(ban_motd, charsmax(ban_motd), _T("<body bgcolor=#9CB3B3><font size=^"18^" color=^"red^">You have been banned.</font><br><br><font color=^"black^">Reason: %s<br>Duration: %s<br>SteamID: %s</font>", player), ban_reason, cTimeLengthPlayer, pl_steamid)
				}
			}
			case 4:
			{
				if (is_user_admin(id))
				{
					client_print(player,print_console,_T("[AMXBans] Banned by Admin: %s"),admin_nick)
					client_print(player,print_console,_T("[AMXBans] You can complain about your ban @ %s"), complain_url)
					formatex(ban_motd, charsmax(ban_motd), _T("<body bgcolor=#9CB3B3><font size=^"18^" color=^"red^">You have been banned.</font><br><br><font color=^"black^">Reason: %s<br>Duration: %s<br>SteamID: %s<br>Admin: %s</font>", player), ban_reason, cTimeLengthPlayer, pl_steamid, admin_nick)
				}
			}
			case 5:
			{
				if (is_user_admin(id))
				{
					client_print(player,print_console,_T("[AMXBans] You have been banned from this Server!"))
					client_print(player,print_console,_T("[AMXBans] You can complain about your ban @ %s"), complain_url)
					formatex(ban_motd, charsmax(ban_motd), _T("<body bgcolor=#9CB3B3><font size=^"18^" color=^"red^">You have been banned.</font><br><br><font color=^"black^">Reason: %s<br>Duration: %s<br>SteamID: %s</font>", player), ban_reason, cTimeLengthPlayer, pl_steamid)
				}
			}
		}
		
		client_print(player,print_console,_T("[AMXBans] Reason: '%s'"), ban_reason)
		client_print(player,print_console,_T("[AMXBans] Duration: '%s'"), cTimeLengthPlayer)
		client_print(player,print_console,_T("[AMXBans] Your SteamID: '%s'"), pl_steamid)
		client_print(player,print_console,_T("[AMXBans] Your IP: '%s'"), pl_ip)
		client_print(player,print_console,"[AMXBans] ===============================================")
		
		new msg[1400]
		
		if ( get_cvarptr_num(pcvar_debug) >= 1 )
			log_amx("[AMXBans cmdBan function 6.2]Bid: %d URL= %s Kickdelay:%f", bid, amxban_motd_url, kick_delay)

		if(contain(amxban_motd_url,"sid=%s&adm=%d&lang=%s") != -1) {
			new bidstr[10],lang[5] 
			formatex(bidstr,9,"B%d",bid)
			get_user_info(player,"lang",lang,charsmax(lang))
			if(equal(lang,""))
				get_cvar_string("amx_language",lang,charsmax(lang))
			formatex(msg, charsmax(msg), amxban_motd_url,bidstr,(show_activity==2)?1:0,lang)
			if ( get_cvarptr_num(pcvar_debug) >= 1 )
				log_amx("[AMXBans cmdBan function 6.3]Motd: %s",msg)
		} else {
			formatex(msg, charsmax(msg), ban_motd)
		}
		
		if(is_user_connected(player)) {
			amxbans_ban_motdopen(player);
	
			new motdTitle[64]
			formatex(motdTitle,charsmax(motdTitle),"Banned by Amxbans %s",VERSION)
			//add(motdTitle,255,VERSION,0)
			show_motd(player, msg, motdTitle)
			
			new pid=player+200
			set_task(kick_delay, "delayed_kick", pid)
		}
	} else { /* The player was not found in server */
		if (serverCmd)
			server_print(_T("[AMXBans] Player %s was not found"), g_ident)
		else
			console_print(id, _T("[AMXBans] Player %s was not found"),g_ident)

		if ( get_cvarptr_num(pcvar_debug) >= 1 )
			log_amx("[AMXBans] Player %s could not be found",g_ident)

		return PLUGIN_HANDLED
		
		
	}
			
	if (equal(ban_type, "S")) {
		if ( serverCmd )
			log_message(_T("[AMXBans] SteamID '%s' banned successfully (IP logged). Player gets kicked."),pl_steamid)
		else
			client_print(id,print_console,_T("[AMXBans] SteamID '%s' banned successfully (IP logged). Player gets kicked."),pl_steamid)
	} else {
		if ( serverCmd )
			log_message("[AMXBans] %s",_T("Banned Players Ip successfully. Player gets kicked."))
		else
			client_print(id,print_console,"[AMXBans] %s",_T("Banned Players Ip successfully. Player gets kicked."))
	}
	
	if (serverCmd) {
		/* If the server does the ban you cant get any steam_ID or team */
		admin_steamid[0] = '^0'
		admin_team[0] = '^0'
	}
			
	// Logs all bans by admins/server to amxx logs
	if (g_choiceTime[id] > 0) {
		log_amx(_T("%s<%d><%s><%s> has banned %s<%s> for %s (%i Minutes). Reason: %s."),admin_nick, get_user_userid(id), admin_steamid, admin_team, \
			pl_nick, pl_steamid, cTimeLengthServer, iBanLength, ban_reason)

		if ( get_cvarptr_num(pcvar_show_in_hlsw) ) {
			// If you use HLSW you will see when someone ban a player if you can see the chatlogs
			log_message("^"%s<%d><%s><%s>^" triggered ^"amx_chat^" (text ^"%s^")", admin_nick, get_user_userid(id), admin_steamid, admin_team, \
				_T("%s<%s> was banned for %s (%i Minutes). Reason: %s."), pl_nick, pl_steamid, cTimeLengthServer, iBanLength, ban_reason)
		}
	} else {
		log_amx(_T("%s<%d><%s><%s> banned %s<%s> Duration: PERMANENT. Reason: %s."), admin_nick, get_user_userid(id), admin_steamid, admin_team, pl_nick, pl_steamid, ban_reason)

		if ( get_cvarptr_num(pcvar_show_in_hlsw) ) {
			// If you use HLSW you will see when someone ban a player if you can see the chatlogs
			log_message("^"%s<%d><%s><%s>^" triggered ^"amx_chat^" (text ^"%s^")", admin_nick, get_user_userid(id), admin_steamid, admin_team, \
				_T("%s<%s> was banned PERMANENT. Reason: %s."), pl_nick, pl_steamid, ban_reason)
		}
	}
	
	new message[191]
	
	
	switch (show_activity)
	{
		case 1:
		{
			new playerCount, idx, players[32]
			get_players(players, playerCount, "ch")
			for (idx=0; idx<playerCount; idx++)
			{
				
				get_time_length(players[idx], iBanLength, timeunit_minutes, cTimeLengthPlayer, 127)
				
				if (g_choiceTime[id] > 0)
					formatex(message,190,_T("'%s' got banned for '%s'.^n Reason: '%s'",players[idx]), pl_nick, cTimeLengthPlayer, ban_reason)
				else
					formatex(message,190,_T("'%s' got banned permanent.^n Reason: '%s'", players[idx]), pl_nick, ban_reason)
				
				if ( get_cvarptr_num(pcvar_show_hud_messages) == 1 ) {
					set_hudmessage(0, 255, 0, 0.05, 0.30, 0, 6.0, 10.0 , 0.5, 0.15, -1)
					show_hudmessage(players[idx], message)
				}
				client_print(players[idx], print_chat, message)
				client_print(players[idx], print_console, message)
			}
		}
		case 2:
		{
			new playerCount, idx, players[32]
			get_players(players, playerCount, "ch")
			for (idx=0; idx<playerCount; idx++)
			{
				get_time_length(players[idx], iBanLength, timeunit_minutes, cTimeLengthPlayer, 127)
				
				if (g_choiceTime[id] > 0)
					formatex(message,190, _T("'%s' got banned for '%s'.^n Reason: '%s'^n Admin: %s", players[idx]), pl_nick, cTimeLengthPlayer, ban_reason, admin_nick)
				else
					formatex(message,190, _T("'%s' got banned for '%s'.^n Reason: '%s'^n Admin: %s", players[idx]), pl_nick, ban_reason, admin_nick)
				
				if ( get_cvarptr_num(pcvar_show_hud_messages) == 1 ) {
					set_hudmessage(0, 255, 0, 0.05, 0.30, 0, 6.0, 10.0 , 0.5, 0.15, -1)
					show_hudmessage(players[idx], message)
				}
				client_print(players[idx], print_chat, message)
				client_print(players[idx], print_console, message)
			}
		}
		case 3:
		{
			if (is_user_admin(id))
			{
				new playerCount, idx, players[32]
				get_players(players, playerCount, "ch")
				
				for (idx=0; idx<playerCount; idx++)
				{
					get_time_length(players[idx], iBanLength, timeunit_minutes, cTimeLengthPlayer, 127)
					
					if (g_choiceTime[id] > 0)
						formatex(message,190, _T("'%s' got banned for '%s'.^n Reason: '%s'^n Admin: %s", players[idx]), pl_nick, cTimeLengthPlayer, ban_reason, admin_nick)
					else
						formatex(message,190, _T("'%s' got banned for '%s'.^n Reason: '%s'^n Admin: %s", players[idx]), pl_nick, ban_reason, admin_nick)
					
					if ( get_cvarptr_num(pcvar_show_hud_messages) == 1 ) {
						set_hudmessage(0, 255, 0, 0.05, 0.30, 0, 6.0, 10.0 , 0.5, 0.15, -1)
						show_hudmessage(players[idx], message)
					}
					client_print(players[idx],print_chat, message)
					client_print(players[idx],print_console, message)
				}
			}
			else
			{
				new playerCount, idx, players[32]
				get_players(players, playerCount, "ch")
				
				for (idx=0; idx<playerCount; idx++)
				{
					
					get_time_length(players[idx], iBanLength, timeunit_minutes, cTimeLengthPlayer, 127)
					
					if (g_choiceTime[id] > 0)
						formatex(message,190,_T("'%s' got banned for '%s'.^n Reason: '%s'",players[idx]), pl_nick, cTimeLengthPlayer, ban_reason)
					else
						formatex(message,190,_T("'%s' got banned permanent.^n Reason: '%s'", players[idx]), pl_nick, ban_reason)
					
					if ( get_cvarptr_num(pcvar_show_hud_messages) == 1 ) {
						set_hudmessage(0, 255, 0, 0.05, 0.30, 0, 6.0, 10.0 , 0.5, 0.15, -1)
						show_hudmessage(players[idx], message)
					}
					client_print(players[idx],print_chat, message)
					client_print(players[idx],print_console, message)
				}
			}
		}
		case 4:
		{
			if (is_user_admin(id))
			{
				new playerCount, idx, players[32]
				get_players(players, playerCount, "ch")
				
				for (idx=0; idx<playerCount; idx++)
				{
					get_time_length(players[idx], iBanLength, timeunit_minutes, cTimeLengthPlayer, 127)
					
					if (g_choiceTime[id] > 0)
						formatex(message,190, _T("'%s' got banned for '%s'.^n Reason: '%s'^n Admin: %s", players[idx]), pl_nick, cTimeLengthPlayer, ban_reason, admin_nick)
					else
						formatex(message,190, _T("'%s' got banned for '%s'.^n Reason: '%s'^n Admin: %s", players[idx]), pl_nick, ban_reason, admin_nick)
					
					if ( get_cvarptr_num(pcvar_show_hud_messages) == 1 ) {
						set_hudmessage(0, 255, 0, 0.05, 0.30, 0, 6.0, 10.0 , 0.5, 0.15, -1)
						show_hudmessage(players[idx], message)
					}
					client_print(players[idx],print_chat, message)
					client_print(players[idx],print_console, message)
				}
			}
		}
		case 5:
		{
			if (is_user_admin(id))
			{
				new playerCount, idx, players[32]
				get_players(players, playerCount, "ch")
				
				for (idx=0; idx<playerCount; idx++)
				{
					
					get_time_length(players[idx], iBanLength, timeunit_minutes, cTimeLengthPlayer, 127)
					
					if (g_choiceTime[id] > 0)
						formatex(message,190,_T("'%s' got banned for '%s'.^n Reason: '%s'",players[idx]), pl_nick, cTimeLengthPlayer, ban_reason)
					else
						formatex(message,190,_T("'%s' got banned permanent.^n Reason: '%s'", players[idx]), pl_nick, ban_reason)
					
					if ( get_cvarptr_num(pcvar_show_hud_messages) == 1 ) {
						set_hudmessage(0, 255, 0, 0.05, 0.30, 0, 6.0, 10.0 , 0.5, 0.15, -1)
						show_hudmessage(players[idx], message)
					}
					client_print(players[idx],print_chat, message)
					client_print(players[idx],print_console, message)
				}
			}
		}
	}

	return PLUGIN_HANDLED
}

//Credits go to AMX Mod Dev.
CmdTargetExtra(const id, const szArg[], const iFlags = 3, const bool:bExtraFeature = false)
{
	new iPlayer = find_player("bl", szArg)
	if(iPlayer)
	{
		if(iPlayer != find_player("blj", szArg))
		{
			#if defined _translator_included
			console_print(id, _T("There is more than one client matching your argument."))
			#else
			console_print(id, "There is more than one client matching your argument.")
			#endif
			return 0
		}
	}
	else if((iPlayer = find_player("c", szArg)) == 0 && (iPlayer = find_player("d", szArg)) == 0 && szArg[0] == '#' && szArg[1])
	{
		iPlayer = find_player("k", strtonum(szArg[1]))
	}
	if(!iPlayer)
	{
		#if defined _translator_included
		if(bExtraFeature == false) console_print(id, _T("Client with that name or userid not found."))
		#else
		if(bExtraFeature == false) console_print(id, "Player with that name or userid not found.")
		#endif
		return bExtraFeature ? -1 : 0
	}
	if((iFlags & 2) == 0 && (iPlayer == id))
	{
		#if defined _translator_included
		console_print(id, _T("That action can't be performed on yourself."))
		#else
		console_print(id, "That action can't be performed on yourself.")
		#endif
		return 0
	}
	if(iFlags & 1)
	{
		if((iPlayer != id) && (get_user_flags(iPlayer) & ADMIN_IMMUNITY) && !(get_user_flags(id) & ADMIN_SUPREME))
		{
			new szPlayerName[32]
			get_user_name(iPlayer, szPlayerName, charsmax(szPlayerName))
			#if defined _translator_included
			console_print(id, _T("Client ^"%s^" has immunity."), szPlayerName)
			#else
			console_print(id, "Client ^"%s^" has immunity.", szPlayerName)
			#endif
			return 0
		}
	}
	if(iFlags & 4) 
	{
		if(!is_user_alive(iPlayer))
		{
			new szPlayerName[32]
			get_user_name(iPlayer, szPlayerName, charsmax(szPlayerName))
			#if defined _translator_included
			console_print(id, _T("That action can't be performed on dead client ^"%s^"."), szPlayerName)
			#else
			console_print(id, "That action can't be performed on dead client ^"%s^".", szPlayerName)
			#endif
			return 0
		}
	}
	if(iFlags & 8)
	{
		if(is_user_bot(iPlayer))
		{
			new szPlayerName[32]
			get_user_name(iPlayer, szPlayerName, charsmax(szPlayerName))
			#if defined _translator_included
			console_print(id, _T("That action can't be performed on bot ^"%s^"."), szPlayerName)
			#else
			console_print(id, "That action can't be performed on bot ^"%s^".", szPlayerName)
			#endif
			return 0
		}
	}
	return iPlayer
}