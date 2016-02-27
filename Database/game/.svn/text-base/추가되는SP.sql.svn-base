USE "GunzDB"
Go

CREATE PROC [spUpdateEquipItem]
	@CID							int,
	@ItemParts				int,
	@CIID							int
AS

SET NoCount ON

DECLARE @Ret int
DECLARE @IF_CIID	int

SELECT @Ret = 1

-- Head
IF @ItemParts = 0
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET head_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		UPDATE Character SET head_slot=@CIID WHERE CID=@CID
	END
END
-- Chest
ELSE IF @ItemParts = 1
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET chest_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		UPDATE Character SET chest_slot=@CIID WHERE CID=@CID
	END
END
-- Hands
ELSE IF @ItemParts = 2
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET hands_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		UPDATE Character SET hands_slot=@CIID WHERE CID=@CID
	END
END
-- Legs
ELSE IF @ItemParts = 3
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET legs_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		UPDATE Character SET legs_slot=@CIID WHERE CID=@CID
	END
END
-- Feet
ELSE IF @ItemParts = 4
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET feet_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		UPDATE Character SET feet_slot=@CIID WHERE CID=@CID
	END
END
-- FingerL
ELSE IF @ItemParts = 5
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET fingerl_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		SELECT @IF_CIID = fingerr_slot FROM Character WHERE CID=@CID
		IF (@IF_CIID IS NOT NULL) AND (@IF_CIID = @CIID)
		BEGIN
			SELECT @Ret = 0
		END
		ELSE
		BEGIN
			UPDATE Character SET fingerl_slot=@CIID WHERE CID=@CID
		END
	END
END
-- FingerR
ELSE IF @ItemParts = 6
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET fingerr_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		SELECT @IF_CIID = fingerl_slot FROM Character WHERE CID=@CID
		IF (@IF_CIID IS NOT NULL) AND (@IF_CIID = @CIID)
		BEGIN
			SELECT @Ret = 0
		END
		ELSE
		BEGIN
			UPDATE Character SET fingerr_slot=@CIID WHERE CID=@CID
		END
	END
END
-- Melee
ELSE IF @ItemParts = 7
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET melee_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		UPDATE Character SET melee_slot=@CIID WHERE CID=@CID
	END
END
-- Primary
ELSE IF @ItemParts = 8
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET primary_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		SELECT @IF_CIID = secondary_slot FROM Character WHERE CID=@CID
		IF (@IF_CIID IS NOT NULL) AND (@IF_CIID = @CIID)
		BEGIN
			SELECT @Ret = 0
		END
		ELSE
		BEGIN
			UPDATE Character SET primary_slot=@CIID WHERE CID=@CID
		END
	END
END
-- Secondary
ELSE IF @ItemParts = 9
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET secondary_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		SELECT @IF_CIID = primary_slot FROM Character WHERE CID=@CID
		IF (@IF_CIID IS NOT NULL) AND (@IF_CIID = @CIID)
		BEGIN
			SELECT @Ret = 0
		END
		ELSE
		BEGIN
			UPDATE Character SET secondary_slot=@CIID WHERE CID=@CID
		END
	END
END
-- Custom1
ELSE IF @ItemParts = 10
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET custom1_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		SELECT @IF_CIID = custom2_slot FROM Character WHERE CID=@CID
		IF (@IF_CIID IS NOT NULL) AND (@IF_CIID = @CIID)
		BEGIN
			SELECT @Ret = 0
		END
		ELSE
		BEGIN
			UPDATE Character SET custom1_slot=@CIID WHERE CID=@CID
		END
	END
END
-- Custom2
ELSE IF @ItemParts = 11
BEGIN
	IF @CIID = 0
	BEGIN
		UPDATE Character SET custom2_slot=NULL WHERE CID=@CID
	END
	ELSE
	BEGIN
		SELECT @IF_CIID = custom1_slot FROM Character WHERE CID=@CID
		IF (@IF_CIID IS NOT NULL) AND (@IF_CIID = @CIID)
		BEGIN
			SELECT @Ret = 0
		END
		ELSE
		BEGIN
			UPDATE Character SET custom2_slot=@CIID WHERE CID=@CID
		END
	END
END



SELECT @Ret AS Ret


Go


