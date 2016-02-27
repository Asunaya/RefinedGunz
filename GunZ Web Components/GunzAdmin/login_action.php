<?php
	session_start();

	require_once("core.php");

	$username = $_POST["username"];
	$password	= $_POST["passwd"];

	setcookie("gap_id", $username, time()+43200);


	if (!$username || !$password)
	{
		header ("Location: /login.html");
		exit();
	}

	include "dbconn2.inc";

	$query = "select * from user where userid='".$username."' AND passwd=PASSWORD('$password')";

	$db2->query ($query);
	if ($db2->getNumRow () != 1)
	{
		header ("Location: /login.html");
		$db2->close();
		exit();
	}

	$row = $db2->fetch ();

	session_start ();
	session_register ("Login");
	session_register ("Level");
	session_register ("DB");
	$_SESSION["Login"] = $username;
	$_SESSION["Level"] = $row->level;
	$_SESSION["DB"] = "GunzDB";
	header ("Location: /index.html");
	exit();

?>
