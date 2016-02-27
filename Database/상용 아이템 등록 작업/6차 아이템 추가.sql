UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=0,Slot=4,Weight=3, HP=0, AP=3, MAXWT=0, 
Description='초보 모험가부터 무거운 장비를 싫어하는 이들에게 권장되는 가벼운 남성용 모자'
WHERE ItemID=520009

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=0,Slot=6,Weight=2, HP=1, AP=6, MAXWT=0, 
Description='초보 모험가부터 무거운 장비를 싫어하는 이들에게 권장되는 가벼운 남성용 장갑'
WHERE ItemID=522004

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=0,Slot=8,Weight=2, HP=1, AP=8, MAXWT=0, 
Description='초보 모험가부터 무거운 장비를 싫어하는 이들에게 권장되는 가벼운 남성용 신발'
WHERE ItemID=524004

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=0,Slot=5,Weight=10, HP=2, AP=10, MAXWT=0, 
Description='초보 모험가부터 무거운 장비를 싫어하는 이들에게 권장되는 가벼운 남성용 자켓'
WHERE ItemID=521004

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=0,Slot=7,Weight=10, HP=1, AP=12, MAXWT=0, 
Description='초보 모험가부터 무거운 장비를 싫어하는 이들에게 권장되는 가벼운 남성용 바지'
WHERE ItemID=523004

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=0,Slot=4,Weight=3, HP=0, AP=3, MAXWT=0, 
Description='초보 모험가부터 무거운 장비를 싫어하는 이들에게 권장되는 가벼운 여성용 모자'
WHERE ItemID=520509

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=0,Slot=6,Weight=2, HP=1, AP=6, MAXWT=0, 
Description='초보 모험가부터 무거운 장비를 싫어하는 이들에게 권장되는 가벼운 여성용 장갑'
WHERE ItemID=522504

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=0,Slot=8,Weight=2, HP=1, AP=8, MAXWT=0, 
Description='초보 모험가부터 무거운 장비를 싫어하는 이들에게 권장되는 가벼운 여성용 신발'
WHERE ItemID=524504

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=0,Slot=5,Weight=10, HP=2, AP=10, MAXWT=0, 
Description='초보 모험가부터 무거운 장비를 싫어하는 이들에게 권장되는 가벼운 여성용 자켓'
WHERE ItemID=521504

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=0,Slot=7,Weight=10, HP=1, AP=12, MAXWT=0, 
Description='초보 모험가부터 무거운 장비를 싫어하는 이들에게 권장되는 가벼운 여성용 바지'
WHERE ItemID=523504



-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520009, 0, 800, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (522004, 0, 800, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (524004, 0, 1000, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (521004, 0, 1800, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (523004, 0, 1600, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520509, 0, 800, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (522504, 0, 800, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (524504, 0, 1000, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (521504, 0, 1800, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (523504, 0, 1600, GETDATE(), 0, '')

-- 



INSERT INTO CashSetShop (Name, Description, CashPrice, NewItemOrder, RegDate, ResSex, ResLevel, Weight, Opened, WebImgName)
VALUES ('남성 모험가 세트', '초보 모험가부터 무거운 장비를 꺼려하는 이들에게 권장할만한 스포티한 남성 모험가 세트', 5400, 0, GETDATE(), 1, 0, 27, 0,
'')

INSERT INTO CashSetShop (Name, Description, CashPrice, NewItemOrder, RegDate, ResSex, ResLevel, Weight, Opened, WebImgName)
VALUES ('여성 모험가 세트', '초보 모험가부터 무거운 장비를 꺼려하는 이들에게 권장할만한 스포티한 여성 모험가 세트', 5400, 0, GETDATE(), 2, 0, 27, 0,
'')

--INSERT INTO CashSetItem (CSSID, CSID) VALUES (, )
