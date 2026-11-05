#include "MainFrame.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <windows.h>

DUI_BEGIN_MESSAGE_MAP(MainFrame, WindowImplBase)
    DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, MainFrame::OnClick)
    DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED, MainFrame::OnTabChanged)
DUI_END_MESSAGE_MAP()

static std::string WToUtf8(LPCWSTR wstr) {
    if (!wstr) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], len, nullptr, nullptr);
    return result;
}

static std::wstring Utf8ToW(const std::string& str) {
    if (str.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    if (len <= 0) return L"";
    std::wstring result(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], len);
    return result;
}

std::string MainFrame::GetCtrlText(const char* name) {
    auto* ctrl = m_pm.FindControl(CDuiString(Utf8ToW(name).c_str()));
    if (!ctrl) return "";
    return WToUtf8(ctrl->GetText().GetData());
}

void MainFrame::SetCtrlText(const char* name, const std::string& text) {
    auto* ctrl = m_pm.FindControl(CDuiString(Utf8ToW(name).c_str()));
    if (!ctrl) return;
    std::wstring w = Utf8ToW(text);
    ctrl->SetText(w.c_str());
}

void MainFrame::AppendCtrlText(const char* name, const std::string& text) {
    auto* ctrl = m_pm.FindControl(CDuiString(Utf8ToW(name).c_str()));
    if (!ctrl) return;
    std::string current = WToUtf8(ctrl->GetText().GetData());
    current += text;
    std::wstring w = Utf8ToW(current);
    ctrl->SetText(w.c_str());
}

void MainFrame::AppendResponse(const std::string& text) {
    auto* msg = new NetResultMsg();
    msg->text = text;
    PostMessage(WM_NET_RESULT, (WPARAM)msg, 0);
}

void MainFrame::HandleNetResult(WPARAM wParam) {
    auto* msg = reinterpret_cast<NetResultMsg*>(wParam);
    if (msg) {
        AppendCtrlText("response", msg->text);
        delete msg;
    }
}

LRESULT MainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {
    if (uMsg == WM_NET_RESULT) {
        HandleNetResult(wParam);
        bHandled = TRUE;
        return 0;
    }
    if (uMsg == WM_UPGRADE_RESULT) {
        UpgradeClient::HandleResult(wParam);
        bHandled = TRUE;
        return 0;
    }
    if (uMsg == WM_DESTROY) {
        if (m_tcpLongClient) {
            m_tcpLongClient->Stop();
            m_tcpLongClient.reset();
        }
    }
    bHandled = FALSE;
    return 0;
}

void MainFrame::OnClick(TNotifyUI& msg) {
    CDuiString name = msg.pSender->GetName();

    if (name == _T("closebtn")) { Close(); return; }
    if (name == _T("minbtn")) { SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); return; }
    if (name == _T("maxbtn")) { SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); return; }
    if (name == _T("clear_response")) { SetCtrlText("response", ""); return; }
    if (name == _T("copy_response")) {
        std::string text = GetCtrlText("response");
        if (!text.empty() && OpenClipboard(GetHWND())) {
            EmptyClipboard();
            std::wstring w = Utf8ToW(text);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (w.size() + 1) * sizeof(wchar_t));
            if (hMem) {
                wchar_t* pMem = (wchar_t*)GlobalLock(hMem);
                memcpy(pMem, w.c_str(), (w.size() + 1) * sizeof(wchar_t));
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
            }
            CloseClipboard();
        }
        return;
    }
    if (name == _T("http_send")) { DoSendHttp(); return; }
    if (name == _T("tcp_send")) { DoSendTcp(); return; }
    if (name == _T("udp_send")) { DoSendUdp(); return; }
    if (name == _T("check_upgrade")) { DoCheckUpgrade(); return; }
}

void MainFrame::OnTabChanged(TNotifyUI& msg) {
    CDuiString name = msg.pSender->GetName();
    auto* tab = dynamic_cast<CTabLayoutUI*>(m_pm.FindControl(_T("tab_content")));
    if (!tab) return;

    if (name == _T("tab_http")) tab->SelectItem(0);
    else if (name == _T("tab_tcp")) tab->SelectItem(1);
    else if (name == _T("tab_udp")) tab->SelectItem(2);
}

