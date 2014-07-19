/*

	AMXBans, managing bans for Half-Life modifications
	Copyright (C) 2003, 2004  Ronald Renes / Jeroen de Rover
	
	Copyright (C) 2009, 2010  Thomas Kurz

	Amxbans Main Plugin
*/
// Rev 2010/04/13

new const AUTHOR[] = "HLXBans Dev Team"
new const PLUGIN_NAME[] = "AMXBans"
new const VERSION[] = "0.0.4-alpha" // This is used in the plugins name

new const amxbans_version[] = "0.0.4-alpha" // This is for the DB

#include <translator>
#include <amxmod>
#include <amxmisc>
#include <mysql>
#include <time>
#include <VexdUM>
#include <fun>

#include "amxbans/global_vars.inl"

// Amxbans Core
#include "amxbans/amxbans_core.inl"

// Amxbans Freeze
#include "amxbans/amxbans_freeze.inl"

// Amxbans Flagged
#include "amxbans/amxbans_flagged.inl"

// Amxbans .inl files
#include "amxbans/init_functions.inl"
#include "amxbans/check_player.inl"
#include "amxbans/check_flag.inl"
#include "amxbans/menu_stocks.inl"
#include "amxbans/menu_ban.inl"
#include "amxbans/menu_disconnected.inl"
#include "amxbans/menu_history.inl"
#include "amxbans/menu_flag.inl"
#include "amxbans/cmdBan.inl"
#include "amxbans/cmdUnban.inl"
#include "amxbans/web_handshake.inl"


// 16k * 4 = 64k stack size
#pragma dynamic 16384 		// Give the plugin some extra memory to use


public plugin_init() {
	plugin_init_core()
	plugin_init_freeze()
	plugin_init_disconnected()
	plugin_init_banmenu()
	plugin_init_history()
	plugin_init_flagging()
	
	register_plugin(PLUGIN_NAME, VERSION, AUTHOR)
	register_cvar("amxbans_version", VERSION, FCVAR_SERVER|FCVAR_EXTDLL|FCVAR_UNLOGGED|FCVAR_SPONLY)
	
	new szGame[20];
	get_modname(szGame, charsmax(szGame));
	
	if (equal(szGame, "cstrike") || equal(szGame, "czero"))
		register_event("HLTV", "event_new_round", "a", "1=0", "2=0");
	else if (equal(szGame, "dod"))
		register_event("RoundState", "event_new_round", "a", "1=1");
	else
		g_supported_game = false;
	
	register_clcmd("amx_banmenu", "cmdBanMenu", ADMIN_BAN, _T("- displays ban menu"))
	register_clcmd("amxbans_custombanreason", "setCustomBanReason", ADMIN_BAN, _T("- configures custom ban message"))
	register_clcmd("amx_banhistorymenu", "cmdBanhistoryMenu", ADMIN_BAN, _T("- displays banhistorymenu"))
	register_clcmd("amx_bandisconnectedmenu", "cmdBanDisconnectedMenu", ADMIN_BAN, _T("- displays bandisconnectedmenu"))
	register_clcmd("amx_flaggingmenu","cmdFlaggingMenu",ADMIN_BAN, _T("- displays flagging menu"))
	
	register_srvcmd("amx_sethighbantimes", "setHighBantimes")
	register_srvcmd("amx_setlowbantimes", "setLowBantimes")
	register_srvcmd("amx_setflagtimes","setFlagTimes")
	
	register_concmd("amx_reloadreasons", "cmdFetchReasons", ADMIN_CFG)
	
	pcvar_serverip		=	register_cvar("amxbans_server_address","")
	pcvar_server_nick 	= 	register_cvar("amxbans_servernick", "")
	pcvar_complainurl	= 	register_cvar("amxbans_complain_url", "www.yoursite.com") // Dont use http:// then the url will not show
	pcvar_debug 		= 	register_cvar("amxbans_debug", "0") // Set this to 1 to enable debug
	pcvar_add_mapname	=	register_cvar("amxbans_add_mapname_in_servername", "0")
	pcvar_flagged_all	=	register_cvar("amxbans_flagged_all_server","1")
	pcvar_show_in_hlsw 	= 	register_cvar("amxbans_show_in_hlsw", "1")
	pcvar_show_hud_messages	= 	register_cvar("amxbans_show_hud_messages", "1")
	pcvar_higher_ban_time_admin = 	register_cvar("amxbans_higher_ban_time_admin", "n")
	pcvar_admin_mole_access = 	register_cvar("amxbans_admin_mole_access", "r")
	pcvar_show_name_evenif_mole = 	register_cvar("amxbans_show_name_evenif_mole", "1")
	pcvar_custom_statictime =	register_cvar("amxbans_custom_statictime","1440")
	pcvar_show_prebanned 	=	register_cvar("amxbans_show_prebanned","1")
	pcvar_show_prebanned_num =	register_cvar("amxbans_show_prebanned_num","2")
	pcvar_default_banreason	=	register_cvar("amxbans_default_ban_reason","unknown")
	
	register_concmd("amx_ban", "cmdBan", ADMIN_BAN, _T("<Time in Minutes> <SteamID | Nickname | #userid> <Reason> <SteamID> <IP>"))
	register_concmd("amx_banip", "cmdBan", ADMIN_BAN, _T("<Time in Minutes> <SteamID | Nickname | #userid> <Reason> <SteamID> <IP>"))
	register_concmd("amx_unban", "cmdUnban", ADMIN_UNBAN, _T("<steamID or IP>"));
	
	register_srvcmd("amx_list", "cmdLst", ADMIN_RCON, _T("sends playerinfos to web"))
	
	g_coloredMenus 		= 	colored_menus()
	g_iMaxPlayers = get_maxplayers();
	
	new configsDir[64]
	get_localinfo("amx_configdir", configsDir, charsmax(configsDir))
	
	server_cmd("exec %s/mysql.cfg", configsDir)
	server_cmd("exec %s/amxbans.cfg", configsDir)
}

