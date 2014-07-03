/*

	AMXBans, managing bans for Half-Life modifications
	Copyright (C) 2003, 2004  Ronald Renes / Jeroen de Rover
	
	Copyright (C) 2009, 2010  Thomas Kurz
	
	Amxbans Flagged Plugin
*/

new authid[33][35],ip[33][22],reason[33][100];
new flagged_end[33]

public amxbans_player_flagged(id,sec_left,reas[]) {
		
	if(!is_user_connected(id)) return PLUGIN_HANDLED
	if(sec_left) {
		flagged_end[id]=get_systime()+sec_left
	} else {
		flagged_end[id]=-1 //permanent
	}
	
	get_user_authid(id,authid[id],sizeof(authid[]))
	get_user_ip(id,ip[id],sizeof(ip[]))
	copy(reason[id],sizeof(reason[]),reas)
	
	set_task(10.0,"announce",id)
	return PLUGIN_HANDLED
}
public amxbans_player_unflagged(id) {
	if(task_exists(id)) remove_task(id)
}
public announce(id) {
	new name[32],left_str[32]
	get_user_name(id,name,sizeof(name))
	
	if(flagged_end[id]==-1) {
		formatex(left_str,charsmax(left_str)," ^4(%s)^1",_T("permanent", 0))
	} else if(flagged_end[id]) {
		new Float:left=float(flagged_end[id]-get_systime())/60
		//if(left <= 0.1 && task_exists(id)) remove_task(id)
		new left_int=floatround(left,floatround_ceil)
		
		formatex(left_str,charsmax(left_str)," ^x04(left: %d min)^x01",left_int)
		if(left_int) set_task(60.0,"announce",id)
	}
	//only show msg to admins with ADMIN_CHAT
	for(new i=1;i<=g_iMaxPlayers;i++) {
		if(!is_user_connected(i)) continue
		if(get_user_flags(i) & ADMIN_CHAT)
			client_print(i, print_chat, _T("[AMXBans] Flagged Player(s): %s <%s> Reason: %s"),name,authid[id],reason[id])
	}
}
public client_disconnect_flagged(id) {
	if(task_exists(id)) remove_task(id)
}
