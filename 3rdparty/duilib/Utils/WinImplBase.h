#ifndef WIN_IMPL_BASE_HPP
#define WIN_IMPL_BASE_HPP

namespace DuiLib
{
	class UILIB_API WindowImplBase
		: public CWindowWnd
		, public CNotifyPump
		, public INotifyUI
		, public IMessageFilterUI
		, public IDialogBuilderCallback
		, public IQueryControlText
	{
	public:
		WindowImplBase() : m_PaintManager(m_pm) {};
		virtual ~WindowImplBase(){};
		// ֻ����������д����ʼ����Դ������Խӿڣ�
		virtual void InitResource(){};
		// ÿ�����ڶ�������д
		virtual void InitWindow(){};
		virtual void OnFinalMessage( HWND hWnd );
		virtual void Notify(TNotifyUI& msg);

		DUI_DECLARE_MESSAGE_MAP()
		virtual void OnClick(TNotifyUI& msg);
		virtual BOOL IsInStaticControl(CControlUI *pControl);

	protected:
		virtual CDuiString GetSkinType() { return _T(""); }
		virtual CDuiString GetSkinFile() = 0;
		virtual LPCTSTR GetWindowClassName(void) const = 0 ;
		virtual LPCTSTR GetManagerName() { return NULL; }
		virtual LRESULT ResponseDefaultKeyEvent(WPARAM wParam);
		CPaintManagerUI m_pm;

	public:
		CPaintManagerUI& m_PaintManager;

	public:
		virtual UINT GetClassStyle() const;
		virtual CControlUI* CreateControl(LPCTSTR pstrClass);
		virtual LPCTSTR QueryControlText(LPCTSTR lpstrId, LPCTSTR lpstrType);

		virtual LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM /*lParam*/, bool& /*bHandled*/);
		virtual LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);

#if defined(WIN32) && !defined(UNDER_CE)
		virtual LRESULT OnNcActivate(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnNcPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
		virtual LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
#endif
		virtual LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnChar(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnKillFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnSetFocus(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnLButtonDown(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled);
		virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
		virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		virtual LONG GetStyle();

		bool IsAddTray() { return m_trayIcon.Enabled(); }
		void AddTrayIcon(UINT nIconID, LPCTSTR pstrTip) { m_trayIcon.CreateTrayIcon(m_hWnd, nIconID, pstrTip); }
		void RemoveTrayIcon() { m_trayIcon.DeleteTrayIcon(); }
		void SetForeground() {
			if (::IsIconic(m_hWnd) || !::IsWindowVisible(m_hWnd))
				::ShowWindow(m_hWnd, SW_RESTORE);
			HWND hFore = ::GetForegroundWindow();
			DWORD dwForeID = ::GetWindowThreadProcessId(hFore, NULL);
			DWORD dwCurID = ::GetCurrentThreadId();
			if (dwForeID != dwCurID) {
				::AttachThreadInput(dwCurID, dwForeID, TRUE);
				::SetForegroundWindow(m_hWnd);
				::BringWindowToTop(m_hWnd);
				::AttachThreadInput(dwCurID, dwForeID, FALSE);
			} else {
				::SetForegroundWindow(m_hWnd);
			}
		}
		HWND GetForeground() { return ::GetForegroundWindow(); }

	protected:
		CTrayIcon m_trayIcon;
	};
}

#endif // WIN_IMPL_BASE_HPP
