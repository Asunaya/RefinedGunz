-- 테이블 수정 ------------------------------------
----------------------------------------------------
ALTER TABLE Clan ADD Losses int not null DEFAULT 0
ALTER TABLE Clan ADD Draws int not null DEFAULT 0
ALTER TABLE Clan ADD Ranking int not null DEFAULT 0
ALTER TABLE Clan ADD TotalPoint int not null DEFAULT 0

ALTER TABLE Clan ADD RankIncrease int not null DEFAULT 0

ALTER TABLE ClanMember ADD ContPoint int not null DEFAULT 0


CREATE INDEX "IX_Clan_Ranking" ON "Clan" ("Ranking")

-- 클랜전 로그 테이블 추가 ------------------------
----------------------------------------------------
-- 클랜전 로그 테이블
CREATE TABLE "ClanGameLog"
(
	"id"			int identity not null,
	"WinnerCLID"		int not null,
	"LoserCLID"		int not null,
	"WinnerClanName"	varchar(24),
	"LoserClanName"		varchar(24),
	"WinnerMembers"		varchar(110),
	"LoserMembers"		varchar(110),
	"RoundWins"		tinyint not null,
	"RoundLosses"		tinyint not null,
	"MapID"			tinyint not null,
	"GameType"		tinyint not null,
	"RegDate"		datetime not null
)
go

ALTER TABLE "ClanGameLog"
	ADD CONSTRAINT "ClanGameLog_PK" primary key ("id")
go

