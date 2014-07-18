
/* AMX Mod script.
*   Admin Base Plugin
*
* by the AMX Mod X Development Team
*  originally developed by OLO
*
* This file is part of AMX Mod X.
*
*
*  This program is free software; you can redistribute it and/or modify it
*  under the terms of the GNU General Public License as published by the
*  Free Software Foundation; either version 2 of the License, or (at
*  your option) any later version.
*
*  This program is distributed in the hope that it will be useful, but
*  WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
*  General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software Foundation,
*  Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
*  In addition, as a special exception, the author gives permission to
*  link the code of this program with the Half-Life Game Engine ("HL
*  Engine") and Modified Game Libraries ("MODs") developed by Valve,
*  L.L.C ("Valve"). You must obey the GNU General Public License in all
*  respects for all of the code used other than the HL Engine and MODs
*  from Valve. If you modify this file, you may extend this exception
*  to your version of the file, but you are not obligated to do so. If
*  you do not wish to do so, delete this exception statement from your
*  version.
*
*  Modified for AMXBans 6.0
*
*  Based on admins plugin v1.8.1.3746
*
*  Rev 2010/04/13
*/

new AdminCount;

enum AdminProp
{
	AdminProp_Auth = 0,
	AdminProp_Password,
	AdminProp_Access,
	AdminProp_Flags
};

enum Field
{
	Field_Name[64],
	Field_Password[64],
	Field_Access,
	Field_Flags
};

new g_szAdmin[MAX_ADMINS][Field]

#define ADMIN_LOOKUP	(1<<0)
#define ADMIN_NORMAL	(1<<1)
#define ADMIN_STEAM	(1<<2)
#define ADMIN_IPADDR	(1<<3)
#define ADMIN_NAME	(1<<4)

new g_cmdLoopback[16]

// pcvars
new amx_mode;
new amx_password_field;
new amx_default_access;

//amxbans
new pcvarip,pcvarprefix,pcvaradminsfile
new g_ServerAddr[100],g_AdminsFromFile
new g_szAdminNick[33][32],g_iAdminUseStaticBantime[33]
new g_AdminNick[MAX_ADMINS][32];
new g_AdminUseStaticBantime[MAX_ADMINS];

//multi forward handles
new bool:g_isAdmin[33]

public plugin_init_core()
{
	load_translations("amxbans");
	load_translations("common");
	amx_mode=register_cvar("amx_mode", "1")
	amx_password_field=register_cvar("amx_password_field", "_pw")
	amx_default_access=register_cvar("amx_default_access", "")

	register_cvar("amx_vote_ratio", "0.02")
	register_cvar("amx_vote_time", "10")
	register_cvar("amx_vote_answers", "1")
	register_cvar("amx_vote_delay", "60")
	register_cvar("amx_last_voting", "0")
	register_cvar("amx_show_activity", "2")
	register_cvar("amx_votekick_ratio", "0.40")
	register_cvar("amx_voteban_ratio", "0.40")
	register_cvar("amx_votemap_ratio", "0.40")

	set_cvar_float("amx_last_voting", 0.0)


	register_srvcmd("amx_sqladmins", "adminSql")
	register_cvar("amx_sql_table", "admins")
// amxbans
	pcvarip=register_cvar("amxbans_server_address","")
	pcvarprefix=register_cvar("amx_sql_prefix", "amx")
	pcvaradminsfile=register_cvar("amxbans_use_admins_file","0")
//
	register_cvar("amx_mysql_host", "127.0.0.1")
	register_cvar("amx_mysql_user", "root")
	register_cvar("amx_mysql_pass", "")
	register_cvar("amx_mysql_db", "amx")

	register_concmd("amx_reloadadmins", "cmdReload", ADMIN_CFG)
	//register_concmd("amx_addadmin", "addadminfn", ADMIN_RCON, "<playername|auth> <accessflags> [password] [authtype] - add specified player as an admin to users.ini")

	formatex(g_cmdLoopback, 15, "amxauth%c%c%c%c", random_num('A', 'Z'), random_num('A', 'Z'), random_num('A', 'Z'), random_num('A', 'Z'))

	register_clcmd(g_cmdLoopback, "ackSignal")

	remove_user_flags(0, read_flags("z"))		// Remove 'user' flag from server rights

	new configsDir[64]
	get_localinfo("amx_configdir", configsDir, charsmax(configsDir))
	
	server_cmd("exec %s/amx.cfg", configsDir)	// Execute main configuration file
	server_cmd("exec %s/mysql.cfg", configsDir)
	//server_cmd("exec %s/amxbans.cfg", configsDir)

}

