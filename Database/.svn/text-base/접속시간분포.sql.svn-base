SELECT DatePart(hh, Time) AS Hour, avg(PlayerCount) AS PlayerCount FROM
(SELECT sum(PlayerCount) As PlayerCount, Time As Time FROM ServerLog(nolock)
WHERE Time Between '2004.07.24' AND '2004.07.25'
GROUP By Time) a

GROUP BY DatePart(hh, Time)



