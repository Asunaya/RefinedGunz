SELECT 
(SELECT SUM(Cash) AS SingleCash FROM ItemPurchaseLogByCash ipl(nolock) 
WHERE Date BETWEEN '2004.07.24' AND '2004.07.25') AS SingleCash,

(SELECT SUM(Cash) AS SetCash FROM SetItemPurchaseLogByCash ipl(nolock) 
WHERE Date BETWEEN '2004.07.24' AND '2004.07.25') AS SetCash,

(SELECT SUM(Cash) AS PresentCash FROM CashItemPresentLog(nolock) 
WHERE Date BETWEEN '2004.07.24' AND '2004.07.25') AS PresentCash
