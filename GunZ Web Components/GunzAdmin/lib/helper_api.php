<?PHP

# 2차원배열에서 $dimension 항목으로 소트한다.
function dArray_sort($arr, $dimension)
{
  if ($dimension !== 0)
  {
    for ($i = 0; $i < sizeof($arr); $i++)
    {
      array_unshift($arr[$i], $arr[$i][$dimension]);
    }
    sort($arr);
    for ($i=0; $i < sizeof($arr); $i++)
    {
      array_shift($arr[$i]);
    }
  } else {
    sort($arr);
  }
  
  return $arr;
}


## 세션 체크 - 건즈관리사이트에서만 사용
function check_session()
{
	session_start();
	$sess_login = $_SESSION["Login"];
	if (!$sess_login || $sess_login=="")
	{
		header("Location: /login.html");
	}
}

?>
