-- cash shop에서 거래된 아이템을 accountitme에 추가.
CREATE  PROC [spBuyCashItem]
	@UserID		varchar(20),
	@CSID		int,
	@Cash		int,
	@RentHourPeriod smallint = NULL
AS
	SET NoCount On

	DECLARE @AID		int
	DECLARE @ItemID		int

	-- Account 검사
	SELECT @AID = AID FROM Account WHERE UserID = @UserID
	IF @AID IS NULL
	BEGIN
		RETURN 0
	END
	ELSE
	BEGIN
		SELECT @ItemID = ItemID FROM CashShop cs (NOLOCK) WHERE cs.CSID = @CSID

		IF @ItemID IS NOT NULL
		BEGIN 
			DECLARE @RentDate	datetime

			-- @RentHourPeriod값을 가지고 기간제인지 검사.
			IF @RentHourPeriod = 0 OR @RentHourPeriod IS NULL
			BEGIN
				-- 기간제 아이템일 경우 영구 아이템 판매 여부 검사
				DECLARE @RentType 	TINYINT
				DECLARE @RCSPID		INT

				SELECT @RentType = RentType FROM CashShop WHERE CSID=@CSID
				IF @RentType = 1
				BEGIN
					SELECT @RCSPID = RCSPID FROM RentCashShopPrice WHERE CSID=@CSID AND RentHourPeriod is NULL
					IF @RCSPID IS NULL
					BEGIN
						RETURN 0
					END
				END

				-- 일반 아이템인 경우
				SET @RentDate = NULL
			END
			ELSE
			BEGIN
				SET @RentDate = GETDATE()
			END


			BEGIN TRAN

			-- 아이템 생성.
			INSERT INTO accountitem(AID, ItemID, RentDate, RentHourPeriod) 
			VALUES (@AID, @ItemID, @RentDate, @RentHourPeriod)

			-- 아이템 거래 log생성.
			INSERT INTO ItemPurchaseLogByCash(AID, ItemID, Date, RentHourPeriod, Cash) 
			VALUES (@AID, @ItemID, GETDATE(), @RentHourPeriod, @Cash)

			COMMIT TRAN

			RETURN 1

		END	
		ELSE
		BEGIN
			RETURN 0
		END
	END

	RETURN 1
GO





-- cash set shop에서 거래된 아이템을 accountitme에 추가.
CREATE   PROC [spBuyCashSetItem]
	@UserID		varchar(20),
	@CSSID		int,
	@Cash		int,
	@RentHourPeriod smallint = NULL
AS
	SET NoCount On

	DECLARE @AID		int
	
	SELECT @AID = AID FROM Account WHERE UserID = @UserID

	-- 존제하는 유저인지 검사.
	IF @AID IS NULL
	BEGIN
		RETURN 0
	END
	ELSE
	BEGIN
		DECLARE @RentDate		datetime			

		-- @RentHourPeriod값을 가지고 기간제인지 검사.
		IF @RentHourPeriod = 0 OR @RentHourPeriod IS NULL
		BEGIN
			-- 기간제 아이템일 경우 영구 아이템 판매 여부 검사
			DECLARE @RentType 	TINYINT
			DECLARE @RCSSPID		INT

			SELECT @RentType = RentType FROM CashSetShop WHERE CSSID=@CSSID
			IF @RentType = 1
			BEGIN
				SELECT @RCSSPID = RCSSPID FROM RentCashSetShopPrice WHERE CSSID=@CSSID AND RentHourPeriod is NULL
				IF @RCSSPID IS NULL
				BEGIN
					RETURN 0
				END
			END

			-- 일반 아이템일 경우
			SET @RentDate = NULL
		END
		ELSE
		BEGIN
			SET @RentDate = GETDATE()
		END



		BEGIN TRAN

		DECLARE curBuyCashSetItem 	INSENSITIVE CURSOR

		FOR
			SELECT CSID FROM CashSetItem (NOLOCK) WHERE CSSID = @CSSID
		FOR READ ONLY

		OPEN curBuyCashSetItem 

		DECLARE @varCSID		int
		DECLARE @ItemID			int

		FETCH FROM curBuyCashSetItem INTO @varCSID

		WHILE @@FETCH_STATUS = 0
		BEGIN
			SELECT @ItemID = cs.ItemID
			FROM CashShop cs (NOLOCK) 
			WHERE cs.CSID = @varCSID 

			IF @ItemID IS NOT NULL
			BEGIN
				-- 아이템 생성.
				INSERT INTO AccountItem(AID, ItemID, RentDate, RentHourPeriod)
				VALUES (@AID, @ItemID, @RentDate, @RentHourPeriod)
			END

			FETCH curBuyCashSetItem  INTO @varCSID
		END

		CLOSE curBuyCashSetItem 
		DEALLOCATE curBuyCashSetItem 

		-- 셋트 아이템 구입 로그.
		INSERT INTO SetItemPurchaseLogByCash (AID, CSSID, Date, RentHourPeriod, Cash)
		VALUES (@AID, @CSSID, GETDATE(), @RentHourPeriod, @Cash)

		COMMIT TRAN
		RETURN 1
	END
