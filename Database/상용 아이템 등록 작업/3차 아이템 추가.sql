UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=0, IsCashItem=1,
ResSex=1, ResLevel=0,Slot=4,Weight=4, HP=1, AP=2, MAXWT=0, 
Description='부담없이 착용하기 좋은 남성용 두건. 전쟁터에서 자신을 위장하기 위해 사용하기도 한다.'
WHERE ItemID=520004

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=0, IsCashItem=1,
ResSex=1, ResLevel=0,Slot=4,Weight=4, HP=1, AP=2, MAXWT=0, 
Description='센스있게 간편히 착용하기에 좋은 남성용 두건. 패션 악세서리로 사용하곤 한다.'
WHERE ItemID=520005

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=0, IsCashItem=1,
ResSex=1, ResLevel=0,Slot=4,Weight=4, HP=1, AP=2, MAXWT=0, 
Description='가볍고 손쉽게 착용가능한 남성용 두건. 빨간색 천위에 귀여운 물방울이 수놓아져 있다.'
WHERE ItemID=520006

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=0, IsCashItem=1,
ResSex=1, ResLevel=0,Slot=4,Weight=4, HP=1, AP=2, MAXWT=0, 
Description='독특한 센스가 담겨진 별무늬의 남성용 두건.'
WHERE ItemID=520007


UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=0, IsCashItem=1,
ResSex=2, ResLevel=0,Slot=4,Weight=4, HP=1, AP=2, MAXWT=0, 
Description='부담없이 착용하기 좋은 여성용 두건. 전쟁터에서 자신을 위장하기 위해 사용하기도 한다.'
WHERE ItemID=520504

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=0, IsCashItem=1,
ResSex=2, ResLevel=0,Slot=4,Weight=4, HP=1, AP=2, MAXWT=0, 
Description='센스있게 간편히 착용하기에 좋은 여성용 두건. 패션 악세서리로 사용하곤 한다.'
WHERE ItemID=520505

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=0, IsCashItem=1,
ResSex=2, ResLevel=0,Slot=4,Weight=4, HP=1, AP=2, MAXWT=0, 
Description='가볍고 손쉽게 착용가능한 두건. 빨간색 천위에 귀여운 물방울이 수놓아져 있다.'
WHERE ItemID=520506

UPDATE Item SET TotalPoint=0, BountyPrice=0, Damage=0, Delay=0, Controllability=0, Magazine=0, ReloadTime=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', MaxBullet=0, LimitSpeed=0, IsCashItem=1,
ResSex=2, ResLevel=0,Slot=4,Weight=4, HP=1, AP=2, MAXWT=0, 
Description='독특한 센스가 담겨진 별무늬의 여성용 두건.'
WHERE ItemID=520507


-- 남성 밀리터리룩 두건
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520004, 0, 800, GETDATE(), 0, '')

-- 남성 민무늬 두건
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520005, 0, 800, GETDATE(), 0, '')

-- 남성 물방울무늬 두건
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520006, 0, 800, GETDATE(), 0, '')

-- 남성 별무늬 두건
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520007, 0, 800, GETDATE(), 0, '')

-- 여성 밀리터리룩 두건
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520504, 0, 800, GETDATE(), 0, '')

-- 여성 민무늬 두건
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520505, 0, 800, GETDATE(), 0, '')

-- 여성 물방울무늬 두건
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520506, 0, 800, GETDATE(), 0, '')

-- 여성 별무늬 두건
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName) 
VALUES (520507, 0, 800, GETDATE(), 0, '')
