/*

	AMXBans, managing bans for Half-Life modifications
	Copyright (C) 2003, 2004  Ronald Renes / Jeroen de Rover
	
	Copyright (C) 2009, 2010  Thomas Kurz

*/

#if defined _cmdunban_included
    #endinput
#endif
#define _cmdunban_included

public cmdUnban(id, level, cid)
{
	/* Checking if the admin has the right access */
	if (!cmd_access(id,level,cid,2))
		return PLUGIN_HANDLED;
	
	read_argv(1, g_choicePlayerAuthid[id], charsmax(g_choicePlayerAuthid[]));
	
	new authid[36];
	new authip[16];
	
	if (contain(g_choicePlayerAuthid[id], "STEAM_") == 0)
		copy(authid, charsmax(authid), g_choicePlayerAuthid[id]);
	else
		copy(authip, charsmax(authip), g_choicePlayerAuthid[id]);
	
	mysql_query(g_SqlX, "SELECT * FROM `%s%s` \
	WHERE (`player_id` = '%s' OR `player_ip` = '%s') AND `expired` = 0", \
	g_dbPrefix, tbl_bans, authid, authip);
	
	cmd_unban_1(id);
	
	return PLUGIN_HANDLED;
}

public cmd_unban_1(id)
{
	new bool:serverCmd = false;
	/* Determine if this was a server command or a command issued by a player in the game */
	if (id == 0)
		serverCmd = true;
	
	while(mysql_nextrow(g_SqlX)) {}
	
	new iRows = mysql_num_rows(g_SqlX);
	
	if (iRows == 1)
	{
		new banid = mysql_getfield(g_SqlX, 1);
		new admin_nick[100];
			
		if (serverCmd)
		{
			new servernick[100];
			get_cvarptr_string(pcvar_server_nick, servernick, 99);
			if (strlen(servernick))
				copy(admin_nick, charsmax(admin_nick), servernick);
		}
		else
			mysql_get_username_safe(id, admin_nick, 99);
		
		mysql_query(g_SqlX, "INSERT INTO `%s%s` \
		(`id`, `bid`, `edit_time`, `admin_nick`, `edit_reason`) \
		VALUES ('', '%d', UNIX_TIMESTAMP(NOW()), '%s', 'Unbanned in-game')", \
		g_dbPrefix, tbl_bans_edit, banid, admin_nick);
			
		cmd_unban_2(id, banid);
	}
	else if(!iRows)
	{
		if (serverCmd)
			server_print(_T("[AMXBans] SteamID / IP %s is not banned."), g_choicePlayerAuthid[id]);
		else
			console_print(id, _T("[AMXBans] SteamID / IP %s is not banned."), g_choicePlayerAuthid[id]);
	}
	else
	{
		if (serverCmd)
			server_print(_T("[AMXBans] SteamID / IP %s has %d active bans, cannot unban."), g_choicePlayerAuthid[id], mysql_num_rows(g_SqlX));
		else
			console_print(id, _T("[AMXBans] SteamID / IP %s has %d active bans, cannot unban."), g_choicePlayerAuthid[id], mysql_num_rows(g_SqlX));
	}
}

public cmd_unban_2(id, banid)
{	
	mysql_query(g_SqlX, "UPDATE `%s%s` \
	SET `ban_length` = '-1', `expired` = '1' \
	WHERE `bid` = %d", \
	g_dbPrefix, tbl_bans, banid);
	
	cmd_unban_3(id);
}

public cmd_unban_3(id)
{
	new bool:serverCmd = false;
	/* Determine if this was a server command or a command issued by a player in the game */
	if (id == 0)
		serverCmd = true;
	
	if (serverCmd)
	{
		server_print(_T("%s has been unbanned"), g_choicePlayerAuthid[id]);
	}
	else
	{
		console_print(id, _T("%s has been unbanned"), g_choicePlayerAuthid[id]);
	}
		
	log_amx(_T("[AMXBans] %s<%d><%s><%s> unbanned %s"), "CONSOLE", 0, "CONSOLE", "", g_choicePlayerAuthid[id]);
}