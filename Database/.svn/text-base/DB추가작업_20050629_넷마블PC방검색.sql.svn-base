-- 프리미엄 IP 검색(넷마블 PC방)
CREATE   PROC [spCheckPremiumIP]
	@IP		varchar(24)
AS
BEGIN
	SELECT cp_no FROM PCBangDB.Netmarble_Master.dbo.bx_vw_premium_iplist2 where ip=@IP
END
GO
