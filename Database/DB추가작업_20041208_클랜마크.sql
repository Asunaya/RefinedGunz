-- 클랜 정보 얻기
CREATE PROC [spGetClanInfo]
	@CLID			int
AS

SELECT cl.CLID AS CLID, cl.Name AS Name, cl.TotalPoint AS TotalPoint, cl.Level AS Level, cl.Ranking AS Ranking,
cl.Point AS Point, cl.Wins AS Wins, cl.Losses AS Losses, cl.Draws AS Draws,
c.Name AS ClanMaster,
(SELECT COUNT(*) FROM ClanMember WHERE CLID=@CLID) AS MemberCount,
cl.EmblemUrl AS EmblemUrl, cl.EmblemChecksum AS EmblemChecksum

FROM Clan cl(nolock), Character c(nolock)
WHERE cl.CLID=@CLID and cl.MasterCID=c.CID


GO
