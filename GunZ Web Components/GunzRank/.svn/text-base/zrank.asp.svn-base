<%
	' PROGRAMMING : JOONGPIL CHO
	Option Explicit
	
	' ---------------------
	'     VARIABLES
	' ---------------------
	
	Dim objDBConn		' DB Connection Handler
	Dim objRS			' DB Result
	Dim strSQL			' DB SQL Statement
	
	Dim nTotalCount		' Total Member Count
	Dim nTotalPage		' Total Page Count
	Dim nPage			' Current Page (retreive the page number from query string if it possible)
	Dim nStartNum		' Start Member Number
	Dim strKeyword		' Keyword For Search
	Dim nSortType		' 0 or null : XP, 1 : Kill
	Dim nSearchType		' 0 or null : Character Name, 1 : Player ID
	
	Dim sqlWhere		' String Buffer For Search Condition
		
	Dim nTemp
	Dim nLoop
	Dim nRank
	Dim nRate
%>
<!-- #include file="_head.asp" -->
<!-- #include file="config.asp" -->
<link rel="STYLESHEET" type="text/css" href="rank_style.css">
<%
	Set objDBConn = Server.CreateObject("ADODB.Connection")
	Set objRS = Server.CreateObject("ADODB.RecordSet")

	objDBConn.Open strDBHost, strDBUser, strDBPass
	
	nSearchType = Request("search")
	If nSearchType = "" Then
		nSearchType = 0
	End If
	
	strKeyword = Request("keyword")
	If strKeyword = "" Then
		sqlWhere = " WHERE (DeleteFlag = 0 AND XP != 0)"
	Else
		If nSearchType = 0 Then
			sqlWhere = " WHERE (DeleteFlag = 0 AND XP != 0 AND c.Name LIKE '%"&strKeyword&"%')"
		Else
			sqlWhere = " WHERE (DeleteFlag = 0 AND XP != 0 AND a.userid LIKE '%"&strKeyword&"%')"
		End If
	End If
	
	nSortType = Request("sort")
	If nSortType = "" Then
		nSortType = 0
	End If
	
	' --------------------
	' Calculate Pages
	' --------------------

	strSQL = "SELECT COUNT(*)"
	strSQL = strSQL & ", CEILING(CAST(COUNT(*) AS FLOAT)/" & nUsersPerPage & ")"
	strSQL = strSQL & " FROM Character c JOIN Account a On a.aid = c.aid" & sqlWhere
	
	objRS.Open strSQL, objDBConn
	
	nTotalCount = objRS(0)
	nTotalPage = objRS(1)
	
	nPage = Request("page")
	If nPage = "" Then
		nPage = 1
	End If

	nStartNum = (nPage-1)*nUsersPerPage
	
	If nTotalPage = 0 Then
		nStartNum = 1
		nUsersPerPage = 0
	End If
	
	objRS.Close
	nRank = nStartNum + 1
		
	' --------------------
	' Display Ranking
	' --------------------
	strSQL = "SELECT TOP " & nPage*nUsersPerPage & " c.Name c_name, a.userid a_userid, Level, c.Sex c_sex, XP, BP, KillCount, DeathCount FROM Character c JOIN Account a On a.aid = c.aid"
	strSQL = strSQL & sqlWhere
	
	' --------------------
	' SORT OPTION
	' --------------------
	
	Select Case nSortType
		Case 1
			strSQL = strSQL & " ORDER BY KillCount Desc"
		Case Else
			strSQL = strSQL & " ORDER BY XP Desc"
	End Select

	objRS.Open strSQL, objDBConn
%>
<table class="zrank" width=<%=strWidth%>>
<tr><td align=left><%=strPage%>:<%=nPage%>/<%=nTotalPage%></td><td align=right><a class="zrank" href="zrank.asp">전체보기</a></td></tr>
</table><br>
<table class="zrank_main" width=<%=strWidth%>>
	<tr align="center">
		<td align=center class="zrank_main_static"><%=strRank%></td>
		<td align=center class="zrank_main_static"><%=strChar%></td>
		<td align=center class="zrank_main_static"><a class="zrank_main_static" href="zrank.asp?sort=0"><%=strLevel%></a></td>
		<td align=center class="zrank_main_static"><a class="zrank_main_static" href="zrank.asp?sort=1"><%=strKillCount%></a></td>
	</tr>
