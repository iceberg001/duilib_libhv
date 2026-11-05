#ifndef TAF_H_
#define TAF_H_

#ifdef TAF_STATICLIB
#define TAF_EXPORT
#else
	#if defined(TAF_IMPLEMENTATION)
	#define TAF_EXPORT __declspec(dllexport)
	#else
	#define TAF_EXPORT __declspec(dllimport)
	#endif
#endif

//#pragma comment(lib, "ws2_32.lib")

//#include <string>
//#include <boost/bind.hpp>
//#include <boost/function.hpp>
#include "wup/base/wup.h"
#include "wup/base/Jce.h"

// HTTP请求代理类型
#define TAF_PROXY_TYPE_NONE		(0)
#define TAF_PROXY_TYPE_HTTP		(1)
#define TAF_PROXY_TYPE_SOCK4	(2)
#define TAF_PROXY_TYPE_SOCK5	(3)

// WUP 请求返回码
#define TAF_WUP_REQUEST_OK			(0)
#define TAF_WUP_REQUEST_FAIL		(1)
#define TAF_WUP_RESPONSE_EMPTY		(2)
#define TAF_WUP_JCE_DECODE_EXP		(3)
#define TAF_WUP_NO_INTERNET			(4)

// PUSH 消息类型
#define TAF_PUSH_MSG_TYPE_NOTIFY		(1)
#define TAF_PUSH_MSG_TYPE_DIALOG		(2)
#define TAF_PUSH_MSG_TYPE_CMD			(3)
#define TAF_PUSH_MSG_TYPE_EXTEND		(4)
#define TAF_PUSH_MSG_TYPE_APP_COMM		(5)

// JCE回包返回码，这里暂时只定义成功类型
// ::wup::UniPacket<> rsp;
// rsp.get<int>("");
#define TAF_WUP_JCE_SERVER_SUCCESS		(0)

// 唯一ID无效标识
#define TAF_INVALID_ID		(-1)

// TAF平台配置相关信息宏定义
#define TAF_COMMON_PLATFORM_NAME		(L"WIN")
#define TAF_COMMON_APP_ID				(L"com.uphummer.client.windows")  // TODO: 只有股票通介入， 暂时定义在此，后面需要提供接口供上层传入
#define TAF_COMMON_MODULE_NAME			(L"hummer")
#define TAF_COMMON_SDK_VERSION			(L"1.0.1")
#define TAF_COMMON_CONFIG_FILE			(L"taf.cfg")				
#define TAF_COMMON_CACHE_FILE			(L"data\\taf.dat")				
//#define TAF_WUP_HTTP_URL_DEFAULT		(L"http://prx.upchina.com:80")
//#define TAF_WUP_HTTP_IP_DEFAULT			(L"http://118.178.30.136:80")
#define	TAF_WUP_GUID_DEFAULT_LEN		(16)
#define TAF_STAT_TOKEN					(L"cc453d26b96b32a0f817907c76d01c9141feb7471f74d4bd")
#define TAF_STAT_TRIGGER_COUNT			(50)
#define TAF_STAT_EXPIRE_DAY				(7)

// WUP login请求协议版本号
#define TAF_WUP_LOGIN_VER_HTTPS		(2)


