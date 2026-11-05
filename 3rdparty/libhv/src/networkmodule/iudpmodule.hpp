#ifndef __I_UDP_MODULE_HPP__
#define __I_UDP_MODULE_HPP__

#include "servercommon/imodule.hpp"

class IUdpCallback;
class IUdpModule : public IModule
{
public:
	virtual int Init() = 0;
	virtual int Start() = 0;
	virtual int Stop() = 0;
	virtual int Release() = 0;
	virtual int Update() = 0;
	virtual void OnEventMessage(const ModuleEventMsg& msg) = 0;

	/*
	 注册 UDP 消息回调
	 @callback	UDP 回调
	*/
	virtual void RegisterCallback(std::shared_ptr<IUdpCallback> callback) = 0;

	/*
	 开始监听 UDP 端口
	 @port		监听端口
	 @host		监听的 host
	 @Return	是否成功监听
	*/
	virtual bool Listen(unsigned short port, const char* host = "0.0.0.0") = 0;

	/*
	 向指定对端发送 UDP 数据
	 @peer_ip	对端地址，点分十进制
	 @peer_port	对端端口号
	 @data		待发送数据
	 @length	数据长度
	 @Return	实际发送字节数，<0 表示失败
	*/
	virtual int SendTo(const char* peer_ip, unsigned short peer_port, const char* data, int length) = 0;
};

#endif