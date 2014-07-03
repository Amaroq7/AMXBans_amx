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
	
	mysql_query(g_SqlX, pquery);
	
	_check_flagged(id);
	
	return PLUGIN_HANDLED;
}

public _check_flagged(id)
{
	if(!mysql_nextrow(g_SqlX))
		return PLUGIN_HANDLED;
		
	new length, reason[128], created, fid, bool:flagged;
	new cur_time = get_systime();
	
	do
	{
		fid = mysql_getfield(g_SqlX, 1);
		mysql_getfield(g_SqlX, 1, reason, charsmax(reason));
		created = mysql_getfield(g_SqlX, 3);
		length = mysql_getfield(g_SqlX, 4);
		
		if(created + length * 60 > cur_time)
		{
			flagged = true;
		}
		else
		{
			remove_flagged(fid);
		}
	} while(mysql_nextrow(g_SqlX) > 0)
	
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