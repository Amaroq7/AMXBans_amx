<?php

/* 	

	AMXBans v6.0
	
	Copyright 2009, 2010 by SeToY & |PJ|ShOrTy

	This file is part of AMXBans.

    AMXBans is free software, but it's licensed under the
	Creative Commons - Attribution-NonCommercial-ShareAlike 2.0

    AMXBans is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    You should have received a copy of the cc-nC-SA along with AMXBans.  
	If not, see <http://creativecommons.org/licenses/by-nc-sa/2.0/>.

*/

if(!$_SESSION["loggedin"]) {
	header("Location:index.php");
}

//Just for now, becuase there isn't any db at the moment to check if there is a new update for amxbans
header("Location:admin.php");

/*

$admin_site="up";
$title2 ="_TITLEUPDATE";

$update_ip="amxbans.de"; 
$update_user="amxbans_vcheck";
$update_pw="";
$update_db="amxbans_version";

//get version from servers
$query=$mysql->query("SELECT `address`,`amxban_version` FROM `".$config->db_prefix."_serverinfo` ORDER BY `address`") or die($mysql->error);
$version_server=array();
while($result = $query->fetch_object()) {
	$version=[
		"address"=>$result->address,
		"version"=>$result->amxban_version
	];
	$version_server[]=$version;
	$server_count++;
}
$query->close();
$smarty->assign("server_count",$server_count);
$smarty->assign("version_server",$version_server);


//get versions from update db
@$mysql_upd = new mysqli($update_ip,$update_user,$update_pw) or $error[]="_UPD_CONNECT_ERROR";
if($mysql_upd) {
	$resource = $mysql_upd->select_db($update_db) or $error[]="_UPD_DB_ERROR";
	if(!$error) {	
		//get newest web versions info
		$query = $mysql_upd->query("SELECT * FROM `version` WHERE `for`='web' ORDER BY `release` DESC LIMIT 1") or $error[]="_UPD_SELECT_ERROR";
		while($result = $query->fetch_object()) {
			$version=[
				"release"=>$result->release,
				"beta"=>$result->beta,
				"recommended_to"=>$result->recommended_to,
				"changelog"=>$result->changelog,
				"url"=>$result->url
			];
		}
		$query->close();
		$smarty->assign("version_db_web",$version);
		//get newest plugin versions info
		$query = $mysql_upd->query("SELECT * FROM `version` WHERE `for`='plugin' ORDER BY `release` DESC LIMIT 1") or $error[]="_UPD_SELECT_ERROR";
		while($result = $query->fetch_object()) {
			$version=[
				"release"=>$result->release,
				"beta"=>$result->beta,
				"recommended_to"=>$result->recommended_to,
				"changelog"=>$result->changelog,
				"url"=>$result->url
			];
		}
		$query->close();
		$smarty->assign("version_db_plugin",$version);
	}
	$mysql_upd->close();
}
$smarty->assign("error",$error);
?>*/
