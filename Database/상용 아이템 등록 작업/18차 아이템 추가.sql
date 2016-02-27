-- 남성용
UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=4,Weight=4, HP=2, AP=8, MAXWT=0, 
Description='제국군 엘리트 부대인 그레이더스트의 남성용 제식 모자'
WHERE ItemID=520016

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=6,Weight=4, HP=2, AP=14, MAXWT=0, 
Description='제국군 엘리트 부대인 그레이더스트의 남성용 제식 장갑.'
WHERE ItemID=522011

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=8,Weight=4, HP=2, AP=17, MAXWT=0, 
Description='제국군 엘리트 부대인 그레이더스트의 남성용 제식 구두.'
WHERE ItemID=524011

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=5,Weight=15, HP=3, AP=24, MAXWT=0, 
Description='제국군 엘리트 부대인 그레이더스트의 남성용 제식 코트.'
WHERE ItemID=521011

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=7,Weight=14, HP=2, AP=26, MAXWT=0, 
Description='제국군 엘리트 부대인 그레이더스트의 남성용 제식 바지.'
WHERE ItemID=523011

-- 여성용
UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=4,Weight=4, HP=2, AP=8, MAXWT=0, 
Description='제국군 엘리트 부대인 그레이더스트의 여성용 제식 모자.'
WHERE ItemID=520516

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=6,Weight=4, HP=2, AP=14, MAXWT=0, 
Description='제국군 엘리트 부대인 그레이더스트의 여성용 제식 장갑.'
WHERE ItemID=522511

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=8,Weight=4, HP=2, AP=17, MAXWT=0, 
Description='제국군 엘리트 부대인 그레이더스트의 여성용 제식 구두.'
WHERE ItemID=524511

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=5,Weight=15, HP=3, AP=24, MAXWT=0, 
Description='제국군 엘리트 부대인 그레이더스트의 여성용 제식 코트.'
WHERE ItemID=521511

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=7,Weight=14, HP=2, AP=26, MAXWT=0, 
Description='종교단체와 관련된 비밀조직의 여성용 바지. 가볍다.'
WHERE ItemID=523511



-- 기간아이템이므로
-- 모자
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (520016, 0, 500, GETDATE(), 0, '', 2)
-- 장갑
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (522011, 0, 600, GETDATE(), 0, '', 2)
-- 신발
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (524011, 0, 700, GETDATE(), 0, '', 2)
-- 상의
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (521011, 0, 1100, GETDATE(), 0, '', 2)
-- 하의
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (523011, 0, 900, GETDATE(), 0, '', 2)

-- 모자
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (520516, 0, 500, GETDATE(), 0, '', 2)
-- 장갑
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (522511, 0, 600, GETDATE(), 0, '', 2)
-- 신발
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (524511, 0, 700, GETDATE(), 0, '', 2)
-- 상의
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (521511, 0, 1100, GETDATE(), 0, '', 2)
-- 하의
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (523511, 0, 900, GETDATE(), 0, '', 2)


--



INSERT INTO CashSetShop (Name, Description, CashPrice, NewItemOrder, RegDate, ResSex, ResLevel, Weight, Opened, WebImgName, RentType)
VALUES ('남성 그레이더스트 세트', '제국군사이에서도 명성 높은 그레이더스트 부대의 남성용 제식의상이다.', 3800, 0, GETDATE(), 1, 15, 41, 0, '', 2)

INSERT INTO CashSetShop (Name, Description, CashPrice, NewItemOrder, RegDate, ResSex, ResLevel, Weight, Opened, WebImgName, RentType)
VALUES ('여성 그레이더스트 세트', '제국군사이에서도 명성 높은 그레이더스트 부대의 여성용 제식의상이다.', 3800, 0, GETDATE(), 2, 15, 41, 0, '', 2)


-- 남성셋트
--INSERT INTO CashSetItem (CSSID, CSID) VALUES (, )
-- 여성셋트
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
