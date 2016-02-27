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

$r_mode = $_POST['mode'];

require_once('core.php');




if ($r_mode=="")
{
	echo ("<CENTER><H2>관리자</H2></CENTER><BR><BR>\n");

	echo ("<CENTER>
	<FORM action='user_create.php' method=post>
	<TABLE border=1 cellspacing=1 cellpadding=3 bordercolor=#AAAAAA width=300>
	<TR>
	<TD colspan=3><B>새로운 관리자 추가</B></TD>
	</TR>
	<TR>
	<TD bgcolor=#EEEEEE>관리자 ID</TD>
	<TD><INPUT type=text name=userid></TD>
	</TR>
	<TR>
	<TD bgcolor=#EEEEEE>암호</TD>
	<TD><INPUT type=password name=password></TD>
	</TR>
	<TR>
	<TD bgcolor=#EEEEEE>암호검증</TD>
	<TD><INPUT type=password name=password_r></TD>
	</TR>
	<TR>
	<TD bgcolor=#EEEEEE>이름</TD>
	<TD><INPUT type=text name=name></TD>
	</TR>
	<TR>
	<TD bgcolor=#EEEEEE>레벨</TD>
	<TD><INPUT type=text name=level></TD>
	</TR>
	<TD align=center colspan=3><INPUT type=hidden name=mode value='process'><INPUT type=submit value='관리자 생성'></TD>
	</TR>
	</TABLE>
	</FORM>
	");

	echo ("</CENTER>\n");
}
else if ($r_mode == "process")
{
	$r_userid = $_POST['userid'];
	$r_password = $_POST['password'];
	$r_password_r = $_POST['password_r'];
	$r_name = $_POST['name'];
	$r_level = $_POST['level'];
	if ($r_level == '') $r_level = 9;

	if ($r_password != $r_password_r)
	{
		echo ("암호를 제대로 입력해 주십시오.\n");
		exit();
	}

	include ("dbconn2.inc");

	$query = "SELECT * FROM user WHERE userid='$r_userid'";
	$db2->query($query);

	if ($db2->getNumRow() > 0)
	{
		echo ("같은 ID가 존재합니다.");
		exit();
	}

	$query = "INSERT INTO user (userid, passwd, name, level, regdate) " .
		     "VALUES ('$r_userid', PASSWORD('$r_password'), '$r_name', $r_level, now())";

	$db2->query($query);

	echo ("<meta http-equiv='Refresh' content='1; URL=user_list.php'>");
	echo ("<center><font size=2>입력되었습니다.</font></center>\n");


	$db2->close();


}







?>
</BODY>
</HTML>