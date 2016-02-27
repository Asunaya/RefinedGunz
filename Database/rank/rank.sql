use "GunzDB"
go

ALTER TABLE Character
ADD md1point int

ALTER TABLE Character
ADD md2point int

ALTER TABLE Character
ADD md3point int

ALTER TABLE Character
ADD md4point int

ALTER TABLE Character
ADD md5point int

ALTER TABLE Character
ADD daypoint int

ALTER TABLE Character
ADD weekpoint int

ALTER TABLE Character
ADD monthpoint int

go

-- default 값 설정, 0으로 세팅
UPDATE Character SET
md1point=0, md2point=0, md3point=0, md4point=0, md5point=0, daypoint=0,
weekpoint=0, monthpoint=0

go


CREATE TABLE TotalRanking
(
	Rank		int IDENTITY NOT NULL,
	UserID	varchar(20),
	Name		varchar(24) NOT NULL,
	Level		smallint NOT NULL,
	XP		int,
	KillCount	int,
	DeathCount	int
)
ALTER TABLE TotalRanking 
	ADD CONSTRAINT PK_TotalRanking_Rank	PRIMARY KEY (Rank)
go

CREATE INDEX INDEX_TotalRanking_name ON TotalRanking (Name)
go

CREATE INDEX INDEX_TotalRanking_UserID ON TotalRanking (UserID)
go



CREATE TABLE ModeRanking
(
	id		int IDENTITY NOT NULL,
	Name		varchar(24) NOT NULL,
	Level		smallint NOT NULL,
	md1point	int,
	md2point	int,
	md3point	int,
	md4point	int,
	md5point	int,
	md1rank		int,
	md2rank		int,
	md3rank		int,
	md4rank		int,
	md5rank		int
)
ALTER TABLE ModeRanking 
	ADD CONSTRAINT PK_ModeRanking_ID	PRIMARY KEY (id)
go

CREATE INDEX INDEX_ModeRanking_md1rank ON ModeRanking (md1rank)
go
CREATE INDEX INDEX_ModeRanking_md2rank ON ModeRanking (md2rank)
go
CREATE INDEX INDEX_ModeRanking_md3rank ON ModeRanking (md3rank)
go
CREATE INDEX INDEX_ModeRanking_md4rank ON ModeRanking (md4rank)
go
CREATE INDEX INDEX_ModeRanking_md5rank ON ModeRanking (md4rank)
go
CREATE INDEX INDEX_ModeRanking_Name ON ModeRanking (Name)
go


CREATE TABLE DayRanking
(
	id		int IDENTITY NOT NULL,
	Name		varchar(24) NOT NULL,
	Level		smallint NOT NULL,
	Point		int,
	Rank		int
)
ALTER TABLE DayRanking ADD
	CONSTRAINT PK_DayRanking_ID	PRIMARY KEY (id)
go

CREATE INDEX INDEX_DayRanking_Rank ON DayRanking (Rank)
go


CREATE TABLE WeekRanking
(
	id		int IDENTITY NOT NULL,
	Name		varchar(24) NOT NULL,
	Level		smallint NOT NULL,
	Point		int,
	Rank		int
)
ALTER TABLE WeekRanking ADD
	CONSTRAINT PK_WeekRanking_ID	PRIMARY KEY (id)
go

CREATE INDEX INDEX_WeekRanking_Rank ON WeekRanking (Rank)
go


CREATE TABLE MonthRanking
(
	id		int IDENTITY NOT NULL,
	Name		varchar(24) NOT NULL,
	Level		smallint NOT NULL,
	Point		int,
	Rank		int
)
ALTER TABLE MonthRanking ADD
	CONSTRAINT PK_MonthRanking_ID	PRIMARY KEY (id)
go

CREATE INDEX INDEX_MonthRanking_Rank ON MonthRanking (Rank)
go




CREATE TABLE MonthHonorRanking
(
	id		int IDENTITY NOT NULL,
	Name		varchar(24) NOT NULL,
	Level		smallint NOT NULL,
	Point		int,
	Rank		int,
	Date		DATETIME,
)
ALTER TABLE MonthHonorRanking ADD
	CONSTRAINT PK_MonthHonorRanking_ID	PRIMARY KEY (id)
go

CREATE INDEX INDEX_MonthHonorRanking_date_rank 
	ON MonthHonorRanking (Date, Rank)
go

