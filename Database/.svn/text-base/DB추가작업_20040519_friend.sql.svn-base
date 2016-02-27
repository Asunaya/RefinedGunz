-- 模备 抛捞喉
CREATE TABLE "Friend"
(
	"id"		int identity not null,
	"CID"		int not null,
	"FriendCID"	int not null,
	"Type"		int not null,
	"Favorite"	tinyint,
	"DeleteFlag"	tinyint
)
go

ALTER TABLE "Friend"
	ADD CONSTRAINT "Friend_PK" primary key ("id")
go

ALTER TABLE "Friend"
	ADD CONSTRAINT "Character_Friend_FK1" foreign key ("CID")
	REFERENCES "Character" ("CID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

ALTER TABLE "Friend"
	ADD CONSTRAINT "Character_Friend_FK2" foreign key ("FriendCID")
	REFERENCES "Character" ("CID") ON UPDATE NO ACTION ON DELETE NO ACTION
Go

CREATE INDEX "IX_Friend_DeleteFlag" ON "Friend" ("DeleteFlag")
CREATE INDEX "IX_Friend_Type" ON "Friend" ("Type")

CREATE INDEX "IX_Friend_CID" ON "Friend" ("CID")



go


-- sp 

-- 模备 眠啊
CREATE PROC [spAddFriend]
	@CID		int
,	@FriendCID	int
,	@Favorite	tinyint
AS
DECLARE @ID	int
INSERT INTO Friend(CID, FriendCID, Favorite, DeleteFlag, Type) Values (@CID, @FriendCID, @Favorite, 0, 1)
SET @ID = @@IDENTITY
SELECT @ID as ID
GO


-- 模备 昏力
CREATE PROC [spRemoveFriend]
	@CID		int
,	@FriendCID	int
AS

UPDATE Friend 
SET DeleteFlag=1
WHERE CID=@CID AND FriendCID=@FriendCID

Go

-- 模备 格废 啊廉坷扁
CREATE PROC [spGetFriendList]
	@CID		int
AS

SELECT  f.FriendCID, f.Favorite,  c.Name 
FROM Friend f(NOLOCK), Character c(NOLOCK) 
WHERE f.CID=@CID AND f.DeleteFlag=0 AND f.Type = 1 AND f.FriendCID=c.CID
GO



