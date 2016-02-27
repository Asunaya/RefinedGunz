-- 팀 테이블
CREATE TABLE "Team4"
(
	"TID4"			int not null,
	"Name"			varchar(24) null,
	"Region"		int,
	"MasterCID"		int,
	"Member1CID"	int,
	"Member2CID"	int,
	"Member3CID"	int,
	"Member4CID"	int,
	"Win"			int,
	"Lose"			int,
	"Draw"			int,
	"Point"			int,
	"Regdate"		datetime
)
go

ALTER TABLE "Team4"
	ADD CONSTRAINT "Team4_PK" primary key ("TID4")
go

ALTER TABLE "Team4"
	ADD CONSTRAINT "Team4_Character_FK1" foreign key ("MasterCID")
	REFERENCES "Character" ("CID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

ALTER TABLE "Team4"
	ADD CONSTRAINT "Team4_Character_FK2" foreign key ("Member1CID")
	REFERENCES "Character" ("CID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

ALTER TABLE "Team4"
	ADD CONSTRAINT "Team4_Character_FK3" foreign key ("Member2CID")
	REFERENCES "Character" ("CID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

ALTER TABLE "Team4"
	ADD CONSTRAINT "Team4_Character_FK4" foreign key ("Member3CID")
	REFERENCES "Character" ("CID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

ALTER TABLE "Team4"
	ADD CONSTRAINT "Team4_Character_FK5" foreign key ("Member4CID")
	REFERENCES "Character" ("CID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

CREATE INDEX "IX_Team4_Member1CID" ON "Team4" ("Member1CID")
Go
CREATE INDEX "IX_Team4_Member2CID" ON "Team4" ("Member2CID")
Go
CREATE INDEX "IX_Team4_Member3CID" ON "Team4" ("Member3CID")
Go
CREATE INDEX "IX_Team4_Member4CID" ON "Team4" ("Member4CID")
Go


-- 전적 로그
CREATE TABLE "Team4GameLog"
(
	"id"			int identity not null,
	"WinnerTeam"	int,
	"LoserTeam"		int,
	"DrawFlag"		tinyint,
	"RegDate"		datetime
)
go

ALTER TABLE "Team4GameLog"
	ADD CONSTRAINT "Team4GameLog_PK" primary key ("id")
go

ALTER TABLE "Team4GameLog"
	ADD CONSTRAINT "Team4GameLog_Team4_FK1" foreign key ("WinnerTeam")
	REFERENCES "Team4" ("TID4") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

ALTER TABLE "Team4GameLog"
	ADD CONSTRAINT "Team4GameLog_Team4_FK2" foreign key ("LoserTeam")
	REFERENCES "Team4" ("TID4") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

CREATE INDEX "IX_Team4GameLog_WinnerTeam" ON "Team4GameLog" ("WinnerTeam")
Go
CREATE INDEX "IX_Team4GameLog_LoserTeam" ON "Team4GameLog" ("LoserTeam")
Go
CREATE INDEX "IX_Team4GameLog_DrawFlag" ON "Team4GameLog" ("DrawFlag")
Go


-- 추가되는 SP --------------------------------------------------------------

-- Team4 ID 알아오기
CREATE PROC [spGetTeamID4]
	@Member1CID			int,
	@Member2CID			int,
	@Member3CID			int,
	@Member4CID			int
AS
	SELECT TID4 FROM Team4
	WHERE Member1CID=@Member1CID AND Member2CID=@Member2CID AND Member3CID = @Member3CID AND Member4CID = @Member4CID
go



-- Team4 결과
CREATE PROC [spTeam4WinTheGame]
	@WinnerTeamTID		int,
	@LoserTeamTID		int,
	@IsDrawGame			tinyint,
	@WinnerPoint		int,
	@LoserPoint			int,
	@DrawPoint			int
AS
	IF @IsDrawGame = 0
	BEGIN
		-- 이긴팀 Win+1
		UPDATE Team4 SET Win=Win+1, Point=Point+@WinnerPoint WHERE TID4=@WinnerTeamTID
		-- 진팀 Lose+1
		UPDATE Team4 SET Lose=Lose+1, Point=Point+(@LoserPoint) WHERE TID4=@LoserTeamTID
		UPDATE Team4 SET Point=0 WHERE TID4=@LoserTeamTID AND Point<0
	END
	ELSE
	BEGIN
		UPDATE Team4 SET Draw=Draw+1, Point=Point+@DrawPoint WHERE TID4=@WinnerTeamTID OR TID4=@LoserTeamTID
	END

	-- 전적 로그
	INSERT INTO Team4GameLog (WinnerTeam, LoserTeam, DrawFlag, RegDate)
	Values (@WinnerTeamTID, @LoserTeamTID, @IsDrawGame, GETDATE())
go



-- 팀멤버 얻어오기
CREATE PROC [spGetLadderTeamMemberByCID]
	@CID		int
AS

DECLARE @TID INT

SELECT @TID=TID4 FROM Team4(NOLOCK)
WHERE (Member1CID = @CID) OR (Member2CID = @CID) OR (Member3CID = @CID) OR (Member4CID = @CID)

SELECT t.TID4, c.Name AS Name FROM Team4 t(NOLOCK), Character c(NOLOCK)
WHERE (t.TID4=@TID) AND ( ( t.Member1CID=c.CID ) OR (t.Member2CID=c.CID) OR (t.Member3CID=c.CID) OR (t.Member4CID=c.CID) )
go









CREATE PROC [spLeague_FetchLeagueInfo]
AS

DECLARE curLeagueTeam INSENSITIVE CURSOR
FOR
	SELECT NO FROM League_info WHERE Game=2
FOR READ ONLY

OPEN curLeagueTeam

DECLARE @varNO 	int
DECLARE @varName	varchar(12)
DECLARE @varRegion	tinyint
DECLARE @sql varchar(100)


FETCH FROM curLeagueTeam INTO @varNO

WHILE @@FETCH_STATUS = 0
BEGIN
	IF (NOT EXISTS (SELECT * FROM Team4 WHERE TID4=@varNO))
	BEGIN
		SELECT @varName = TeamName, @varRegion=Area FROM League_info WHERE NO=@varNO

		INSERT Team4 (TID4, Name, Region, Win, Lose, Draw, Point, Regdate)
		VALUES (@varNO, @varName, @varRegion, 0, 0, 0, 0, GETDATE())
	END


	FETCH FROM curLeagueTeam INTO @varNO
END

CLOSE curLeagueTeam
DEALLOCATE curLeagueTeam


Go


CREATE PROC [spLeague_GetCID]
AS

SELECT li.NO, a.AID,
(SELECT TOP 1 CID FROM Character c(nolock) WHERE a.aid=c.aid AND c.DeleteFlag=0 ORDER BY c.XP desc) AS CID
FROM League_info li, League_Mbr lm, Account a(nolock)
WHERE li.NO=lm.NO AND lm.MemberID=a.UserID AND li.Game=2
ORDER BY li.NO
Go

CREATE PROC [spUpdateTeam4]
	@TID4			int,
	@Member1CID		int,
	@Member2CID		int,
	@Member3CID		int,
	@Member4CID		int
AS

UPDATE Team4
SET Member1CID=@Member1CID, Member2CID=@Member2CID, Member3CID=@Member3CID, Member4CID=@Member4CID
WHERE TID4=@TID4
Go










-- 수정될 SP --------------------------------------------------------------

/* 캐릭터 삭제 */
CREATE PROC [spDeleteChar]
	@AID		int,
	@CharNum	smallint,
	@CharName	varchar(24)
AS
DECLARE @CID		int

SELECT @CID=CID FROM Character WITH (nolock) WHERE AID=@AID and CharNum=@CharNum

IF (@CID IS NULL) OR
   (EXISTS (SELECT TOP 1 CLID FROM ClanMember WHERE CID=@CID)) OR
   (EXISTS (SELECT TOP 1 CIID FROM CharacterItem WHERE CID=@CID AND ItemID>=500000))
BEGIN
	SELECT 0 AS Ret
	return (-1)
END

UPDATE Character SET CharNum = -1, DeleteFlag = 1, Name='', DeleteName=@CharName
WHERE AID=@AID AND CharNum=@CharNum AND Name=@CharName

SELECT 1 AS Ret

GO
