// TAF接口文件
// ！！！字符串参数必须转UTF8再传入！！！
namespace taf {

namespace wup {

} // namespace wup
namespace statistics {
// TODO: 与业务taf_reporter.h统一管理，上报代理接口可以梳理到framework
namespace bizz_key {
static const char *kLogin = "pc_upgpt_login";
static const char *kClick = "pc_upgpt_click";
static const char *kStockSwitch = "pc_upgpt_stock_switch";
static const char *kKeyboardSearch = "pc_upgpt_keyboard_search";
static const char *kWup = "pc_upgpt_taf_wup";
static const char *kAppUseTime = "APP_USING_TIME";
static const char *kAppAdsLoadErr = "APP_ADS_LOAD_ERROR";
static const char *kAppSaveZxgErr = "APP_SAVE_ZXG_ERROR";
static const char *kSSOLoginErr = "SSO_LOGIN_ERR";
static const char *kHQServerCheckErr = "HQ_SERVER_CHECK_ERR";
static const char *kBlockZipUpdateError = "BLOCK_ZIP_UPDATE_ERROR";
}  // namespace bizz_key
}  // namespace statistics

namespace config {
static const int kConfigChangeTypeGuid = 1;
static const int kConfigChangeTypeUid = 2;
static const int kConfigChangeTypeXua = 3;
}

typedef void (* OnGuidCallback)(const char *uid, const char *guid);

class TAF_EXPORT IConfigManager {
public:	
	//virtual bool SetHomeDirPath(const char *home_dir_path) = 0;
	virtual bool SetFilePath(const char *app_data_path, const char *config_file_path, const char *temp_dir_name) = 0;
	virtual bool SetAppInfo(__int64 ver_code, const char *ver_str, int build_no, const char *ver_type) = 0;
	virtual bool SetChannelNum(int orig_num, int curr_num) = 0;  // 渠道号传0，不生效
	virtual bool SetUserID(const char *uid, OnGuidCallback guid_cb=NULL) = 0;
	virtual bool SetProxyInfo(int proxy_type, const char *ip_port, const char *user_pwd) = 0;
	virtual const char * GetXua() = 0;
	virtual const char * GetDeviceID() = 0;
	virtual const char * GetMac() = 0;
	virtual const char * GetUid() = 0;
	virtual const char * GetGuidStr() = 0;
	virtual const char * GetGuid(int *len) = 0;
	virtual const char * GetWupDefAddr() = 0;
	virtual const char * GetCacheFilePath() = 0;
};

// params: req_id, WUP_REQ_CODE, uniPacketData, uniPacketLen
typedef void (* WupCallBack2)(long, int, const char *uniPacketData, unsigned int dataLen);

class TAF_EXPORT IWupManager {
public:
	// return req_id, fail REQ_ID_INVALID
	virtual long Post(const char *uniPacketData, unsigned int dataLen, WupCallBack2 callback) = 0;
	// relogin
	virtual bool ReLogin() = 0;
};

class TAF_EXPORT IStaticsManager {
public:
	virtual bool EventCount(const char *event_id, const char *event_subid = NULL, bool sync=false) = 0;
	virtual bool EventValue(const char *event_id, long value, const char *event_subid = NULL, bool sync=false) = 0;
	//virtual bool TimeStart(const std::string &bid, const std::string &key) = 0;
	//virtual bool TimeStop(const std::string &bid, const std::string &key) = 0;
	//virtual bool Avg(const std::string &bid, const std::string &key, long value, const std::string &name) = 0;
	//virtual bool Count(const std::string &bid, const std::string &key, const std::string &name) = 0;
};

//typedef void (* PushCallback)(int msg_type, const char *data, unsigned int length);

//************************************
// Method:    PushCallback2
// FullName:  taf::PushCallback2
// Access:    public 
// Returns:   void
// Qualifier: (const char *data, unsigned int length)
// Parameter: 
// 1. const char *data----Push::PushRsp jce buffer
// 2. unsigned int length----buffer length
//************************************
typedef void (* PushCallback2)(const char *data, unsigned int length);
class TAF_EXPORT IPushManager {
public:
	virtual bool SetCallback(PushCallback2 callback) = 0;
	virtual bool Start(const char *uid) = 0;
	virtual bool Stop() = 0;
	virtual bool SendXua() = 0;
	virtual bool SetCookie(const char *cookie, unsigned int length) = 0;
	virtual bool SendFeedback(const char *app_id, int msg_id, int ret) = 0;
};


typedef enum enum_ConnStatus
{
	eConnConnected = 0,				//连接
	eConnConnecting = 1,			//正在连接
	eConnDisConnect = 2,			//连接断开
	//eConnClose = 3,				//连接关闭
}EMConnStatusType;

typedef enum enum_ProxyType {
	eProxyNone = 0,					//无代理
	eProxy_HTTP = 1,				//http代理
	eProxy_SOCKS4 = 2,				//socket4代理
	eProxy_SOCKS5 = 3,				//socket5代理
	eProxy_BROWSER = 4,				//使用浏览器代理
}EMProxyType;


typedef void (* ConnectionRecvDataCallback)(long conn_id, const char *data, unsigned int length);
typedef void (* ConnectionSendDataCallback)(long conn_id, long req_id, bool succ);
typedef void (* ConnectionStatusChangeCallback)(long conn_id, EMConnStatusType connStatus);
//typedef void (* ConnectionCloseCallback)(long conn_id);
typedef void (* ConnectionHeartbeatCallback)(long conn_id);

class TAF_EXPORT IConnectionManager {
public:
	virtual bool Open(long *conn_id) = 0;
	virtual bool SetRecvDataCallback(long conn_id, ConnectionRecvDataCallback recv_data_callback) = 0;
	virtual bool SetSendDataCallback(long conn_id, ConnectionSendDataCallback send_data_callback) = 0;
	virtual bool SetConnStatusChangeCallback(long conn_id, ConnectionStatusChangeCallback conn_callback) = 0;
	//virtual bool SetCloseCallback(long conn_id, ConnectionCloseCallback close_callback) = 0;
	virtual bool SetHeartbeatCallback(long conn_id, ConnectionHeartbeatCallback heartbeat_callback) = 0;
	virtual bool Connected(long conn_id, const char *addr) = 0;
	virtual long Send(long conn_id, const char *buff, unsigned int length) = 0;   // fail ret<0 
	virtual bool Close(long conn_id) = 0;
	virtual bool SetAutoReconnect(long conn_id, bool autoRecon) = 0;
	virtual bool EnableHeartBeat(long conn_id,bool enableHeartBeat) = 0;
	virtual void SetProxyInfo(long conn_id, EMProxyType emProxyType, const char * szProxyAddr,
		const char *szProxyUser = NULL, const char *szProxyPwd = NULL) = 0;
	virtual bool IsConnConnected(long conn_id) = 0;
	virtual bool RefreshConnect(long conn_id, const char *addr) = 0;
	virtual bool IsMaxReconnectTimes(long conn_id) = 0;
	virtual bool SetReconnectTimes(long conn_id, int reconnectTimes) = 0;
};

// 异步HTTP请求
class IHttpClient;
typedef void(*HttpCallback)(IHttpClient *http_client, bool suc);
typedef void(*ProgressCallback)(IHttpClient *http_client, __int64 total_size, __int64 download_size);
class TAF_EXPORT IHttpClient {
public:
	virtual bool SetAdditionalRequestHeaders(const char *headers) = 0;
	virtual bool SetPostData(const char *data, unsigned int size) = 0;
	virtual bool SendHttpRequest(bool is_post = false, bool disable_auto_redirect = false) = 0;
	
