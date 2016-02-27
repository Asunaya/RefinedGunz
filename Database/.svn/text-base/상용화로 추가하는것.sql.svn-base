USE GunzDB
go

-- Account의 UserID에 인덱스를 만든다.
CREATE INDEX "IX_Account_UserID" ON "Account" ("UserID")
go


-- 캐쉬 세트 아이템 
CREATE TABLE "CashSetItem" 
(
	CSIID 		int identity not null,
	CSSID		int not null,
	CSID		int not null
)
go

alter table "CashSetItem"
	ADD CONSTRAINT "CashSetItem_PK" primary key ("CSIID")

go

-- 캐쉬 세트 아이템 샵
CREATE TABLE "CashSetShop"
(
	"CSSID"		int identity not null,
	"Name"		varchar(64) null,
	"Description"	text null,
	"CashPrice"	int not null,
	"WebImgName"	varchar(64) null,
	"NewItemOrder"	tinyint null,
	"ResSex"	tinyint,
	"ResLevel"	int,
	"Weight"		int,
	"Opened"		tinyint,
	"RegDate" 	datetime null
)
go

ALTER TABLE "CashSetShop"
	ADD CONSTRAINT "CashSetShop_PK" primary key ("CSSID")
go
CREATE INDEX "IX_CashSetShop_NewItemOrder" ON "CashSetShop" ("NewItemOrder")
go
CREATE INDEX "IX_CashSetShop_Opened" ON "CashSetShop" ("Opened")
go


-- 캐쉬 아이템 샵
CREATE TABLE "CashShop"
(
	CSID		int identity not null,
	ItemID		int not null,
	NewItemOrder	tinyint null,
	CashPrice	int not null,
	WebImgName varchar(64) null,
	Opened			tinyint,
	RegDate		datetime null
)
go

ALTER TABLE "CashShop"
	ADD CONSTRAINT "CashShop_PK" PRIMARY KEY ("CSID")
go

CREATE INDEX "IX_CashShop_NewItemOrder" ON "CashShop" ("NewItemOrder")
go
CREATE INDEX "IX_CashShop_Opened" ON "CashShop" ("Opened")
go


/* 기존 테이블 업데이트 */

ALTER TABLE "Item" ADD "Description" text null
go

ALTER TABLE "Item" ADD "MaxBullet" int
go

/* 속도 - 100 = 100% */
ALTER TABLE "Item" ADD "LimitSpeed" tinyint
go

/* 캐쉬아이템인지 여부 */
ALTER TABLE "Item" ADD "IsCashItem" tinyint
go

-- Item에 인덱스를 만든다.
CREATE INDEX "IX_Item_Slot" ON "Item" ("Slot")
go
CREATE INDEX "IX_Item_ResLevel" ON "Item" ("ResLevel")
go
CREATE INDEX "IX_Item_ResSex" ON "Item" ("ResSex")
go


-- 외부키
ALTER TABLE "CashSetItem"
	ADD CONSTRAINT "Item_CashSetItem_FK1" FOREIGN KEY (
		"CSID")
	REFERENCES "CashShop" ("CSID") ON UPDATE NO ACTION ON DELETE NO ACTION
go

ALTER TABLE "CashSetItem"
	ADD CONSTRAINT "CashSetShop_CashSetItem_FK1" FOREIGN KEY (
		"CSSID")
	REFERENCES "CashSetShop" (
		"CSSID") ON UPDATE NO ACTION ON DELETE NO ACTION
go

ALTER TABLE "CashShop"
	ADD CONSTRAINT "Item_CashShop_FK1" FOREIGN KEY (
		"ItemID")
	REFERENCES "Item" (
		"ItemID") ON UPDATE NO ACTION ON DELETE NO ACTION
go






/* 캐쉬 세트아이템 구매로그 */
CREATE TABLE "SetItemPurchaseLogByCash"
(
	"id" int identity not null,
	"AID"		int null,
	"CSSID"	int null,
	"Date"		datetime not null,
	"Cash" 		int null
) ON 'PRIMARY'

Go

ALTER TABLE "SetItemPurchaseLogByCash"
	ADD CONSTRAINT "SetItemPurchaseLogByCash_PK" primary key clustered ("id")
Go

ALTER TABLE "SetItemPurchaseLogByCash"
	ADD CONSTRAINT "CashSetShop_SetItemPurchaseLogByCash_FK1" foreign key ("CSSID")
	REFERENCES "CashSetShop" ("CSSID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

ALTER TABLE "SetItemPurchaseLogByCash"
	ADD CONSTRAINT "Account_SetItemPurchaseLogByCash_FK1" foreign key ("AID")
	REFERENCES "Account" ("AID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go




/* 캐쉬아이템구매로그 */
CREATE TABLE "ItemPurchaseLogByCash"
(
	"id" int identity not null,
	"AID"		int null,
	"ItemID"	int not null,
	"Date"		datetime not null,
	"Cash" 		int null
) ON 'PRIMARY'

Go

ALTER TABLE "ItemPurchaseLogByCash"
	ADD CONSTRAINT "PurchaseItemByCashLog_PK" primary key clustered ("id")
Go

ALTER TABLE "ItemPurchaseLogByCash"
	ADD CONSTRAINT "Item_PurchaseLogByCash_FK1" foreign key ("ItemID")
	REFERENCES "Item" ("ItemID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

ALTER TABLE "ItemPurchaseLogByCash"
	ADD CONSTRAINT "Account_PurchaseLogByCash_FK1" foreign key ("AID")
	REFERENCES "Account" ("AID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go



/* 내캐릭으로 계정 아이템 가져오는 로그 */
CREATE TABLE "BringAccountItemLog"
(
	"id" int identity not null,
	"AID"		int null,
	"CID"		int null,
	"ItemID"	int not null,
	"Date"		datetime not null,
) ON 'PRIMARY'

Go

ALTER TABLE "BringAccountItemLog"
	ADD CONSTRAINT "BringAccountItemLog_PK" primary key clustered ("id")
Go

ALTER TABLE "BringAccountItemLog"
	ADD CONSTRAINT "Item_BringAccountItemLog_FK1" foreign key ("ItemID")
	REFERENCES "Item" ("ItemID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

ALTER TABLE "BringAccountItemLog"
	ADD CONSTRAINT "Account_BringAccountItemLog_FK1" foreign key ("AID")
	REFERENCES "Account" ("AID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

ALTER TABLE "BringAccountItemLog"
	ADD CONSTRAINT "Character_BringAccountItemLog_FK1" foreign key ("CID")
	REFERENCES "Character" ("CID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go





/* 뷰 - 캐쉬아이템 구매 */
CREATE VIEW viewItemPurchaseLogByCash
AS
SELECT ipl.id AS id, a.UserID AS UserID, i.Name AS ItemName, ipl.Date AS Date, ipl.Cash
FROM ItemPurchaseLogByCash ipl, Account a, Item i
WHERE ipl.AID = a.AID AND ipl.ItemID=i.ItemID
Go


/* 뷰 - 캐쉬 세트아이템 구매 */
CREATE VIEW viewSetItemPurchaseLogByCash
AS
SELECT id, a.UserID AS UserID, css.Name AS SetItemName, sipl.Date AS Date, sipl.Cash
FROM SetItemPurchaseLogByCash sipl, Account a, CashSetShop css
WHERE sipl.AID = a.AID AND css.cssid = sipl.cssid

Go

