<?PHP

require_once("db_api.php");

###### 전역 변수 ############
$g_ugrade_str = array (
  0 => "일반유저",
  1 => "정액유저",
  2 => "게임짱",
  100 => "전과자",
  101 => "1차경고",
  102 => "2차경고",
  103 => "3차경고",
  104 => "채팅금지",
  105 => "기간블럭",
  252 => "이벤트마스터",
  253 => "영구블럭",
  254 => "개발자",
  255 => "관리자"
);

# 웹 관리자 권한
$g_web_user_level_str = array (
  9 => "guest",
  5 => "GM",
  3 => "개발자",
  1 => "슈퍼관리자"

);


## 유저 권한 문자열 반환
function view_get_ugrade_str($ugrade)
{
	global $g_ugrade_str;

	reset($g_ugrade_str);
	while(list($key, $value) = each($g_ugrade_str))
	{
		if ($key == $ugrade) return $value;

	}
	return "일반유저";
}

function view_from_ugrade_select($select_form_name, $sel_ugradeid)
{
	global $g_ugrade_str;


	echo ("<SELECT name=$select_form_name>\n");

	reset($g_ugrade_str);
	while(list($key, $value) = each($g_ugrade_str))
	{
		$selected_str = '';
		if ($key == $sel_ugradeid) $selected_str = "selected";
		echo ("<option value='$key' $selected_str>$value</option>\n");
	}
	echo("</SELECT>\n");
}


## 웹 관리자 권한 문자열 반환
function view_get_web_user_level_str($level)
{
	global $g_web_user_level_str;

	reset($g_web_user_level_str);
	while(list($key, $value) = each($g_web_user_level_str))
	{
		if ($key == $level) return $value;
	}
	return "noname";
}

function view_form_web_user_level_select($select_form_name, $sel_level)
{
	global $g_web_user_level_str;


	echo ("<SELECT name=$select_form_name>\n");

	reset($g_web_user_level_str);
	while(list($key, $value) = each($g_web_user_level_str))
	{
		$selected_str = '';
		if ($key == $sel_level) $selected_str = "selected";
		echo ("<option value='$key' $selected_str>$value</option>\n");
	}
	echo("</SELECT>\n");
}





