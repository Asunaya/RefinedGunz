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
$r_uid = $_POST['uid'];

require_once("core.php");


include ("dbconn2.inc");

if ($r_mode=="")
{
	echo ("<CENTER>
	정말로 삭제하시겠습니까?
	<FORM action='user_delete.php' method=post>
		<INPUT type=hidden name=uid value='$r_uid'>
		<INPUT type=hidden name=mode value='process'><INPUT type=submit value='실행'></TD>
	</FORM>
	</CENTER>
");
}
else if ($r_mode == "process")
{
	$r_uid = $_POST['uid'];
	if ($r_uid == "") exit();

	$query = "DELETE FROM user WHERE uid=$r_uid";
	$result = $db2->query($query);

	if (!$result)
	{
		exit();
	}

	echo ("<meta http-equiv='Refresh' content='1; URL=user_list.php'>");
	echo ("<center><font size=2>삭제되었습니다.</font></center>\n");





}

$db2->close();





?>
</BODY>
</HTML>