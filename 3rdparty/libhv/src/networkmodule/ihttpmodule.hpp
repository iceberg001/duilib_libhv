#ifndef __I_HTTP_MODULE_HPP__
#define __I_HTTP_MODULE_HPP__

#include "servercommon/imodule.hpp"
#include "libhv/HttpService.h"

class IHttpModule : public IModule
{
public:
	virtual int Init() = 0;
	virtual int Start() = 0;
	virtual int Stop() = 0;
	virtual int Release() = 0;
	virtual int Update() = 0;
	virtual void OnEventMessage(const ModuleEventMsg& msg) = 0;

	/*
	 开始监听 HTTP 端口
	 @port		监听端口
	 @host		监听的 host
	 @Return	是否成功监听
	*/
	virtual bool Listen(int port, const char* host = "0.0.0.0") = 0;

	/*
	 注册同步 HTTP 路由处理器
	 @method	HTTP 方法，如 "GET"/"POST"/"PUT"/"DELETE"/"PATCH"
	 @path		路由路径，如 "/ping"
	 @handler	同步处理器，返回 HTTP 状态码
	*/
	virtual void RegisterHandler(const char* method, const char* path, http_sync_handler handler) = 0;

	/*
	 注册异步 HTTP 路由处理器
	 @method	HTTP 方法
	 @path		路由路径
	 @handler	异步处理器，通过 HttpResponseWriter 异步写响应
	*/
	virtual void RegisterAsyncHandler(const char* method, const char* path, http_async_handler handler) = 0;

	/*
	 设置静态文件服务根目录
	 @document_root		静态文件根目录
	*/
	virtual void SetDocumentRoot(const char* document_root) = 0;

	/*
	 设置 HTTPS 端口（启用 TLS）
	 @https_port		HTTPS 端口
	 @cert_file			证书文件路径
	 @key_file			私钥文件路径
	*/
	virtual void SetHttps(int https_port, const char* cert_file, const char* key_file) = 0;
};

#endif