-- 테이블 수정
ALTER TABLE Clan ADD EmblemUrl varchar(48) null
ALTER TABLE Clan ADD EmblemChecksum int NOT NULL DEFAULT 0
ALTER TABLE Clan ADD LastDayRanking int NOT NULL DEFAULT 0
ALTER TABLE Clan ADD LastMonthRanking int NOT NULL DEFAULT 0



CREATE TABLE ClanHonorRanking
(
	id		int IDENTITY NOT NULL,
	CLID		int,
	ClanName	varchar(24) NOT NULL,
	Point		int,
	Wins		int,
	Losses		int,
	Ranking		int,
	Year		smallint,
	Month		tinyint,
	RankIncrease	int NOT NULL Default 0
)
ALTER TABLE ClanHonorRanking ADD
	CONSTRAINT PK_ClanHonorRanking_ID	PRIMARY KEY (id)
go

ALTER TABLE ClanHonorRanking
	ADD CONSTRAINT "ClanHonorRanking_CLID_FK1" foreign key ("CLID")
	REFERENCES "Clan" ("CLID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go


CREATE INDEX "IX_ClanHonorRanking_CLID" ON "ClanHonorRanking" ("CLID")
Go

CREATE INDEX "IX_ClanHonorRanking_YearMonthRanking" ON "ClanHonorRanking" ("Year", "Month", "Ranking")
Go

CREATE INDEX "IX_ClanHonorRanking_ClanName" ON "ClanHonorRanking" ("ClanName")
Go

CREATE INDEX "IX_ClanHonorRanking_Ranking" ON "ClanHonorRanking" ("Ranking")
Go


-- 저장프로시져 추가

-- 한달에 한번씩 명예의 전당 업데이트, 꼭 그 다음달의 1일에 해야한다.
CREATE PROC [spRegularUpdateHonorRanking]
AS

BEGIN TRAN -------------

EXEC [spRegularUpdateClanRanking]

DECLARE @Year		int
DECLARE @Month		int

SELECT @Year = YEAR(GETDATE())
SELECT @Month = MONTH(GETDATE())

IF (@Month = 1) 
BEGIN
	SELECT @Year = @Year-1
END

SELECT @Month = @Month - 1

IF (@Month = 0)
BEGIN
	SELECT @Month = 12
END

INSERT INTO ClanHonorRanking(CLID, ClanName, Point, Wins, Losses, Ranking, Year, Month)
SELECT CLID, Name AS ClanName, Point, Wins, Losses, Ranking, @Year, @Month
FROM Clan 
WHERE DeleteFlag=0 AND Ranking>0
ORDER BY Ranking

-- 클랜 리셋
UPDATE Clan SET Ranking=0, Wins=0, Losses=0, Point=1000, RankIncrease=0, LastDayRanking=0

COMMIT TRAN -----------

Go