get_ban_type(type[],len,steamid[],ip[])
{
	if(contain(steamid,"STEAM_0:") == 0 && contain(steamid,"STEAM_0:2") == -1) {
		formatex(type,len,"S")
	} else {
		formatex(type,len,"SI")
	}
	if(equal(ip,"127.0.0.1") && equal(type,"SI")) return 0
	return 1
}

public plugin_cfg() {
	//fixx to be sure cfgs are loaded
	set_task(0.1,"delayed_plugin_cfg")
}
public delayed_plugin_cfg()
{
	pause("ace", "admin.amx");
	get_cvarptr_string(pcvarprefix,g_dbPrefix,charsmax(g_dbPrefix))
	get_cvarptr_string(pcvarip,g_ServerAddr,charsmax(g_ServerAddr))
	g_AdminsFromFile=get_cvarptr_num(pcvaradminsfile)
	
	if(strlen(g_ServerAddr) < 9) {
		new ip[32]
		get_user_ip(0,ip,31)
		formatex(g_ServerAddr,charsmax(g_ServerAddr),ip)
	}
	if(get_cvar_num("amxbans_debug") >= 1) server_print("[AMXBans] plugin_cfg: ip %s / prefix %s",g_ServerAddr,g_dbPrefix)
	
	server_cmd("amx_sqladmins")
	server_exec();

	set_task(6.1, "delayed_load")
}

public delayed_load()
{
	new configFile[128], curMap[64], configDir[128]

	get_localinfo("amx_configdir", configDir, charsmax(configDir))
	get_mapname(curMap, sizeof(curMap)-1)

	new i=0;
	
	while (curMap[i] != '_' && curMap[i++] != '^0') {/*do nothing*/}
	
	if (curMap[i]=='_')
	{
		// this map has a prefix
		curMap[i]='^0';
		formatex(configFile, sizeof(configFile)-1, "%s/maps/prefix_%s.cfg", configDir, curMap);

		if (file_exists(configFile))
		{
			server_cmd("exec %s", configFile);
		}
	}

	get_mapname(curMap, sizeof(curMap)-1)

	
	formatex(configFile, sizeof(configFile)-1, "%s/maps/%s.cfg", configDir, curMap)

	if (file_exists(configFile))
	{
		server_cmd("exec %s", configFile)
	}
	
}

loadSettings(szFilename[])
{
	new Text[512];
	new Flags[32];
	new Access[32]
	new AuthData[44];
	new Password[32];
	new Name[32];
	new Static[2];
	
	new TextLen;
		
	for(new i=0;read_file(szFilename, i, Text, charsmax(Text), TextLen);i++)
	{
		trim(Text);
			
		// comment
		if (Text[0]==';') 
		{
			continue;
		}
			
		// not enough parameters
		if(parse(Text, AuthData, charsmax(AuthData), Password, charsmax(Password), Access, charsmax(Access), Flags, charsmax(Flags), Name, charsmax(Name), Static, charsmax(Static)) < 2)
		{
			continue;
		}
			
		admins_push(AdminCount,AuthData,Password,read_flags(Access),read_flags(Flags));
		formatex(g_AdminNick[AdminCount], 31, Name);
		g_AdminUseStaticBantime[AdminCount] = str_to_num(Static);
			
		AdminCount++;
	}

	if (AdminCount == 1)
	{
		server_print(_T("[AMXBans] Loaded 1 admin from file"));
	}
	else
	{
		server_print(_T("[AMXBans] Loaded %d admins from file"), AdminCount);
	}
	
	return 1;
}

