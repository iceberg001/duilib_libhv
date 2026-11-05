#ifndef __NET_CLIENT_HPP__
#define __NET_CLIENT_HPP__

#include <string>
#include <functional>
#include <memory>
#include "libhv/hv.h"
#include "libhv/http_client.h"
#include "libhv/requests.h"
#include "libhv/TcpClient.h"
#include "libhv/UdpClient.h"
#include "libhv/HttpMessage.h"
#include "libhv/HttpService.h"

/*
 NetClient: 客户端网络工具类
 提供以下能力：
 1. HTTP 同步请求（Get/Post/Put/Delete/Patch）
 2. HTTP 异步请求（带回调）
 3. TCP 短链接同步调用（连接→发送→接收→关闭，阻塞当前线程）
 4. TCP 长连接客户端（异步连接 + 回调，支持自动重连）
 5. UDP 客户端（异步收发）
*/
class NetClient
{
public:
	// ==================== HTTP 同步请求 ====================

	/*
	 同步 HTTP GET 请求
	 @url			请求 URL，如 "http://127.0.0.1:8080/ping"
	 @headers		自定义请求头（可选）
	 @Return		HttpResponsePtr，失败返回 nullptr
	*/
	static requests::Response HttpGet(const std::string& url, const http_headers& headers = DefaultHeaders);

	/*
	 同步 HTTP POST 请求
	 @url			请求 URL
	 @body			请求体
	 @headers		自定义请求头（可选）
	 @Return		HttpResponsePtr，失败返回 nullptr
	*/
	static requests::Response HttpPost(const std::string& url, const http_body& body = NoBody, const http_headers& headers = DefaultHeaders);

	/*
	 同步 HTTP PUT 请求
	*/
	static requests::Response HttpPut(const std::string& url, const http_body& body = NoBody, const http_headers& headers = DefaultHeaders);

	/*
	 同步 HTTP DELETE 请求
	*/
	static requests::Response HttpDelete(const std::string& url, const http_body& body = NoBody, const http_headers& headers = DefaultHeaders);

	/*
	 同步 HTTP PATCH 请求
	*/
	static requests::Response HttpPatch(const std::string& url, const http_body& body = NoBody, const http_headers& headers = DefaultHeaders);

	/*
	 同步通用 HTTP 请求
	 @method		HTTP 方法
	 @url			请求 URL
	 @body			请求体
	 @headers		请求头
	 @Return		HttpResponsePtr，失败返回 nullptr
	*/
	static requests::Response HttpSend(http_method method, const std::string& url, const http_body& body = NoBody, const http_headers& headers = DefaultHeaders);

	// ==================== HTTP 异步请求 ====================

	/*
	 异步 HTTP 请求
	 @method		HTTP 方法
	 @url			请求 URL
	 @body			请求体
	 @headers		请求头
	 @resp_cb		响应回调
	 @Return		0 成功，非 0 失败
	*/
	static int HttpSendAsync(http_method method, const std::string& url, const http_body& body, const http_headers& headers, requests::ResponseCallback resp_cb);

	/*
	 异步 HTTP GET 请求
	*/
	static int HttpGetAsync(const std::string& url, requests::ResponseCallback resp_cb);

	/*
	 异步 HTTP POST 请求
	*/
	static int HttpPostAsync(const std::string& url, const http_body& body, requests::ResponseCallback resp_cb);

	// ==================== TCP 短链接同步调用 ====================

	/*
	 TCP 短链接同步调用：连接→发送→接收→关闭，阻塞当前线程
	 @host			目标主机
	 @port			目标端口
	 @send_data		待发送数据
	 @send_len		发送数据长度
	 @recv_buf		接收缓冲区
	 @recv_buf_size	接收缓冲区大小
	 @timeout_ms	超时时间（毫秒），<=0 表示阻塞无超时
	 @Return		实际接收字节数，<0 表示失败
	*/
	static int TcpSyncRequest(const std::string& host, int port, const char* send_data, int send_len, char* recv_buf, int recv_buf_size, int timeout_ms = 5000);

	/*
	 TCP 短链接异步调用：连接→发送→接收→关闭，不阻塞当前线程
	 @host			目标主机
	 @port			目标端口
	 @send_data		待发送数据
	 @send_len		发送数据长度
	 @callback		接收完成回调，回调在内部 EventLoop 线程执行
	                @data		接收到的数据（回调结束后自动释放，不要持有）
	                @length		接收长度，<0 表示失败
	 @timeout_ms	超时时间（毫秒）
	 @Return		0 表示已发起异步请求，<0 表示发起失败

	 注意：回调在异步线程执行，如需在主线程处理请自行投递
	*/
	static int TcpAsyncRequest(const std::string& host, int port, const char* send_data, int send_len,
		std::function<void(const char* data, int length)> callback, int timeout_ms = 5000);

	// ==================== TCP 长连接客户端 ====================

	/*
	 TCP 长连接客户端封装
	 用法：
	   auto client = NetClient::CreateTcpLongClient("127.0.0.1", 9000);
	   client->SetOnConnect([]{ ... });
	   client->SetOnMessage([](const char* data, int len){ ... });
	   client->SetOnDisconnect([]{ ... });
	   client->Start();
	   client->Send(data, len);
*/
	class TcpLongClient
	{
	public:
		typedef std::function<void()> ConnectCallback;
		typedef std::function<void(const char* data, int length)> MessageCallback;
		typedef std::function<void()> DisconnectCallback;

		TcpLongClient(const std::string& host, int port);
		~TcpLongClient();

		bool Start();
		void Stop();

		int Send(const char* data, int length);
		int Send(const std::string& data);

		bool IsConnected() const;

		void SetOnConnect(ConnectCallback cb);
		void SetOnMessage(MessageCallback cb);
		void SetOnDisconnect(DisconnectCallback cb);

		void SetConnectTimeout(int ms);
		void EnableReconnect(uint32_t min_delay_ms = 1000, uint32_t max_delay_ms = 60000, uint32_t max_retry_cnt = 0xFFFFFFFF);
		void WithTLS(const char* cert_file = nullptr, const char* key_file = nullptr);
		void SetUnpack(unpack_setting_t* setting);

	private:
		std::shared_ptr<hv::TcpClient> m_tcp_client;
		std::string m_host;
		int m_port;

		ConnectCallback m_on_connect;
		MessageCallback m_on_message;
		DisconnectCallback m_on_disconnect;
	};

	static std::shared_ptr<TcpLongClient> CreateTcpLongClient(const std::string& host, int port);

	// ==================== UDP 客户端 ====================

	/*
	 UDP 客户端封装
	 用法：
	   auto client = NetClient::CreateUdpClient("127.0.0.1", 9000);
	   client->SetOnMessage([](const char* data, int len){ ... });
	   client->Start();
	   client->SendTo(data, len);
*/
	class UdpClientWrap
	{
	public:
		typedef std::function<void(const char* data, int length)> MessageCallback;

		UdpClientWrap(const std::string& host, int port);
		~UdpClientWrap();

		bool Start();
		void Stop();

		int SendTo(const char* data, int length);
		int SendTo(const std::string& data);

		void SetOnMessage(MessageCallback cb);

	private:
		std::shared_ptr<hv::UdpClient> m_udp_client;
		std::string m_host;
		int m_port;

		MessageCallback m_on_message;
	};

	static std::shared_ptr<UdpClientWrap> CreateUdpClient(const std::string& host, int port);
};

#endif