public amxbans_sql_initialized(sqlTuple,dbPrefix[])
{
	copy(g_dbPrefix,charsmax(g_dbPrefix),dbPrefix)
	//db was already initialized, second init can be caused by a second forward from main plugin
	//this should never happen!!
	if(g_SqlX) {
		log_amx("[AMXBans Error] DB Info Tuple from amxbans_core initialized twice!!")
		return PLUGIN_HANDLED
	}
	
	g_SqlX=sqlTuple
	if ( get_cvarptr_num(pcvar_debug) >= 1 )
		log_amx("[AMXBans] Received DB Info Tuple from amxbans_core: %d | %s", sqlTuple,g_dbPrefix)
	if(!g_SqlX) {
		log_amx("[AMXBans Error] DB Info Tuple from amxbans_main is empty! Trying to get a valid one")
		new host[64], user[64], pass[64], db[64], error[128];

		get_cvar_string("amx_mysql_host", host, 63)
		get_cvar_string("amx_mysql_user", user, 63)
		get_cvar_string("amx_mysql_pass", pass, 63)
		get_cvar_string("amx_mysql_db", db, 63)
		
		g_SqlX = mysql_connect(host, user, pass, db, error, charsmax(error), 1)
		
		get_cvar_string("amx_sql_prefix",g_dbPrefix,charsmax(g_dbPrefix))
	}
	set_task(0.1, "banmod_online")
	set_task(0.2, "fetchReasons")
	
	return PLUGIN_HANDLED
}
//////////////////////////////////////////////////////////////////
public get_admin_mole_access_flag() {
	new flags[24]
	get_cvarptr_string(pcvar_admin_mole_access, flags, 23)
	
	return(read_flags(flags))
}
public delayed_kick(player_id) {
	
	player_id-=200
	new userid = get_user_userid(player_id)
	new kick_message[128]
	
	formatex(kick_message,127,_T("You are BANNED. Check your console.", player_id))

	if ( get_cvarptr_num(pcvar_debug) >= 1 )
		log_amx("[AMXBANS DEBUG] Delayed Kick ID: <%d>", player_id)

	server_cmd("kick #%d  %s",userid, kick_message)
	
	g_kicked_by_amxbans[player_id]=true
	g_being_banned[player_id] = false
	
	return PLUGIN_CONTINUE
}

public client_connect(id)
{
	client_connect_freeze(id);
}

public event_new_round()
{
	for(new i=1;i <= g_iMaxPlayers; i++)
	{
		if(g_nextround_kick[i])
		{
			if ( get_cvarptr_num(pcvar_debug) >= 1 )
				log_amx("[AMXBans] New Round Kick ID: <%d> | bid:%d",i,g_nextround_kick_bid[i])
			
			if(!is_user_connected(i) || is_user_bot(i)) continue
			//player is banned, so select motd and kick him
			select_amxbans_motd(0,i,g_nextround_kick_bid[i])
		}
	}
}
/*********    client functions     ************/
public client_authorized(id) {
	client_authorized_core(id);
	//fix for the invalid tuple error at mapchange, only a fast fix now
	if(!g_SqlX) {
		set_task(2.0,"client_authorized",id)
		return PLUGIN_HANDLED
	}
	//check if an activ ban exists
	check_player(id)
	return PLUGIN_CONTINUE
}
public client_putinserver(id) {
	client_putinserver_core(id);
	//fix for the invalid tuple error at mapchange, only a fast fix now
	if(!g_SqlX) {
		set_task(5.0,"client_putinserver",id)
		return PLUGIN_HANDLED
	}
	//check if the player was banned before
	prebanned_check(id)
	
	#if defined MAX_DISCONNECTED_PLAYERS
	//remove the player from the disconnect player list because he is already connected ;-)
	disconnect_remove_player(id)
	#endif
	
	return PLUGIN_CONTINUE
}

public plugin_end()
	mysql_close(g_SqlX);

