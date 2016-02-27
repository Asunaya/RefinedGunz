#ifndef _SERVER_KEEPER_CONST
#define _SERVER_KEEEPR_CONST

/// Server state
enum CONNECTION_STATE
{
	CNN_START_CHECK					= 0,
	CNN_FAIL_CONNECT,
	CNN_NOT_CONNECTED_SERVER,
	CNN_NOT_CONNECTED_KEEPER,
	CNN_FAIL_CONNECT_KEEPER,
	CNN_CONNECTING_TO_KEEPER,
	CNN_CONNECTED_SERVER,
	CNN_CONNECTING_TO_SERVER,
	CNN_CHECKING_SERVER_HEARTBEAT,

	CNN_END,
};

// for PatchInterface.
static const char* CONNECTION_STATE_LIST[] = { "START CHECK",  
											   "FAIL TO CONNECT", "NOT CONNECTED SERVER", "NOT CONNECTED KEEPER",
											   "FAIL CONNECT KEEPER", "CONNECTING TO KEEPER", "CONNECTED SERVER", "CONNECTING TO SERVER",
											   "CHECKING SERVER HEARTBEAT" };

/// job state
enum JOB_STATE
{
	JOB_NON	= 0,
	JOB_SUCCESS_CONNECT_FTP_SERVER,			JOB_FAIL_CONNECT_FTP_SERVER,
	JOB_SUCCESS_DOWNLOAD_SERVER_PATCH_FILE,	JOB_FAIL_DOWNLOAD_SERVER_PATCH_FILE,
	JOB_SUCCESS_PREPARE_SERVER_PATCH,		JOB_FAIL_PREPARE_SERVER_PATCH,
	JOB_SUCCESS_STOP_AGENT,					JOB_FAIL_STOP_AGENT,
	JOB_SUCCESS_STOP_SERVER,				JOB_FAIL_STOP_SERVER,
	JOB_SUCCESS_PATCH_SERVER,				JOB_FAIL_PATCH_SERVER,	
	JOB_SUCCESS_START_SERVER,				JOB_FAIL_START_SERVER,
	JOB_SUCCESS_START_AGENT,				JOB_FAIL_START_AGENT,
	JOB_FAIL_CREATE_SERVER_DOWNLOAD_THREAD,
	JOB_FAIL_CREATE_SERVER_PREPARE_THREAD,
	JOB_FAIL_CREATE_SERVER_PATCH_THREAD,
	JOB_DOWNLOADING_SERVER_PATCH,
	JOB_PREPARING_SERVER_PATCH,
	JOB_PATCHING_SERVER,
	JOB_SUCCESS_DOWNLOAD_AGENT_PATCH_FILE,	JOB_FAIL_DOWNLOAD_AGENT_PATCH_FILE,
	JOB_SUCCESS_PREPARE_AGENT_PATCH,		JOB_FAIL_PREPARE_AGENT_PATCH,
	JOB_SUCCESS_PATCH_AGENT,				JOB_FAIL_PATCH_AGENT,
	JOB_FAIL_CREATE_AGENT_DOWNLOAD_THREAD,
	JOB_FAIL_CREATE_AGENT_PREPARE_THREAD,
	JOB_FAIL_CREATE_AGENT_PATCH_THREAD,
	JOB_DOWNLOADING_AGENT_PATCH,
	JOB_PREPARING_AGENT_PATCH,
	JOB_PATCHING_AGENT,
	JOB_FAIL_REBOOT,
	JOB_ANNOUNCE_STOP_SERVER,
	JOB_SUCCESS_ADD_ANNOUNCE_SCHEDULE,
	JOB_FAIL_ADD_ANNOUNCE_SCHEDULE,
	JOB_SUCCESS_ADD_SERVER_STOP_SCHEDULE,
	JOB_FAIL_ADD_SERVER_STOP_SCHEDULE,
	JOB_SUCCESS_ADD_RESTART_SERVER_SCHEDULE,
	JOB_FAIL_ADD_RESTART_SERVER_SCHEDULE,
	JOB_DISABLE_SERVER_PATCH,
	JOB_DISABLE_AGENT_PATCH,
	JOB_RELOAD_SERVER_CONFIG,
	
	JOB_END,
};

// for PatchInterface.
static const char* JOB_STATE_LIST[] = { "없음", 
										"FTP 접속 성공",		"FTP 접속 실패", 
										"S 패치 받기 성공",		"S 패치 받기 실패",
										"S 패치 준비 성공",		"S 패치 준비 실패", 
										"A 정지 성공",			"A 정시 실패",
										"S 정지 성공",			"S 정지 실패",
										"S 패치 성공",			"S 패치 실패", 
										"S 시작 성공",			"S 시작 실패",
										"A 시작 성공",			"A 시작 실패",
										"S 패치받기 쓰레드 생성 실패",
										"S 패치준비 쓰레드 생성 실패",
										"S 패치 쓰레드 생성 실패",
										"S 패치 파일 받는 중",
										"S 패치 작업 준비 중",
										"S 패치 중",
										"A 패치 받기 성공",		"A 패치 받기 실패",
										"A 패치 준비 성공",		"A 패치 준비 실패",
										"A 패치 성공",			"A 패치 실패",
										"A 패치받기 쓰레드 생성 실패",
										"A 패치준비 쓰레드 생성 실패",
										"A 패치쓰레드 생성 실패",
										"A 패치 받는 중",
										"A 패치 작업 준비 중",
										"A 패치 중",
										"재부팅 실패",
										"공지후 서버 종료",
										"공지 스케쥴 등록 성공",
										"공지 스케쥴 등록 실패", 
										"서버정지 스케줄 등록 성공",
										"서버정지 스케줄 등록 실패",
										"서버재시작 스케줄 등록 성공",
										"서버재시작 스케줄 등록 실패", 
										"S 패치 제외",
										"A 패치 제외",
										"서버 설정파일 리로드.",
										"END OF JOB" };


enum CONFIG_STATE
{
	CONFIG_ERROR = 0,
	CONFIG_NO,
	CONFIG_AUTO_START_SERVER,
	CONFIG_MANUAL_START_SERVER,
	CONFIG_AUTO_START_AGENT,
	CONFIG_MANUAL_START_AGENT,
	CONFIG_OK_DOWNLOAD,
	CONFIG_NO_DOWNLOAD,
	CONFIG_OK_PREPARE,
	CONFIG_NO_PREPARE,
	CONFIG_OK_PATCH,
	CONFIG_NO_PATCH,

	CONFIG_END,
};

static const char* CONFIG_STATE_LIST[] = { "Error",
										   "NO",
										   "자동", "수동", 
										   "자동", "수동", 
										   "OK", "NO", 
										   "OK", "NO", 
										   "OK", "NO" };

enum COLUMN_ID
{
	COLUMN_SERVER_START = 7,
	COLUMN_AGENT_START,
	COLUMN_DOWNLOAD,
	COLUMN_PREPARE,
	COLUMN_PATCH,

	COLUMN_END,
};

enum RUN_STATE
{
	RS_NO = 0,
	RS_RUN,
	RS_STOP,

	RS_END,
};

static const char* RUN_STATE_LIST[] = { "NO", "RUN", "STOP" };

static const char* SERVER_ERR_STATE_LIST[] = { "NO", "DB ERROR" };

#define MAX_ELAPSE_RESPONSE_REQUEST_HEARBEAT (1000 * 30)	// 10초에 한번씩 검사 함.

#endif /// _SERVER_KEEPER_CONST