GO



-- 단일 아이템 선물하기
CREATE PROC [spPresentCashItem]
	@SenderUserID	varchar(20)
,	@ReceiverUserID	varchar(20)
,	@CSID		int
,	@Cash		int
,	@RentHourPeriod	smallint = NULL
AS
	SET NoCount On

	DECLARE	@ItemID		int
	DECLARE @ReceiverAID	int

	SELECT @ReceiverAID = AID FROM Account (NOLOCK) WHERE UserID = @ReceiverUserID
	
	IF @ReceiverAID IS NULL
	BEGIN
		RETURN 0
	END
	ELSE
	BEGIN
		DECLARE @RentDate	datetime
		-- @RentHourPeriod값을 가지고 기간제인지 검사.
		IF @RentHourPeriod = 0 OR @RentHourPeriod IS NULL
		BEGIN
			-- 기간제 아이템일 경우 영구 아이템 판매 여부 검사
			DECLARE @RentType 	TINYINT
			DECLARE @RCSPID		INT

				SELECT @RentType = RentType FROM CashShop WHERE CSID=@CSID
			IF @RentType = 1
			BEGIN
				SELECT @RCSPID = RCSPID FROM RentCashShopPrice WHERE CSID=@CSID AND RentHourPeriod is NULL
				IF @RCSPID IS NULL
				BEGIN
					RETURN 0
				END
			END

			-- 일반 아이템인 경우
			SET @RentDate = NULL
		END
		ELSE
		BEGIN
			SET @RentDate = GETDATE()
		END



		SELECT @ItemID = ItemID FROM CashShop (NOLOCK) WHERE CSID = @CSID

		IF @ItemID IS NOT NULL
		BEGIN
			BEGIN TRAN
			-- 아이템 생성.
			INSERT INTO AccountItem (AID, ItemID, RentDate, RentHourPeriod)
			VALUES (@ReceiverAID, @ItemID, @RentDate, @RentHourPeriod)

			-- 선물 로그 생성.
			INSERT INTO CashItemPresentLog (SenderUserID, ReceiverAID, CSID, Date, Cash, RentHourPeriod)
			VALUES (@SenderUserID, @ReceiverAID, @CSID, GETDATE(), @Cash, @RentHourPeriod)

			COMMIT TRAN
		END
		ELSE
		BEGIN
			RETURN 0
		END

		
		RETURN 1
	END
GO



-- 셋트 아이템 선물하기.
CREATE PROC [spPresentCashSetItem]
	@SenderUserID	varchar(20)
