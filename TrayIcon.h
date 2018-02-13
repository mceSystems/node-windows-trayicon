/*
* Copyright (c) Istvan Pasztor
* This source has been published on www.codeproject.com under the CPOL license.
*/
#ifndef __TRAY_ICON_H__
#define __TRAY_ICON_H__
#pragma once

// NOTE: include the following headers in your stdafx.h: <shellapi.h> <map> <string> <assert.h>
#include <winsock2.h>
#include <Windows.h>
#include <shellapi.h>
#include <string>
#include <map>
#include <assert.h>
#include <TCHAR.H>
#include <thread>
#include "nbind/nbind.h"

struct ITrayIconListener;

// You can use this class either by inheriting from it and overriding the OnMessage() method,
// or by instantiating this class directly and setting its listener object or function.
class CTrayIcon
{
public:
	CTrayIcon(const char* name="tray_icon", bool visible=false, HICON hIcon=NULL, bool destroy_icon_in_destructor=false);
	// destroys the current m_hIcon if set
	virtual ~CTrayIcon();

	virtual void SetName(const char* name);
	const char* GetName() const			{ return m_Name.c_str(); }

	virtual bool SetVisible(bool visible);
	bool IsVisible() const					{ return m_Visible; }

	// The destructor may destroy the specified hIcon. If you want to avoid that, call
	// SetIcon(NULL, false) or SetDestroyIconInDestructor(false).
	virtual void SetIcon(HICON hNewIcon, bool destroy_current_icon=true);
	HICON GetIcon() const						{ return m_hIcon; }

	void SetDestroyIconInDestructor(bool b)	{ m_DestroyIconInDestructor = b; }
	bool GetDestroyIconInDestructor() const { return m_DestroyIconInDestructor; }

	enum ETooltipIcon
	{
		eTI_None,			// NIIF_NONE(0)
		eTI_Info,			// NIIF_INFO(1)
		eTI_Warning,		// NIIF_WARNING(2)
		eTI_Error			// NIIF_ERROR(3)
	};
	// ShowBalloonTooltip() works only on win2k and later
	bool ShowBalloonTooltip(const char* title, const char* msg, ETooltipIcon icon=eTI_None);

	typedef void (*POnMessageFunc)(CTrayIcon* pTrayIcon, UINT uMsg);
	void SetListener(POnMessageFunc pOnMessageFunc) { m_pOnMessageFunc = pOnMessageFunc; }
	void SetListener(ITrayIconListener *pListener) { m_pListener = pListener; }

protected:
	// uMsg can be one of the following window messages:
	// - WM_MOUSEMOVE
	// - WM_LBUTTONDOWN, WM_LBUTTONUP, WM_LBUTTONDBLCLK
	// - WM_RBUTTONDOWN, WM_RBUTTONUP, WM_RBUTTONDBLCLK
	// - WM_MBUTTONDOWN, WM_MBUTTONUP, WM_MBUTTONDBLCLK
	// WinXP and later:
	// - NIN_BALLOONXXX messages (eg.: NIN_BALLOONUSERCLICK)
	//
	// Use GetCursorPos() if you need the location of the cursor.
	// The default implementation calls the listener.
	virtual void OnMessage(UINT uMsg);

private:
	void FillNotifyIconData(NOTIFYICONDATAA& data);
	// Never returns NULL! If GetIcon()==NULL, then this returns a system icon
	HICON InternalGetIcon() const;
	bool AddIcon();
	bool RemoveIcon();
	void OnTaskbarCreated();

private:
	UINT m_Id;
	std::string m_Name;
	bool m_Visible;
	HICON m_hIcon;
	bool m_DestroyIconInDestructor;
	POnMessageFunc m_pOnMessageFunc;
	ITrayIconListener* m_pListener;

	static LRESULT CALLBACK MessageProcessorWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static HWND GetMessageProcessorHWND();
};


//-------------------------------------------------------------------------------------------------


