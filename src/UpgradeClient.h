#pragma once
#include <string>
#include <functional>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

struct UpgradeParams {
    std::string url;
    std::string guid;
    std::string uid;
};

struct UpgradeResult {
    bool success = false;
    std::string error;
    int wupRet = -1;
    int rspRet = -1;
    bool hasNewVersion = false;
    std::string downURL;
    unsigned int packageSize = 0;
    std::string packageMd5;
    std::string title;
    std::string upgradeInfo;
    long long verCode = 0;
    std::string verName;
    int upgradeType = 0;
    std::string defaultUrl;
};

#define WM_UPGRADE_RESULT (WM_USER + 200)

class UpgradeClient {
public:
    using Callback = std::function<void(const UpgradeResult&)>;

    static void CheckUpgrade(HWND mainWnd, const UpgradeParams& params, Callback callback);
    static void HandleResult(WPARAM wParam);
};