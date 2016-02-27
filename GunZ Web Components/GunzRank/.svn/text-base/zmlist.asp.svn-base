<%
	Option Explicit
	
	' -----------------------
	' Programming by Venister
	'
	' YOU HAVE TO CHECK
	' _HEAD.PHP
	' _FOOT.PHP
	' CONFIG.ASP
	' IN YOUR LOCAL
	' -----------------------
	
	Dim objDBConn			' DB Connection Handler
	Dim objRS				' DB Result
	Dim strSQL				' DB SQL Statement
	
	Dim strOpenStat
	
	Dim valYear
	Dim valMonth
	Dim valDay
	
	Dim strQueryTime
	Dim strQueryTime2
	
	valYear = Request("y")
	If valYear = "" Then
		valYear = year(now)
	End If
	valMonth = Request("m")
	If valMonth = "" Then
		valMonth = month(now)
	End If
	valDay = Request("d")
	If valDay = "" Then
		valDay = day(now)
	End If
	
	strQueryTime = valYear & "." & valMonth & "." & valDay
	strQueryTime2 = valYear & "." & valMonth & "." & INT(valDay+1)
%>
<!-- #include file="_head.asp" -->
<!-- #include file="config.asp" -->
<link rel="STYLESHEET" type="text/css" href="rank_style.css">
<form method="post" action="zmlist.asp" ID="Form1">
Year : <input type=text name=y value=<%=valYear%> ID="Text1" size=4 maxlength=4 style='background-color:eeeeee; border:1 solid black;height:16'> &nbsp;&nbsp;
Month : <input type=text name=m value=<%=valMonth%> size=2 maxlength=2 style='background-color:eeeeee; border:1 solid black;height:16'> &nbsp;&nbsp;
Day : <input type=text name=d value=<%=valDay%> size=2 maxlength=2 style='background-color:eeeeee; border:1 solid black;height:16'> &nbsp;&nbsp;
<input type="submit" value="MOVE" ID="Submit1" NAME="Submit1">
</form>

<b><%=valYear%>년 <%=valMonth%>월 <%=valDay%>일 건즈 통계집계</b> - 총 누적가입자

<%
	Set objDBConn = Server.CreateObject("ADODB.Connection")
	Set objRS = Server.CreateObject("ADODB.RecordSet")

	objDBConn.Open strDBHost, strDBUser, strDBPass

	strSQL = "select count(*) from Account"
	objRS.Open strSQL, objDBConn
%>
<%=objRS(0)%>명, 
<%
	objRS.Close
	strSQL = "select count(*) from Account where RegDate='" & strQueryTime & "'"
	objRS.Open strSQL, objDBConn
%>
당일 신규가입자 : <%=objRS(0)%>명, 

<%
	objRS.Close
	strSQL = "SELECT COUNT(*) FROM Account WHERE (DATEDIFF(dd,RegDate,'"& strQueryTime &"') <= 7) AND (DATEDIFF(dd,RegDate,'"& strQueryTime &"') > 0)"
	objRS.Open strSQL, objDBConn
%>
금주 신규가입자 : <%=objRS(0)%>명, 

<%
	objRS.Close
	strSQL = "SELECT COUNT(distinct(aid)) FROM ConnLog WHERE (DateDiff(dd,Time, '"& strQueryTime &"')=0)"
	objRS.Open strSQL, objDBConn
%>
당일 유니크 이용자수 : <%=objRS(0)%>명,

<%
	objRS.Close
	strSQL = "SELECT TOP 1 sum(PlayerCount) AS PlayerCount, (DATEDIFF(mi, Time, '"& strQueryTime2 &"')) As MinDiff FROM ServerLog GROUP BY (DATEDIFF(mi, Time, '"& strQueryTime2 &"')) HAVING (DATEDIFF(mi, Time, '"& strQueryTime2 &"')) > 0 AND (DATEDIFF(mi, Time, '"& strQueryTime2 &"')) < (60*24) ORDER BY PlayerCount desc"
	objRS.Open strSQL, objDBConn
%>
당일 최고동접자수 : <%=objRS(0)%>명

<br><br>
<%
	objRS.Close
	strSQL = "select * from ServerStatus"
	objRS.Open strSQL, objDBConn	
%>

<hr size =1>
<b>서버현황</b><br>

<table class="zrank_main" width=100% border=1 cellpadding=2 cellspacing=0>
<tr>
<td align=center>번호</td>
<td align=center>서버이름</td>
<td align=center>아이피(포트)</td>
<td align=center>상태</td>
<td align=center>기록</td>
</tr>

<%
	If objRS.BOF or objRS.EOF Then
		Response.Write "<tr><td>검색된 결과가 없습니다.</td></tr>"
	Else
		Do Until objRS.EOF
%>
			<tr>
				<td align=center><%=objRS("ServerID")%></td>
				<td align=center><%=objRS("ServerName")%> (<%=objRS("CurrPlayer")%>/<%=objRS("MaxPlayer")%>)</td>
				<td align=center><%=objRS("IP")%>(<%=objRS("Port")%>)</td>
<%
If objRS("Time") = 1 Then
	strOpenStat = "Open"
Else
	strOpenStat = "Closed"
End If
%>
				<td align=center><%=strOpenStat%></td>
				<td align=center><%=objRS("Time")%></td>
			</tr>
<%
			objRS.MoveNext
		Loop
	End If

%>
</table>

<%
	' Delete All Object
	objRS.Close
	set objRS = nothing
	objDBConn.Close
	set objDBConn = nothing
%>
<!-- #include file="_foot.asp" -->