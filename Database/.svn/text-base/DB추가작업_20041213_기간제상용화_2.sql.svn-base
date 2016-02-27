-- 변경되는 SP --
DROP PROC [spGetCashItemInfo]
DROP PROC [spGetCashItemList]
DROP PROC [spGetCashSetItemInfo]
DROP PROC [spGetCashSetItemList]
DROP PROC [spGetNewCashItem]
DROP PROC [spSearchCashItem]
DROP PROC [spSelectCharItem]
DROP PROC [spSelectAccountItem]
DROP PROC [spBringAccountItem]

/* 아이템의 상세 정보 보기 */
CREATE  PROC [spGetCashItemInfo]
	@CSID		int
AS
	SELECT cs.csid AS CSID, i.name AS Name, i.Slot AS Slot, 
		cs.CashPrice AS Cash, cs.WebImgName As WebImgName,
		i.ResSex AS ResSex, i.ResLevel AS ResLevel, i.Weight AS Weight,
		i.Damage AS Damage, i.Delay AS Delay, i.Controllability AS Controllability,
		i.Magazine AS Magazine, i.MaxBullet AS MaxBullet, i.ReloadTime AS ReloadTime, 
		i.HP AS HP, i.AP AS AP,	i.MAXWT AS MaxWeight, i.LimitSpeed AS LimitSpeed,
		i.FR AS FR, i.CR AS CR, i.PR AS PR, i.LR AS LR,
		i.Description AS Description, cs.NewItemOrder AS IsNewItem,
		cs.RentType AS RentType
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND cs.csid = @CSID
GO

/* 아이템 리스트 가져오기 */
CREATE PROC [spGetCashItemList]
	@ItemType	int,
	@Page		int,		
	@PageCount	int OUTPUT
AS
SET NoCount On

DECLARE @Rows int
DECLARE @ViewCount int

SELECT @Rows = @Page * 8	/* 한페이지에 8개씩 보여준다 */

