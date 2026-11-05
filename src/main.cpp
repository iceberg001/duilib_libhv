#include "MainFrame.h"
#include "resource.h"
#include "UIlib.h"
#include <windows.h>
#include <string>

using namespace DuiLib;

static std::string ExtractEmbeddedZip() {
    HRSRC hRsrc = FindResource(nullptr, MAKEINTRESOURCE(IDR_RESOURCES_ZIP), RT_RCDATA);
    if (!hRsrc) return "";

    DWORD size = SizeofResource(nullptr, hRsrc);
    HGLOBAL hGlobal = LoadResource(nullptr, hRsrc);
    if (!hGlobal) return "";

    void *data = LockResource(hGlobal);
    if (!data || size == 0) return "";

    char tempPath[MAX_PATH] = {0};
    GetTempPathA(MAX_PATH, tempPath);
    std::string zipPath = std::string(tempPath) + "net_api_test_res.zip";

    FILE *fp = fopen(zipPath.c_str(), "wb");
    if (!fp) return "";
    fwrite(data, 1, size, fp);
    fclose(fp);

    return zipPath;
}

#if defined(_MSC_VER) && defined(NDEBUG)
#pragma comment(linker,"/subsystem:windows")
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR cmdLine, int show) {
#else
int main() {
#endif
    std::string zipPath = ExtractEmbeddedZip();
    if (!zipPath.empty()) {
        int wlen = MultiByteToWideChar(CP_UTF8, 0, zipPath.c_str(), -1, nullptr, 0);
        std::wstring wzip(wlen - 1, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, zipPath.c_str(), -1, &wzip[0], wlen);
        CPaintManagerUI::SetResourceZip(wzip.c_str());
    } else {
        CPaintManagerUI::SetResourcePath(
            (CPaintManagerUI::GetCurrentPath() + _T("\\res")).GetData());
    }

    MainFrame mainFrame;
    mainFrame.Create(nullptr, _T("NetAPI Debugger"),
        UI_WNDSTYLE_FRAME, 0, 0, 0, 960, 720);

    HICON hIcon = LoadIconA(GetModuleHandleA(nullptr), MAKEINTRESOURCEA(IDI_APP_ICON));
    if (hIcon) {
        SendMessage((HWND)mainFrame, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage((HWND)mainFrame, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
    }

    RECT rc;
    GetWindowRect((HWND)mainFrame, &rc);
    int cx = rc.right - rc.left;
    int cy = rc.bottom - rc.top;
    int sx = (GetSystemMetrics(SM_CXSCREEN) - cx) / 2;
    int sy = (GetSystemMetrics(SM_CYSCREEN) - cy) / 2;
    SetWindowPos((HWND)mainFrame, nullptr, sx, sy, cx, cy, SWP_NOZORDER);

    ::ShowWindow((HWND)mainFrame, SW_SHOW);
    CPaintManagerUI::MessageLoop();
    return 0;
}