	virtual int GetResponseStatusCode(void) = 0;
	virtual const char * GetResponseHeaders() = 0;
	virtual const char * GetResponseContent(void) = 0;
	
	virtual bool SetDownloadFilePath(const char *path) = 0;  // 路径编码utf-8
	virtual bool SetProgressCallback(ProgressCallback callback) = 0;
	virtual bool DownloadFile() = 0;
};

class TAF_EXPORT IHttpManager {
public:
	virtual IHttpClient *Open(const char *url) = 0;
	virtual void Close(IHttpClient *http_client) = 0;
	virtual bool SendRequest(IHttpClient * http_client, HttpCallback callback) = 0;
	virtual bool DownloadFile(IHttpClient *http_client, HttpCallback callback) = 0;
};

class TAF_EXPORT TAFFactory {
public:
	// 进程启动时调用，如CWinApp::InitInstance
	// 初始化各manager
	virtual bool Init() = 0;
	// 进程退出时调用，如CWinApp::ExitInstance
	// 反初始化各manager
	virtual bool UnInit() = 0;
	virtual IConfigManager *GetConfigManager() = 0;
	virtual IWupManager *GetWupManager() = 0;
	virtual IStaticsManager *GetStaticsManager() = 0;
	virtual IPushManager *GetPushManager() = 0;
	virtual IConnectionManager *GetConnectionManager() = 0;
	virtual IHttpManager *GetHttpManager() = 0;
};

// 主线程调用，非多线程安全
TAF_EXPORT TAFFactory *GetFactory();
TAF_EXPORT void DestoryFactory();

}


// ========================================================================
// up_live client patch: minimal TC_Common shim.
// The jce generator emits taf::TC_Common::equal(...) for float/double
// default-value checks, but this slim runtime ships no util layer.
// Semantics aligned with taf util/tc_common.cpp (fabs(x-y) < epsilon).
// NOTE: re-apply this block if the jce package is ever re-extracted.
// ========================================================================
#include <cmath>

namespace taf
{
    struct TC_Common
    {
        static bool equal(double x, double y, double epsilon = 1e-10)
        {
            return fabs(x - y) < epsilon;
        }
        static bool equal(float x, float y, float epsilon = 1e-10f)
        {
            return fabsf(x - y) < epsilon;
        }
        static bool equal(float x, double y, double epsilon = 1e-10)
        {
            return fabs(x - y) < epsilon;
        }
        static bool equal(double x, float y, double epsilon = 1e-10)
        {
            return fabs(x - y) < epsilon;
        }
        template<typename T>
        static bool equal(const T& x, const T& y)
        {
            return x == y;
        }
    };
}

#endif  // TAF_H_
