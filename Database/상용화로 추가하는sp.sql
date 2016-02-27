/* 일반 아이템 구매가능인지 확인하기 */
CREATE PROC [spConfirmBuyCashItem]
	@UserID								varchar(20),
	@CSID									int,
	@RetEnableBuyItem			int OUTPUT,
	@RetRepeatBuySameItem	int OUTPUT
AS
SET NoCount On
	
DECLARE @AID		int
DECLARE @ItemID	int
DECLARE @AIID		int


SELECT @AID = AID FROM Account(nolock) where UserID = @UserID
SELECT @ItemID = ItemID FROM CashShop(nolock) WHERE CSID=@CSID

IF @AID IS NULL
BEGIN
	SELECT @RetEnableBuyItem = 0
	SELECT @RetRepeatBuySameItem = 0
END
ELSE
BEGIN
	SELECT @RetEnableBuyItem = 1


	IF (@ItemID IS NOT NULL)
	BEGIN
		SELECT TOP 1 @AIID = AIID FROM AccountItem(nolock) WHERE AID=@AID AND ItemID=@ItemID
		IF (@AIID IS NOT NULL)
		BEGIN
			SELECT @RetRepeatBuySameItem = 1
		END
		ELSE
		BEGIN
			SELECT @RetRepeatBuySameItem = 0
		END
	END
	ELSE
	BEGIN
		SELECT @RetRepeatBuySameItem = 0
	END


END

Go


/* 세트 아이템 구매가능인지 확인하기 */
CREATE PROC [spConfirmBuyCashSetItem]
	@UserID								varchar(20),
	@CSSID								int,
	@RetEnableBuyItem			int OUTPUT,
	@RetRepeatBuySameItem	int OUTPUT
AS
SET NoCount On
	
DECLARE @AID		int
DECLARE @SIL_ID	int
DECLARE @LAST_ID int

SELECT @AID = AID FROM Account(nolock) where UserID = @UserID


IF @AID IS NULL
BEGIN
	SELECT @RetEnableBuyItem = 0
	SELECT @RetRepeatBuySameItem = 0
END
ELSE
BEGIN
	SELECT @RetEnableBuyItem = 1

	SELECT TOP 1 @LAST_ID = id FROM SetItemPurchaseLogByCash spl(nolock) order by id desc

	SELECT TOP 1 @SIL_ID = id FROM SetItemPurchaseLogByCash spl(nolock) 
	WHERE id > (@LAST_ID-10000) AND AID=@AID AND CSSID=@CSSID

	IF (@SIL_ID IS NOT NULL)
	BEGIN
		SELECT @RetRepeatBuySameItem = 1
	END
	ELSE
	BEGIN
		SELECT @RetRepeatBuySameItem = 0
	END

END

Go






/* 아이템 구매 */
CREATE PROC [spBuyCashItem]
	@UserID		varchar(20)
,	@CSID		int
,	@Cash		int
AS
SET NoCount On

DECLARE @ItemID		int
DECLARE @AID		int

/* Account가 없으면 RETURN 0 */
SELECT @AID=AID FROM Account where UserID = @UserID

IF @AID IS NULL
BEGIN
	RETURN 0

END
ELSE
BEGIN
	SELECT @ItemID = cs.ItemID FROM CashShop cs
	WHERE @CSID = cs.csid

	if @ItemID IS NOT NULL
	BEGIN
		BEGIN TRAN
		/* 실제 아이템 생성 */
		INSERT INTO AccountItem (AID, ItemID)
		Values	(@AID, @ItemID)

		/* 로그 생성 */
		INSERT INTO ItemPurchaseLogByCash
		(ItemID, AID, Date, Cash)
		VALUES
		(@ItemID, @AID, GETDATE(), @Cash)

		COMMIT TRAN
		RETURN 1
	END
	ELSE
	BEGIN
		RETURN 0
	END
END

Go

--------------------------------------------------------------------------
/* 세트 아이템 구매 */
CREATE PROC [spBuyCashSetItem]
	@UserID		varchar(20)
,	@CSSID		int
,	@Cash		int
AS
SET NoCount On

DECLARE @ItemID		int
DECLARE @AID		int

/* Account가 없으면 RETURN 0 */
SELECT @AID=AID FROM Account where UserID = @UserID

IF @AID IS NULL
BEGIN
	RETURN 0
