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
	INSERT INTO AccountItem (AID, ItemID) Values (@AID, @ItemID)
	COMMIT TRAN -----------
END
Go

