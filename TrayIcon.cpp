/*
* Copyright (c) Istvan Pasztor
* This source has been published on www.codeproject.com under the CPOL license.
*/
#include "stdafx.h"
#include "TrayIcon.h"
#include <thread>

#define TRAY_WINDOW_MESSAGE (WM_USER+100)


namespace
{
	// A map that never holds allocated memory when it is empty. This map will be created with placement new as a static variable,
	// and its destructor will be never called, and it shouldn't leak memory if it contains no items at program exit.
	// This dirty trick is useful when you create your trayicon object as static. In this case we can not control the
	// order of destruction of this map object and the static trayicon object. However this dirty trick ensures that
	// the map is freed exactly when the destructor of the last static trayicon is unregistering itself.
	class CIdToTrayIconMap
	{
	public:
		typedef UINT KeyType;
		typedef CTrayIcon* ValueType;

		// typedef didn't work with VC++6
		struct StdMap : public std::map<KeyType,ValueType> {};
		typedef StdMap::iterator iterator;

		CIdToTrayIconMap() : m_Empty(true) {}
		ValueType& operator[](KeyType k)
		{
			return GetOrCreateStdMap()[k];
		}
		ValueType* find(KeyType k)
		{
			if (m_Empty)
				return false;
			StdMap::iterator it = GetStdMap().find(k);
			if (it == GetStdMap().end())
				return NULL;
			return &it->second;
		}
		int erase(KeyType k)
		{
			if (m_Empty)
				return 0;
			StdMap& m = GetStdMap();
			int res = (int)m.erase(k);
			if (m.empty())
			{
				m.~StdMap();
				m_Empty = true;
			}
			return res;
		}
		bool empty() const
		{
			return m_Empty;
		}
		// Call this only when the container is not empty!!!
		iterator begin()
		{
			assert(!m_Empty);	// Call this only when the container is not empty!!!
			return m_Empty ? iterator() : GetStdMap().begin();
		}
		// Call this only when the container is not empty!!!
		iterator end()
		{
			assert(!m_Empty);	// Call this only when the container is not empty!!!
			return m_Empty ? iterator() : GetStdMap().end();
		}

	private:
		StdMap &GetStdMap()
		{
			assert(!m_Empty);
			return (StdMap&)m_MapBuffer;
		}
		StdMap &GetOrCreateStdMap()
		{
			if (m_Empty)
			{
				new ((void*)&m_MapBuffer) StdMap();
				m_Empty = false;
			}
			return (StdMap&)m_MapBuffer;
		}
	private:
		bool m_Empty;
		char m_MapBuffer[sizeof(StdMap)];
	};

	static CIdToTrayIconMap& GetIdToTrayIconMap()
	{
		// This hack prevents running the destructor of our map, so it isn't problem if someone tries to reach this from a static destructor.
		// Because of using MyMap this will not cause a memory leak if the user removes all items from the container before exiting.
		static char id_to_tray_icon_buffer[sizeof(CIdToTrayIconMap)];
		static bool initialized = false;
		if (!initialized)
		{
			initialized = true;
			new ((void*)id_to_tray_icon_buffer) CIdToTrayIconMap();
		}
		return (CIdToTrayIconMap&)id_to_tray_icon_buffer;
	}

	static UINT GetNextTrayIconId()
	{
		static UINT next_id = 1;
		return next_id++;
	}
}