# 캐릭터 정보를 보여준다
function view_character_info($char_arr, $cnt)
{
  echo("<B>캐릭터정보</B><BR>\n");
    echo("<TABLE border=1 cellpadding=0 cellspacing=0>
<TR bgcolor=#EEEEEE>
<TD width=70 align=center>AID</TD>
<TD width=70 align=center>CID</TD>
<TD width=100 align=center>이름</TD>
<TD width=80 align=center>레벨</TD>
<TD width=50 align=center>성별</TD>
<TD width=50 align=center>CharNum</TD>
<TD width=50 align=center>XP</TD>
<TD width=50 align=center>BP</TD>
<TD width=50 align=center>RegDate</TD>
<TD width=50 align=center>LastTime</TD>
<TD width=50 align=center>PlayTime(분)</TD>
<TD width=50 align=center>킬수</TD>
<TD width=50 align=center>죽은수</TD>
</TR>
");

  for($i = 0; $i < $cnt; $i++)
  {
		$row = $char_arr[$i];
    $playtime_min = (int)($char_arr[$i]->PlayTime / 60);
    	$aid = $row->AID;
    	$cid = $row->CID;

    $aid_str = "<A href='AccountInfo.html?aid=" . $aid . "&mode=process'>$aid</A>";
    $cid_str = "<A href='CharacterInfo.html?cid=" . $cid . "&mode=process'>$cid</A>";

    echo("<TR>
<TD align=center>$aid_str</TD>
<TD align=center>$cid_str</TD>
<TD align=center>{$row->Name}</TD>
<TD align=center>{$row->Level}</TD>
<TD align=center>{$row->Sex}</TD>
<TD align=center>{$row->CharNum}</TD>
<TD align=center>{$row->XP}</TD>
<TD align=center>{$row->BP}</TD>
<TD align=center>{$row->RegDate}</TD>
<TD align=center>{$row->LastTime}</TD>
<TD align=center>$playtime_min</TD>
<TD align=center>{$row->KillCount}</TD>
<TD align=center>{$row->DeathCount}</TD>
</TR>\n");

  }


  echo ("</TABLE>\n");
}

# 디비에서 직접 캐릭터 정보를 보여준다
function view_db_character_info($a_cid, $a_charname='')
{
	global $db;

	if ($charname != '')
	{
		$query = "SELECT * FROM Character c(nolock) WHERE c.name='$a_charname'";
	}
	else
	{
		$query = "SELECT * FROM Character c(nolock) WHERE c.cid=" . $a_cid;
	}


  echo("<B>캐릭터정보</B><BR>\n");
    echo("<TABLE border=1 cellpadding=0 cellspacing=0>
<TR bgcolor=#EEEEEE>
<TD width=70 align=center>AID</TD>
<TD width=70 align=center>CID</TD>
<TD width=100 align=center>이름</TD>
<TD width=80 align=center>레벨</TD>
<TD width=50 align=center>성별</TD>
<TD width=50 align=center>CharNum</TD>
<TD width=50 align=center>XP</TD>
<TD width=50 align=center>BP</TD>
<TD width=50 align=center>RegDate</TD>
<TD width=50 align=center>LastTime</TD>
<TD width=50 align=center>PlayTime(분)</TD>
<TD width=50 align=center>킬수</TD>
<TD width=50 align=center>죽은수</TD>
</TR>
");

	$db->query($query);

  while ($row = $db->fetch())
  {
		$playtime_min = (int)($char_arr[$i]->PlayTime / 60);
    	$aid = $row->AID;
    	$cid = $row->CID;

    $aid_str = "<A href='AccountInfo.html?aid=" . $aid . "&mode=process'>$aid</A>";
    $cid_str = "<A href='CharacterInfo.html?cid=" . $cid . "&mode=process'>$cid</A>";

    echo("<TR>
<TD align=center>$aid_str</TD>
<TD align=center>$cid_str</TD>
<TD align=center><B>{$row->Name}</B></TD>
<TD align=center>{$row->Level}</TD>
<TD align=center>{$row->Sex}</TD>
<TD align=center>{$row->CharNum}</TD>
<TD align=center>{$row->XP}</TD>
<TD align=center>{$row->BP}</TD>
<TD align=center>{$row->RegDate}</TD>
<TD align=center>{$row->LastTime}</TD>
<TD align=center>$playtime_min</TD>
<TD align=center>{$row->KillCount}</TD>
<TD align=center>{$row->DeathCount}</TD>
</TR>\n");

  }


  echo ("</TABLE>\n");
}


# 장비된 아이템 목록을 보여준다
function view_character_equiped_item($equiped_item_arr)
{

  echo ("<BR>
  <TABLE border=1 width=300>
  <TR bgcolor=#EEEEEE>
  <TD colspan=2><B>장착한 아이템</B></TD>
  </TR>
  <TR><TD width=100>근접무기</TD><TD align=center>$equiped_item_arr->melee_Item&nbsp;</TD></TR>
  <TR><TD>주무기</TD><TD align=center>$equiped_item_arr->primary_Item&nbsp;</TD></TR>
  <TR><TD>보조무기</TD><TD align=center>$equiped_item_arr->secondary_Item&nbsp;</TD></TR>
  <TR><TD>아이템1</TD><TD align=center>$equiped_item_arr->custom1_Item&nbsp;</TD></TR>
  <TR><TD>아이템2</TD><TD align=center>$equiped_item_arr->custom2_Item&nbsp;</TD></TR>
  <TR><TD>머리</TD><TD align=center>$equiped_item_arr->head_Item&nbsp;</TD></TR>
  <TR><TD>가슴</TD><TD align=center>$equiped_item_arr->chest_Item&nbsp;</TD></TR>
  <TR><TD>팔</TD><TD align=center>$equiped_item_arr->hands_Item&nbsp;</TD></TR>
  <TR><TD>다리</TD><TD align=center>$equiped_item_arr->legs_Item&nbsp;</TD></TR>
  <TR><TD>발</TD><TD align=center>$equiped_item_arr->feet_Item&nbsp;</TD></TR>
  <TR><TD>왼쪽손가락</TD><TD align=center>$equiped_item_arr->fingerl_Item&nbsp;</TD></TR>
  <TR><TD>오른쪽손가락</TD><TD align=center>$equiped_item_arr->fingerr_Item&nbsp;</TD></TR>
  </TABLE>
  ");


}




## 캐쉬아이템 목록 보기
function view_accountitem_info($item_arr, $cnt, $delete_enable, $userid='')
{
  echo("<B>중앙은행</B><BR>\n");

  echo("<TABLE border=1>
<TR bgcolor=#EEEEEE>
<TD align=center>AIID</TD>
<TD align=center>Item</TD>
<TD align=center>잔여일</TD>
<TD width=70 align=center>명령</TD>
</TR>
");

  for($i = 0; $i < $cnt; $i++)
  {
    $aiid = $item_arr[$i]->AIID;
    echo("
<TR>
<TD align=center>$aiid</TD>
<TD align=center>{$item_arr[$i]->ItemName}</TD>
<TD align=center>{$item_arr[$i]->RentRemain}</TD>
<TD align=center>
");

    if ($delete_enable)
    {


      echo("
<FORM action='./AccountItemDelete.html' method=post>
<INPUT type=image src='/img/del.gif'></TD>
<INPUT type=hidden name=userid value='$userid'>
<INPUT type=hidden name=aiid value='$aiid'>
</FORM>
");

    }


    echo("&nbsp;
</TD>
</TR>
");
  } // for

  echo("</TABLE>\n");

}


## 캐릭터 아이템 목록 보기
function view_character_item_info($item_arr, $cnt)
{
  echo("<B>갖고 있는 아이템</B>(장착한 것 포함)<BR>\n");


  echo("<TABLE border=1>
  <TR bgcolor=#EEEEEE>
  <TD align=center width=40>No</TD>
  <TD align=center width=80>CIID</TD>
  <TD align=center width=200>Item</TD>
  <TD align=center width=150>등록일</TD>
  <TD align=center width=50>잔여일</TD>
  </TR>
");

  for ($i = 0; $i < $cnt; $i++)
  {
  	$no++;
  	$itemID = $item_arr[$i]->ItemID;
  	if (($itemID >= 400000) && ($itemID < 500000)) echo("<TR bgcolor=#EFDABB>\n");
  	else if ($itemID >= 500000) echo("<TR bgcolor=#DDDABB>\n");
  	else echo ("<TR>\n");

  	echo("
  <TD align=center>$i</TD>
  <TD align=center>{$item_arr[$i]->CIID}</TD>
  <TD align=center>{$item_arr[$i]->ItemName}({$item_arr[$i]->ItemID})</TD>
  <TD align=center>{$item_arr[$i]->RegDate}</TD>
  <TD align=center>{$item_arr[$i]->RentRemain}</TD>
  </TR>
  ");
  }


  echo("</TABLE>\n");

} // function


## 계정 정보 보기
function view_account_info($account_arr, $cnt)
{
  echo("<B>계정정보</B><BR>\n");
  echo("<TABLE border=1 cellpadding=0 cellspacing=0>
<TR bgcolor=#EEEEEE>
<TD width=70 align=center>AID</TD>
<TD width=100 align=center>넷마블ID</TD>
<TD width=80 align=center>이름</TD>
<TD width=50 align=center>Age</TD>
<TD width=50 align=center>Sex</TD>
<TD width=50 align=center>등급</TD>
<TD width=150 align=center>등록일</TD>
</TR>
");
  for ($i = 0; $i < $cnt; $i++)
  {
	  $ugrade_str = view_get_ugrade_str($account_arr[$i]->UGradeID);

    echo("<TR>
<TD align=center>{$account_arr[$i]->AID}</TD>
<TD align=center>{$account_arr[$i]->UserID}</TD>
<TD align=center>{$account_arr[$i]->Name}</TD>
<TD align=center>{$account_arr[$i]->Age}</TD>
<TD align=center>{$account_arr[$i]->Sex}</TD>
<TD align=center>$ugrade_str</TD>
<TD align=center>{$account_arr[$i]->RegDate}</TD>
</TR>
");
  } // for

  echo ("</TABLE>\n<BR><BR>\n");
} // function


## 계정 정보 보기
function view_db_account_info($userid)
{
  global $db;

  $result = db_query_account_info_from_userid($userid);
  $rownum = $db->getNumRow();
  if ($rownum == 0)
  {
    echo ("<BR>\n해당 계정은 존재하지 않습니다.<BR>\n");
    exit();
  }

  $cnt = 0;
  while ($row = $db->fetch())
  {
    if ($row->Sex == 0) $account_arr[$cnt]->Sex="남자";
    else $account_arr[$cnt]->Sex="여자";

    $account_arr[$cnt]->AID = $row->AID;
    $account_arr[$cnt]->UserID = $row->UserID;
    $account_arr[$cnt]->Name = $row->Name;
    $account_arr[$cnt]->Age = $row->Age;
    $account_arr[$cnt]->Sex = $row->Sex;
    $account_arr[$cnt]->UGradeID = $row->UGradeID;
    $account_arr[$cnt]->RegDate = $row->RegDate;

    $cnt++;
  } // while

  view_account_info($account_arr, $cnt);

}

## 아이템 정보 보기
function view_db_item_info_from_itemid($itemid)
{
  global $db;

  $query = "SELECT ItemID, Name FROM Item(nolock) WHERE ItemID=" . $itemid;
  $result = $db->query($query);

  $row = $db->fetch();

  if ($row)
  {
    echo("<B>아이템 정보</B><BR>\n");
    echo("<TABLE border=1 cellpadding=0 cellspacing=0>
<TR bgcolor=#EEEEEE>
<TD width=100 align=center>ItemID</TD>
<TD width=100 align=center>이름</TD>
</TR>
  ");

    echo("<TR>
<TD align=center>{$row->ItemID}</TD>
<TD align=center>{$row->Name}</TD>
</TR>
");

    echo ("</TABLE>\n<BR><BR>\n");
  }
}


## 상점 일반아이템 정보 보기
function view_db_cash_item_info($csid)
{
  global $db;

  $query = "SELECT cs.csid AS CSID, i.name AS ItemName, cs.CashPrice AS CashPrice FROM CashShop cs(nolock), Item i(nolock) WHERE cs.ItemID=i.ItemID AND " .
            "cs.CSID=" . $csid ;

  $result = $db->query($query);

  $row = $db->fetch();

  if ($row)
  {
    echo("<B>상점 일반아이템 정보</B><BR>\n");
    echo("<TABLE border=1 cellpadding=0 cellspacing=0>
<TR bgcolor=#EEEEEE>
<TD width=100 align=center>CSID</TD>
<TD width=200 align=center>이름</TD>
<TD width=100 align=center>캐쉬</TD>
</TR>
  ");

    echo("<TR>
<TD align=center>{$row->CSID}</TD>
<TD align=center>{$row->ItemName}</TD>
<TD align=center>{$row->CashPrice}</TD>
</TR>
");

    echo ("</TABLE>\n<BR><BR>\n");
  }
}


## 상점 세트아이템 정보 보기
function view_db_cash_setitem_info($cssid)
{
  global $db;

  $query = "SELECT CSSID, Name, CashPrice FROM CashSetShop css(nolock) WHERE CSSID=" . $cssid;

  $result = $db->query($query);

  $row = $db->fetch();

  if ($row)
  {
    echo("<B>상점 세트아이템 정보</B><BR>\n");
    echo("<TABLE border=1 cellpadding=0 cellspacing=0>
<TR bgcolor=#EEEEEE>
<TD width=100 align=center>CSSID</TD>
<TD width=200 align=center>이름</TD>
<TD width=100 align=center>캐쉬</TD>
</TR>
  ");

    echo("<TR>
<TD align=center>{$row->CSSID}</TD>
<TD align=center>{$row->Name}</TD>
<TD align=center>{$row->CashPrice}</TD>
</TR>
");

    echo ("</TABLE>\n<BR><BR>\n");
  }


}

## 아이템 정보 보기
function view_db_item_info($itemid)
{
  global $db;

  $query = "SELECT i.ItemID AS ItemID, i.name AS ItemName FROM Item i(nolock) WHERE i.ItemID=" . $itemid ;

  $result = $db->query($query);

  $row = $db->fetch();

  if ($row)
  {
    echo("<B>아이템 정보</B><BR>\n");
    echo("<TABLE border=1 cellpadding=0 cellspacing=0>
<TR bgcolor=#EEEEEE>
<TD width=100 align=center>ItemID</TD>
<TD width=200 align=center>이름</TD>
</TR>
  ");

    echo("<TR>
<TD align=center>{$row->ItemID}</TD>
<TD align=center>{$row->ItemName}</TD>
</TR>
");

    echo ("</TABLE>\n<BR><BR>\n");
  }
}


## 날자 폼 출력
function view_form_select_date($flag=0, $name_pre_header="", $default_year = 2005, $default_month=1, $default_day=1) /* year = 1, month=2, day = 4 */
{
	if (($flag & 1) || ($flag==0))
	{
		echo("<SELECT name=" . $name_pre_header . "year>\n");
		for ($i = 2004; $i < 2006; $i++)
		{
			$selected_str = "";
			if ($default_year == $i) $selected_str = "selected";
			echo("<OPTION value='" . $i . "' " . $selected_str . ">$i</OPTION>\n");
		}
		echo ("</SELECT>년\n");
	}

	if (($flag & 2) || ($flag==0))
	{
		echo("<SELECT name=" . $name_pre_header . "month>\n");
		for ($i = 1; $i <= 12; $i++)
		{
			$selected_str = "";
			if ($default_month == $i) $selected_str = "selected";

			$str = sprintf("<OPTION value='%d' %s>%02d</OPTION>\n", $i, $selected_str, $i);
			echo($str);
		}
		echo ("</SELECT>월\n");
	}

	if (($flag & 4) || ($flag==0))
	{
		echo("<SELECT name=" . $name_pre_header . "day>\n");
		for ($i = 1; $i <= 31; $i++)
		{
			$selected_str = "";
			if ($default_day == $i) $selected_str = "selected";

			$str = sprintf("<OPTION value='%d' %s>%02d</OPTION>\n", $i, $selected_str, $i);
			echo($str);
		}
		echo ("</SELECT>일\n");
	}
}


## account 정보 명령
function view_accountinfo_order($aid)
{
	if ($aid == 0) return;

	echo ("<BR><B>실행</B><BR>\n");
	echo ("<FORM action=./account_update_ugrade.php method=post>
<INPUT type=hidden name=aid value=$aid>
<INPUT type=submit value='유저등급변경'>
</FORM");


}


## character 정보 명령
function view_characterinfo_order($_cid, $_aid)
{
	if ((!$_aid) || (!$_cid)) return;

	echo ("<BR><B>실행</B><BR>\n");
	echo ("<TABLE border=0>
<TR>
<TD>\n");

	echo ("<FORM action=./account_update_ugrade.php method=post>
<INPUT type=hidden name=aid value=$_aid>
<INPUT type=submit value='유저등급변경'>
</FORM>\n");

	echo ("</TD><TD>\n
");

	echo ("<FORM action=./ChangeCharName.php method=post>
<INPUT type=hidden name=aid value=$_aid>
<INPUT type=hidden name=cid value=$_cid>
<INPUT type=submit value='캐릭터이름변경'>
</FORM>\n");

	echo ("</TD></TR></TABLE>\n");
}



## penaltylog 보기
function view_penaltylog_from_aid($_aid)
{
	global $db;
	$query = "SELECT id, AID, UGradeID, Date FROM PenaltyLog(nolock) WHERE AID=$_aid ORDER by id desc";
	$db->query($query);

	echo("<TABLE border=0 cellspacing=0 cellpadding=0>
<TR height=28><TD colspan=3><B>패널티 로그</TD></TR>
<TR height=28 bgcolor=#EEEEE style='padding:3 7 0 0'>
	<TD align=center width=60><B>No</B></TD>
	<TD align=center width=100><B>패널티 등급</B></TD>
	<TD align=center width=120><B>날짜</B></TD>
</TR>
");
	$cnt = $db->getNumRow();
	while ($row = $db->fetch())
  	{
	  	$ugrade_str = view_get_ugrade_str($row->UGradeID);

	  	echo("<TR bgcolor=#EEEEEE style='padding:3 7 0 0'>
<TD align=center>$cnt</TD>
<TD align=center>$ugrade_str</TD>
<TD align=center>$row->Date</TD>
</TR>
");
		$cnt--;

  	} // while

	echo("</TABLE>\n");


}


## 캐릭터 퀘스트 아이템 목록 보기
function view_character_questitem_info($questitem, $cid)
{
  require_once("questitem_list.php");
  
  ## 가지고 있는 퀘스트 아이템
  
    echo("<BR><BR><B>갖고있는 퀘스트 아이템</B><BR>
<TABLE border=1 cellpadding=0 cellspacing=0>
<TR bgcolor=#EEEEEE>
<TD width=160 align=center>퀘스트 아이템</TD>
<TD width=80 align=center>갯수</TD>
</TR>
");
  
  for ($i = 0; $i < 256; $i++)
  {
    $cnt = ord($questitem[$i+4]);
    if ($cnt > 0)
    {
      $cnt -= 1;
      $str_index = 200000 + $i + 1;
      echo ("<TR><TD align=center>$questitem_str[$str_index]</TD><TD align=center>$cnt</TD></TR>");
    }
  }
 echo ("</TABLE>\n<BR><BR>\n");

  
  ## 먹어본 유니크 아이템
	global $db;
	$query = "SELECT QIID FROM QUniqueItemLog(nolock) WHERE CID=$cid ORDER by QUILID desc";
	$db->query($query);
  
    echo("<TABLE border=1 cellpadding=0 cellspacing=0>
<TR bgcolor=#EEEEEE>
<TD width=200> <B>유니크 퀘스트 아이템 로그</B></TD>
</TR>
  ");

  while ($row = $db->fetch())
  {
      $qi_id = $row->QIID;

    echo("<TR>
<TD align=center>$questitem_str[$qi_id]</TD>
</TR>\n");

  }

    echo ("</TABLE>\n<BR><BR>\n");

  
} // function



?>