END
ELSE
BEGIN

	BEGIN TRAN --------------------------------------------

		DECLARE curBuyCashSetItem INSENSITIVE CURSOR
		FOR
			SELECT csid FROM CashSetItem WHERE CSSID = @CSSID
		FOR READ ONLY

		OPEN curBuyCashSetItem

		DECLARE @varCSID int
		DECLARE @sql varchar(100)

		FETCH FROM curBuyCashSetItem INTO @varCSID

		WHILE @@FETCH_STATUS = 0
		BEGIN
			SELECT @ItemID = cs.ItemID FROM CashShop cs	WHERE @varCSID = cs.csid

			IF @ItemID IS NOT NULL
			BEGIN
				/* 실제 아이템 생성 */
				INSERT INTO AccountItem (AID, ItemID) Values	(@AID, @ItemID)
			END
	
			FETCH FROM curBuyCashSetItem INTO @varCSID
		END

		CLOSE curBuyCashSetItem
		DEALLOCATE curBuyCashSetItem

		/* 로그 생성 */
		INSERT INTO SetItemPurchaseLogByCash (AID, CSSID, Date, Cash)
		VALUES (@AID, @CSSID, GETDATE(), @Cash)

	COMMIT TRAN ------------------------------------------

	RETURN 1
END

Go
--------------------------------------------------------------------------

/* 신규아이템 보기 */
CREATE PROC [spGetNewCashItem]
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
		i.Description AS Description
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND cs.NewItemOrder > 0 AND Opened=1 
	order by cs.NewItemOrder asc
Go


/* 신규아이템 랜덤으로 하나 보기 0부터 7까지 입력 
CREATE PROC [spGetNewRandomCashItem]
	@ItemIndex	int
AS
SET NoCount On

SELECT TOP 1 cs.CSID, i.Name, cs.WebImgName, i.Description FROM CashShop cs, Item i 
WHERE cs.ItemID=i.ItemID AND cs.NewItemOrder >= (@ItemIndex+1)

Go
*/










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
		i.Description AS Description, cs.RegDate As RegDate, cs.NewItemOrder AS IsNewItem
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
		i.Description AS Description, cs.RegDate As RegDate, cs.NewItemOrder AS IsNewItem
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
		i.Description AS Description, cs.RegDate As RegDate, cs.NewItemOrder AS IsNewItem
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
		i.Description AS Description, cs.RegDate As RegDate, cs.NewItemOrder AS IsNewItem
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

Go


/* 세트 아이템 목록 가져오기 */
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
	Description AS Description, RegDate AS RegDate, NewItemOrder AS IsNewItem
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

Go



/* 세트 아이템의 세부아이템 목록 보기 */
CREATE PROC [spGetCashSetItemComposition]
	@CSSID		int,
	@OutRowCount	int OUTPUT
AS

SELECT @OutRowCount = COUNT(*)
FROM CashSetItem csi, CashShop cs, Item i
WHERE @CSSID = csi.CSSID AND csi.csid = cs.csid	AND cs.ItemID = i.ItemID

SELECT cs.csid AS CSID, i.name AS Name, i.Slot AS Slot, 
	cs.CashPrice AS Cash, cs.WebImgName As WebImgName,
	i.ResSex AS ResSex, i.ResLevel AS ResLevel, i.Weight AS Weight,
	i.Description AS Description, cs.RegDate As RegDate,
	cs.NewItemOrder AS IsNewItem

FROM CashSetItem csi, CashShop cs, Item i
WHERE @CSSID = csi.CSSID AND csi.csid = cs.csid	AND cs.ItemID = i.ItemID

Go


/* 아이템의 상세 정보 보기 */
CREATE PROC [spGetCashItemInfo]
	@CSID		int
AS
	SELECT cs.csid AS CSID, i.name AS Name, i.Slot AS Slot, 
		cs.CashPrice AS Cash, cs.WebImgName As WebImgName,
		i.ResSex AS ResSex, i.ResLevel AS ResLevel, i.Weight AS Weight,
		i.Damage AS Damage, i.Delay AS Delay, i.Controllability AS Controllability,
		i.Magazine AS Magazine, i.MaxBullet AS MaxBullet, i.ReloadTime AS ReloadTime, 
		i.HP AS HP, i.AP AS AP,	i.MAXWT AS MaxWeight, i.LimitSpeed AS LimitSpeed,
		i.FR AS FR, i.CR AS CR, i.PR AS PR, i.LR AS LR,
		i.Description AS Description, cs.NewItemOrder AS IsNewItem
	FROM CashShop cs, Item i
	WHERE i.ItemID = cs.ItemID AND cs.csid = @CSID
Go


/* 세트아이템의 상세 정보 보기 */
CREATE PROC [spGetCashSetItemInfo]
	@CSSID	int
AS
	SELECT CSSID AS CSSID, Name AS Name, CashPrice AS Cash, WebImgName AS WebImgName, 
	ResSex AS ResSex, ResLevel AS ResLevel, Weight AS Weight,
	Description AS Description, NewItemOrder As IsNewItem

	FROM CashSetShop css
	WHERE CSSID = @CSSID