ALTER TABLE "ClanGameLog"
	ADD CONSTRAINT "ClanGameLog_WinnerCLID_FK1" foreign key ("WinnerCLID")
	REFERENCES "Clan" ("CLID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

ALTER TABLE "ClanGameLog"
	ADD CONSTRAINT "ClanGameLog_LoserCLID_FK1" foreign key ("LoserCLID")
	REFERENCES "Clan" ("CLID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

CREATE INDEX "IX_ClanGameLog_WinnerCLID" ON "ClanGameLog" ("WinnerCLID")
Go
CREATE INDEX "IX_ClanGameLog_LoserCLID" ON "ClanGameLog" ("LoserCLID")
Go


-- 저장프로시져 변경 -------------------------------
----------------------------------------------------

-- 클랜 정보 얻기
CREATE PROC [spGetClanInfo]
	@CLID			int
AS

SELECT cl.CLID AS CLID, cl.Name AS Name, cl.TotalPoint AS TotalPoint, cl.Level AS Level, cl.Ranking AS Ranking,
cl.Point AS Point, cl.Wins AS Wins, cl.Losses AS Losses, cl.Draws AS Draws,
c.Name AS ClanMaster,
(SELECT COUNT(*) FROM ClanMember WHERE CLID=@CLID) AS MemberCount

FROM Clan cl(nolock), Character c(nolock)
WHERE cl.CLID=@CLID and cl.MasterCID=c.CID

GO


-- 캐릭터 정보 가져오기
CREATE PROC [spGetCharInfoByCharNum]
	@AID		int
,	@CharNum	smallint
AS

DECLARE @CID		int
DECLARE @CLID		int
DECLARE @ClanName	varchar(24)
DECLARE @ClanGrade	int
DECLARE @ClanContPoint	int

SELECT @CID=CID FROM Character WITH (nolock) WHERE AID=@AID and CharNum=@CharNum
SELECT @CLID=cl.CLID, @ClanName=cl.Name, @ClanGrade=cm.Grade, @ClanContPoint=cm.ContPoint FROM ClanMember cm(nolock), Clan cl(nolock) WHERE cm.cid=@CID AND cm.CLID=cl.CLID

SELECT *, @CLID AS CLID, @ClanName AS ClanName, @ClanGrade AS ClanGrade, @ClanContPoint AS ClanContPoint FROM Character WITH (nolock) where cid=@CID

GO


-- 클랜 생성하기 수정
-- 클랜 생성하기
CREATE PROC [spCreateClan]
	@ClanName		varchar(24),
	@MasterCID		int,
	@Member1CID		int,
	@Member2CID		int,
	@Member3CID		int,
	@Member4CID		int
AS
	DECLARE @NewCLID	int

	-- 클랜이름이 중복인지 검사해야한다.
	SELECT @NewCLID=CLID FROM Clan WHERE Name=@ClanName

	IF @NewCLID IS NOT NULL
	BEGIN
		SELECT 0 AS Ret, 0 AS NewCLID
		RETURN
	END


	DECLARE @CNT		int

	-- 클랜원이 모두 가입 가능한지 검사해야한다.
	SELECT @CNT = COUNT(*) FROM ClanMember cm, Character c WHERE ((cm.CID=@MasterCID) OR (cm.CID=@Member1CID) OR (cm.CID=@Member2CID) OR (cm.CID=@Member3CID) OR
(cm.CID=@Member4CID) ) AND cm.CID=c.CID AND c.DeleteFlag=0

	IF @CNT != 0
	BEGIN
		SELECT 0 AS Ret, 0 AS NewCLID
		RETURN
	END


	-- 클랜 생성
	INSERT INTO Clan (Name, MasterCID, Point, RegDate) VALUES (@ClanName, @MasterCID, 1000, GETDATE())

	SELECT @NewCLID = @@IDENTITY
	IF (@NewCLID IS not NULL)
	BEGIN
		-- 클랜원 가입
		INSERT INTO ClanMember (CLID, CID, Grade, RegDate) VALUES (@NewCLID, @MasterCID, 1, GETDATE())
		INSERT INTO ClanMember (CLID, CID, Grade, RegDate) VALUES (@NewCLID, @Member1CID, 9, GETDATE())
		INSERT INTO ClanMember (CLID, CID, Grade, RegDate) VALUES (@NewCLID, @Member2CID, 9, GETDATE())
		INSERT INTO ClanMember (CLID, CID, Grade, RegDate) VALUES (@NewCLID, @Member3CID, 9, GETDATE())
		INSERT INTO ClanMember (CLID, CID, Grade, RegDate) VALUES (@NewCLID, @Member4CID, 9, GETDATE())
	END

	-- 마스터 바운티 삭제
	--UPDATE Character SET BP=BP-1000 WHERE CID=@MasterCID


	SELECT 1 AS Ret, @NewCLID AS NewCLID

GO


-- 저장프로시져 추가 -------------------------------
----------------------------------------------------

-- 클랜전 결과 업데이트
CREATE PROC [spWinTheClanGame]
	@WinnerCLID		int,
	@LoserCLID		int,
	@IsDrawGame		tinyint,
	@WinnerPoint		int,
	@LoserPoint		int,
	@WinnerClanName		varchar(24),
	@LoserClanName		varchar(24),
	@RoundWins		tinyint,
	@RoundLosses		tinyint,
	@MapID			tinyint,
	@GameType		tinyint,
	@WinnerMembers		varchar(110),
	@LoserMembers		varchar(110)
AS
	IF @IsDrawGame = 0
	BEGIN
		-- 이긴팀 Wins+1
		UPDATE Clan SET Wins=Wins+1, Point=Point+@WinnerPoint, TotalPoint=TotalPoint+@WinnerPoint WHERE CLID=@WinnerCLID
		-- 진팀 Losses+1
		UPDATE Clan SET Losses=Losses+1, Point=Point+(@LoserPoint) WHERE CLID=@LoserCLID
		UPDATE Clan SET Point=0 WHERE CLID=@LoserCLID AND Point<0

		-- 전적 로그를 남긴다.
		INSERT INTO ClanGameLog(WinnerCLID, LoserCLID, WinnerClanName, LoserClanName, RoundWins, RoundLosses, MapID, GameType, RegDate, WinnerMembers, LoserMembers)
		VALUES (@WinnerCLID, @LoserCLID, @WinnerClanName, @LoserClanName, @RoundWins, @RoundLosses, @MapID, @GameType, GETDATE(), @WinnerMembers, @LoserMembers)
	END
	ELSE
	BEGIN
		UPDATE Clan SET Draws=Draws+1 WHERE CLID=@WinnerCLID OR CLID=@LoserCLID
	END

Go

-- 클랜기여도 업데이트
CREATE PROC [spUpdateCharClanContPoint]
	@CID		int,
	@CLID		int,
	@AddedContPoint	int
AS
	Update ClanMember SET ContPoint=ContPoint+@AddedContPoint WHERE CID=@CID AND CLID=@CLID
Go




-- 일정시간마다 클랜 랭킹 업데이트 - 2시간에 한번씩으로 생각중
CREATE PROC [spRegularUpdateClanRanking]
AS

DECLARE @varRanking int
SELECT @varRanking = 0

DECLARE curRankClan INSENSITIVE CURSOR
FOR
	SELECT CLID
	FROM Clan(nolock)
	WHERE DeleteFlag=0 AND ((Wins != 0) OR (Losses != 0)) 
	ORDER BY Point Desc, Wins Desc, Losses Asc

FOR READ ONLY

OPEN curRankClan

DECLARE @varCLID int
DECLARE @sql varchar(100)

FETCH FROM curRankClan INTO @varCLID

WHILE @@FETCH_STATUS = 0
BEGIN
	SELECT @varRanking = @varRanking + 1

	-- 랭킹 업데이트
	UPDATE Clan SET Ranking = @varRanking WHERE CLID=@varCLID


	FETCH FROM curRankClan INTO @varCLID
END

CLOSE curRankClan
DEALLOCATE curRankClan

Go


-- 클랜 랭킹 증가분 업데이트 - 매일 새벽 12시 5분에 업데이트
CREATE PROC [spRegularUpdateClanRankIncrease]
AS

	-- 꼴등랭킹을 구한다.
	DECLARE @LowestRank int
	SELECT TOP 1 @LowestRank=Ranking FROM Clan 
	WHERE DeleteFlag=0 AND Ranking>0 
	order by ranking desc

	IF @LowestRank is NULL SELECT @LowestRank = 0

	UPDATE Clan
	SET RankIncrease=(LastDayRanking-Ranking)
	WHERE DeleteFlag=0 AND Ranking>0 AND LastDayRanking != 0

	-- 처음 랭킹에 진입했을 경우
	UPDATE Clan
	SET RankIncrease=@LowestRank-Ranking
	WHERE DeleteFlag=0 AND Ranking>0 AND LastDayRanking = 0

	-- LastDayRanking 업데이트
	UPDATE Clan 
	SET LastDayRanking=Ranking 
	where DeleteFlag=0 and Ranking>0
Go




----------------------------------------------------------------
-- 넷마블에서 사용하는 랭크 뷰
CREATE VIEW viewRankGunzClan
AS

SELECT CLID, Name as ClanName, Point, Wins, Losses, EmblemUrl, Ranking, RankIncrease
FROM Clan(nolock)
WHERE DeleteFlag=0 and Ranking>0


Go


-- 매달 1일에 한번 명예의전당 업데이트
CREATE VIEW viewHonorRankGunzClan
AS

SELECT chr.CLID, chr.ClanName, chr.Point, chr.Wins, chr.Losses, c.EmblemUrl, chr.Ranking, chr.RankIncrease, chr.Year, chr.Month
FROM ClanHonorRanking chr(nolock), Clan c(nolock)
WHERE chr.CLID=c.CLID

Go