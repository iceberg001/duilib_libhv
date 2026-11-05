#include "netclient.hpp"
#include "libhv/HttpMessage.h"
#include "libhv/hsocket.h"
#include "libhv/hlog.h"
#include <atomic>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#endif

// ==================== HTTP 同步请求 ====================

requests::Response NetClient::HttpGet(const std::string& url, const http_headers& headers /*= DefaultHeaders*/)
{
	return requests::get(url.c_str(), headers);
}

requests::Response NetClient::HttpPost(const std::string& url, const http_body& body /*= NoBody*/, const http_headers& headers /*= DefaultHeaders*/)
{
	return requests::post(url.c_str(), body, headers);
}

requests::Response NetClient::HttpPut(const std::string& url, const http_body& body /*= NoBody*/, const http_headers& headers /*= DefaultHeaders*/)
{
	return requests::put(url.c_str(), body, headers);
}

requests::Response NetClient::HttpDelete(const std::string& url, const http_body& body /*= NoBody*/, const http_headers& headers /*= DefaultHeaders*/)
{
	return requests::Delete(url.c_str(), body, headers);
}

requests::Response NetClient::HttpPatch(const std::string& url, const http_body& body /*= NoBody*/, const http_headers& headers /*= DefaultHeaders*/)
{
	return requests::patch(url.c_str(), body, headers);
}

requests::Response NetClient::HttpSend(http_method method, const std::string& url, const http_body& body /*= NoBody*/, const http_headers& headers /*= DefaultHeaders*/)
{
	return requests::request(method, url.c_str(), body, headers);
}

// ==================== HTTP 异步请求 ====================

int NetClient::HttpSendAsync(http_method method, const std::string& url, const http_body& body, const http_headers& headers, requests::ResponseCallback resp_cb)
{
	requests::Request req(new ::HttpRequest);
	req->method = method;
	req->url = url;
	if (&body != &NoBody)
	{
		req->body = body;
	}
	if (&headers != &DefaultHeaders)
	{
		req->headers = headers;
	}
	return requests::async(req, resp_cb);
}

int NetClient::HttpGetAsync(const std::string& url, requests::ResponseCallback resp_cb)
{
	return HttpSendAsync(HTTP_GET, url, NoBody, DefaultHeaders, resp_cb);
}

int NetClient::HttpPostAsync(const std::string& url, const http_body& body, requests::ResponseCallback resp_cb)
{
	return HttpSendAsync(HTTP_POST, url, body, DefaultHeaders, resp_cb);
}

// ==================== TCP 短链接同步调用 ====================

int NetClient::TcpSyncRequest(const std::string& host, int port, const char* send_data, int send_len, char* recv_buf, int recv_buf_size, int timeout_ms /*= 5000*/)
{
	if (host.empty() || port <= 0 || !send_data || send_len <= 0 || !recv_buf || recv_buf_size <= 0)
	{
		return -1;
	}

#ifdef _WIN32
	WSADATA wsaData;
	static bool wsa_init = false;
	if (!wsa_init)
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			return -2;
		}
		wsa_init = true;
	}
