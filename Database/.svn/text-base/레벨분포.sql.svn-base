SELECT 
(SELECT COUNT(*) FROM Character(nolock) WHERE DeleteFlag=0 AND Level=1) AS level1, 
(SELECT COUNT(*) FROM Character(nolock) WHERE DeleteFlag=0 AND Level BETWEEN 2 AND 5) AS level2_5,
(SELECT COUNT(*) FROM Character(nolock) WHERE DeleteFlag=0 AND Level BETWEEN 6 AND 10) AS level6_10,
(SELECT COUNT(*) FROM Character(nolock) WHERE DeleteFlag=0 AND Level BETWEEN 11 AND 20) AS level11_20,
(SELECT COUNT(*) FROM Character(nolock) WHERE DeleteFlag=0 AND Level >= 21) AS level21,
(SELECT COUNT(*) FROM Character(nolock) WHERE DeleteFlag=0) As Total