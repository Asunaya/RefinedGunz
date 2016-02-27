USE "GunzDB"
Go

/* 전체 랭킹 보기 */
CREATE PROC [spGetTotalRanking]
	@MinRank		int,
	@MaxRank		int
AS

SELECT Rank, Level, UserID, Name, XP, KillCount, DeathCount FROM TotalRanking(nolock)
WHERE Rank BETWEEN @MinRank AND @MaxRank
ORDER BY Rank

Go


/* 순위 검색 */
CREATE PROC [spSearchTotalRankingByName]
	@Name				varchar(24)
AS
-- 와일드카드 문자 처리
SELECT @Name = REPLACE(@Name, '[', '[[]')
SELECT @Name = REPLACE(@Name, '%', '[%]')
SELECT @Name = REPLACE(@Name, '_', '[_]')

SELECT TOP 30 Rank, Level, UserID, Name, XP, KillCount, DeathCount 
FROM TotalRanking(nolock)
WHERE Name LIKE @Name + '%'
ORDER BY rank asc


GO


/* 내 캐릭터 아이디 검색 */
CREATE PROC [spSearchTotalRankingByNetmarbleID]
	@UserID				varchar(20)
AS

SELECT Rank, Level, Name, XP, KillCount, DeathCount FROM TotalRanking(nolock)
WHERE UserID=@UserID

Go



/* 전체 랭킹을 산출한다. - 디비 에이전트에서 실행시키는 프로시져 */
CREATE PROC [spFetchTotalRanking]
AS

TRUNCATE TABLE TotalRanking

INSERT into TotalRanking(UserID, Name, Level, XP, KillCount, DeathCount)

SELECT Account.UserID, c.name, c.Level, c.XP, c.KillCount, c.DeathCount
FROM Character c(nolock), Account(nolock)
WHERE Account.AID=c.aid AND c.DeleteFlag=0 AND c.XP >= 500
ORDER BY c.xp DESC, c.KillCount DESC, c.DeathCount ASC, c.PlayTime DESC


Go


