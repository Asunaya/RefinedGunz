SELECT AVG(TotalPlayerCount) FROM

(SELECT sum(PlayerCount) AS TotalPlayerCount FROM ServerLog(nolock) 
WHERE Time BETWEEN '2004.07.24' AND '2004.07.25'
Group By Time) a