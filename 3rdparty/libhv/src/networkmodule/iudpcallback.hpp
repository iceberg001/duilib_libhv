#ifndef __I_UDP_CALLBACK_HPP__
#define __I_UDP_CALLBACK_HPP__

#include "servercommon/imodule.hpp"

class IUdpCallback
{
public:
	virtual ~IUdpCallback() {}

	void SetCallbackModule(std::shared_ptr<IModule> module_ptr)
	{
		m_module_ptr = module_ptr;
	}

	/*
	 当 UDP 收到数据时回调该函数
	 @peer_ip		对端地址，点分十进制
	 @peer_port		对端端口号
	 @data			网络数据
	 @length		数据长度
	*/
	virtual void OnUdpRecv(const char* peer_ip, unsigned short peer_port, const char* data, int length) = 0;

	hv::EventLoopPtr GetEventLoopPtr() const
	{
		auto module_ptr = m_module_ptr.lock();
		if (module_ptr)
		{
			return module_ptr->GetEventLoop();
		}
		return nullptr;
	}

protected:
	std::weak_ptr<IModule> m_module_ptr;
};

#endif