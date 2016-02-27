SELECT TID4 AS TeamID, Name AS TeamName, Region AS 지역, 
 (SELECT Name FROM Character c(nolock) WHERE t.Member1CID=c.cid) AS Member1캐릭터,
 (SELECT a.UserID FROM Character c(nolock), Account a(nolock) WHERE t.Member1CID=c.cid AND c.aid=a.aid) AS Member1NetmarbleID,

 (SELECT Name FROM Character c(nolock) WHERE t.Member2CID=c.cid) AS Member2캐릭터,
 (SELECT a.UserID FROM Character c(nolock), Account a(nolock) WHERE t.Member2CID=c.cid AND c.aid=a.aid) AS Member2NetmarbleID,

 (SELECT Name FROM Character c(nolock) WHERE t.Member3CID=c.cid) AS Member3캐릭터,
 (SELECT a.UserID FROM Character c(nolock), Account a(nolock) WHERE t.Member3CID=c.cid AND c.aid=a.aid) AS Member3NetmarbleID,

 (SELECT Name FROM Character c(nolock) WHERE t.Member4CID=c.cid) AS Member4캐릭터,
 (SELECT a.UserID FROM Character c(nolock), Account a(nolock) WHERE t.Member4CID=c.cid AND c.aid=a.aid) AS Member4NetmarbleID,

Win, Lose, Draw, Point
FROM Team4 t
WHERE Member1CID IS NOT NULL AND Member2CID IS NOT NULL AND Member3CID IS NOT NULL AND Member4CID IS NOT NULL
ORDER BY TeamID