admins_push(pos, authdata[], password[], access, flags)
{
	if(pos >= MAX_ADMINS)
		return;  //Later it'll be information about increasing MAX_ADMINS define
	formatex(g_szAdmin[pos][Field_Name], 63, authdata);
	formatex(g_szAdmin[pos][Field_Password], 63, password);
	g_szAdmin[pos][Field_Access] = access;
	g_szAdmin[pos][Field_Flags] = flags;
}

admins_flush()
{
	for(new i=0;i<MAX_ADMINS;i++)
	{
		formatex(g_szAdmin[i][Field_Name], 63, "0");
		formatex(g_szAdmin[i][Field_Password], 63, "0");
		g_szAdmin[i][Field_Access] = 0;
		g_szAdmin[i][Field_Flags] = 0;
	}
}

stock admins_lookup(pos, AdminProp:prop, auth[]="", len=0)
{
	if(pos >= MAX_ADMINS)
		return 0;  //Later it'll be information about increasing MAX_ADMINS define
		
	if(prop == AdminProp_Access)
		return g_szAdmin[pos][Field_Access];
		
	else if(prop == AdminProp_Auth)
		formatex(auth, len, g_szAdmin[pos][Field_Name]);
		
	else if(prop == AdminProp_Password)
		formatex(auth, len, g_szAdmin[pos][Field_Password]);
		
	else if(prop == AdminProp_Flags)
		return g_szAdmin[pos][Field_Flags];
		
	return 0;
}

public adminSql()
{
	new table[32], error[128];
	AdminCount = 0;
	admins_flush();
	
	new host[64], user[32], pass[32], db[128];
	
	get_cvar_string("amx_mysql_host", host, 63);
	get_cvar_string("amx_mysql_user", user, 31);
	get_cvar_string("amx_mysql_pass", pass, 31);
	get_cvar_string("amx_mysql_db", db, 127);
	
	if(g_SqlX)
	{
		mysql_close(g_SqlX);
		g_SqlX = 0;
	}
		
	new sql = mysql_connect(host, user, pass, db, error, charsmax(error))
	
	get_cvar_string("amx_sql_table", table, 31)	
	
	//sql error or amxbans_use_admins_file == 1
	if (!sql || g_AdminsFromFile == 1)
	{
		if(!g_AdminsFromFile) server_print(_T("[AMXBans] SQL error: can't connect: '%s'"), error)
		
		//backup to users.ini
		new configsDir[64]
		
		get_localinfo("amx_configdir", configsDir, charsmax(configsDir))
		format(configsDir, 63, "%s/users.ini", configsDir)
		loadSettings(configsDir) // Load admins accounts
		
		new players[32], num, pv
		new name[32]
		get_players(players, num)
		for (new i=0; i<num; i++)
		{
			pv = players[i]
			get_user_name(pv, name, 31)
			accessUser(pv, name)
		}
		
		new szPrefix[12];
		get_cvarptr_string(pcvarprefix, szPrefix, 11);
		amxbans_sql_initialized(sql, szPrefix);
		
		return PLUGIN_HANDLED
	}
	if(g_AdminsFromFile > 1) return PLUGIN_HANDLED
	
	for(new i=0;i<MAX_ADMINS;i++)
	{
		formatex(g_AdminNick[i], charsmax(g_AdminNick[]), "0");
		g_AdminUseStaticBantime[i] = 0;
	}
	
//amxbans	

	mysql_query(sql, "SELECT aa.steamid,aa.password,aa.access,aa.flags,aa.nickname,ads.custom_flags,ads.use_static_bantime \
		FROM %s_amxadmins as aa, %s_admins_servers as ads, %s_serverinfo as si \
		WHERE ((ads.admin_id=aa.id) AND (ads.server_id=si.id) AND \
		((aa.days=0) OR (aa.expired>UNIX_TIMESTAMP(NOW()))) AND (si.address='%s'))", g_dbPrefix, g_dbPrefix, g_dbPrefix, g_ServerAddr)
		
//

	new AuthData[44];
	new Password[44];
	new Access[32];
	new Flags[32];
	new Nick[32];
	new Static[5]
	new iStatic
	
	while(mysql_nextrow(sql))
	{
		mysql_getresult(sql, "steamid", AuthData, sizeof(AuthData)-1);
		mysql_getresult(sql, "password", Password, sizeof(Password)-1);
		mysql_getresult(sql, "use_static_bantime", Static, sizeof(Static)-1);
		mysql_getresult(sql, "custom_flags", Access, sizeof(Access)-1);
		mysql_getresult(sql, "nickname", Nick, sizeof(Nick)-1);
		mysql_getresult(sql, "flags", Flags, sizeof(Flags)-1);
			
		//if custom access not set get the global
		trim(Access)
		if(equal(Access,"")) mysql_getresult(sql, "access", Access, sizeof(Access)-1);
			
		admins_push(AdminCount,AuthData,Password,read_flags(Access),read_flags(Flags));
			
		//save nick
		formatex(g_AdminNick[AdminCount], 31, Nick)
			
		//save static bantime
		iStatic=1
		if(equal(Static,"no")) iStatic=0
		g_AdminUseStaticBantime[AdminCount] = iStatic
			
		++AdminCount;
	}

	if (AdminCount == 1)
	{
		server_print(_T("[AMXBans] Loaded 1 admin from database"))
	}
	else
	{
		server_print(_T("[AMXBans] Loaded %d admins from database"), AdminCount)
	}
	
	new szPrefix[12];
	get_cvarptr_string(pcvarprefix, szPrefix, 11);
	amxbans_sql_initialized(sql, szPrefix);
	
	new players[32], num, pv
	new name[32]
	get_players(players, num)
	for (new i=0; i<num; i++)
	{
		pv = players[i]
		get_user_name(pv, name, 31)
		accessUser(pv, name)
	}
	
	return PLUGIN_HANDLED
}

