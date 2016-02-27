-- 게임내 상점에서 아이템 구매
-- Ex) EXEC spBuyBountyItem 1077, 7, 100
CREATE   PROC [spBuyBountyItem]
	@CID		INT,
	@ItemID		INT,
	@Price		INT
AS
BEGIN
	DECLARE @OrderCIID	int
	DECLARE @Bounty	INT

	BEGIN TRAN
		-- 잔액검사
		SELECT @Bounty=BP FROM Character(NOLOCK) WHERE CID=@CID
		IF @Bounty IS NULL OR @Bounty < @Price
		BEGIN
			ROLLBACK
			RETURN 0
		END

		-- Bounty 감소
		UPDATE Character SET BP=BP-@Price WHERE CID=@CID
		IF @@ERROR <> 0
		BEGIN
			ROLLBACK
			RETURN (-1)
		END

		-- Item 추가
		INSERT INTO CharacterItem (CID, ItemID, RegDate) Values (@CID, @ItemID, GETDATE())
		IF @@ERROR <> 0
		BEGIN
			ROLLBACK
			RETURN (-1)
		END

		SELECT @OrderCIID = @@IDENTITY
		
		-- Item 구매로그 추가
		INSERT INTO ItemPurchaseLogByBounty (ItemID, CID, Date, Bounty, CharBounty, Type)
		VALUES (@ItemID, @CID, GETDATE(), @Price, @Bounty, '구입')

		SELECT @OrderCIID as ORDERCIID
	COMMIT TRAN

	RETURN 1
END
GO


-- 게임내 상점에서 아이템 판매
CREATE   PROC [spSellBountyItem]
	@CID		INT,
	@ItemID		INT,
	@CIID		INT,
	@Price		INT,
	@CharBP		INT
AS
BEGIN
	BEGIN TRAN
		-- Item 삭제
		UPDATE CharacterItem SET CID=NULL WHERE CID=@CID AND CIID=@CIID
		IF (@@ERROR <> 0) OR (@@ROWCOUNT = 0)
		BEGIN
			ROLLBACK
			RETURN (-1)
		END

		-- Bounty 증가
		UPDATE Character SET BP=BP+@Price WHERE CID=@CID
		IF (@@ERROR <> 0) OR (@@ROWCOUNT = 0)
		BEGIN
			ROLLBACK
			RETURN (-1)
		END

		-- Item 판매 로그 추가
		INSERT INTO ItemPurchaseLogByBounty (ItemID, CID, Date, Bounty, CharBounty, Type)
		VALUES (@ItemID, @CID, GETDATE(), @Price, @CharBP, '판매')

		SELECT 1 as Ret
	COMMIT TRAN

	RETURN 1
END
GO

