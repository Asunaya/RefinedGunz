UPDATE Item SET HP=0, AP=3, MAXWT=0, TotalPoint=0, BountyPrice=0, SlugOutput=0, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', LimitSpeed=100, IsCashItem=1,
ResSex=3, Slot=1,
ResLevel=0, Weight=4, Damage=8, Delay=90, Controllability=0, Magazine=0, MaxBullet=0, ReloadTime=0,
Description='(한정기간발매) 리딕의 단검. 장착만으로도 AP가 오르는 희귀한 단검'
WHERE ItemID=500004


UPDATE Item SET HP=0, AP=2, MAXWT=0, TotalPoint=0, BountyPrice=0, SlugOutput=1, Gadget=0, SF=0, FR=0,CR=0,PR=0,LR=0, BlendColor=0, ModelName='', LimitSpeed=0, IsCashItem=1,
ResSex=3, Slot=2, 
ResLevel=0, Weight=12, Damage=16, Delay=240, Controllability=36, Magazine=13, MaxBullet=78, ReloadTime=4, 
Description='(한정기간발매) 리딕 헌터의 권총. 장착만으로도 AP가 오르는 희귀한 권총.'
WHERE ItemID=504002


--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName)
VALUES (500004, 0, 1500, GETDATE(), 0, '')
--
INSERT INTO CashShop (ItemID, NewItemOrder, CashPrice, RegDate, Opened, WebImgName)
VALUES (504002, 0, 1500, GETDATE(), 0, '')


--INSERT INTO CashSetItem (CSSID, CSID) VALUES (, )