#endif

	sockaddr_u peeraddr;
	memset(&peeraddr, 0, sizeof(peeraddr));
	int ret = sockaddr_set_ipport(&peeraddr, host.c_str(), port);
	if (ret != 0)
	{
		struct hostent* he = gethostbyname(host.c_str());
		if (!he)
		{
			printf("NetClient::TcpSyncRequest resolve host %s failed\n", host.c_str());
			return -3;
		}
		peeraddr.sin.sin_family = AF_INET;
		peeraddr.sin.sin_addr = *(struct in_addr*)he->h_addr;
		peeraddr.sin.sin_port = htons(port);
	}

	int sock = (int)socket(peeraddr.sa.sa_family, SOCK_STREAM, 0);
	if (sock < 0)
	{
		printf("NetClient::TcpSyncRequest create socket failed\n");
		return -4;
	}

	if (timeout_ms > 0)
	{
#ifdef _WIN32
		DWORD timeout_val = (DWORD)timeout_ms;
		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout_val, sizeof(timeout_val));
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_val, sizeof(timeout_val));
#else
		struct timeval tv;
		tv.tv_sec = timeout_ms / 1000;
		tv.tv_usec = (timeout_ms % 1000) * 1000;
		setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
		setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
	}

	if (connect(sock, &peeraddr.sa, SOCKADDR_LEN(&peeraddr)) < 0)
	{
		printf("NetClient::TcpSyncRequest connect %s:%d failed\n", host.c_str(), port);
#ifdef _WIN32
		closesocket(sock);
#else
		close(sock);
#endif
		return -5;
	}

	int total_sent = 0;
	while (total_sent < send_len)
	{
		int sent = (int)send(sock, send_data + total_sent, send_len - total_sent, 0);
		if (sent <= 0)
		{
			printf("NetClient::TcpSyncRequest send failed\n");
#ifdef _WIN32
			closesocket(sock);
#else
			close(sock);
#endif
			return -6;
		}
		total_sent += sent;
	}

	int total_recv = 0;
	while (total_recv < recv_buf_size)
	{
		int recvd = (int)recv(sock, recv_buf + total_recv, recv_buf_size - total_recv, 0);
		if (recvd <= 0)
		{
			break;
		}
		total_recv += recvd;

		if (total_recv > 0)
		{
			break;
		}
	}

#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif

	return total_recv;
}

// ==================== TCP 短链接异步调用 ====================

int NetClient::TcpAsyncRequest(const std::string& host, int port, const char* send_data, int send_len,
	std::function<void(const char* data, int length)> callback, int timeout_ms /*= 5000*/)
{
	if (host.empty() || port <= 0 || !send_data || send_len <= 0 || !callback)
	{
		return -1;
	}

	auto tcp_client = std::make_shared<hv::TcpClient>();
	tcp_client->setConnectTimeout(timeout_ms);

	auto send_buf = std::make_shared<std::string>(send_data, send_len);
	auto done = std::make_shared<std::atomic<bool>>(false);
	auto recv_data = std::make_shared<std::string>();

	tcp_client->onConnection = [tcp_client, callback, done, recv_data, send_buf, timeout_ms](const hv::SocketChannelPtr& channel) {
		if (channel->isConnected())
		{
			channel->write(send_buf->data(), send_buf->size());

			tcp_client->loop()->setTimeout(timeout_ms, [tcp_client, callback, done](hv::TimerID tid) {
				if (!done->exchange(true))
				{
					callback(nullptr, -2);
					std::thread([tcp_client]() {
						tcp_client->onConnection = nullptr;
						tcp_client->onMessage = nullptr;
						tcp_client->stop(true);
					}).detach();
				}
			});
		}
		else if (channel->isClosed())
		{
			if (!done->exchange(true))
			{
				if (recv_data->empty())
					callback(nullptr, -1);
				else
					callback(recv_data->data(), (int)recv_data->size());

				std::thread([tcp_client]() {
					tcp_client->onConnection = nullptr;
					tcp_client->onMessage = nullptr;
					tcp_client->stop(true);
				}).detach();
			}
		}
	};

	tcp_client->onMessage = [recv_data](const hv::SocketChannelPtr& channel, hv::Buffer* buffer) {
		if (!buffer || buffer->size() <= 0) return;
		recv_data->append((const char*)buffer->data(), buffer->size());
	};

	int fd = tcp_client->createsocket(port, host.c_str());
	if (fd < 0)
	{
		return -2;
	}

	tcp_client->start(false);

	return 0;
}

// ==================== TCP 长连接客户端 ====================

NetClient::TcpLongClient::TcpLongClient(const std::string& host, int port)
	: m_host(host)
	, m_port(port)
{
	m_tcp_client = std::make_shared<hv::TcpClient>();
}

NetClient::TcpLongClient::~TcpLongClient()
{
	Stop();
}