void MainFrame::DoSendHttp() {
    std::string method = GetCtrlText("http_method");
    std::string url = GetCtrlText("http_url");
    std::string headersText = GetCtrlText("http_headers");
    std::string body = GetCtrlText("http_body");

    if (url.empty()) {
        AppendResponse("[Error] URL is empty\n");
        return;
    }

    http_headers headers;
    {
        std::istringstream iss(headersText);
        std::string line;
        while (std::getline(iss, line)) {
            auto pos = line.find(':');
            if (pos == std::string::npos) continue;
            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);
            auto p = val.find_first_not_of(" \t");
            if (p != std::string::npos) val = val.substr(p);
            else val.clear();
            if (!key.empty()) headers[key] = val;
        }
    }

    AppendResponse("[HTTP] " + method + " " + url + " ...\n");

    std::thread([this, method, url, body, headers]() {
        std::string result;
        result += "------ HTTP Response ------\n";

        requests::Request req(new HttpRequest);
        req->url = url;
        req->headers = headers;
        req->timeout = 10;
        if (!body.empty()) req->body = body;

        if (method == "GET")           req->method = HTTP_GET;
        else if (method == "POST")     req->method = HTTP_POST;
        else if (method == "PUT")      req->method = HTTP_PUT;
        else if (method == "DELETE")   req->method = HTTP_DELETE;
        else if (method == "PATCH")    req->method = HTTP_PATCH;
        else { result += "Failed: unsupported method\n"; goto done; }

        {
            auto resp = requests::request(req);
            if (resp) {
                result += "Status: " + std::to_string(resp->status_code) + "\n";
                result += "Body:\n" + std::string(resp->body.c_str(), resp->body.size()) + "\n";
            } else {
                result += "Failed: no response (timeout or error)\n";
            }
        }

    done:
        result += "---------------------------\n\n";
        AppendResponse(result);
    }).detach();
}

void MainFrame::DoSendTcp() {
    std::string host = GetCtrlText("tcp_host");
    std::string portStr = GetCtrlText("tcp_port");
    std::string data = GetCtrlText("tcp_data");

    auto* modeCtrl = m_pm.FindControl(_T("tcp_mode"));
    CDuiString mode = modeCtrl ? modeCtrl->GetText() : _T("");

    if (host.empty() || portStr.empty()) {
        AppendResponse("[Error] Host or Port is empty\n");
        return;
    }

    int port = atoi(portStr.c_str());

    if (mode == _T("短连接同步")) {
        AppendResponse("[TCP] Sync " + host + ":" + portStr + " ...\n");
        std::thread([this, host, port, data]() {
            char recv_buf[4096] = {0};
            int recv_len = NetClient::TcpSyncRequest(host, port,
                data.data(), (int)data.size(), recv_buf, sizeof(recv_buf), 5000);

            std::string result = "------ TCP Sync Response ------\n";
            if (recv_len > 0) {
                result += "Received " + std::to_string(recv_len) + " bytes:\n";
                result += std::string(recv_buf, recv_len) + "\n";
            } else {
                result += "Failed, ret=" + std::to_string(recv_len) + "\n";
            }
            result += "------------------------------\n\n";
            AppendResponse(result);
        }).detach();
    } else if (mode == _T("短连接异步")) {
        AppendResponse("[TCP] Async " + host + ":" + portStr + " ...\n");
        NetClient::TcpAsyncRequest(host, port, data.data(), (int)data.size(),
            [this](const char* resp, int length) {
                std::string result = "------ TCP Async Response ------\n";
                if (length > 0) {
                    result += "Received " + std::to_string(length) + " bytes:\n";
                    result += std::string(resp, length) + "\n";
                } else {
                    result += "Failed, ret=" + std::to_string(length) + "\n";
                }
                result += "-------------------------------\n\n";
                AppendResponse(result);
            }, 5000);
    } else if (mode == _T("长连接")) {
        if (m_tcpLongClient) {
            m_tcpLongClient->Stop();
            m_tcpLongClient.reset();
        }

        AppendResponse("[TCP] Long connection to " + host + ":" + portStr + " ...\n");
        m_tcpLongClient = NetClient::CreateTcpLongClient(host, port);

        m_tcpLongClient->SetOnConnect([this]() {
            AppendResponse("[TCP] Connected\n");
            auto* ctrl = m_pm.FindControl(_T("tcp_status"));
            if (ctrl) ctrl->SetText(_T("状态: 已连接"));
        });
        m_tcpLongClient->SetOnMessage([this](const char* d, int len) {
            std::string result = "------ TCP Long Response ------\n";
            result += "Received " + std::to_string(len) + " bytes:\n";
            result += std::string(d, len) + "\n";
            result += "------------------------------\n\n";
            AppendResponse(result);
        });
        m_tcpLongClient->SetOnDisconnect([this]() {
            AppendResponse("[TCP] Disconnected\n");
            auto* ctrl = m_pm.FindControl(_T("tcp_status"));
            if (ctrl) ctrl->SetText(_T("状态: 已断开"));
        });

        m_tcpLongClient->SetConnectTimeout(3000);
        m_tcpLongClient->EnableReconnect(1000, 60000, 3);
        m_tcpLongClient->Start();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (!data.empty()) {
            m_tcpLongClient->Send(data);
            AppendResponse("[TCP] Sent: " + data + "\n");
        }
    } else {
        AppendResponse("[Error] Unknown TCP mode\n");
    }
}

