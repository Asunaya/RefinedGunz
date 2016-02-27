<?
	require_once("core.php");
	check_session();

	$sess_login = $_SESSION["Login"];
	$sess_level = $_SESSION["Level"];
	$sess_DB = $_SESSION["DB"];


	function print_menu_link($url, $menu_str, $level)
	{
		global $sess_level;
		if ($sess_level	<= $level)
		{
			echo ("<A href='$url' target='gapmain'>$menu_str</A><BR>\n");
		}

	}
?>
<HTML>
<BODY>
<table border=0 style="font-size:9pt">
<tr>
<td>
<a href='main.html' target="gapmain">HOME</A> <BR><BR>
<B>사용자 관리</B> <BR>

<?
	print_menu_link("ServerCurrentInfo.html", "서버 현황", 9);
	print_menu_link("AccountInfo.html", "계정 정보", 5);
	print_menu_link("CharacterInfo.html", "캐릭터 정보", 5);
	print_menu_link("CharacterCareLog.html", "캐릭터 관리기록 보기", 5);
	print_menu_link("112.html", "불량유저 신고 접수", 5);

	print_menu_link("ShopBuyItem.html", "캐쉬아이템 구매", 3);
	print_menu_link("InsertItemToAccountItem.html", "중앙은행에 아이템 추가", 3);
?>



<BR>
<B>정보보기</B> <BR>
<?
	print_menu_link("connlog.html", "접속로그 보기", 5);
	print_menu_link("ShopSellData.html", "캐쉬아이템 구매정보 보기", 5);
	print_menu_link("ShopBountyItemLog.html", "일반아이템 구매정보 보기", 5);
    print_menu_link("ShopPresentLog.html", "캐쉬아이템 선물기록 보기", 5);
	print_menu_link("ShopItemUseInfo.html", "캐쉬아이템 사용기록 보기", 5);

?>

</td>
</tr>

<tr>
<td>
<BR><b>상점관리 </B> <BR>
<?
	print_menu_link("ShopSingleItem.html", "아이템샵 관리", 3);
	print_menu_link("ShopSetItem.html", "셋트 아이템샵 관리", 3);
	print_menu_link("ShopNewItem.html", "신상품 관리", 3);
	print_menu_link("ShopReceiptLog.html", "아이템샵 로그", 3);
?>
</TD>
</TR>

<tr>
<td>
<BR>
<b>통계</B> <BR>
<?
	print_menu_link("Statistics_Total.html", "요약 정보", 3);
	print_menu_link("Sale.html", "매출 현황", 3);

?>
</TD>
</TR>

<tr>
<td>
<BR>
<b>관리자</B> <BR>
<?
	print_menu_link("user_list.php", "사용자 관리", 1);
?>
</TD>
</TR>


<tr>
<td>
<BR><b>설정</B><BR>
DB:
<?PHP
require_once('core.php');
echo ("$sess_DB");
#echo ("$g_db_name");
?>

<?
	if ($sess_level	<= 1)
	{
		echo ("(<A href='config.html' target='gapmain'><FONT color=#DDDDDD>modify</FONT></A>)\n");
	}


	echo ("<BR><A href='user_edit.php?userid=$sess_login&myself_edit=1' target='gapmain'>정보수정</A><BR>\n");

?>


</TD>
</TR>

<BR><BR>

<tr>
<td>
<BR><b>도움말</B><BR>

</TD>
</TR>


</table>
<BR>
</BODY>
</HTML>
