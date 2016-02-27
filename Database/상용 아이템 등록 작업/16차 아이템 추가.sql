UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=3, ResLevel=15,Slot=9,Weight=2, HP=0, AP=0, MAXWT=0,
Description='[인챈트 Lv.1] 카노푸스의 힘이 깃든 코인. 기를 모았을때 근접무기에 불이 붙는다. 인챈트 공격에 명중된 상대는 일정시간동안 매초 데미지를 입는다.'
WHERE ItemID=600001

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=3, ResLevel=15,Slot=9,Weight=2, HP=0, AP=0, MAXWT=0,
Description='[인챈트 Lv.1] 물과 얼음의 신, 에니아의 힘이 깃든 코인. 기를 모았을때 근접무기에 한기가 깃든다. 인챈트 공격에 명중당한 상대는 행동이 느려지고 특수동작이 봉쇄된다.'
WHERE ItemID=600101





-- 기간아이템이므로
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (600001, 0, 1000, GETDATE(), 0, 'icon_redcoin_600001.gif', 1)
--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName, RentType)
VALUES (600101, 0, 1000, GETDATE(), 0, 'icon_bluecoin_600101.gif', 1)
--




-- 가격목록
---
DECLARE @ret int
DECLARE @csid_male int

DECLARE @hourperiod1 int
DECLARE @hourperiod2 int
DECLARE @hourperiod3 int

DECLARE @price1 int
DECLARE @price2 int
DECLARE @price3 int

SELECT @csid_male=386
SELECT @hourperiod1 = 168, @price1 = 1000
SELECT @hourperiod2 = 360, @price2 = 1400
SELECT @hourperiod3 = 720, @price3 = 2000

EXEC spInsertRentCashShopPrice @csid_male, @hourperiod1, @price1
EXEC spInsertRentCashShopPrice @csid_male, @hourperiod2, @price2
EXEC spInsertRentCashShopPrice @csid_male, @hourperiod3, @price3





