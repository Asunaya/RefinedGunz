SELECT TOP 1 sum(PlayerCount) AS TotalPlayerCount, Time FROM ServerLog(nolock) 
WHERE Time BETWEEN '2004.07.24' AND '2004.07.25'
Group By Time
Order by TotalPlayerCount desc