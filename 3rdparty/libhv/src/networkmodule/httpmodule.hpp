#ifndef __HTTP_MODULE_HPP__
#define __HTTP_MODULE_HPP__

#include <string>
#include "libhv/hv.h"
#include "libhv/HttpServer.h"
#include "libhv/HttpService.h"
#include "servercommon/networkmodule/ihttpmodule.hpp"

class HttpModule : public IHttpModule
{
public:
	HttpModule();
	virtual ~HttpModule();

	virtual int Init();
	virtual int Start();
	virtual int Stop();
	virtual int Release();
	virtual int Update();

	virtual bool Listen(int port, const char* host = "0.0.0.0");

	virtual void RegisterHandler(const char* method, const char* path, http_sync_handler handler);
	virtual void RegisterAsyncHandler(const char* method, const char* path, http_async_handler handler);
	virtual void SetDocumentRoot(const char* document_root);
	virtual void SetHttps(int https_port, const char* cert_file, const char* key_file);

	virtual void OnEventMessage(const ModuleEventMsg& msg) {}

private:
	hv::HttpServer m_http_server;
	hv::HttpService m_http_service;

	int m_port;
	int m_https_port;
	std::string m_cert_file;
	std::string m_key_file;
	bool m_service_registered;
};

#endif