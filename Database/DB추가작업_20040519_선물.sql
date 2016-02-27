-- 선물하기 로그 테이블
CREATE TABLE "CashItemPresentLog"
(
	"id"		int identity not null,
	"SenderUserID"	varchar(20) not null,
	"ReceiverAID"	int not null,
	"CSID"		int null,
	"CSSID"		int null,
	"Cash"		int not null,
	"Date"		datetime not null
)
go

ALTER TABLE "CashItemPresentLog"
	ADD CONSTRAINT "CashItemPresentLog_PK" primary key ("id")
go
ALTER TABLE "CashItemPresentLog"
	ADD CONSTRAINT "Account_CashItemPresentLog_FK1" foreign key ("ReceiverAID")
	REFERENCES "Account" ("AID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

CREATE INDEX "IX_CashItemPresentLog_SenderUserID" ON "CashItemPresentLog" ("SenderUserID")
go


-- 선물하기 추가 sp
CREATE PROC [spPresentCashItem]
	@SenderUserID		varchar(20)
,	@ReceiverUserID		varchar(20)
,	@CSID			int
,	@Cash			int
AS
SET NoCount On

DECLARE @ItemID		int
DECLARE @ReceiverAID	int

/* Account가 없으면 RETURN 0 */
SELECT @ReceiverAID=AID FROM Account(nolock) where UserID = @ReceiverUserID

IF @ReceiverAID IS NULL
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
		Values	(@ReceiverAID, @ItemID)

		/* 로그 생성 */
		INSERT INTO CashItemPresentLog
		(SenderUserID, ReceiverAID, CSID, Date, Cash)
		VALUES
		(@SenderUserID, @ReceiverAID, @CSID, GETDATE(), @Cash)

		COMMIT TRAN
		RETURN 1
	END
	ELSE
	BEGIN
		RETURN 0
	END
END

Go

-- 선물하기 추가 sp : 세트아이템 선물
CREATE PROC [spPresentCashSetItem]
	@SenderUserID		varchar(20)
,	@ReceiverUserID		varchar(20)
,	@CSSID			int
,	@Cash			int
AS

SET NoCount On

DECLARE @ItemID			int
DECLARE @ReceiverAID		int

/* Account가 없으면 RETURN 0 */
SELECT @ReceiverAID=AID FROM Account where UserID = @ReceiverUserID

IF @ReceiverAID IS NULL
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
				INSERT INTO AccountItem (AID, ItemID) Values	(@ReceiverAID, @ItemID)
			END
	
			FETCH FROM curBuyCashSetItem INTO @varCSID
		END

		CLOSE curBuyCashSetItem
		DEALLOCATE curBuyCashSetItem

		/* 로그 생성 */
		INSERT INTO CashItemPresentLog
		(SenderUserID, ReceiverAID, CSSID, Date, Cash)
		VALUES
		(@SenderUserID, @ReceiverAID, @CSSID, GETDATE(), @Cash)

	COMMIT TRAN ------------------------------------------

	RETURN 1
END



Go

-- 캐쉬 일반아이템 이미지파일 알아오기
CREATE PROC [spGetCashItemImageFile]
	@CSID			int
,	@RetImageFileName	varchar(64) OUTPUT
AS
SELECT @RetImageFileName = WebImgName
FROM CashShop cs(nolock)
WHERE cs.csid=@CSID

RETURN 1

Go

-- 캐쉬 세트아이템 이미지파일 알아오기
CREATE PROC [spGetCashSetItemImageFile]
	@CSSID			int
,	@RetImageFileName	varchar(64) OUTPUT
AS
SELECT @RetImageFileName=WebImgName FROM CashSetShop css(nolock) WHERE CSSID=@CSSID

RETURN 1
Go


-- 뷰: 캐쉬아이템 선물 로그
CREATE VIEW viewCashItemPresentLog
AS

SELECT cpl.id AS id, cpl.SenderUserID AS SenderUserID, a.UserID AS ReceiverUserID, 
i.Name AS ItemName, cpl.Date AS Date FROM CashItemPresentLog cpl, Account a(nolock), CashShop cs(nolock), Item i(nolock)
WHERE cpl.ReceiverAID=a.AID AND cpl.CSID IS Not NULL AND cpl.csid=cs.csid AND cs.ItemID=i.ItemID


Go


-- 뷰: 캐쉬 세트아이템 선물 로그
CREATE VIEW viewCashSetItemPresentLog
AS

SELECT cpl.id AS id, cpl.SenderUserID AS SenderUserID, a.UserID AS ReceiverUserID, 
css.Name AS SetItemName, cpl.Date AS Date FROM CashItemPresentLog cpl, Account a(nolock), CashSetShop css(nolock)
WHERE cpl.ReceiverAID=a.AID AND cpl.CSSID IS Not NULL AND cpl.cssid=css.cssid


Go

----------------------------------------------------------------------------------------------------------------
