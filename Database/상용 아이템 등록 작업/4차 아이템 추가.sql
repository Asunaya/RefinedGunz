UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=4,Weight=4, HP=2, AP=8, MAXWT=0, 
Description='바다의 무법자, 이터널블루의 남성용 제복 모자'
WHERE ItemID=520008

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=6,Weight=4, HP=2, AP=14, MAXWT=0, 
Description='바다의 무법자, 이터널블루의 남성용 제복 장갑'
WHERE ItemID=522003

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=8,Weight=4, HP=2, AP=17, MAXWT=0, 
Description='바다의 무법자, 이터널블루의 남성용 제복 부츠'
WHERE ItemID=524003

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=5,Weight=15, HP=3, AP=24, MAXWT=0, 
Description='바다의 무법자, 이터널블루의 남성용 제복 상의'
WHERE ItemID=521003

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=1, ResLevel=15,Slot=7,Weight=14, HP=2, AP=26, MAXWT=0, 
Description='바다의 무법자, 이터널블루의 남성용 제복 바지'
WHERE ItemID=523003

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=4,Weight=4, HP=2, AP=8, MAXWT=0, 
Description='바다의 무법자, 이터널블루의 여성용 제복 모자'
WHERE ItemID=520508

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=6,Weight=4, HP=2, AP=14, MAXWT=0, 
Description='바다의 무법자, 이터널블루의 여성용 제복 장갑'
WHERE ItemID=522503

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=8,Weight=4, HP=2, AP=17, MAXWT=0, 
Description='바다의 무법자, 이터널블루의 여성용 제복 부츠'
WHERE ItemID=524503

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=5,Weight=15, HP=3, AP=24, MAXWT=0, 
Description='바다의 무법자, 이터널블루의 여성용 제복 상의'
WHERE ItemID=521503

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=100, IsCashItem=1,
ResSex=2, ResLevel=15,Slot=7,Weight=14, HP=2, AP=26, MAXWT=0, 
Description='바다의 무법자, 이터널블루의 여성용 제복 바지'
WHERE ItemID=523503



-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520008, 0, 1100, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (522003, 0, 1200, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (524003, 0, 1400, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (521003, 0, 2300, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (523003, 0, 2000, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520508, 0, 1100, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (522503, 0, 1200, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (524503, 0, 1400, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (521503, 0, 2300, GETDATE(), 0, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (523503, 0, 2000, GETDATE(), 0, '')



INSERT INTO CashSetShop (Name, Description, CashPrice, NewItemOrder, RegDate, ResSex, ResLevel, Weight, Opened, WebImgName)
VALUES ('남성 이터널블루 제복세트', '바다의 무법자로 악명을 떨친 이터널블루의 남성용 제복세트', 7200, 0, GETDATE(), 1, 15, 41, 1,
'')

INSERT INTO CashSetShop (Name, Description, CashPrice, NewItemOrder, RegDate, ResSex, ResLevel, Weight, Opened, WebImgName)
VALUES ('여성 이터널블루 제복세트', '바다의 무법자로 악명을 떨친 이터널블루의 여성용 제복세트', 7200, 0, GETDATE(), 2, 15, 41, 1,
'')

--INSERT INTO CashSetItem (CSSID, CSID) VALUES (, )
