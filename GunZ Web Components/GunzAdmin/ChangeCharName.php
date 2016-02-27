<?
	require_once("core.php");
	check_session();
?>
<HTML>
<HEAD>
<LINK REL='StyleSheet' HREF='style.css'>
</HEAD>
<BODY>
<?php

echo("<CENTER><H1>캐릭터 이름 변경</H1></CENTER>\n");

include ("dbconn.inc");

//// Declare Functions ////////////////////////////////////////////////////////
function print_update_charname_form($_aid, $_cid)
{
	echo ("<CENTER><TABLE border=0>
<FORM method=post name=f action='./ChangeCharName.php'>
<INPUT type=hidden name=aid value=$_aid>
<INPUT type=hidden name=cid value=$_cid>
<INPUT type=hidden name=mode value='process'>
<TR><TD colspan=2><B>변경할 이름을 입력해주세요</B></TD></TR>
<TR>
	<TD><INPUT type=text name='newname'></TD>
	<TD><INPUT type=image src='img/btn_ctcheck.gif'> &nbsp;&nbsp;</TD>
	<TD><INPUT type=submit value='이름 변경'></TD>
</TR>
<TR>

</TR>
</FORM>
</TABLE>
</CENTER>
<SCRIPT>
	f.newname.focus();
</SCRIPT>
");

}


//// Main /////////////////////////////////////////////////////////////////////
$mode = $_POST['mode'];
$aid = $_POST['aid'];
$cid = $_POST['cid'];

if ($aid == '')
{
	echo ("aid가 없습니다.");
	$db->close();
	exit();
}

if ($cid == '')
{
	echo ("cid가 없습니다.");
	$db->close();
	exit();
}



/// mode /////////////////
if ($mode == '')
{
	view_db_character_info($cid);

	echo("<BR><BR><BR><BR>");

	print_update_charname_form($aid, $cid);
}
else if ($mode == 'process')
{
	$newname = $_POST['newname'];
	if ($newname == '')
	{
		echo ("이름이 없습니다.");
		$db->close();
		exit();
	}

	$query = "EXEC spChangeCharName ". $aid . ", ". $cid . ", '$newname'";
//	$db->query($query);

//	$row = $db->fetch();

/*
	if ($row->Ret == '1')
	{
		echo ("<meta http-equiv='Refresh' content='1; URL=./CharacterInfo.html?cid=" . $cid . "'>");
		echo ("<center><font size=2>수정되었습니다.</font></center>\n");

	}
	else
	{
		echo ("<meta http-equiv='Refresh' content='3; URL=./CharacterInfo.html?cid=" . $cid . "'>");
		echo ("<center><font size=2>실패하였습니다.</font></center>\n");
	}
*/
}


?>

<? $db->close(); ?>
<? include ("sign.inc"); ?>
</BODY>
</HTML>