public client_disconnect(id)
{
	client_disconnect_core(id);
	client_disconnect_freeze(id);
	client_disconnect_flagged(id);
	g_being_banned[id]=false
	
	if(!g_kicked_by_amxbans[id]) {
		#if defined MAX_DISCONNECTED_PLAYERS
		//only add players to disconnect list if not kicked by amxbans
		disconnected_add_player(id)
		#endif
	} else if(g_being_flagged[id]) {
		// if kicked by amxbans maybe remove the flagged, not added yet
		/*****///remove_flagged_by_steam(0,id,0)
	}
	//reset some vars
	g_kicked_by_amxbans[id]=false
	g_being_flagged[id]=false
	g_nextround_kick[id]=false
}
/*********    timecmd functions     ************/
public setHighBantimes() {
	new arg[32]
	new argc = read_argc() - 1
	g_highbantimesnum = argc

	if(argc < 1 || argc > 14) {
		log_amx("[AMXBANS] You have more than 14 or less than 1 bantimes set in amx_sethighbantimes")
		log_amx("[AMXBANS] Loading default bantimes")
		loadDefaultBantimes(1)

		return PLUGIN_HANDLED
	}

	new i = 0
	new num[32], flag[32]
	while (i < argc)	{
		read_argv(i + 1, arg, 31)
		parse(arg, num, 31, flag, 31)

		if(equali(flag, "m")) { 
			g_HighBanMenuValues[i] = str_to_num(num)
		} else if(equali(flag, "h")) {
			g_HighBanMenuValues[i] = (str_to_num(num) * 60)
		} else if(equali(flag, "d")) {
			g_HighBanMenuValues[i] = (str_to_num(num) * 1440)
		} else if(equali(flag, "w")) {
			g_HighBanMenuValues[i] = (str_to_num(num) * 10080)
		}
		i++
	}
	return PLUGIN_HANDLED
}
public setLowBantimes() {
	new arg[32]
	new argc = read_argc() - 1
	g_lowbantimesnum = argc
	
	if(argc < 1 || argc > 14) {
		log_amx("[AMXBANS] You have more than 14 or less than 1 bantimes set in amx_setlowbantimes")
		log_amx("[AMXBANS] Loading default bantimes")
		loadDefaultBantimes(2)
		
		return PLUGIN_HANDLED
	}

	new i = 0
	new num[32], flag[32]
	while (i < argc) {
		read_argv(i + 1, arg, 31)
		parse(arg, num, 31, flag, 31)

		if(equali(flag, "m")) { 
			g_LowBanMenuValues[i] = str_to_num(num)
		} else if(equali(flag, "h")) {
			g_LowBanMenuValues[i] = (str_to_num(num) * 60)
		} else if(equali(flag, "d")) {
			g_LowBanMenuValues[i] = (str_to_num(num) * 1440)
		} else if(equali(flag, "w")) {
			g_LowBanMenuValues[i] = (str_to_num(num) * 10080)
		}
		i++
	}
	return PLUGIN_HANDLED
}
public setFlagTimes() {
	new arg[32]
	new argc = read_argc() - 1
	g_flagtimesnum = argc
	if(argc < 1 || argc > 14) {
		log_amx("[AMXBANS] You have more than 14 or less than 1 flagtimes set in amx_setflagtimes")
		log_amx("[AMXBANS] Loading default flagtimes")
		loadDefaultBantimes(3)
		
		return PLUGIN_HANDLED
	}
	
	new i = 0
	new num[32], flag[32]
	while (i < argc) {
		read_argv(i + 1, arg, 31)
		parse(arg, num, 31, flag, 31)

		if(equali(flag, "m")) { 
			g_FlagMenuValues[i] = str_to_num(num)
		} else if(equali(flag, "h")) {
			g_FlagMenuValues[i] = (str_to_num(num) * 60)
		} else if(equali(flag, "d")) {
			g_FlagMenuValues[i] = (str_to_num(num) * 1440)
		} else if(equali(flag, "w")) {
			g_FlagMenuValues[i] = (str_to_num(num) * 10080)
		}
		i++
	}
	return PLUGIN_HANDLED
}
loadDefaultBantimes(num) {
	if(num == 1 || num == 0)
		server_cmd("amx_sethighbantimes 5 60 240 600 6000 0")
	if(num == 2 || num == 0)
		server_cmd("amx_setlowbantimes 5 30 60 480 600 1440")
	if(num == 3 || num == 0)
		server_cmd("amx_setflagtimes 60 240 600 1440 10080 40320 90720 0")
}
/*********    mysql escape functions     ************/
mysql_escape_string(const source[],dest[],len)
{
	copy(dest, len, source);
	replace_all(dest,len,"\\","\\\\");
	replace_all(dest,len,"\0","\\0");
	replace_all(dest,len,"\n","\\n");
	replace_all(dest,len,"\r","\\r");
	replace_all(dest,len,"\x1a","\Z");
	replace_all(dest,len,"'","\'");
	replace_all(dest,len,"^"","\^"");
}
mysql_get_username_safe(id,dest[],len) {
	new name[128]
	get_user_name(id,name,127)
	mysql_escape_string(name,dest,len)
}
mysql_get_servername_safe(dest[],len) {
	new server_name[256]
	get_cvar_string("hostname", server_name, charsmax(server_name))
	mysql_escape_string(server_name,dest,len)
}
