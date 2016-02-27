<?PHP

# 캐릭터 장비 아이템 쿼리
function db_query_equiped_itemname_from_cid($cid)
{
  global $db;

  $query="SELECT " .
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.head_slot = ci.CIID AND ci.ItemID=i.ItemID) head_Item, ".
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.chest_slot = ci.CIID AND ci.ItemID=i.ItemID) chest_Item, ".
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.hands_slot = ci.CIID AND ci.ItemID=i.ItemID) hands_Item, ".
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.legs_slot = ci.CIID AND ci.ItemID=i.ItemID) legs_Item, ".
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.feet_slot = ci.CIID AND ci.ItemID=i.ItemID) feet_Item, ".
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.fingerl_slot = ci.CIID AND ci.ItemID=i.ItemID) fingerl_Item, ".
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.fingerr_slot = ci.CIID AND ci.ItemID=i.ItemID) fingerr_Item, ".
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.melee_slot = ci.CIID AND ci.ItemID=i.ItemID) melee_Item, ".
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.primary_slot = ci.CIID AND ci.ItemID=i.ItemID) primary_Item, ".
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.secondary_slot = ci.CIID AND ci.ItemID=i.ItemID) secondary_Item, ".
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.custom1_slot = ci.CIID AND ci.ItemID=i.ItemID) custom1_Item, ".
    "(SELECT i.Name FROM CharacterItem ci(nolock), Item i(nolock) WHERE c.custom2_slot = ci.CIID AND ci.ItemID=i.ItemID) custom2_Item ".
  "FROM Character c(nolock) ".
  "WHERE c.CID=$cid";

  $result = $db->query($query);


  return $result;
}


# 계정 정보 쿼리
function db_query_account_info_from_userid($userid)
{
  global $db;

  $query = "SELECT AID, UserID, Name, Age, Sex, UGradeID, RegDate, DayLeft 
			FROM Account(nolock), AccountPenaltyPeriod p(NOLOCK) 
			WHERE a.AID=p.AID AND userid='$userid'";
  $result = $db->query($query);



  return $result;
}

# 계정 정보 쿼리
function db_query_account_info_from_aid($aid)
{
  global $db;

  $query = "SELECT AID, UserID, Name, Age, Sex, UGradeID, RegDate FROM Account(nolock) where aid='$aid'";
  $result = $db->query($query);

  return $result;
}


function db_get_aid_from_userid($userid)
{
	global $db;

	$query="SELECT AID FROM Account(nolock) WHERE UserID='" . $userid . "'";

	$db->query($query);
	$row = $db->fetch();

	if ($row->AID)
	{
		return $row->AID;
	}
	else
	{
		return 0;
	}
}

## 매출정보 알아오기
function db_get_cash_info($start_date_str, $end_date_str)
{
	global $db;

	$query = "SELECT COUNT(*) AS SingleCount, SUM(Cash) AS SingleCash FROM ItemPurchaseLogByCash ipl(nolock) WHERE Date BETWEEN '" . $start_date_str . "' AND '" . $end_date_str . "'";

	$db->query($query);
	$row = $db->fetch();

	$single_cash = $row->SingleCash;
	$single_cnt = $row->SingleCount;


	$query = "SELECT COUNT(*) AS SetCount, SUM(Cash) AS SetCash FROm SetItemPurchaseLogByCash ipl(nolock) WHERE Date BETWEEN '" . $start_date_str . "' AND '" . $end_date_str . "'";
	$db->query($query);
	$row = $db->fetch();

	$set_cash = $row->SetCash;
	$set_cnt = $row->SetCount;

	$query = "SELECT COUNT(*) AS PresentCount, SUM(Cash) AS PresentCash FROm CashItemPresentLog(nolock) WHERE Date BETWEEN '" . $start_date_str . "' AND '" . $end_date_str . "'";
	$db->query($query);
	$row = $db->fetch();

	$present_cash = $row->PresentCash;
	$present_cnt = $row->PresentCount;

	$total_cash = $single_cash + $set_cash + $present_cash;

	$ret[SingleCash] = $single_cash;
	$ret[SetCash] = $set_cash;
	$ret[PresentCash] = $present_cash;
	$ret[SingleCnt] = $single_cnt;
	$ret[SetCnt] = $set_cnt;
	$ret[PresentCnt] = $present_cnt;
	$ret[TotalCash] = $total_cash;

	return $ret;
}


## 쿼리에서 필요한 날짜 스트링
function db_get_date_str($year, $month, $day)
{
	$the_day = mktime(0, 0, 0, $month, $day, $year);
	$str = date("Y.n.j", $the_day);
	return $str;
}

?>