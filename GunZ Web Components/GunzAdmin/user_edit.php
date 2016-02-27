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
$r_uid = $_GET['uid'];
$r_myself_edit = $_GET['myself_edit'];

require_once("core.php");


include ("dbconn2.inc");

if ($r_uid == '')
{
	$r_userid = $_GET['userid'];
	$query = "SELECT uid FROM user WHERE userid='$r_userid'";
	$db2->query($query);

	$row = $db2->fetch();
	$r_uid = $row->uid;
}



if ($r_mode=="")
{
	$query = "SELECT uid, userid, name, level FROM user WHERE uid=$r_uid";
	$db2->query($query);

	$row = $db2->fetch();

	$userid=$row->userid;
	$name=$row->name;
	$level = $row->level;
	$password = $row->passwd;

	echo ("<CENTER><H2>관리자</H2></CENTER><BR><BR>\n");

	echo ("<CENTER>
	<TABLE border=1 cellspacing=1 cellpadding=3 bordercolor=#AAAAAA width=300>
	<TR>
	<FORM action='user_edit.php' method=post>
	<TD colspan=3><B>관리자 수정</B></TD>
	</TR>
	<TR>
	<TD bgcolor=#EEEEEE>관리자 ID</TD>
	<TD><INPUT type=text name=userid value='$userid'></TD>
	</TR>
	<TR>
	<TD bgcolor=#EEEEEE>암호</TD>
	<TD><INPUT type=password name=password value=''></TD>
	</TR>
	<TR>
	<TD bgcolor=#EEEEEE>암호검증</TD>
	<TD><INPUT type=password name=password_r value=''></TD>
	</TR>
	<TR>
	<TD bgcolor=#EEEEEE>이름</TD>
	<TD><INPUT type=text name=name value='$name'></TD>
	</TR>
");

	if ($r_myself_edit != 1)
	{
		echo ("
	<TR>
	<TD bgcolor=#EEEEEE>레벨</TD>
	<TD >
");
	view_form_web_user_level_select("level", $level);
echo("
	</TD>
	</TR>
");

	}

	echo("
	<TR>
	<TD align=center colspan=3>
		<INPUT type=hidden name=uid value='$r_uid'>
		<INPUT type=hidden name=mode value='process'><INPUT type=submit value='수정완료'></TD>
	</FORM>
	</TR>
	</TABLE>
<BR><BR>
");
	if ($r_myself_edit != 1)
	{
		echo ("<TABLE><TR><TD>
	<FORM action='user_delete.php' method=post>
		<INPUT type=hidden name=uid value='$r_uid'>
		<INPUT type=submit value='사용자 삭제'>
	</FORM>
	</TD></TR></TABLE>
");
	}

	echo ("</CENTER>\n");
}
else if ($r_mode == "process")
{
	$r_uid = $_POST['uid'];
	$r_userid = $_POST['userid'];
	$r_password = $_POST['password'];
	$r_password_r = $_POST['password_r'];
	$r_name = $_POST['name'];
	$r_level = $_POST['level'];


	if ($r_password != $r_password_r)
	{
		echo ("암호를 제대로 입력해 주십시오.\n");
		exit();
	}



	$query = "SELECT * FROM user WHERE userid='$r_userid' AND uid != $r_uid";
	$db2->query($query);
	if ($db2->getNumRow() > 0)
	{
		echo ("같은 ID가 존재합니다.");
		exit();
	}

	$query = "";
	$level_query = "";
	if ($r_level != '')
	{
		$level_query = ", level=" . $r_level . " ";

	}

	if ($r_password != '')
	{
		$query = "UPDATE user SET userid='$r_userid', passwd=PASSWORD('$r_password'), name='$r_name' " . $level_query .
    	         "WHERE uid=$r_uid";

	}
	else
	{
		$query = "UPDATE user SET userid='$r_userid', name='$r_name' " . $level_query .
        	     "WHERE uid=$r_uid";
	}

	$result = $db2->query($query);

	if (!$result)
	{
		exit();
	}

	echo ("<meta http-equiv='Refresh' content='1; URL=main.html'>");
	echo ("<center><font size=2>수정되었습니다.</font></center>\n");





}


	$db2->close();




?>
</BODY>
</HTML>