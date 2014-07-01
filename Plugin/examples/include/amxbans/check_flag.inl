/*

	AMXBans, managing bans for Half-Life modifications
	Copyright (C) 2003, 2004  Ronald Renes / Jeroen de Rover
	
	Copyright (C) 2009, 2010  Thomas Kurz

*/

#if defined _check_flag_included
    #endinput
#endif
#define _check_flag_included

check_flagged(id)
{
	if(g_being_flagged[id])
		return PLUGIN_HANDLED;
	
	new authid[35], ip[22], pquery[1024];
	get_user_authid(id, authid, charsmax(authid));
	get_user_ip(id, ip, charsmax(ip), 1);
	
	if(get_cvarptr_num(pcvar_flagged_all))
		formatex(pquery, charsmax(pquery), "SELECT `fid`,`reason`,`created`,`length` FROM `%s%s` WHERE player_id='%s' OR player_ip='%s' ORDER BY `length` ASC", g_dbPrefix, tbl_flagged, authid, ip);
	else
		formatex(pquery, charsmax(pquery), "SELECT `fid`,`reason`,`created`,`length` FROM `%s%s` WHERE (player_id='%s' OR player_ip='%s') AND `server_ip`='%s:%s' ORDER BY `length` ASC", g_dbPrefix, tbl_flagged, authid,ip, g_ip, g_port);
	
	new query = mysql_query(g_SqlX, pquery);
	
	_check_flagged(id, query);
	
	return PLUGIN_HANDLED;
}

public _check_flagged(id, query)
{
	if(!mysql_num_rows(query))
		return PLUGIN_HANDLED;
	
	new length, reason[128], created, fid, bool:flagged;
	new cur_time = get_systime();
	
	while(mysql_num_rows(query))
	{
		fid = mysql_getfield(query, 0);
		mysql_getfield(query, 1, reason, charsmax(reason));
		created = mysql_getfield(query, 2);
		length = mysql_getfield(query, 3);
		
		if(created + length * 60 > cur_time)
		{
			flagged = true;
		}
		else
		{
			remove_flagged(fid);
		}
		
		mysql_nextrow(query);
	}
	
	if(!flagged)
		return PLUGIN_HANDLED;
	
	//the last result contains the longest flagg time, using this of course
	
	g_flaggedTime[id] = length;
	copy(g_flaggedReason[id], charsmax(g_flaggedReason[]), reason);
	
	if(!g_being_flagged[id])
	{
		amxbans_player_flagged(id,(g_flaggedTime[id] * 60),g_flaggedReason[id])
	}
	g_being_flagged[id] = true;
	return PLUGIN_HANDLED;
}

remove_flagged(fid)
{
	mysql_query(g_SqlX, "DELETE FROM `%s%s` WHERE `fid`=%d", g_dbPrefix, tbl_flagged, fid);
	
	return PLUGIN_CONTINUE;
}