public cmdReload(id, level, cid)
{
	if (!cmd_access(id, level, cid, 1))
		return PLUGIN_HANDLED

	//strip original flags (patch submitted by mrhunt)
	remove_user_flags(0, read_flags("z"))
	
	AdminCount = 0
	adminSql()

	if (id != 0)
	{
		if (AdminCount == 1)
			console_print(id, _T("[AMXBans] Loaded 1 admin from file"))
		else
			console_print(id, _T("[AMXBans] Loaded %d admins from file"), AdminCount)
	}

	return PLUGIN_HANDLED
}

getAccess(id, name[], authid[], ip[], password[])
{
	new index = -1
	new result = 0
	
	static Flags;
	static Access;
	static AuthData[44];
	static Password[32];

	for (new i = 0; i < AdminCount; ++i)
	{
		Flags=admins_lookup(i,AdminProp_Flags);
		admins_lookup(i,AdminProp_Auth,AuthData,sizeof(AuthData)-1);
		
		if (Flags & FLAG_AUTHID)
		{
			if (equal(authid, AuthData))
			{
				index = i
				break
			}
		}
		else if (Flags & FLAG_IP)
		{
			new c = strlen(AuthData)
			
			if (AuthData[c - 1] == '.')	/* check if this is not a xxx.xxx. format */
			{
				if (equal(AuthData, ip, c))
				{
					index = i
					break
				}
			}				/* in other case an IP must just match */
			else if (equal(ip, AuthData))
			{
				index = i
				break
			}
		} 
		else 
		{
			if (Flags & FLAG_TAG)
			{
				if (containi(name, AuthData) != -1)
				{
					index = i
					break
				}
			}
			else if (equali(name, AuthData))
			{
				index = i
				break
			}
		}
	}

	if (index != -1)
	{
		Access=admins_lookup(index,AdminProp_Access);
//amxbans
		formatex(g_szAdminNick[id],31,g_AdminNick[index])
		g_iAdminUseStaticBantime[id]=g_AdminUseStaticBantime[index]
//

		if (Flags & FLAG_NOPASS)
		{
			result |= 8
			new sflags[32]
			
			get_flags(Access, sflags, 31)
			set_user_flags(id, Access)
			
			g_isAdmin[id]=true
			
			log_amx("Login: ^"%s<%d><%s><>^" became an admin (account ^"%s^") (access ^"%s^") (address ^"%s^") (nick ^"%s^") (static %d)", \
				name, get_user_userid(id), authid, AuthData, sflags, ip,g_szAdminNick[id],g_iAdminUseStaticBantime[id])
		}
		else 
		{
		
			admins_lookup(index,AdminProp_Password,Password,sizeof(Password)-1);

			if (equal(password, Password))
			{
				result |= 12
				set_user_flags(id, Access)
				
				new sflags[32]
				get_flags(Access, sflags, 31)
				
				g_isAdmin[id]=true
				
				log_amx("Login: ^"%s<%d><%s><>^" became an admin (account ^"%s^") (access ^"%s^") (address ^"%s^") (nick ^"%s^") (static %d)", \
					name, get_user_userid(id), authid, AuthData, sflags, ip,g_szAdminNick[id],g_iAdminUseStaticBantime[id])
			} 
			else 
			{
				result |= 1
				
				if (Flags & FLAG_KICK)
				{
					result |= 2
					g_isAdmin[id]=false
					log_amx("Login: ^"%s<%d><%s><>^" kicked due to invalid password (account ^"%s^") (address ^"%s^")", name, get_user_userid(id), authid, AuthData, ip)
				}
			}
		}
	}
	else if (get_cvarptr_float(amx_mode) == 2.0)
	{
		result |= 2
	} 
	else 
	{
		new defaccess[32]
		
		get_cvarptr_string(amx_default_access, defaccess, 31)
		
		if (!strlen(defaccess))
		{
			copy(defaccess, 32, "z")
		}
		
		new idefaccess = read_flags(defaccess)
		
		if (idefaccess)
		{
			result |= 8
			set_user_flags(id, idefaccess)
		}
	}
	
	return result
}