void MainFrame::DoSendUdp() {
    std::string host = GetCtrlText("udp_host");
    std::string portStr = GetCtrlText("udp_port");
    std::string data = GetCtrlText("udp_data");

    if (host.empty() || portStr.empty()) {
        AppendResponse("[Error] Host or Port is empty\n");
        return;
    }

    int port = atoi(portStr.c_str());
    AppendResponse("[UDP] Send to " + host + ":" + portStr + " ...\n");

    auto udpClient = NetClient::CreateUdpClient(host, port);
    udpClient->SetOnMessage([this](const char* d, int len) {
        std::string result = "------ UDP Response ------\n";
        result += "Received " + std::to_string(len) + " bytes:\n";
        result += std::string(d, len) + "\n";
        result += "-------------------------\n\n";
        AppendResponse(result);
    });

    if (!udpClient->Start()) {
        AppendResponse("[UDP] Start failed\n");
        return;
    }

    int sent = udpClient->SendTo(data);
    AppendResponse("[UDP] Sent " + std::to_string(sent) + " bytes: " + data + "\n");

    std::thread([this, udpClient]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        udpClient->Stop();
    }).detach();
}

void MainFrame::DoCheckUpgrade() {
    std::string upgradeUrl = GetCtrlText("upgrade_url");
    if (upgradeUrl.empty()) {
        AppendResponse("[Error] Upgrade URL is empty\n");
        return;
    }

    UpgradeParams params;
    params.url = upgradeUrl;

    char exePath[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    std::string dir(exePath);
    size_t pos = dir.find_last_of("\\/");
    if (pos != std::string::npos) dir = dir.substr(0, pos + 1);
    std::string iniPath = dir + "config.ini";
    char buf[1024] = {0};
    GetPrivateProfileStringA("Upgrade", "guid", "", buf, sizeof(buf), iniPath.c_str());
    params.guid = buf;
    GetPrivateProfileStringA("Upgrade", "uid", "", buf, sizeof(buf), iniPath.c_str());
    params.uid = buf;

    AppendResponse("[Upgrade] Checking remote upgrade...\n");

    UpgradeClient::CheckUpgrade(GetHWND(), params, [this](const UpgradeResult& r) {
        std::string result = "------ Upgrade Check ------\n";

        if (!r.success && !r.error.empty()) {
            result += "Failed: " + r.error + "\n";
        } else {
            result += "WUP ret: " + std::to_string(r.wupRet) + "\n";
            result += "rsp.ret: " + std::to_string(r.rspRet) + "\n";

            if (r.hasNewVersion) {
                result += "\n=== New Version Available! ===\n";
                result += "downURL: " + r.downURL + "\n";
                result += "packageSize: " + std::to_string(r.packageSize) + "\n";
                result += "packageMd5: " + r.packageMd5 + "\n";
                result += "title: " + r.title + "\n";
                result += "upgradeInfo: " + r.upgradeInfo + "\n";
                result += "verCode: " + std::to_string(r.verCode) + "\n";
                result += "verName: " + r.verName + "\n";
                result += "upgradeType: " + std::to_string(r.upgradeType) + "\n";
                result += "defaultUrl: " + r.defaultUrl + "\n";
            } else if (r.rspRet == 1) {
                result += "No new version available\n";
            } else {
                result += "Error: rsp.ret=" + std::to_string(r.rspRet) + "\n";
            }
        }

        result += "--------------------------\n\n";
        AppendResponse(result);
    });
}