<% If objRs.BOF or objRs.EOF Then %>
  <tr align="center">
    <td colspan="5">검색된 결과가 없습니다.</td>
  </tr>
<%
	Else
		objRS.Move nStartNum
		Do Until objRs.EOF
%>
	<tr align="center">
		<td width=20% align=center class="zrank_main"><%=nRank%></td>
		<td width=40% align=left class="zrank_main"><%=objRS("c_name")%> <font class="zrank_small">(<%=objRS("a_userid")%>)</font></td>
		<td width=20% align=center class="zrank_main"><%=objRS("Level")%> <font class="zrank_small">(XP:<%=objRS("XP")%>)</font></td>
		<td width=20% align=center class="zrank_main"><%=objRS("KillCount")%>
		<%
			If objRS("DeathCount") = 0 Then
				If objRS("KillCount") = 0 Then
					Response.Write(" <font class=zrank_small>(-)</font>")
				Else
					Response.Write(" <font class=zrank_small>(1)</font>")
				End If
			Else
				nRate = Int(objRS("KillCount")/objRS("DeathCount")*100)/100
				Response.Write(" <font class=zrank_small>("&nRate&")</font>")
			End If
			
		%>
		</td>
	</tr>
<%
        objRs.MoveNext
        nRank = nRank + 1
        Loop
	End If
%>
</table><br>
<table class="zrank" width=<%=strWidth%>>
<tr><td align="center">
<%
	If nPage - nBlockPage > 0 Then
		Response.Write "<a class=zrank href=zrank.asp?page=1&keyword="&strKeyword&"&sort="&nSortType&"&search="&nSearchType&">["&strFirst&"]</a>&nbsp;"
	End If

	nTemp = Int((nPage-1)/nBlockPage) * nBlockPage + 1
	If nTemp = 1 Then
		Response.Write "["&strPrev&"]&nbsp;"
	Else
		Response.Write "<a class=zrank href=zrank.asp?page=" & nTemp - nBlockPage & "&keyword="&strKeyword&"&sort="&nSortType&"&search="&nSearchType&">["&strPrev&"]</a>&nbsp;"
	End If
	
	nLoop = 1
	
	Do Until nLoop > nBlockPage Or nTemp > nTotalPage
		If nTemp = CInt(nPage) Then
			Response.Write "<b>[" & nTemp & "]</b>&nbsp;"
		Else
			Response.Write "<a class=zrank href=zrank.asp?page=" & nTemp & "&keyword="&strKeyword&"&sort="&nSortType&"&search="&nSearchType&">["&nTemp&"]</a>&nbsp;"
		End If
		nTemp = nTemp + 1
		nLoop = nLoop + 1
	Loop
	
	If nTemp > nTotalPage Then
		Response.Write "["&strNext&"]&nbsp;"
	Else
		Response.Write "<a class=zrank href=zrank.asp?page=" & nTemp & "&keyword="&strKeyword&"&sort="&nSortType&"&search="&nSearchType&">["&strNext&"]</a>&nbsp;"
	End If

	If nPage + nBlockPage < nTotalPage Then
		Response.Write "<a class=zrank href=zrank.asp?page=" & nTotalPage & "&keyword="&strKeyword&"&sort="&nSortType&"&search="&nSearchType&">["&strLast&"]</a>&nbsp;"
	End If
%>
</td></tr>
<tr>
<td align=center>
<!-- SEARCH -->
	<form method="post" action="zrank.asp">
	<input type="hidden" name="page" value="1">
	<input type="hidden" name="sort" value="<%=nSortType%>">

	<table cellpadding=0 cellspacing=0 border=0>
	<tr>
		<td>
			<select class="zrank" name="search" size="1">
				<option value="0"><%=strSearchName%></option>
				<option value="1"><%=strSearchID%></option>
			</select>&nbsp;&nbsp;
		</td>
		<td>
			<input type="text" name="keyword" size="12" class="zrank">&nbsp;&nbsp;</td>
		<td>
			<input type="submit" value="<%=strSearchButton%>">
		</td>
	</tr>
	</table>
	<!------------------검색 폼 끝--------------------->
    </form>
</td>
</tr>
</table>
<%
	' Delete All Object
	objRS.Close
	set objRS = nothing
	objDBConn.Close
	set objDBConn = nothing
%>
<!-- #include file="_foot.asp" -->