-- UV테이블 생성 쿼리.
DROP TABLE UV
GO

CREATE TABLE UV(
	id	int		identity PRIMARY KEY
,	Time	smalldatetime 	NOT NULL
,	Count	int		NOT NULL )

CREATE NONCLUSTERED INDEX UVTimeIndex
ON UV( Time )


CREATE PROC [spRegularUpdateConnLog]
AS
-- 다음 날자의 ConnLog테이블 생성 쿼리.
DECLARE @NewTableName varchar(20)
DECLARE @CreateConnLogQuery varchar( 255 )
DECLARE @NextDate smalldatetime

DECLARE @Month int
DECLARE @Day int

EXEC spGetAgoDay -1, @NextDate OUTPUT

SET @Month = DATEPART(mm, @NextDate)
SET @Day = DATEPART(dd, @NextDate)

SELECT @NewTableName = 'ConnLog_' + 
	CAST( DATEPART(yy, @NextDate) AS varchar(4) ) + 
	CASE WHEN @Month < 10 THEN '0'  ELSE '' END + CAST(@Month AS varchar(4)) +
	CASE WHEN @Day < 10 THEN '0' ELSE '' END + CAST(@Day AS varchar(4))

SELECT @CreateConnLogQuery = 'CREATE TABLE ' + @NewTableName + '(id int identity PRIMARY KEY, AID int NOT NULL, Time smalldatetime NOT NULL, IP char(12) NOT NULL)'

EXEC (@CreateConnLogQuery )



-- 이전 날짜ConnLog의 UV처리 쿼리
DECLARE @TableName varchar(20) 
DECLARE @UVQuery varchar(512)
DECLARE @BefDate datetime


EXEC spGetAgoDay 1, @BefDate OUTPUT

SET @Month = DATEPART(mm, @BefDate)
SET @Day  = DATEPART(dd, @BefDate)

SELECT @TableName = 'ConnLog_' + 
	CAST( DATEPART(yy, @BefDate) AS varchar(4) ) + 
	CASE WHEN @Month < 10 THEN '0' ELSE '' END + CAST(@Month AS varchar(4)) +
	CASE WHEN @Day < 10 THEN '0' ELSE '' END + CAST(@Day AS varchar(4)) 

SELECT @UVQuery = '
DECLARE @Count int
DECLARE @Date smalldatetime

EXEC spGetAgoDay 1, @Date OUTPUT

SELECT @Count = COUNT(*) FROM (SELECT AID FROM ' + @TableName + ' (NOLOCK) GROUP BY AID) A

INSERT INTO UV(Time, Count) VALUES (@Date, @Count)'

EXEC (@UVQuery)

--EXEC ('TRUNCATE TABLE ' + @TableName)
--EXEC ('DROP TABLE ' + @TableName)

GO






ALTER PROC [spInsertConnLog] 
	@AID		int
,	@IP		varchar(20)
AS
	DECLARE @TableName varchar(20)
	DECLARE @SQLString nvarchar(80)
	DECLARE @ParmDefinition nvarchar(55)
	DECLARE @CurDate datetime
	DECLARE @S_CurDate smalldatetime

	DECLARE @Month int
	DECLARE @Day int

	SET @CurDate = GETDATE()

	SET @Month = DATEPART(mm, @CurDate)
	SET @Day = DATEPART(dd, @CurDate)

	SELECT @TableName = 'ConnLog_' + 
		CAST( DATEPART(yy, @CurDate) AS varchar(4) ) + 
		CASE WHEN @Month  < 10 THEN '0' ELSE '' END + CAST(@Month AS varchar(4)) +
		CASE WHEN @Day < 10 THEN '0' ELSE '' END + CAST(@Day AS varchar(4))

	SELECT @SQLString 	= N'INSERT INTO ' + @TableName + '(AID, Time, IP) VALUES(@S_AID, @S_Time, @S_IP)'
	SELECT @ParmDefinition	= '@S_AID int, @S_Time smalldatetime, @S_IP char(12)'

	SET @S_CurDate = CAST( @CurDate AS smalldatetime)

	EXEC sp_executesql @SQLString, @ParmDefinition, @S_AID = @AID, @S_Time = @S_CurDate, @S_IP = @IP
GO