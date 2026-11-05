#ifndef __UDP_MODULE_HPP__
#define __UDP_MODULE_HPP__

#include <string>
#include <vector>
#include <mutex>
#include "libhv/hv.h"
#include "libhv/hloop.h"
#include "libhv/Channel.h"
#include "libhv/UdpServer.h"
#include "servercommon/networkmodule/iudpmodule.hpp"

class IUdpCallback;

struct UdpModuleInitData
{
	UdpModuleInitData() : worker_num(1) {}

	int worker_num;
};

class UdpModule : public IUdpModule
{
public:
	typedef std::shared_ptr<hv::UdpServer> UdpServerPtr;
	typedef std::weak_ptr<IUdpCallback> CallbackPtr;
	typedef std::vector<CallbackPtr> CallbackList;

public:
	UdpModule();
	virtual ~UdpModule();

	virtual int Init();
	virtual int Start();
	virtual int Stop();
	virtual int Release();
	virtual int Update();

	virtual void RegisterCallback(std::shared_ptr<IUdpCallback> callback);

	virtual bool Listen(unsigned short port, const char* host = "0.0.0.0");

	virtual int SendTo(const char* peer_ip, unsigned short peer_port, const char* data, int length);

	virtual void OnEventMessage(const ModuleEventMsg& msg) {}

	void OnMessage(const hv::SocketChannelPtr& channel, hv::Buffer* buffer);

private:
	void CallbackOnRecv(const char* peer_ip, unsigned short peer_port, const char* data, int length);

	UdpModuleInitData m_init_data;
	UdpServerPtr m_udp_server;
	CallbackList m_callback_list;
	std::mutex m_send_mutex;
};

#endif