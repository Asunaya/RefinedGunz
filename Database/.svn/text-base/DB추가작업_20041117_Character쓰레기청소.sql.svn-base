-- 저장프로시져 추가 -------------------------------
----------------------------------------------------
-- 삭제된 캐릭터 쓰레기 청소
CREATE PROC [spClearCharacterGarbage]
AS

BEGIN TRAN -------------

-- 캐릭터의 장비를 벗긴다.
UPDATE Character
SET 
head_slot=NULL, chest_slot=NULL, hands_slot=NULL, legs_slot=NULL, feet_slot=NULL, fingerl_slot=NULL, fingerr_slot=NULL, 
melee_slot=NULL, primary_slot=NULL, secondary_slot=NULL, custom1_slot=NULL, custom2_slot=NULL
WHERE DeleteFlag=1 AND
(LastTime IS NULL OR DATEDIFF(month, LastTime, GETDATE()) > 3)

-- 삭제된 캐릭터아이템 청소
DELETE FROM CharacterItem WHERE CID IS NULL


DECLARE @varCID int
DECLARE curClearCharacter INSENSITIVE CURSOR
FOR
	SELECT CID
	FROM Character(nolock)
	WHERE DeleteFlag=1 AND 
	(LastTime IS NULL OR DATEDIFF(month, LastTime, GETDATE()) > 3)
FOR READ ONLY
OPEN curClearCharacter

FETCH FROM curClearCharacter INTO @varCID

WHILE @@FETCH_STATUS = 0
BEGIN
	-- 캐릭터 아이템 삭제
	DELETE FROM CharacterItem WHERE CID=@varCID

	--클랜멤버에서 탈퇴
	DELETE FROM ClanMember WHERE CID=@varCID

	--클랜마스터이면 마스터 변경
	UPDATE Clan SET MasterCID=NULL WHERE MasterCID=@varCID

	-- Team4 멤버 삭제
	UPDATE Team4 Set MasterCID=NULL WHERE MasterCID=@varCID
	UPDATE Team4 Set Member1CID=NULL WHERE Member1CID=@varCID
	UPDATE Team4 Set Member2CID=NULL WHERE Member2CID=@varCID
	UPDATE Team4 Set Member3CID=NULL WHERE Member3CID=@varCID
	UPDATE Team4 Set Member4CID=NULL WHERE Member4CID=@varCID

	-- 친구삭제
	DELETE FROM Friend WHERE CID=@varCID or FriendCID=@varCID

	FETCH FROM curClearCharacter INTO @varCID
END
CLOSE curClearCharacter
DEALLOCATE curClearCharacter


-- 실제 Character 삭제
DELETE FROM Character
WHERE DeleteFlag=1 AND 
(LastTime IS NULL OR DATEDIFF(month, LastTime, GETDATE()) > 3)

COMMIT TRAN -----------

Go