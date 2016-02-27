<?
	require_once("core.php");
	check_session();
?>
<HTML>
<HEAD>
<LINK REL='StyleSheet' HREF='style.css'>
</HEAD>
<BODY>
<?

require_once('core.php');

include ("dbconn2.inc");


echo ("<CENTER><H2>관리자</H2></CENTER><BR><BR>\n");

$query = "SELECT uid, userid, name, level, regdate FROM user ORDER BY uid DESC";
$db2->query($query);


echo ("<CENTER>
<TABLE border=0 cellspacing=1 cellpadding=3>
<TR>
<TD colspan=3><a href='user_create.php'>추가</a></TD>
</TR>
<TR bgcolor=#EEEEEE>
<TD align=center width=140><B>아이디</B></TD>
<TD align=center width=120><B>이름</B></TD>
<TD align=center width=80><B>등급</B></TD>
</TR>
");

while ($row = $db2->fetch())
{
	$uid = $row->uid;
	$userid = stripslashes($row->userid);
	$name = stripslashes($row->name);
	$level_str = view_get_web_user_level_str($row->level);

	echo("<TR>
<TD><A href='user_edit.php?uid=$uid'>$userid</A></TD>
<TD>$name</TD>
<TD>$level_str</TD>
</TR>
");

}


echo ("</TABLE></CENTER>\n");



$db2->close();
?>
</BODY>
</HTML>