Go


/* 아이템 검색 */
CREATE PROC [spSearchCashItem]
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
		Description Text, IsNewItem tinyint)

SELECT @sql = 'SET ROWCOUNT ' + CONVERT(varchar(8), @Rows) + 
       ' SELECT cs.csid AS CSID, i.name AS Name, i.Slot AS Slot, cs.CashPrice AS Cash, ' +
       'cs.WebImgName AS WebImgName, i.ResSex AS ResSex, i.ResLevel AS ResLevel, i.Weight AS Weight, ' +
       'i.Description AS Description, cs.NewItemOrder AS IsNewItem FROM CashShop cs, Item i ' +
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
Go






-- 게임에서 필요한 저장프로시져 ----------------------------------------
CREATE PROC [spSelectAccountItem]
	@AID			int
AS

SELECT AIID, AID, ItemID FROM AccountItem
WHERE AID=@AID ORDER BY AIID
Go


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

SELECT @ItemID = ItemID FROM AccountItem WHERE AIID = @AIID

SELECT @CAID = AID FROM Character WHERE CID=@CID

IF @ItemID IS NOT NULL AND @CAID = @AID
BEGIN
	BEGIN TRAN ----------------
	DELETE FROM AccountItem WHERE AIID = @AIID
	INSERT INTO CharacterItem (CID, ItemID, RegDate)
	VALUES (@CID, @ItemID, GETDATE())

	INSERT INTO BringAccountItemLog	(ItemID, AID, CID, Date)
	VALUES (@ItemID, @AID, @CID, GETDATE())

	COMMIT TRAN ---------------

	SET @OrderCIID = @@IDENTITY

	SELECT @OrderCIID AS ORDERCIID, @ItemID AS ItemID
END
Go


-- 내 캐릭터 캐쉬아이템을 창고에 넣기 ---------
CREATE PROC [spBringBackAccountItem]
	@AID		int,
	@CID		int,
	@CIID		int
AS

DECLARE @ItemID int

DECLARE @HeadCIID 	int
DECLARE @ChestCIID	int
DECLARE @HandsCIID	int
DECLARE @LegsCIID	int
DECLARE @FeetCIID	int
DECLARE @FingerLCIID	int
DECLARE @FingerRCIID	int
DECLARE @MeleeCIID	int
DECLARE @PrimaryCIID	int
DECLARE @SecondaryCIID	int
DECLARE @Custom1CIID	int
DECLARE @Custom2CIID	int

SELECT 
@HeadCIID=head_slot, @ChestCIID=chest_slot, @HandsCIID=hands_slot, 
@LegsCIID=legs_slot, @FeetCIID=feet_slot, @FingerLCIID=fingerl_slot, @FingerRCIID=fingerr_slot, 
@MeleeCIID=melee_slot, @PrimaryCIID=primary_slot, @SecondaryCIID=secondary_slot, 
@Custom1CIID=custom1_slot, @Custom2CIID=custom2_slot
FROM Character(nolock) WHERE cid=@CID AND aid=@AID

SELECT @ItemID = ItemID FROM CharacterItem WHERE CIID=@CIID AND CID=@CID

IF ((@ItemID IS NOT NULL) AND 
   (@HeadCIID IS NULL OR @HeadCIID != @CIID) AND
   (@ChestCIID IS NULL OR @ChestCIID != @CIID) AND 
   (@HandsCIID IS NULL OR @HandsCIID != @CIID) AND
   (@LegsCIID IS NULL OR @LegsCIID != @CIID) AND 
   (@FeetCIID IS NULL OR @FeetCIID != @CIID) AND
   (@FingerLCIID IS NULL OR @FingerLCIID != @CIID) AND 
   (@FingerRCIID IS NULL OR @FingerRCIID != @CIID) AND
   (@MeleeCIID IS NULL OR @MeleeCIID != @CIID) AND 
   (@PrimaryCIID IS NULL OR @PrimaryCIID != @CIID) AND
   (@SecondaryCIID IS NULL OR @SecondaryCIID != @CIID) AND 
   (@Custom1CIID IS NULL OR @Custom1CIID != @CIID) AND
   (@Custom2CIID IS NULL OR @Custom2CIID != @CIID))
BEGIN
	BEGIN TRAN -------------
	UPDATE CharacterItem SET CID=NULL WHERE CIID=@CIID AND CID=@CID
	INSERT INTO AccountItem (AID, ItemID) Values (@AID, @ItemID)
	COMMIT TRAN -----------
END
Go

