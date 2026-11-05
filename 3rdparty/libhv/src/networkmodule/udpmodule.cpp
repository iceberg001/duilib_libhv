#include "udpmodule.hpp"
#include "libhv/hloop.h"
#include "libhv/hsocket.h"
#include "iudpcallback.hpp"
#include "servercommon/module_manager.hpp"
#include "servercommon/module_def.hpp"
#include <sstream>

UdpModule::UdpModule()
{
	m_udp_server = std::make_shared<hv::UdpServer>();
}

UdpModule::~UdpModule()
{
}

int UdpModule::Init()
{
	m_init_data.worker_num = 1;
	return 0;
}

int UdpModule::Start()
{
	m_udp_server->onMessage = std::bind(&UdpModule::OnMessage, this, std::placeholders::_1, std::placeholders::_2);

	this->GetEventLoop()->run();

	return 0;
}

int UdpModule::Stop()
{
	m_udp_server->stop(true);
	this->GetEventLoop()->stop();
	return 0;
}

int UdpModule::Release()
{
	return 0;
}

int UdpModule::Update()
{
	return 0;
}

void UdpModule::RegisterCallback(std::shared_ptr<IUdpCallback> callback)
{
	for (CallbackPtr cb : m_callback_list)
	{
		if (auto shared_cb = cb.lock())
		{
			if (shared_cb == callback)
			{
				throw std::exception("callback duplicate");
			}
		}
	}

	m_callback_list.push_back(std::shared_ptr<IUdpCallback>(callback));
}

bool UdpModule::Listen(unsigned short port, const char* host /*= "0.0.0.0"*/)
{
	int fd = m_udp_server->createsocket(port, host);
	if (fd < 0)
	{
		printf("UdpModule::Listen createsocket failed port[%d]\n", port);
		return false;
	}

	m_udp_server->start(true);

	printf("UdpModule start listen to port[%d]\n", port);

	return true;
}

int UdpModule::SendTo(const char* peer_ip, unsigned short peer_port, const char* data, int length)
{
	if (!data || length <= 0) return -1;

	sockaddr_u peeraddr;
	memset(&peeraddr, 0, sizeof(peeraddr));
	int ret = sockaddr_set_ipport(&peeraddr, peer_ip, peer_port);
	if (ret != 0)
	{
		printf("UdpModule::SendTo invalid peer %s:%u\n", peer_ip, peer_port);
		return -1;
	}

	std::lock_guard<std::mutex> locker(m_send_mutex);
	return m_udp_server->sendto(data, length, &peeraddr.sa);
}

void UdpModule::OnMessage(const hv::SocketChannelPtr& channel, hv::Buffer* buffer)
{
	if (!buffer || buffer->size() <= 0) return;

	std::string peeraddr_str = channel->peeraddr();

	sockaddr_u* peeraddr = (sockaddr_u*)hio_peeraddr(channel->io());
	char ip_buffer[64] = { 0 };
	unsigned short peer_port = sockaddr_port(peeraddr);
	std::string peer_ip = sockaddr_ip(peeraddr, ip_buffer, sizeof(ip_buffer));

	int length = (int)buffer->size();
	const char* body = (const char*)buffer->data();

	this->CallbackOnRecv(peer_ip.c_str(), peer_port, body, length);
}

void UdpModule::CallbackOnRecv(const char* peer_ip, unsigned short peer_port, const char* data, int length)
{
	if (!data) return;
	if (length <= 0) return;

	for (CallbackPtr cb : m_callback_list)
	{
		auto callback = cb.lock();
		if (!callback)
		{
			continue;
		}

		auto loop = callback->GetEventLoopPtr();
		if (!loop)
		{
			continue;
		}

		char* buffer = new char[length];
		memcpy(buffer, data, length);
		std::string ip_copy(peer_ip);
		loop->runInLoop([=] {
			callback->OnUdpRecv(ip_copy.c_str(), peer_port, buffer, length);
			delete[] buffer;
			});
	}
}