static const UINT g_WndMsgTaskbarCreated = RegisterWindowMessage(TEXT("TaskbarCreated"));
LRESULT CALLBACK CTrayIcon::MessageProcessorWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == TRAY_WINDOW_MESSAGE)
	{
		if (CTrayIcon** ppIcon = GetIdToTrayIconMap().find((UINT)wParam))
			(*ppIcon)->OnMessage((UINT)lParam);
		return 0;
	}
	else if (uMsg == g_WndMsgTaskbarCreated)
	{
		CIdToTrayIconMap &id_to_tray = GetIdToTrayIconMap();
		if (!id_to_tray.empty())
		{
			for (std::map<UINT,CTrayIcon*>::const_iterator it=id_to_tray.begin(),eit=id_to_tray.end(); it!=eit; ++it)
			{
				CTrayIcon *pTrayIcon = it->second;
				pTrayIcon->OnTaskbarCreated();
			}
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND CTrayIcon::GetMessageProcessorHWND()
{
	static HWND hWnd = NULL;
	if (!hWnd)
	{
		static const TCHAR TRAY_ICON_MESSAGE_PROCESSOR_WND_CLASSNAME[] = TEXT("TRAY_ICON_MESSAGE_PROCESSOR_WND_CLASS");
		HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);

		WNDCLASSEX wc;
		wc.cbSize = sizeof(wc);
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wc.hIconSm = NULL;
		wc.hInstance = hInstance;
		wc.lpfnWndProc = MessageProcessorWndProc;
		wc.lpszClassName = TRAY_ICON_MESSAGE_PROCESSOR_WND_CLASSNAME;
		wc.lpszMenuName = NULL;
		wc.style = 0;
		if (!RegisterClassEx(&wc))
			return NULL;

		hWnd = CreateWindowEx(
			0,
			TRAY_ICON_MESSAGE_PROCESSOR_WND_CLASSNAME,
			TEXT("TRAY_ICON_MESSAGE_PROCESSOR_WND"),
			WS_POPUP,
			0, 0, 0, 0,
			NULL,
			NULL,
			hInstance,
			NULL
			);
	}
	return hWnd;
}

CTrayIcon::CTrayIcon(const char* name, bool visible, HICON hIcon, bool destroy_icon_in_destructor)
: m_Id(GetNextTrayIconId())
, m_Name(name)
, m_hIcon(hIcon)
, m_Visible(false)
, m_DestroyIconInDestructor(destroy_icon_in_destructor)
, m_pOnMessageFunc(NULL)
, m_pListener(NULL)
{
	GetIdToTrayIconMap()[m_Id] = this;
	SetVisible(visible);
}

CTrayIcon::~CTrayIcon()
{
	SetVisible(false);
	SetIcon(NULL, m_DestroyIconInDestructor);
	GetIdToTrayIconMap().erase(m_Id);
}

HICON CTrayIcon::InternalGetIcon() const
{
	return m_hIcon ? m_hIcon : ::LoadIcon(NULL, IDI_APPLICATION);
}

bool CTrayIcon::AddIcon()
{
	NOTIFYICONDATAA data;
	FillNotifyIconData(data);
	data.uFlags |= NIF_MESSAGE|NIF_ICON|NIF_TIP;
	data.uCallbackMessage = TRAY_WINDOW_MESSAGE;
	data.hIcon = InternalGetIcon();

	size_t tip_len = max(sizeof(data.szTip)-1, strlen(m_Name.c_str()));
	memcpy(data.szTip, m_Name.c_str(), tip_len);
	data.szTip[tip_len] = 0;

	return FALSE != Shell_NotifyIconA(NIM_ADD, &data);
}

bool CTrayIcon::RemoveIcon()
{
	NOTIFYICONDATAA data;
	FillNotifyIconData(data);
	return FALSE != Shell_NotifyIconA(NIM_DELETE, &data);
}

void CTrayIcon::OnTaskbarCreated()
{
	if (m_Visible)
		AddIcon();
}

void CTrayIcon::SetName(const char* name)
{
	m_Name = name;
	if (m_Visible)
	{
		NOTIFYICONDATAA data;
		FillNotifyIconData(data);
		data.uFlags |= NIF_TIP;

		size_t tip_len = max(sizeof(data.szTip)-1, strlen(name));
		memcpy(data.szTip, name, tip_len);
		data.szTip[tip_len] = 0;

		Shell_NotifyIconA(NIM_MODIFY, &data);
	}
}

bool CTrayIcon::SetVisible(bool visible)
{
	if (m_Visible == visible)
		return true;
	m_Visible = visible;
	if (m_Visible)
		return AddIcon();
	return RemoveIcon();
}

void CTrayIcon::SetIcon(HICON hNewIcon, bool destroy_current_icon)
{
	if (m_hIcon == hNewIcon)
		return;
	if (destroy_current_icon && m_hIcon)
		DestroyIcon(m_hIcon);
	m_hIcon = hNewIcon;

	if (m_Visible)
	{
		NOTIFYICONDATAA data;
		FillNotifyIconData(data);
		data.uFlags |= NIF_ICON;
		data.hIcon = InternalGetIcon();
		Shell_NotifyIconA(NIM_MODIFY, &data);
	}
}

bool CTrayIcon::ShowBalloonTooltip(const char* title, const char* msg, ETooltipIcon icon)
{
#ifndef NOTIFYICONDATA_V2_SIZE
	return false;
#else
	if (!m_Visible)
		return false;

	NOTIFYICONDATAA data;
	FillNotifyIconData(data);
	data.cbSize = NOTIFYICONDATAA_V2_SIZE;	// win2k and later
	data.uFlags |= NIF_INFO;
	data.dwInfoFlags = icon;
	data.uTimeout = 10000;	// deprecated as of Windows Vista, it has a min(10000) and max(30000) value on previous Windows versions.
	
	strcpy_s(data.szInfoTitle, title);
	strcpy_s(data.szInfo, msg);
	
	return FALSE != Shell_NotifyIconA(NIM_MODIFY, &data);
#endif
}

void CTrayIcon::OnMessage(UINT uMsg)
{
	if (m_pOnMessageFunc)
		m_pOnMessageFunc(this, uMsg);
	if (m_pListener)
		m_pListener->OnTrayIconMessage(this, uMsg);
}

void CTrayIcon::FillNotifyIconData(NOTIFYICONDATAA& data)
{
	memset(&data, 0, sizeof(data));
	// the basic functions need only V1
#ifdef NOTIFYICONDATA_V1_SIZE
	data.cbSize = NOTIFYICONDATA_V1_SIZE;
#else
	data.cbSize = sizeof(data);
#endif
	data.hWnd = GetMessageProcessorHWND();
	assert(data.hWnd);
	data.uID = m_Id;
}

LRESULT CALLBACK pWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

CTrayIcon* g_tray = nullptr;
std::vector<CTrayIconMenuItem> g_menuItems;
nbind::cbFunction* g_OnMenuItem = nullptr;

typedef void (*MenuItemCallback)(uv_async_t * handle) ;

void callAsync(MenuItemCallback fn, void * data)
{
	uv_async_t * async = new uv_async_t();
	uv_async_init(uv_default_loop(), async, fn);
	async->data = data;
	uv_async_send(async);
}

void callMenuItemCallback(uv_async_t * handle)
{
	Nan::HandleScope scope;
	char* id = (char*)handle->data;
	(*g_OnMenuItem)(std::string(id));
	free(id);
}


void menu()
{
	static const TCHAR* class_name = TEXT("MCE_HWND_MESSAGE");
	WNDCLASSEX wx = {};
	wx.cbSize = sizeof(WNDCLASSEX);
	wx.lpfnWndProc = pWndProc;        // function which will handle messages
	wx.hInstance = 0;
	wx.lpszClassName = class_name;
	RegisterClassEx(&wx);
	HWND hwnd = CreateWindowEx(0, class_name, TEXT("MCE_HWND_MESSAGE"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);

	POINT pt;
	if (GetCursorPos(&pt))
	{
		HMENU menu = CreatePopupMenu();
		int i = 0;
		for (auto const& item : g_menuItems){
			AppendMenuA(menu, MF_STRING, i++, item.m_caption.c_str());
		}
		UINT cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
		callAsync(callMenuItemCallback, strdup(g_menuItems[cmd].m_id.c_str()));
	}

	DestroyWindow(hwnd);
}

void TrayOnMessage(CTrayIcon* pTrayIcon, UINT uMsg)
{
	switch (uMsg)
	{
	case WM_LBUTTONUP:
		ShellExecuteA(nullptr, "open", g_menuItems[0].m_id.c_str(), nullptr, nullptr, SW_SHOW);
		break;
	case WM_RBUTTONUP:
		menu();
		break;
	}
}

CTrayIconContainer::CTrayIconContainer()
{
}

void CTrayIconContainer::Stop()
{
	PostThreadMessage(GetThreadId(m_worker->native_handle()), WM_QUIT, NULL, NULL);
	m_worker->join();
	delete m_worker;
	m_worker = nullptr;
}


void CTrayIconContainer::Start(std::string iconPath, std::string title)
{
	m_worker = new std::thread([this, iconPath, title] {
		HICON ico = (HICON)LoadImage(NULL, iconPath.c_str(),IMAGE_ICON,0,0,LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
		g_tray = new CTrayIcon(title.c_str(), true, ico, true);
		g_tray->SetListener(TrayOnMessage);
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return 0;
	});
}

void CTrayIconContainer::AddMenuItem(std::string id, std::string caption)
{
	g_menuItems.push_back(CTrayIconMenuItem(id, caption));
}

void CTrayIconContainer::OnMenuItem(nbind::cbFunction & cb)
{
	g_OnMenuItem = new nbind::cbFunction(cb);
}

void CTrayIconContainer::ShowBalloon(std::string title, std::string text, int timeout, nbind::cbFunction & cb)
{
	g_tray->ShowBalloonTooltip(title.c_str(), text.c_str());
}