accessUser(id, name[] = "")
{
	remove_user_flags(id)
	
	new userip[32], userauthid[32], password[32], passfield[32], username[32]
	
	get_user_ip(id, userip, 31, 1)
	get_user_authid(id, userauthid, 31)
	
	if (name[0])
	{
		copy(username, 31, name)
	}
	else
	{
		get_user_name(id, username, 31)
	}
	
	get_cvarptr_string(amx_password_field, passfield, 31)
	get_user_info(id, passfield, password, 31)
	
	new result = getAccess(id, username, userauthid, userip, password)
	
	if (result & 1)
	{
		client_cmd(id, "echo ^"* %s^"", _T("Invalid Password!"))
	}
	
	if (result & 2)
	{
		client_cmd(id, "%s", g_cmdLoopback)
		return PLUGIN_HANDLED
	}
	
	if (result & 4)
	{
		client_cmd(id, "echo ^"* %s^"", _T("Password accepted"))
	}
	
	if (result & 8)
	{
		client_cmd(id, "echo ^"* %s^"", _T("Privileges set"))
	}
	
	return PLUGIN_CONTINUE
}

public client_infochanged(id)
{
	if (!is_user_connected(id) || !get_cvarptr_num(amx_mode))
	{
		return PLUGIN_CONTINUE
	}

	new newname[32], oldname[32]
	
	get_user_name(id, oldname, 31)
	get_user_info(id, "name", newname, 31)

	if (!equali(newname, oldname))
	{
		accessUser(id, newname)
	}
	return PLUGIN_CONTINUE
}
public client_disconnect_core(id) {
	g_isAdmin[id]=false
}
public ackSignal(id)
{
	server_cmd("kick #%d ^"%s^"", get_user_userid(id), _T("You have no entry to the server..."))
	return PLUGIN_HANDLED
}

public client_authorized_core(id)
	return get_cvarptr_num(amx_mode) ? accessUser(id) : PLUGIN_CONTINUE

public client_putinserver_core(id)
{
	if (!is_dedicated_server() && id == 1)
		return get_cvarptr_num(amx_mode) ? accessUser(id) : PLUGIN_CONTINUE
	
	return PLUGIN_CONTINUE
}
