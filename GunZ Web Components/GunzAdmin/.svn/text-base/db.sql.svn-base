DROP TABLE IF EXISTS report_abuse_info;

/* 119 신고 정보 */
CREATE TABLE report_abuse_info
(
	rid				int DEFAULT '0' NOT NULL auto_increment primary key,
	reporter		varchar(24),
	filename		varchar(64),
	title			varchar(64),
	comment			text,		/* 채팅 내용 */
	admin_comment	text,		/* 관리자 코멘트 */
	state			int,		/* 0: 신규, 1: 처리됨, 2: 신고아님 */
	regdate			datetime
);

