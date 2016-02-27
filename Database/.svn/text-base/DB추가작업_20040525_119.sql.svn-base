INSERT INTO UserGrade(UGradeID, Name) VALUES (100, '전과자')
INSERT INTO UserGrade(UGradeID, Name) VALUES (101, '1차경고')
INSERT INTO UserGrade(UGradeID, Name) VALUES (102, '2차경고')
INSERT INTO UserGrade(UGradeID, Name) VALUES (103, '3차경고')
INSERT INTO UserGrade(UGradeID, Name) VALUES (104, '채팅금지')
INSERT INTO UserGrade(UGradeID, Name) VALUES (105, '기간정지')

-- 패널티 로그
CREATE TABLE "PenaltyLog"
(
	"id"		int identity not null,
	"AID"		int not null,
	"UGradeID"	int not null,
	"Date"		datetime not null
)
go

ALTER TABLE "PenaltyLog"
	ADD CONSTRAINT "PenaltyLog_PK" primary key ("id")
go

-- 패널티 기간
CREATE TABLE "AccountPenaltyPeriod"
(
	"id"		int identity not null,
	"AID"		int not null,
	"DayLeft"	int not null
)
go

ALTER TABLE "AccountPenaltyPeriod"
	ADD CONSTRAINT "AccountPenaltyPeriod_PK" primary key ("id")
go

ALTER TABLE "AccountPenaltyPeriod"
	ADD CONSTRAINT "AccountPenaltyPeriod_Account_FK1" foreign key ("AID")
	REFERENCES "Account" ("AID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go


--SP------------------------------------
-- 패널티 적용
CREATE PROC [spUpdateAccountUGrade]
	@AID		int
,	@UGrade		int
,	@Period		int
AS

UPDATE Account SET UGradeID=@UGrade WHERE AID=@AID


IF (@UGrade >= 100) AND (@UGrade<=253)
BEGIN
	INSERT INTO PenaltyLog(AID, UGradeID, Date) Values(@AID, @UGrade, GETDATE())
END

IF @UGrade = 104 OR @UGrade=105
BEGIN
	INSERT INTO AccountPenaltyPeriod(AID, DayLeft) VALUES(@AID, @Period)
END
ELSE
BEGIN
	-- 기간 패널티 해제
	DELETE FROM AccountPenaltyPeriod WHERE AID=@AID
END

Go

-- 정기적으로 해주는 패널티 업데이트
CREATE PROC [spRegularUpdateAccountPenaltyPeriod]
AS

UPDATE AccountPenaltyPeriod SET DayLeft=DayLeft-1

DECLARE curAccountPenaltyPeriod INSENSITIVE CURSOR
FOR
	SELECT AID FROM AccountPenaltyPeriod WHERE DayLeft <= 0
FOR READ ONLY

OPEN curAccountPenaltyPeriod

DECLARE @varAID int
DECLARE @sql varchar(100)

FETCH FROM curAccountPenaltyPeriod INTO @varAID

WHILE @@FETCH_STATUS = 0
BEGIN
	SELECT @sql = 'UPDATE Account SET UGradeID=100 WHERE AID=' + CONVERT(varchar(16), @varAID)
	EXEC(@sql)
	SELECT @sql = 'DELETE FROM AccountPenaltyPeriod WHERE AID=' + CONVERT(varchar(16), @varAID)
	EXEC(@sql)

	FETCH FROM curAccountPenaltyPeriod INTO @varAID
END

CLOSE curAccountPenaltyPeriod
DEALLOCATE curAccountPenaltyPeriod

Go







