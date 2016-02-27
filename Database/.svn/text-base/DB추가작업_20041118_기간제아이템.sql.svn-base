-- 테이블 변경 -------------------------------------
----------------------------------------------------
ALTER TABLE CharacterItem ADD RentDate DATETIME null DEFAULT NULL
ALTER TABLE CharacterItem ADD RentHourPeriod SMALLINT null DEFAULT NULL
ALTER TABLE CharacterItem ADD Cnt SMALLINT NULL DEFAULT NULL

ALTER TABLE AccountItem ADD RentDate DATETIME null DEFAULT NULL
ALTER TABLE AccountItem ADD RentHourPeriod SMALLINT null DEFAULT NULL
ALTER TABLE AccountItem ADD Cnt SMALLINT NULL DEFAULT NULL


-- 저장프로시져 추가 -------------------------------
----------------------------------------------------
-- 캐릭바꿀때 
CREATE PROC [spCheckExpireRentItem]
	@AID			int
AS

Go



















-- 저장프로시져 변경 -------------------------------
----------------------------------------------------
/* 캐릭터 아이템 보기 */
CREATE PROC [spSelectCharItem]
	@CID		int
AS
SELECT CIID, ItemID, RentHourPeriod - DateDiff(hh, RentDate, GETDATE()) AS RentPeriodRemainder, Cnt
FROM CharacterItem 
WHERE CID=@CID ORDER BY CIID

GO


CREATE PROC [spSelectAccountItem]
	@AID			int
AS

SELECT AIID, ItemID, RentHourPeriod - DateDiff(hh, RentDate, GETDATE()) AS RentPeriodRemainder, Cnt
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

	

	SELECT @OrderCIID AS ORDERCIID, @ItemID AS ItemID, @RentHourPeriod - DateDiff(hh, @RentDate, GETDATE()) AS RentPeriodRemainder
END

GO


-- 내 캐릭터 캐쉬아이템을 창고에 넣기 ---------
CREATE PROC [spBringBackAccountItem]
	@AID		int,
	@CID		int,
	@CIID		int
AS

DECLARE @ItemID int
DECLARE @RentDate		DATETIME
DECLARE @RentHourPeriod	SMALLINT
DECLARE @Cnt			SMALLINT

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

SELECT @ItemID=ItemID, @RentDate=RentDate, @RentHourPeriod=RentHourPeriod, @Cnt=Cnt
FROM CharacterItem WHERE CIID=@CIID AND CID=@CID

IF ((@ItemID IS NOT NULL) AND (@ItemID >= 400000) AND
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
	INSERT INTO AccountItem (AID, ItemID, RentDate, RentHourPeriod, Cnt) 
	VALUES (@AID, @ItemID, @RentDate, @RentHourPeriod, @Cnt)
	COMMIT TRAN -----------
END

GO




----------------------------------------------------
-- 캐릭터만 쓰는 거
spDeleteChar			-> 변경할것 없음
spDeleteCharItem		-> 변경할것 없음
spGetCharInfoExByCID	-> 변경할것 없음
spGetCharList			-> 변경할것 없음
spInsertChar			-> 변경할것 없음
spInsertCharItem		-> 현재는 변경할 것 없음. 나중에 상점에서 기간제들어갈때 변경해야함
spSelectCharItem		-> 변경할것 없음


-- 중앙은행만 쓰는거
spSelectAccountItem		-> 변경됨



-- 웹과 연동
spBuyCashItem				-> 변경할것없음 -> 나중에 넷마블작업시 변경해야함
spBuyCashSetItem			-> 변경할것없음 -> 나중에 넷마블작업시 변경해야함
spConfirmBuyCashItem		-> 변경할것없음 -> 나중에 넷마블작업시 변경해야함
spIsRepeatBuySameCashItem	-> 변경할것없음 -> 나중에 넷마블작업시 변경해야함
spPresentCashItem			-> 변경할것없음 -> 나중에 넷마블작업시 변경해야함
spPresentCashSetItem		-> 변경할것없음 -> 나중에 넷마블작업시 변경해야함


-- 같이 쓰는거
spBringAccountItem		-> 변경됨. cpp도 변경되야한다.
spBringbackAccountItem	-> 변경됨.