bool NetClient::TcpLongClient::Start()
{
	int fd = m_tcp_client->createsocket(m_port, m_host.c_str());
	if (fd < 0)
	{
		printf("TcpLongClient::Start createsocket %s:%d failed\n", m_host.c_str(), m_port);
		return false;
	}

	m_tcp_client->onConnection = [this](const hv::SocketChannelPtr& channel) {
		if (channel->isConnected())
		{
			printf("TcpLongClient connected to %s:%d\n", m_host.c_str(), m_port);
			if (m_on_connect) m_on_connect();
		}
		else if (channel->isClosed())
		{
			printf("TcpLongClient disconnected from %s:%d\n", m_host.c_str(), m_port);
			if (m_on_disconnect) m_on_disconnect();
		}
	};

	m_tcp_client->onMessage = [this](const hv::SocketChannelPtr& channel, hv::Buffer* buffer) {
		if (!buffer || buffer->size() <= 0) return;
		if (m_on_message) m_on_message((const char*)buffer->data(), (int)buffer->size());
	};

	m_tcp_client->start(true);

	return true;
}

void NetClient::TcpLongClient::Stop()
{
	m_tcp_client->stop(true);
}

int NetClient::TcpLongClient::Send(const char* data, int length)
{
	return m_tcp_client->send(data, length);
}

int NetClient::TcpLongClient::Send(const std::string& data)
{
	return m_tcp_client->send(data);
}

bool NetClient::TcpLongClient::IsConnected() const
{
	return m_tcp_client->isConnected();
}

void NetClient::TcpLongClient::SetOnConnect(ConnectCallback cb)
{
	m_on_connect = cb;
}

void NetClient::TcpLongClient::SetOnMessage(MessageCallback cb)
{
	m_on_message = cb;
}

void NetClient::TcpLongClient::SetOnDisconnect(DisconnectCallback cb)
{
	m_on_disconnect = cb;
}

void NetClient::TcpLongClient::SetConnectTimeout(int ms)
{
	m_tcp_client->setConnectTimeout(ms);
}

void NetClient::TcpLongClient::EnableReconnect(uint32_t min_delay_ms /*= 1000*/, uint32_t max_delay_ms /*= 60000*/, uint32_t max_retry_cnt /*= INFINITE*/)
{
	hv::ReconnectInfo info;
	info.min_delay = min_delay_ms;
	info.max_delay = max_delay_ms;
	info.max_retry_cnt = max_retry_cnt;
	m_tcp_client->setReconnect(&info);
}

void NetClient::TcpLongClient::WithTLS(const char* cert_file /*= nullptr*/, const char* key_file /*= nullptr*/)
{
	m_tcp_client->withTLS(cert_file, key_file, false);
}

void NetClient::TcpLongClient::SetUnpack(unpack_setting_t* setting)
{
	m_tcp_client->setUnpack(setting);
}

std::shared_ptr<NetClient::TcpLongClient> NetClient::CreateTcpLongClient(const std::string& host, int port)
{
	return std::make_shared<TcpLongClient>(host, port);
}

// ==================== UDP 客户端 ====================

NetClient::UdpClientWrap::UdpClientWrap(const std::string& host, int port)
	: m_host(host)
	, m_port(port)
{
	m_udp_client = std::make_shared<hv::UdpClient>();
}

NetClient::UdpClientWrap::~UdpClientWrap()
{
	Stop();
}

bool NetClient::UdpClientWrap::Start()
{
	int fd = m_udp_client->createsocket(m_port, m_host.c_str());
	if (fd < 0)
	{
		printf("UdpClientWrap::Start createsocket %s:%d failed\n", m_host.c_str(), m_port);
		return false;
	}

	m_udp_client->onMessage = [this](const hv::SocketChannelPtr& channel, hv::Buffer* buffer) {
		if (!buffer || buffer->size() <= 0) return;
		if (m_on_message) m_on_message((const char*)buffer->data(), (int)buffer->size());
	};

	m_udp_client->start(true);

	return true;
}

void NetClient::UdpClientWrap::Stop()
{
	m_udp_client->stop(true);
}

int NetClient::UdpClientWrap::SendTo(const char* data, int length)
{
	return m_udp_client->sendto(data, length);
}

int NetClient::UdpClientWrap::SendTo(const std::string& data)
{
	return m_udp_client->sendto(data);
}

void NetClient::UdpClientWrap::SetOnMessage(MessageCallback cb)
{
	m_on_message = cb;
}

std::shared_ptr<NetClient::UdpClientWrap> NetClient::CreateUdpClient(const std::string& host, int port)
{
	return std::make_shared<UdpClientWrap>(host, port);
}