struct ITrayIconListener
{
	virtual void OnTrayIconMouseMove(CTrayIcon* pTrayIcon) {}

	virtual void OnTrayIconLButtonDown(CTrayIcon* pTrayIcon) {}
	virtual void OnTrayIconLButtonUp(CTrayIcon* pTrayIcon) {}
	virtual void OnTrayIconLButtonDblClk(CTrayIcon* pTrayIcon) {}

	virtual void OnTrayIconRButtonDown(CTrayIcon* pTrayIcon) {}
	virtual void OnTrayIconRButtonUp(CTrayIcon* pTrayIcon) {}
	virtual void OnTrayIconRButtonDblClk(CTrayIcon* pTrayIcon) {}

	virtual void OnTrayIconMButtonDown(CTrayIcon* pTrayIcon) {}
	virtual void OnTrayIconMButtonUp(CTrayIcon* pTrayIcon) {}
	virtual void OnTrayIconMButtonDblClk(CTrayIcon* pTrayIcon) {}

	// WinXP and later
	virtual void OnTrayIconSelect(CTrayIcon* pTrayIcon) {}
	virtual void OnTrayIconBalloonShow(CTrayIcon* pTrayIcon) {}
	virtual void OnTrayIconBalloonHide(CTrayIcon* pTrayIcon) {}
	virtual void OnTrayIconBalloonTimeout(CTrayIcon* pTrayIcon) {}
	virtual void OnTrayIconBalloonUserClick(CTrayIcon* pTrayIcon) {}

	// Use GetCursorPos() if you need the location of the cursor.
	virtual void OnTrayIconMessage(CTrayIcon* pTrayIcon, UINT uMsg)
	{
		switch (uMsg)
		{
		case WM_MOUSEMOVE: OnTrayIconMouseMove(pTrayIcon); break;
		case WM_LBUTTONDOWN: OnTrayIconLButtonDown(pTrayIcon); break;
		case WM_LBUTTONUP: OnTrayIconLButtonUp(pTrayIcon); break;
		case WM_LBUTTONDBLCLK: OnTrayIconLButtonDblClk(pTrayIcon); break;
		case WM_RBUTTONDOWN: OnTrayIconRButtonDown(pTrayIcon); break;
		case WM_RBUTTONUP: OnTrayIconRButtonUp(pTrayIcon); break;
		case WM_RBUTTONDBLCLK: OnTrayIconRButtonDblClk(pTrayIcon); break;
		case WM_MBUTTONDOWN: OnTrayIconMButtonDown(pTrayIcon); break;
		case WM_MBUTTONUP: OnTrayIconMButtonUp(pTrayIcon); break;
		case WM_MBUTTONDBLCLK: OnTrayIconMButtonDblClk(pTrayIcon); break;

#ifdef NIN_SELECT
		case NIN_SELECT: OnTrayIconSelect(pTrayIcon); break;
		case NIN_BALLOONSHOW: OnTrayIconBalloonShow(pTrayIcon); break;
		case NIN_BALLOONHIDE: OnTrayIconBalloonHide(pTrayIcon); break;
		case NIN_BALLOONTIMEOUT: OnTrayIconBalloonTimeout(pTrayIcon); break;
		case NIN_BALLOONUSERCLICK: OnTrayIconBalloonUserClick(pTrayIcon); break;
#endif
		}
	}
};


class CTrayIconMenuItem
{
public:
	CTrayIconMenuItem(std::string id, std::string caption):
		m_id(id), m_caption(caption){}

	std::string m_id;
	std::string m_caption;
};

class CTrayIconContainer
{
public:
	CTrayIconContainer();

	void Start(std::string iconPath, std::string title);
	void AddMenuItem(std::string id, std::string caption);
	void OnMenuItem(nbind::cbFunction & cb);
	void ShowBalloon(std::string title, std::string text, int timeout, nbind::cbFunction & cb);
	void Stop();

private:
	std::thread* m_worker;
};

#endif //!__TRAY_ICON_H__