,	@ReceiverUserID	varchar(20)
,	@CSSID		int
,	@Cash		int
,	@RentHourPeriod	smallint = NULL
AS
	SET NoCount On

	DECLARE @ReceiverAID	int

	SELECT @ReceiverAID = AID FROM Account WHERE UserID = @ReceiverUserID

	IF @ReceiverAID IS NOT NULL
	BEGIN
		DECLARE @RentDate		datetime			

		-- @RentHourPeriod값을 가지고 기간제인지 검사.
		IF @RentHourPeriod = 0 OR @RentHourPeriod IS NULL
		BEGIN
			-- 기간제 아이템일 경우 영구 아이템 판매 여부 검사
			DECLARE @RentType 	TINYINT
			DECLARE @RCSSPID	INT

			SELECT @RentType = RentType FROM CashSetShop WHERE CSSID=@CSSID
			IF @RentType = 1
			BEGIN
				SELECT @RCSSPID = RCSSPID FROM RentCashSetShopPrice WHERE CSSID=@CSSID AND RentHourPeriod is NULL
				IF @RCSSPID IS NULL
				BEGIN
					RETURN 0
				END
			END

			-- 일반 아이템일 경우
			SET @RentDate = NULL
		END
		ELSE
		BEGIN
			SET @RentDate = GETDATE()
		END


		BEGIN TRAN
			DECLARE curBuyCashSetItem 	INSENSITIVE CURSOR

			FOR
				SELECT CSID FROM CashSetItem WHERE CSSID = @CSSID
			FOR READ ONLY

			OPEN curBuyCashSetItem

			DECLARE @varCSID	int
			DECLARE @ItemID		int

			FETCH FROM curBuyCashSetItem INTO @varCSID

			WHILE @@FETCH_STATUS = 0
			BEGIN
				SELECT @ItemID = ItemID FROM CashShop WHERE CSID = @varCSID

				IF @ItemID IS NOT NULL
				BEGIN	
					-- 아이템 생성.
					INSERT INTO AccountItem (AID, ItemID, RentDate, RentHourPeriod)
					VALUES (@ReceiverAID, @ItemID, @RentDate, @RentHourPeriod)
				END
				
				FETCH FROM curBuyCashSetItem INTO @varCSID
			END

		CLOSE curBuyCashSetItem
		DEALLOCATE curBuyCashSetItem

		-- 셋트아이템 선물 로그 생성.
		INSERT INTO CashItemPresentLog (SenderUserID, ReceiverAID, CSSID, Date, RentHourPeriod, Cash)
		VALUES (@SenderUserID, @ReceiverAID, @CSSID, GETDATE(), @RentHourPeriod, @Cash)

		COMMIT TRAN
		RETURN 1

	END
	ELSE
	BEGIN
		RETURN 0
	END


GO


ALTER PROC [spGetRentCashSetShopPriceByHour]
	@CSSID		int
,	@RentHourPeriod	smallint
AS
	IF @RentHourPeriod IS NOT NULL
	BEGIN
		-- 기간제 아이템
		SELECT CashPrice
		FROM RentCashSetShopPrice 
		WHERE CSSID = @CSSID AND RentHourPeriod = @RentHourPeriod
	END
	ELSE IF @RentHourPeriod IS NULL
	BEGIN
		-- 영구 아이템.
		SELECT CashPrice
		FROM RentCashSetShopPrice
		WHERE CSSID = @CSSID AND RentHourPeriod IS NULL
	END
GO


ALTER   PROC [spGetRentCashShopPriceByHour]
	@CSID		int
,	@RentHourPeriod	smallint
AS
	IF @RentHourPeriod IS NOT NULL
	BEGIN
		-- 기간제 아이템
		SELECT CashPrice 
		FROM RentCashShopPrice 
		WHERE CSID = @CSID AND RentHourPeriod = @RentHourPeriod
	END
	ELSE IF @RentHourPeriod IS NULL
	BEGIN
		-- 영구 아이템
		SELECT CashPrice
		FROM RentCashShopPrice
		WHERE CSID = @CSID AND RentHourPeriod IS NULL
	END


GO
