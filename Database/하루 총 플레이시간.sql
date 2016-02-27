SELECT (sum(PlayTime) / 60 / 60) FROM PlayerLog(nolock) 
WHERE DisTime Between '2004.07.24' AND '2004.07.25'