IF @ItemType = 1 /* 근접무기 */
BEGIN
	SELECT @PageCount = (COUNT(*) + 7) / 8, @ViewCount = (COUNT(*)%8)
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND i.Slot = 1 AND cs.Opened=1

	SET ROWCOUNT @Rows

	SELECT cs.csid AS CSID, i.name AS Name, i.Slot AS Slot,
		cs.CashPrice AS Cash, cs.WebImgName As WebImgName,
		i.ResSex AS ResSex, i.ResLevel AS ResLevel, i.Weight AS Weight,
		i.Description AS Description, cs.RegDate As RegDate, cs.NewItemOrder AS IsNewItem,
		cs.RentType AS RentType
	INTO #tempcashitem1
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND i.Slot = 1 AND cs.Opened=1
	ORDER BY cs.csid DESC
	
	SET ROWCOUNT 0

	IF @Page=@PageCount AND @ViewCount > 0 AND @ViewCount < 8
	BEGIN
		SET ROWCOUNT @ViewCount
		SELECT * INTO #tempcashview1 FROM #tempcashitem1 a ORDER BY a.csid ASC
		SELECT * FROM #tempcashview1 a ORDER BY a.csid DESC
		DROP TABLE #tempcashview1
	END
	ELSE
	BEGIN
		SELECT *
		FROM (SELECT TOP 8 * FROM #tempcashitem1 a ORDER BY a.csid asc) a
		ORDER BY a.csid desc
	END

	DROP TABLE #tempcashitem1
END
ELSE
IF @ItemType=2 		/* 원거리무기 */
BEGIN
	SELECT @PageCount = (COUNT(*) + 7) / 8, @ViewCount = (COUNT(*)%8)
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND i.Slot = 2 AND cs.Opened=1


	SET ROWCOUNT @Rows

	SELECT cs.csid AS CSID, i.name AS Name, i.Slot AS Slot,
		cs.CashPrice AS Cash, cs.WebImgName As WebImgName,
		i.ResSex AS ResSex, i.ResLevel AS ResLevel, i.Weight AS Weight,
		i.Description AS Description, cs.RegDate As RegDate, cs.NewItemOrder AS IsNewItem,
		cs.RentType AS RentType
	INTO #tempcashitem2
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND i.Slot = 2 AND cs.Opened=1
	ORDER BY cs.csid DESC

	
	SET ROWCOUNT 0

	IF @Page=@PageCount AND @ViewCount > 0 AND @ViewCount < 8
	BEGIN
		SET ROWCOUNT @ViewCount
		SELECT * INTO #tempcashview2 FROM #tempcashitem2 a ORDER BY a.csid ASC
		SELECT * FROM #tempcashview2 a ORDER BY a.csid DESC
		DROP TABLE #tempcashview2
	END
	ELSE
	BEGIN
		SELECT *
		FROM (SELECT TOP 8 * FROM #tempcashitem2 a ORDER BY a.csid asc) a
		ORDER BY a.csid desc
	END


	DROP TABLE #tempcashitem2

END
ELSE
IF @ItemType=3 		/* 방어구 */
BEGIN
	SELECT @PageCount = (COUNT(*) + 7) / 8, @ViewCount = (COUNT(*)%8)
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND i.Slot BETWEEN 4 AND 8 AND cs.Opened=1

	SET ROWCOUNT @Rows

	SELECT cs.csid AS CSID, i.name AS Name, i.Slot AS Slot,
		cs.CashPrice AS Cash, cs.WebImgName As WebImgName,
		i.ResSex AS ResSex, i.ResLevel AS ResLevel, i.Weight AS Weight,
		i.Description AS Description, cs.RegDate As RegDate, cs.NewItemOrder AS IsNewItem,
		cs.RentType AS RentType
	INTO #tempcashitem3
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND i.Slot BETWEEN 4 AND 8 AND cs.Opened=1
	ORDER BY cs.csid DESC
	
	SET ROWCOUNT 0

	IF @Page=@PageCount AND @ViewCount > 0 AND @ViewCount < 8
	BEGIN
		SET ROWCOUNT @ViewCount
		SELECT * INTO #tempcashview3 FROM #tempcashitem3 a ORDER BY a.csid ASC
		SELECT * FROM #tempcashview3 a ORDER BY a.csid DESC
		DROP TABLE #tempcashview3
	END
	ELSE
	BEGIN
		SELECT *
		FROM (SELECT TOP 8 * FROM #tempcashitem3 a ORDER BY a.csid asc) a
		ORDER BY a.csid desc
	END


	DROP TABLE #tempcashitem3

END
ELSE
IF @ItemType=4 		/* 특수아이템 */
BEGIN
	SELECT @PageCount = (COUNT(*) + 7) / 8, @ViewCount = (COUNT(*)%8)
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND (i.Slot = 3 OR i.Slot=9) AND cs.Opened=1


	SET ROWCOUNT @Rows

	SELECT cs.csid AS CSID, i.name AS Name, i.Slot AS Slot,
		cs.CashPrice AS Cash, cs.WebImgName As WebImgName,
		i.ResSex AS ResSex, i.ResLevel AS ResLevel, i.Weight AS Weight,
		i.Description AS Description, cs.RegDate As RegDate, cs.NewItemOrder AS IsNewItem,
		cs.RentType AS RentType
	INTO #tempcashitem4
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND (i.Slot=3 OR i.Slot=9) AND cs.Opened=1
	ORDER BY cs.csid DESC

	SET ROWCOUNT 0

	IF @Page=@PageCount AND @ViewCount > 0 AND @ViewCount < 8
	BEGIN
		SET ROWCOUNT @ViewCount
		SELECT * INTO #tempcashview4 FROM #tempcashitem4 a ORDER BY a.csid ASC
		SELECT * FROM #tempcashview4 a ORDER BY a.csid DESC
		DROP TABLE #tempcashview4
	END
	ELSE
	BEGIN
		SELECT *
		FROM (SELECT TOP 8 * FROM #tempcashitem4 a ORDER BY a.csid asc) a
		ORDER BY a.csid desc
	END


	DROP TABLE #tempcashitem4
END

SET ROWCOUNT 0



GO





/* 세트아이템의 상세 정보 보기 */
CREATE  PROC [spGetCashSetItemInfo]
	@CSSID	int
AS
	SELECT CSSID AS CSSID, Name AS Name, CashPrice AS Cash, WebImgName AS WebImgName, 
	ResSex AS ResSex, ResLevel AS ResLevel, Weight AS Weight,
	Description AS Description, NewItemOrder As IsNewItem, RentType AS RentType

	FROM CashSetShop css
	WHERE CSSID = @CSSID
GO


/* 세트아이템 목록 보기 */
CREATE PROC [spGetCashSetItemList]
	@Page		int,
	@PageCount	int OUTPUT
AS
SET NoCount On
DECLARE @Rows int
DECLARE @ViewCount int

SELECT @Rows = @Page * 8	/* 한페이지에 8개씩 보여준다 */
SET ROWCOUNT @Rows

SELECT @PageCount = (COUNT(*) + 7) / 8, @ViewCount = (COUNT(*)%8) FROM CashSetShop css WHERE css.Opened=1

SELECT CSSID AS CSSID, Name AS Name, CashPrice AS Cash, WebImgName AS WebImgName, 
	ResSex AS ResSex, ResLevel AS ResLevel, Weight AS Weight,
	Description AS Description, RegDate AS RegDate, NewItemOrder AS IsNewItem,
	RentType AS RentType
INTO #tempcashsetitem
FROM CashSetShop css
WHERE css.Opened=1
ORDER BY css.cssid DESC

IF @Page=@PageCount AND @ViewCount > 0 AND @ViewCount < 8
BEGIN
	SET ROWCOUNT @ViewCount
	SELECT * INTO #tempcashview FROM #tempcashsetitem a ORDER BY a.cssid ASC
	SELECT * FROM #tempcashview a ORDER BY a.cssid DESC
	DROP TABLE #tempcashview
END
ELSE
BEGIN
	SELECT * 
	FROM (SELECT TOP 8 * FROM #tempcashsetitem a ORDER BY a.cssid asc) a
	ORDER BY a.cssid DESC
END


DROP TABLE #tempcashsetitem

SET ROWCOUNT 0



GO


/* 신규아이템 보기 */
CREATE  PROC [spGetNewCashItem]
	@ItemCount	int 	= 0
AS
SET NoCount On

IF @ItemCount != 0
BEGIN
	SET ROWCOUNT @ItemCount
END

	SELECT cs.csid AS CSID, i.name AS Name, i.Slot AS Slot, 
		cs.CashPrice AS Cash, cs.WebImgName As WebImgName,
		i.ResSex AS ResSex, i.ResLevel AS ResLevel, i.Weight AS Weight,
		i.Description AS Description, cs.RentType AS RentType
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND cs.NewItemOrder > 0 AND Opened=1 
	order by cs.NewItemOrder asc


GO




/* 아이템 검색 */
CREATE  PROC [spSearchCashItem]
	@Slot		tinyint,
	@ResSex		tinyint,
	@ResMinLevel	int = NULL,
	@ResMaxLevel	int = NULL,
	@ItemName	varchar(256) = '',
	@Page		int = 1,
	@PageCount	int OUTPUT
AS

SET NoCount On
DECLARE @Rows int
DECLARE @ViewCount int

SELECT @Rows = @Page * 8	/* 한페이지에 8개씩 보여준다 */
SET ROWCOUNT @Rows

DECLARE @slot_str varchar(64)
DECLARE @ressex_str varchar(64)
DECLARE @resminlevel_str varchar(64)
DECLARE @resmaxlevel_str varchar(64)
DECLARE @itemname_str varchar(256)
DECLARE @sql_where 			varchar(512)
DECLARE @sql						varchar(1024)


IF @Slot = 0
BEGIN
	SELECT @slot_str = ''
END
ELSE
BEGIN
	SELECT @slot_str = 'AND i.Slot = ' + CONVERT(varchar(16), @Slot) + ' '
END

IF @ResSex = 1 OR @ResSex = 2
BEGIN
	SELECT @ressex_str = 'AND (i.ResSex = ' + CONVERT(varchar(8), @ResSex) + ' OR i.ResSex = 3) '
END
ELSE
BEGIN
	SELECT @ressex_str = ''
END

IF @ResMinLevel IS NOT NULL
BEGIN
	SELECT @resminlevel_str = 'AND i.ResLevel >= ' + CONVERT(varchar(8), @ResMinLevel) + ' '
END
ELSE
BEGIN
	SELECT @resminlevel_str = ' '
END

IF @ResMaxLevel IS NOT NULL
BEGIN
	SELECT @resmaxlevel_str = 'AND i.ResLevel <= ' + CONVERT(varchar(8), @ResMaxLevel) + ' '
END
ELSE
BEGIN
	SELECT @resmaxlevel_str = ''
END

IF @ItemName = ''
BEGIN
	SELECT @itemname_str = ' '
END
ELSE
BEGIN
	SELECT @itemname_str = 'AND i.Name LIKE ''%' + @ItemName + '%'' '
END

SELECT @sql_where = 'WHERE i.ItemID = cs.ItemID ' + @slot_str + @ressex_str + @resminlevel_str +
						       @resmaxlevel_str + @itemname_str + ' AND cs.Opened=1 '

DECLARE @TotalRowCount int
SELECT @TotalRowCount = 0


SELECT @sql = 'SELECT COUNT(*) AS CNT FROM CashShop cs, Item i ' + @sql_where

CREATE TABLE #imsi (cnt int)
INSERT INTO #imsi EXEC(@sql)
SELECT @TotalRowCount = CNT FROM #imsi
DROP TABLE #imsi

SELECT @PageCount = (@TotalRowCount+7) / 8, @ViewCount = (@TotalRowCount%8)


CREATE TABLE #imsi2 (CSID int, Name varchar(256), Slot tinyint, Cash int, WebImgName varchar(64), 
		ResSex tinyint, ResLevel int, Weight int, 
		Description Text, IsNewItem tinyint, RentType tinyint)

SELECT @sql = 'SET ROWCOUNT ' + CONVERT(varchar(8), @Rows) + 
       ' SELECT cs.csid AS CSID, i.name AS Name, i.Slot AS Slot, cs.CashPrice AS Cash, ' +
       'cs.WebImgName AS WebImgName, i.ResSex AS ResSex, i.ResLevel AS ResLevel, i.Weight AS Weight, ' +
       'i.Description AS Description, cs.NewItemOrder AS IsNewItem, cs.RentType AS RentType FROM CashShop cs, Item i ' +
			 @sql_where + ' ORDER BY cs.csid DESC'

INSERT INTO #imsi2 EXEC(@sql)


SET ROWCOUNT 0

IF @Page=@PageCount AND @ViewCount > 0 AND @ViewCount < 8
BEGIN
	SET ROWCOUNT @ViewCount
	SELECT * INTO #tempcashsearchview FROM #imsi2 a ORDER BY a.csid ASC
	SELECT * FROM #tempcashsearchview a ORDER BY a.csid DESC
	DROP TABLE #tempcashsearchview
END
ELSE
BEGIN
	SELECT *
	FROM (SELECT TOP 8 * FROM #imsi2 a ORDER BY a.csid asc) a
	ORDER BY a.csid desc
END


DROP TABLE #imsi2


GO




/* 캐릭터 아이템 보기 */
CREATE PROC [spSelectCharItem]
	@CID		int
AS
SELECT CIID, ItemID, (RentHourPeriod*60) - (DateDiff(n, RentDate, GETDATE())) AS RentPeriodRemainder, Cnt
FROM CharacterItem 
WHERE CID=@CID ORDER BY CIID

GO


/* 중앙은행 아이템 보기 */
CREATE PROC [spSelectAccountItem]
	@AID			int
AS

SELECT AIID, ItemID, (RentHourPeriod*60) - (DateDiff(n, RentDate, GETDATE())) AS RentPeriodRemainder, Cnt
FROM AccountItem
WHERE AID=@AID ORDER BY AIID

GO



-- 창고 아이템 내 캐릭터로 가져오기 ----------
CREATE PROC [spBringAccountItem]
	@AID		int,
	@CID		int,
	@AIID		int
AS
SET NoCount On

DECLARE @ItemID int
DECLARE @CAID int
DECLARE @OrderCIID int

DECLARE @RentDate			DATETIME
DECLARE @RentHourPeriod		SMALLINT
DECLARE @Cnt				SMALLINT

SELECT @ItemID=ItemID, @RentDate=RentDate, @RentHourPeriod=RentHourPeriod, @Cnt=Cnt
FROM AccountItem WHERE AIID = @AIID


SELECT @CAID = AID FROM Character WHERE CID=@CID

IF @ItemID IS NOT NULL AND @CAID = @AID
BEGIN
	BEGIN TRAN ----------------
	DELETE FROM AccountItem WHERE AIID = @AIID
	INSERT INTO CharacterItem (CID, ItemID, RegDate, RentDate, RentHourPeriod, Cnt)
	VALUES (@CID, @ItemID, GETDATE(), @RentDate, @RentHourPeriod, @Cnt)

	SET @OrderCIID = @@IDENTITY

	INSERT INTO BringAccountItemLog	(ItemID, AID, CID, Date)
	VALUES (@ItemID, @AID, @CID, GETDATE())

	COMMIT TRAN ---------------

	

	SELECT @OrderCIID AS ORDERCIID, @ItemID AS ItemID, (@RentHourPeriod*60) - (DateDiff(n, @RentDate, GETDATE())) AS RentPeriodRemainder
END

GO






-- 추가되는 SP --
-----------------------------------------------------------------

/* 중앙은행의 기간만료 아이템 삭제 */
CREATE PROC [spDeleteExpiredAccountItem]
	@AIID		int
AS

DELETE FROM AccountItem WHERE AIID=@AIID AND RentDate IS NOT NULL

Go