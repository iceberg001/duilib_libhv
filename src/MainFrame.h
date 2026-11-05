#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "UIlib.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include "servercommon/networkmodule/netclient.hpp"
#include "UpgradeClient.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

using namespace DuiLib;

#define WM_NET_RESULT (WM_USER + 100)

struct NetResultMsg {
    std::string text;
};

class MainFrame : public WindowImplBase {
    DUI_DECLARE_MESSAGE_MAP()
public:
    virtual CDuiString GetSkinFile() override { return _T("skin.xml"); }
    virtual LPCTSTR GetWindowClassName() const override { return _T("NetAPIDebuggerWnd"); }

    virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) override;
    virtual void OnClick(TNotifyUI& msg) override;

private:
    void OnTabChanged(TNotifyUI& msg);

    void DoSendHttp();
    void DoSendTcp();
    void DoSendUdp();
    void DoCheckUpgrade();

    void AppendResponse(const std::string& text);
    void HandleNetResult(WPARAM wParam);

    std::string GetCtrlText(const char* name);
    void SetCtrlText(const char* name, const std::string& text);
    void AppendCtrlText(const char* name, const std::string& text);

    std::shared_ptr<NetClient::TcpLongClient> m_tcpLongClient;
    std::mutex m_responseMutex;
};
