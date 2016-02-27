UPDATE Item SET HP=0, AP=0, MAXWT=0, TotalPoint=0, BountyPrice=0, SlugOutput=1, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', LimitSpeed=0, IsCashItem=1,
ResSex=3, Slot=2, 
ResLevel=0, Weight=28, Damage=14, Delay=85, Controllability=90, Magazine=40, MaxBullet=200, ReloadTime=5, 
Description='초보 모험가들이 주로 사용하는 SMG. 상인들이 레너드의 탄창을 불법개조하여 판매하고 있는듯 하다.'
WHERE ItemID=505001

UPDATE Item SET HP=0, AP=0, MAXWT=0, TotalPoint=0, BountyPrice=0, SlugOutput=1, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', LimitSpeed=0, IsCashItem=1,
ResSex=3, Slot=2, 
ResLevel=0, Weight=12, Damage=18, Delay=250, Controllability=36, Magazine=12, MaxBullet=72, ReloadTime=4, 
Description='초보 모험가들이 주로 사용하는 권총.'
WHERE ItemID=504001

UPDATE Item SET HP=0, AP=0, MAXWT=0, TotalPoint=0, BountyPrice=0, SlugOutput=1, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', LimitSpeed=0, IsCashItem=1,
ResSex=3, Slot=2, 
ResLevel=7, Weight=28, Damage=16, Delay=95, Controllability=90, Magazine=40, MaxBullet=200, ReloadTime=5, 
Description='공화국의 병사들이 사용하던 SMG. 자신들의 근거지의 이름을 붙였다.'
WHERE ItemID=505002

UPDATE Item SET HP=0, AP=0, MAXWT=0, TotalPoint=0, BountyPrice=0, SlugOutput=1, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', LimitSpeed=0, IsCashItem=1,
ResSex=3, Slot=2, 
ResLevel=7, Weight=18, Damage=15, Delay=110, Controllability=30, Magazine=36, MaxBullet=180, ReloadTime=5, 
Description='공화국의 병사들이 사용하던 소총. 자신들의 근거지의 이름을 붙였다.'
WHERE ItemID=507001

UPDATE Item SET HP=0, AP=0, MAXWT=0, TotalPoint=0, BountyPrice=0, SlugOutput=1, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', LimitSpeed=0, IsCashItem=1,
ResSex=3, Slot=2, 
ResLevel=7, Weight=18, Damage=7, Delay=1100, Controllability=60, Magazine=5, MaxBullet=25, ReloadTime=5, 
Description='공화국의 병사들이 사용하던 샷건. 자신들의 근거지의 이름을 붙였다.'
WHERE ItemID=506001

UPDATE Item SET HP=0, AP=0, MAXWT=0, TotalPoint=0, BountyPrice=0, SlugOutput=1, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', LimitSpeed=0, IsCashItem=1,
ResSex=3, Slot=2, 
ResLevel=15, Weight=28, Damage=16, Delay=90, Controllability=80, Magazine=40, MaxBullet=200, ReloadTime=5, 
Description='무법자들의 도시 하바니를 주름잡던 아이언크로우에서 사용하는 SMG'
WHERE ItemID=505003

UPDATE Item SET HP=0, AP=0, MAXWT=0, TotalPoint=0, BountyPrice=0, SlugOutput=1, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', LimitSpeed=0, IsCashItem=1,
ResSex=3, Slot=2, 
ResLevel=15, Weight=18, Damage=16, Delay=100, Controllability=30, Magazine=36, MaxBullet=180, ReloadTime=5, 
Description='무법자들의 도시 하바니를 주름잡던 아이언크로우에서 사용하는 소총'
WHERE ItemID=507002

UPDATE Item SET HP=0, AP=0, MAXWT=0, TotalPoint=0, BountyPrice=0, SlugOutput=1, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', LimitSpeed=0, IsCashItem=1,
ResSex=3, Slot=2, 
ResLevel=15, Weight=18, Damage=7, Delay=900, Controllability=60, Magazine=5, MaxBullet=25, ReloadTime=5, 
Description='무법자들의 도시 하바니를 주름잡던 아이언크로우에서 사용하는 샷건'
WHERE ItemID=506002







-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (505001, 0, 1500, GETDATE(), 1, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (504001, 0, 1500, GETDATE(), 1, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (505002, 0, 2500, GETDATE(), 1, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (507001, 0, 2500, GETDATE(), 1, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (506001, 0, 2500, GETDATE(), 1, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (505003, 0, 3500, GETDATE(), 1, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (507002, 0, 3500, GETDATE(), 1, '')

-- 
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (506002, 0, 3500, GETDATE(), 1, '')







