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

echo("<CENTER><H1>계정 권한 수정</H1></CENTER>\n");

include ("dbconn.inc");

//// Declare Functions ////////////////////////////////////////////////////////
function print_update_account_ugrade_confirm($aid)
{
	////
	global $db;


	$query = "SELECT aid, userid, UGradeID FROM Account(nolock) where aid=" . $aid;
	$db->query($query);

	$row = $db->fetch();

	$ugradeid_str = view_get_ugrade_str($row->ugradeid);

	echo("<FORM action='./account_update_ugrade.php' method=post>
<INPUT type=hidden name=aid value='$aid'>
<INPUT type=hidden name=mode value='process'>
<CENTER><BR><BR>
<TABLE cellpadding=1 cellspacing=1>
<TR><TD bgcolor=#EEEEE width=120 height=30 align=center>넷마블아이디</TD><TD bgcolor=#F0F0F0 width=120 align=center><B>$row->userid</B></TD></TR>
<TR><TD bgcolor=#EEEEE height=30 align=center>권한</TD><TD bgcolor=#F0F0F0 align=center>");

	view_from_ugrade_select('ugradeid', $row->UGradeID);

	echo("</TD></TR>
<TR><TD bgcolor=#EEEEE width=120 align=center>기간</TD>
<TD bgcolor=#F0F0F0 width=120 height=30 align=center>
	<SELECT name='period'>
		<OPTION value='0'></OPTION>
		<OPTION value='1'>1일</OPTION>
		<OPTION value='2'>2일</OPTION>
		<OPTION value='3'>3일</OPTION>
		<OPTION value='7'>7일</OPTION>
		<OPTION value='14'>14일</OPTION>
		<OPTION value='30'>30일</OPTION>
	</SELECT>
</TD></TR>

</TABLE>
<BR><BR>
<INPUT TYPE=submit value='수정완료'>
</FORM>
");

}

//// Main /////////////////////////////////////////////////////////////////////
$mode = $_POST['mode'];
$aid = $_POST['aid'];

if ($aid == '')
{
	echo ("aid가 없습니다.");
	exit();
}

if ($mode == '')
{
	print_update_account_ugrade_confirm($aid);
	echo("<BR><BR>");
	view_penaltylog_from_aid($aid);
}
else if ($mode == 'process')
{

	$ugradeid = $_POST['ugradeid'];
	$period = $_POST['period'];

	$query = "EXEC spUpdateAccountUGrade ". $aid . ", ". $ugradeid . ", " . $period;
	$db->query($query);

	echo ("<meta http-equiv='Refresh' content='1; URL=./AccountInfo.html?mode=process&aid=" . $aid . "'>");
	echo ("<center><font size=2>수정되었습니다.</font></center>\n");


}

?>

<? $db->close(); ?>
<? include ("sign.inc"); ?>
</BODY>
</HTML>