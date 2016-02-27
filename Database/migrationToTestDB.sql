DECLARE @AID 		int
DECLARE @UserID 	varchar(20)
DECLARE @CID	 	int
DECLARE @ChrName	varchar(24)
DECLARE @ChrNum	smallint

SELECT @AID = 2			-- zelinde
SELECT @UserID = 'zelinde'
SELECT @CID = 24		-- 발렌타인
SELECT @ChrName = '발렌타인'
SELECT @ChrNum = 1

use GunzDB
DECLARE @Ret int

DECLARE @Cert tinyint
DECLARE @Name varchar(128)
DECLARE @Age smallint
DECLARE @Sex tinyint
DECLARE @Password varchar(20)


DECLARE @TestAID	int
DECLARE @TestCID	int
DECLARE @ChrSex	tinyint
DECLARE @ChrHair	int
DECLARE @ChrFace	int
DECLARE @ChrXP	int
DECLARE @ChrBP	int


BEGIN TRAN

-- Account, Login 레코드가 존재하는지 검색하여 없으면 추가해준다.
SELECT @Age=age FROM TestDB.dbo.Account WHERE userid=@Userid

IF @Age IS NULL
BEGIN
	SELECT @Name=a.Name, @Cert=a.Cert, @Age=a.Age, @Sex=a.Sex, @Password=l.Password 
	FROM GunzDB.dbo.Login l, GunzDB.dbo.Account a
	WHERE l.aid=a.aid AND a.aid = @AID

--	EXEC TestDB.dbo.spInsertAccount @UserID, @Password, @Cert, @Name, @Age, @Sex
END

SELECT @TestAID = aid FROM TestDB.dbo.Account  WHERE userID=@UserID

IF @TestAID IS NOT NULL
BEGIN

SELECT @TestCID = cid FROM TestDB.dbo.Character 
WHERE aid=@TestAID AND name=@ChrName AND DeleteFlag=0

SELECT @ChrXP=xp, @ChrBP=bp, @ChrHair = hair, @ChrSex = sex, @ChrFace = face FROM GunzDB.dbo.Character WHERE cid=@CID

IF @TestCID IS NULL
BEGIN
	INSERT INTO TestDB.dbo.Character 
		(AID, Name, CharNum, Country, Level, Sex, Hair, Face, XP, BP, BonusRate, MaxWeight, SafeFalls, FR, CR, ER, WR, 
	         GameCount, KillCount, DeathCount, RegDate, PlayTime, DeleteFlag)
	Values
		(@TestAID, @ChrName, @CharNum, 0, 1, @TestChrSex, @TestChrHair, @TestChrFace, 0, 0, 0, 100, 500, 0, 0, 0, 0, 0, 0, 0, GETDATE(), 0, 0)

SET @TestCID = @@IDENTITY

END		
ELSE
BEGIN
	UPDATE TestDB.dbo.Character
	SET XP=@XP, BP=@BP, hair=@ChrHair, face=@ChrFace
	WHERE cid=@TestCID


END		-- IF @TestCID IS NULL


END 		-- IF @TestAID IS NOT NULL











SELECT @Ret AS Ret

COMMIT TRAN