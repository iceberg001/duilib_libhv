#include "UpgradeClient.h"
#include "servercommon/networkmodule/netclient.hpp"
#include <thread>
#include <iphlpapi.h>
#include <sstream>

#pragma comment(lib, "iphlpapi.lib")

#define TAF_STATICLIB
#include "jce/plugin/taf.h"
#include "jce/Upgrade.h"

struct UpgradeTaskContext {
    UpgradeClient::Callback callback;
    UpgradeResult result;
};

static std::string GetLocalMac() {
    std::string mac;
    ULONG bufLen = sizeof(IP_ADAPTER_INFO);
    auto* adapterInfo = (IP_ADAPTER_INFO*)malloc(bufLen);
    if (GetAdaptersInfo(adapterInfo, &bufLen) != ERROR_SUCCESS) {
        free(adapterInfo);
        adapterInfo = (IP_ADAPTER_INFO*)malloc(bufLen);
    }
    if (GetAdaptersInfo(adapterInfo, &bufLen) == NO_ERROR) {
        auto* p = adapterInfo;
        while (p) {
            if (strstr(p->Description, "PCI") || p->Type == 71) {
                char hex[3] = {0};
                for (UINT i = 0; i < p->AddressLength; i++) {
                    sprintf(hex, "%02X", p->Address[i]);
                    mac += hex;
                    if (i != p->AddressLength - 1) mac += ":";
                }
                break;
            }
            p = p->Next;
        }
    }
    free(adapterInfo);
    return mac;
}

static std::string BuildXua() {
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    OSVERSIONINFOEX osv = {};
    osv.dwOSVersionInfoSize = sizeof(osv);
    GetVersionEx((OSVERSIONINFO*)&osv);

    char xua[512] = {0};
    sprintf(xua,
        "SN=WIN_com.upgpt.client.windows&VN=0_0_1_GA&RL=%d_%d&OS=%lu.%lu.%lu&CHID=4001_4001&MN=stockpc&SDK=1.0.1&MO=&VC=&RV=",
        sw, sh,
        osv.dwMajorVersion, osv.dwMinorVersion, osv.dwBuildNumber);
    return xua;
}

void UpgradeClient::CheckUpgrade(HWND mainWnd, const UpgradeParams& params, Callback callback) {
    std::thread([mainWnd, params, callback = std::move(callback)]() {
        UpgradeResult result;

        if (params.url.empty()) {
            result.error = "URL is empty";
            auto* ctx = new UpgradeTaskContext;
            ctx->callback = std::move(callback);
            ctx->result = std::move(result);
            PostMessage(mainWnd, WM_UPGRADE_RESULT, (WPARAM)ctx, 0);
            return;
        }

        try {
            std::string mac = GetLocalMac();
            std::string xua = BuildXua();

            std::string sendBuff;
            ::wup::UniPacket<> uniPacket;
            Base::CheckUpgradeReq req;

            req.baseInfo.guid = params.guid;
            req.baseInfo.xua = xua;
            req.baseInfo.deviceID = mac;
            req.baseInfo.mac = mac;
            req.baseInfo.uid = params.uid;

            uniPacket.setServantName("upgrade");
            uniPacket.setFuncName("checkWinPC");
            uniPacket.put<Base::CheckUpgradeReq>("req", req);
            uniPacket.encode(sendBuff);

            if (sendBuff.empty()) {
                result.error = "WUP encode empty";
                goto done;
            }

            {
                requests::Request httpReq(new HttpRequest);
                httpReq->method = HTTP_POST;
                httpReq->url = params.url;
                httpReq->body = sendBuff;
                httpReq->timeout = 10;
                httpReq->headers["Content-Type"] = "application/octet-stream";

                auto resp = requests::request(httpReq);
                if (!resp) {
                    result.error = "no response (timeout or error)";
                    goto done;
                }

                if (resp->body.empty()) {
                    result.error = "empty response body";
                    goto done;
                }

                ::wup::UniPacket<> rspPacket;
                rspPacket.decode(resp->body.c_str(), resp->body.size());
                result.wupRet = rspPacket.get<int>("");

                if (result.wupRet != 0) {
                    result.error = "WUP ret != 0";
                    goto done;
                }

                Base::CheckUpgradeRsp upRsp = rspPacket.get<Base::CheckUpgradeRsp>("rsp");
                result.rspRet = upRsp.ret;
                result.success = true;

                if (upRsp.ret == Base::EUEC_SUCC) {
                    result.hasNewVersion = true;
                    result.downURL = upRsp.downURL;
                    result.packageSize = upRsp.packageSize;
                    result.packageMd5 = upRsp.packageMd5;
                    result.title = upRsp.title;
                    result.upgradeInfo = upRsp.upgradeInfo;
                    result.verCode = upRsp.verCode;
                    result.verName = upRsp.verName;
                    result.upgradeType = (int)upRsp.upgradeType;
                    result.defaultUrl = upRsp.defaultUrl;
                }
            }
        } catch (const std::exception& e) {
            result.error = std::string("Exception: ") + e.what();
        }

    done:
        auto* ctx = new UpgradeTaskContext;
        ctx->callback = std::move(callback);
        ctx->result = std::move(result);
        PostMessage(mainWnd, WM_UPGRADE_RESULT, (WPARAM)ctx, 0);
    }).detach();
}

void UpgradeClient::HandleResult(WPARAM wParam) {
    auto* ctx = reinterpret_cast<UpgradeTaskContext*>(wParam);
    if (ctx) {
        if (ctx->callback) ctx->callback(ctx->result);
        delete ctx;
    }
}