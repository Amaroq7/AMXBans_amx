/*

	AMXBans, managing bans for Half-Life modifications
	Copyright (C) 2003, 2004  Ronald Renes / Jeroen de Rover
	
	Copyright (C) 2009, 2010  Thomas Kurz

*/

#if defined _init_functions_included
    #endinput
#endif
#define _init_functions_included

/*********************  Banmod online  ********************/
public banmod_online(id)
{
	// This is a new way of getting the port number
	new ip_port[100]
	get_cvarptr_string(pcvar_serverip,ip_port,99)
	if(contain(ip_port,":") == -1) {
		get_user_ip(0, ip_port, 99, 0) // Takes in the whole IP:port string.. (0 is always the server)
	}
	strtok(ip_port, g_ip, 90, g_port, 9, ':')

	if ( get_cvarptr_num(pcvar_debug) >= 1 )
	{
		server_print("[AMXBans] The server IP:PORT is: %s:%s", g_ip, g_port)
		log_amx("[AMXBans] The server IP:PORT is: %s:%s", g_ip, g_port)
	}
	
	mysql_query(g_SqlX, "SELECT `motd_delay` FROM `%s%s` WHERE address = '%s:%s'", g_dbPrefix, tbl_serverinfo,g_ip,g_port);

	banmod_online_(id);
}

public banmod_online_(id)
{
	new timestamp = get_systime(0)
	new servername[100]
	mysql_get_servername_safe(servername,charsmax(servername))
	new modname[32]
	get_modname(modname,charsmax(modname))
		
	if (!mysql_num_rows(g_SqlX)) {
		if ( get_cvarptr_num(pcvar_debug) >= 1 ) {
			server_print("[AMXBans] INSERT INTO `%s%s` VALUES ('', %i,'%s', '%s:%s', '%s', '', '%s', '', '', '0')", g_dbPrefix, tbl_serverinfo, timestamp, servername, g_ip, g_port, modname, amxbans_version)
			log_amx("[AMXBans] INSERT INTO `%s%s` VALUES ('', %i,'%s', '%s:%s', '%s', '', '%s', '', '', '0')", g_dbPrefix, tbl_serverinfo, timestamp, servername, g_ip, g_port, modname, amxbans_version)
		}
		
		formatex(pquery, charsmax(pquery),"INSERT INTO `%s%s` (timestamp, hostname, address, gametype, amxban_version, amxban_menu) VALUES \
			(%i, '%s', '%s:%s', '%s', '%s', 1)", g_dbPrefix, tbl_serverinfo, timestamp, servername, g_ip, g_port, modname, amxbans_version)
	} else {
		new kick_delay_str[10]
		mysql_nextrow(g_SqlX);
		mysql_getfield(g_SqlX, 1, kick_delay_str, 9)

		if (floatstr(kick_delay_str)>2.0) {
			kick_delay=floatstr(kick_delay_str)
		} else {
			kick_delay=10.0
		}

		if ( get_cvarptr_num(pcvar_debug) >= 1 ) {
			server_print("AMXBANS DEBUG] UPDATE `%s%s` SET timestamp=%i,hostname='%s',gametype='%s',amxban_version='%s', amxban_menu=1 WHERE address = '%s:%s'", g_dbPrefix, tbl_serverinfo, timestamp, servername, modname, amxbans_version, g_ip, g_port)
			log_amx("[AMXBANS DEBUG] UPDATE `%s%s` SET timestamp=%i,hostname='%s',gametype='%s',amxban_version='%s', amxban_menu=1 WHERE address = '%s:%s'", g_dbPrefix, tbl_serverinfo, timestamp, servername, modname, amxbans_version, g_ip, g_port)
		}
		formatex(pquery, charsmax(pquery), "UPDATE `%s%s` SET timestamp='%i',hostname='%s',gametype='%s',amxban_version='%s', amxban_menu='1' WHERE address = '%s:%s'", g_dbPrefix, tbl_serverinfo, timestamp, servername, modname, amxbans_version, g_ip, g_port)
	
	}
	
	log_amx(_T("[AMXBans] AMXBans %s is online"), VERSION)
	
	mysql_query(g_SqlX, pquery);
	
	return PLUGIN_CONTINUE
}

/************  Start fetch reasons  *****************/
public cmdFetchReasons(id,level,cid) {
	if (!cmd_access(id,level,cid,1))
		return PLUGIN_HANDLED
		
	fetchReasons(id)
	return PLUGIN_HANDLED
}
public fetchReasons(id)
{		
	mysql_query(g_SqlX, "SELECT re.reason,re.static_bantime FROM %s%s as re,%s%s as rs ,%s%s as si \
				WHERE si.address = '%s:%s' AND si.reasons = rs.setid and rs.reasonid = re.id \
				ORDER BY re.id", g_dbPrefix, tbl_reasons, g_dbPrefix, tbl_reasons_to_set, g_dbPrefix, tbl_serverinfo, g_ip,g_port);
	
	fetchReasons_();
	
	return PLUGIN_HANDLED
}

public fetchReasons_()
{
	new aNum;
	
	new iRows = mysql_num_rows(g_SqlX);
	
	if(!iRows) {
		server_print("[AMXBans] %s",_T("No Reasons found"))
		new temp[128]
		formatex(temp,charsmax(temp), _T("Cheater",0))
		ArrayPushReasons(0,temp,0)
		formatex(temp,charsmax(temp), _T("Laming",0))
		ArrayPushReasons(1,temp,0)
		formatex(temp,charsmax(temp), _T("Swearing",0))
		ArrayPushReasons(2,temp,0)
		formatex(temp,charsmax(temp), _T("Wallhack",0))
		ArrayPushReasons(3,temp,0)
		formatex(temp,charsmax(temp), _T("Aimbot",0))
		ArrayPushReasons(4,temp,0)
		formatex(temp,charsmax(temp), _T("Wallhack + Aimbot",0))
		ArrayPushReasons(5,temp,0)
		formatex(temp,charsmax(temp), _T("Camper",0))
		ArrayPushReasons(6,temp,0)
	
		server_print("[AMXBans] %s", _T("No Reasons found in Database. Static reasons were loaded instead."))
		log_amx("[AMXBans] %s",_T("No Reasons found in Database. Static reasons were loaded instead."))

		aNum = 7

		return PLUGIN_HANDLED
	} 
	else
	{
		new reason[128]
		new reason_time
		while(mysql_nextrow(g_SqlX))
		{
			mysql_getfield(g_SqlX, 1, reason,charsmax(reason))
			reason_time=mysql_getfield(g_SqlX,2)
			ArrayPushReasons(aNum,reason,reason_time)
			mysql_nextrow(g_SqlX)
			aNum++
		}
	}
	
	if (aNum == 1)
		server_print(_T("[AMXBans] 1 Reason loaded from Database.") )
	else
		server_print(_T("[AMXBans] %d Reasons loaded from Database."), aNum )
	
	return PLUGIN_HANDLED
}
ArrayPushReasons(pos,reason[],bantime)
{
	if(pos >= MAX_REASONS)
		return;
		
	formatex(g_banReasons[pos], 127, reason)
	g_banReasons_Bantime[pos] = bantime;
}