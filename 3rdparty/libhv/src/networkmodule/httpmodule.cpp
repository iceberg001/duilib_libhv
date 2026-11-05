#include "httpmodule.hpp"
#include "libhv/hlog.h"

HttpModule::HttpModule()
{
	m_port = 0;
	m_https_port = 0;
	m_service_registered = false;
}

HttpModule::~HttpModule()
{
}

int HttpModule::Init()
{
	m_http_server.setThreadNum(4);
	return 0;
}

int HttpModule::Start()
{
	if (!m_service_registered)
	{
		m_http_server.registerHttpService(&m_http_service);
		m_service_registered = true;
	}

	if (m_https_port > 0 && !m_cert_file.empty() && !m_key_file.empty())
	{
		m_http_server.setPort(m_port, m_https_port);
	}
	else
	{
		m_http_server.setPort(m_port);
	}

	if (m_port <= 0 && m_https_port <= 0)
	{
		printf("HttpModule::Start no port configured\n");
		return -1;
	}

	m_http_server.start();

	this->GetEventLoop()->run();

	return 0;
}

int HttpModule::Stop()
{
	m_http_server.stop();
	this->GetEventLoop()->stop();
	return 0;
}

int HttpModule::Release()
{
	return 0;
}

int HttpModule::Update()
{
	return 0;
}

bool HttpModule::Listen(int port, const char* host /*= "0.0.0.0"*/)
{
	if (port <= 0)
	{
		printf("HttpModule::Listen invalid port[%d]\n", port);
		return false;
	}

	m_port = port;
	m_http_server.setHost(host);

	printf("HttpModule will listen to port[%d]\n", port);

	return true;
}

void HttpModule::RegisterHandler(const char* method, const char* path, http_sync_handler handler)
{
	if (!method || !path || !handler) return;

	m_http_service.Handle(method, path, handler);
	printf("HttpModule register handler %s %s\n", method, path);
}

void HttpModule::RegisterAsyncHandler(const char* method, const char* path, http_async_handler handler)
{
	if (!method || !path || !handler) return;

	m_http_service.Handle(method, path, handler);
	printf("HttpModule register async handler %s %s\n", method, path);
}

void HttpModule::SetDocumentRoot(const char* document_root)
{
	if (document_root)
	{
		m_http_service.document_root = document_root;
	}
}

void HttpModule::SetHttps(int https_port, const char* cert_file, const char* key_file)
{
	if (https_port <= 0 || !cert_file || !key_file) return;

	m_https_port = https_port;
	m_cert_file = cert_file;
	m_key_file = key_file;
}