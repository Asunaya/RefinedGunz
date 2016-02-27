UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=4,Weight=3, HP=2, AP=6, MAXWT=0, 
Description='종교단체와 관련된 비밀조직의 남성용 모자. 가볍다.'
WHERE ItemID=520015

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=6,Weight=3, HP=2, AP=12, MAXWT=0, 
Description='종교단체와 관련된 비밀조직의 남성용 장갑. 가볍다.'
WHERE ItemID=522010

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=8,Weight=3, HP=2, AP=15, MAXWT=0, 
Description='종교단체와 관련된 비밀조직의 남성용 신발. 가볍다.'
WHERE ItemID=524010

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=5,Weight=14, HP=3, AP=22, MAXWT=0, 
Description='종교단체와 관련된 비밀조직의 남성용 상의. 가볍다.'
WHERE ItemID=521010

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=7,Weight=13, HP=2, AP=24, MAXWT=0, 
Description='종교단체와 관련된 비밀조직의 남성용 바지. 가볍다.'
WHERE ItemID=523010

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=4,Weight=3, HP=2, AP=6, MAXWT=0, 
Description='종교단체와 관련된 비밀조직의 여성용 모자. 가볍다.'
WHERE ItemID=520515

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=6,Weight=3, HP=2, AP=12, MAXWT=0, 
Description='종교단체와 관련된 비밀조직의 여성용 장갑. 가볍다.'
WHERE ItemID=522510

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=8,Weight=3, HP=2, AP=15, MAXWT=0, 
Description='종교단체와 관련된 비밀조직의 여성용 신발. 가볍다.'
WHERE ItemID=524510

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=5,Weight=14, HP=3, AP=22, MAXWT=0, 
Description='종교단체와 관련된 비밀조직의 여성용 상의. 가볍다.'
WHERE ItemID=521510

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=7,Weight=13, HP=2, AP=24, MAXWT=0, 
Description='종교단체와 관련된 비밀조직의 여성용 바지. 가볍다.'
WHERE ItemID=523510



-- 기간아이템이므로
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (520015, 0, 500, GETDATE(), 0, '', 2)
--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (522010, 0, 600, GETDATE(), 0, '', 2)
--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (524010, 0, 700, GETDATE(), 0, '', 2)
--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (521010, 0, 1100, GETDATE(), 0, '', 2)
--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (523010, 0, 900, GETDATE(), 0, '', 2)

--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (520515, 0, 500, GETDATE(), 0, '', 2)
--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (522510, 0, 600, GETDATE(), 0, '', 2)
--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (524510, 0, 700, GETDATE(), 0, '', 2)
--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (521510, 0, 1100, GETDATE(), 0, '', 2)
--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (523510, 0, 900, GETDATE(), 0, '', 2)


--



INSERT INTO CashSetShop (Name, Description, CashPrice, NewItemOrder, RegDate, ResSex, ResLevel, Weight, Opened, WebImgName, RentType)
VALUES ('공화국 건프리스트 세트', '종교단체와 관련된 비밀조직의 남성용 의상세트. 전반적으로 가볍다.', 3800, 0, GETDATE(), 1, 15, 36, 0, '', 2)

INSERT INTO CashSetShop (Name, Description, CashPrice, NewItemOrder, RegDate, ResSex, ResLevel, Weight, Opened, WebImgName, RentType)
VALUES ('공화국 건프리스티스 세트', '종교단체와 관련된 비밀조직의 여성용 의상세트. 전반적으로 가볍다.', 3800, 0, GETDATE(), 2, 15, 36, 0, '', 2)


--INSERT INTO CashSetItem (CSSID, CSID) VALUES (, )



-- 가격목록
---
DECLARE @ret int
DECLARE @csid_male int
DECLARE @csid_female int

DECLARE @hourperiod1 int
DECLARE @hourperiod2 int
DECLARE @hourperiod3 int
DECLARE @hourperiod4 int

DECLARE @price1 int
DECLARE @price2 int
DECLARE @price3 int
DECLARE @price4 int

SELECT @csid_male=380, @csid_female=385
SELECT @hourperiod1 = 168, @price1 = 700
SELECT @hourperiod2 = 360, @price2 = 900
SELECT @hourperiod3 = 720, @price3 = 1300
SELECT @hourperiod4 = NULL, @price4 = 1800


-- 남자
EXEC spInsertRentCashShopPrice @csid_male, @hourperiod1, @price1
EXEC spInsertRentCashShopPrice @csid_male, @hourperiod2, @price2
EXEC spInsertRentCashShopPrice @csid_male, @hourperiod3, @price3
EXEC spInsertRentCashShopPrice @csid_male, @hourperiod4, @price4

-- 여자
EXEC spInsertRentCashShopPrice @csid_female, @hourperiod1, @price1
EXEC spInsertRentCashShopPrice @csid_female, @hourperiod2, @price2
EXEC spInsertRentCashShopPrice @csid_female, @hourperiod3, @price3
EXEC spInsertRentCashShopPrice @csid_female, @hourperiod4, @price4





-- 세트 가격목록
DECLARE @ret int
DECLARE @cssid_male int
DECLARE @cssid_female int

DECLARE @hourperiod1 int
DECLARE @hourperiod2 int
DECLARE @hourperiod3 int
DECLARE @hourperiod4 int

DECLARE @price1 int
DECLARE @price2 int
DECLARE @price3 int
DECLARE @price4 int

SELECT @cssid_male=72, @cssid_female=73
SELECT @hourperiod1 = 168, @price1 = 2800
SELECT @hourperiod2 = 360, @price2 = 3500
SELECT @hourperiod3 = 720, @price3 = 4800
SELECT @hourperiod4 = NULL, @price4 = 6300

-- 남자
EXEC spInsertRentCashSetShopPrice @cssid_male, @hourperiod1, @price1
EXEC spInsertRentCashSetShopPrice @cssid_male, @hourperiod2, @price2
EXEC spInsertRentCashSetShopPrice @cssid_male, @hourperiod3, @price3
EXEC spInsertRentCashSetShopPrice @cssid_male, @hourperiod4, @price4

-- 여자
EXEC spInsertRentCashSetShopPrice @cssid_female, @hourperiod1, @price1
EXEC spInsertRentCashSetShopPrice @cssid_female, @hourperiod2, @price2
EXEC spInsertRentCashSetShopPrice @cssid_female, @hourperiod3, @price3
EXEC spInsertRentCashSetShopPrice @cssid_female, @hourperiod4, @price4
