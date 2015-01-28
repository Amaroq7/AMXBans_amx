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
	return (page + 1 >= get_pages(var, ratio)) ? 1 : 0;
}

stock next_page(page, var, ratio)
{
	return (page + 1 < get_pages(var, ratio)) ? page+1 : get_pages(var, ratio);
}

stock is_firstpage(page)
{
	return (!page) ? 1 : 0;
}

stock left_entries(page, var, ratio)
{
	return ((page+1)*ratio < var) ? 1 : 0;
}

move_array(array[][], array_size, size, &position)
{
	new bool:empty, pos;
	for(new i=0;i<size;i++)
	{
		if(empty && strlen(array[i]))
		{
			pos = i;
			copy(array[--pos], array_size-1, array[i]);
			copy(array[i], array_size-1, "");
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
stock get_flagtime_string(id,btime,text[],len,without=0)
{
	if(btime <=0 )
	{
		if(!without)
		{
			formatex(text,len,_T("Flag permanent",id))
		}
		else
		{
			formatex(text,len,_T("permanently",id))
		}
	}
	else
	{
		if(!without)
		{
			new szText[128]
			get_time_length(id,btime,timeunit_minutes,szText,charsmax(szText))
			formatex(text,len,_T("Flag for %s",id),szText)
		}
		else
		{
			get_time_length(id,btime,timeunit_minutes,text,len)
		}
	}
}
/*******************************************************************************************************************/
public setCustomBanReason(id,level,cid)
{
	if (!cmd_access(id,level,cid,1)) {
		return PLUGIN_HANDLED
	}

	if(!set_custom_reason[id]) return PLUGIN_HANDLED
	
	new szReason[128]
	read_argv(1,szReason,127)
	copy(g_choiceReason[id],127,szReason)
	
	set_custom_reason[id]=false
	
	if(get_cvarptr_num(pcvar_debug) >= 2)
		log_amx("[AMXBans CustomReason] %d choice: %s (%d min)",id,g_choiceReason[id],g_choiceTime[id])
	
	if(g_in_flagging[id])
	{
		g_in_flagging[id]=false
		FlagPlayer(id)
	}
	else if(g_choicePlayerId[id] == -1)
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
