//////////////////////////////////////////////////////////////////////////
// Name:	    PictureBox Header
// Version:		1.0
// Date:	    24-7-2004
// Author:	    Trilobyte
// Copyright:   (c) Trilobyte-Solutions 2004-2008
// Desciption:
// Defines CPictureBox and CMemDC in the WTL namespace
//////////////////////////////////////////////////////////////////////////
#ifndef __PICTUREBOX_H
#define __PICTUREBOX_H

#ifndef __ATLCRACK_H__
#error please include atlcrack.h first
#endif

#ifndef __ATLCTRLS_H__
#include <atlctrls.h>
#endif

#ifndef __ATLSCRL_H__
#include <atlscrl.h>
#endif

#ifndef __ATLMISC_H__
#define _ATL_TMP_NO_CSTRING
#include <atlmisc.h>
#endif

#ifndef __ATLIMAGE_H_
#define __ATLTYPES_H__	// Use the WTL types
#include <atlimage.h>
#endif

namespace WTL
{
	// CMemDC is an assistant for drawing
#ifndef _MEMDC
#define _MEMDC
	class CMemDC: public CDC
	{
	public:
		CDCHandle     m_hOwnerDC;	// Owner DC
		CBitmap       m_bitmap;		// Offscreen bitmap
		CBitmapHandle m_hOldBitmap;	// Originally selected bitmap
		RECT          m_rcOwner;	// Rectangle of drawing area

		CMemDC(HDC hDC, LPRECT pRect = NULL)
		{
			ATLASSERT(hDC!=NULL);
			m_hOwnerDC = hDC;
			if( pRect != NULL )
				m_rcOwner = *pRect; 
			else
				m_hOwnerDC.GetClipBox(&m_rcOwner);

			CreateCompatibleDC(m_hOwnerDC);
			::LPtoDP(m_hOwnerDC, (LPPOINT) &m_rcOwner, sizeof(RECT)/sizeof(POINT));
			m_bitmap.CreateCompatibleBitmap(m_hOwnerDC, m_rcOwner.right - m_rcOwner.left, m_rcOwner.bottom - m_rcOwner.top);
			m_hOldBitmap = SelectBitmap(m_bitmap);
			::DPtoLP(m_hOwnerDC, (LPPOINT) &m_rcOwner, sizeof(RECT)/sizeof(POINT));
			SetWindowOrg(m_rcOwner.left, m_rcOwner.top);
		}
		~CMemDC()
		{
			// Copy the offscreen bitmap onto the screen.
			m_hOwnerDC.BitBlt(m_rcOwner.left, m_rcOwner.top, m_rcOwner.right - m_rcOwner.left, m_rcOwner.bottom - m_rcOwner.top, m_hDC, m_rcOwner.left, m_rcOwner.top, SRCCOPY);
			//Swap back the original bitmap.
			SelectBitmap(m_hOldBitmap);
		}
	};
#endif
    // Picture box Styles and class
#define PICTUREBOX_MENU_CENTER	WM_APP + 1
#define PICTUREBOX_MENU_STRETCH	WM_APP + 2
	enum PICTUREBOX
	{
		PICTUREBOX_CENTER	= 0x0001,
		PICTUREBOX_STRETCH	= 0x0002,
		PICTUREBOX_MENU		= 0x0004,
		PICTUREBOX_OWNER	= 0x0008
	};
	class CPictureBox:	public WTL::CScrollWindowImpl<CPictureBox>
	{
	public:
		// Constructor/Destructor
				CPictureBox()
				{
					m_dwStyle = PICTUREBOX_CENTER|PICTUREBOX_MENU|PICTUREBOX_OWNER;
				}
				~CPictureBox()
				{
					if (!m_Image.IsNull() && m_dwStyle & PICTUREBOX_OWNER)
						m_Image.Destroy();
					if (m_Menu.IsMenu())
						m_Menu.DestroyMenu();
				}
		// ATL Message map stuff
		DECLARE_WND_CLASS(NULL)
		BEGIN_MSG_MAP(CPictureBox)
			MSG_WM_CREATE(OnCreate)
			MSG_WM_ERASEBKGND(OnEraseBkGnd)
			MSG_WM_RBUTTONDOWN(OnRMouseDown)
			COMMAND_ID_HANDLER_EX(PICTUREBOX_MENU_CENTER, OnCenterImage)
			COMMAND_ID_HANDLER_EX(PICTUREBOX_MENU_STRETCH, OnStretchImage)
			CHAIN_MSG_MAP(WTL::CScrollWindowImpl<CPictureBox>)
			CHAIN_MSG_MAP_ALT(WTL::CScrollWindowImpl<CPictureBox>, 1)
			DEFAULT_REFLECTION_HANDLER()
		END_MSG_MAP()
		// Message handlers
		LRESULT	OnCreate(LPCREATESTRUCT)
		{
			SetMsgHandled(false);

			// Create the menu
			m_Menu.CreatePopupMenu();
			m_Menu.AppendMenu(MF_STRING, PICTUREBOX_MENU_CENTER, _T("Center Image"));
			m_Menu.AppendMenu(MF_STRING, PICTUREBOX_MENU_STRETCH, _T("Stretch Image"));

			// Update the scroll view
			UpdateScrollView();
			UpdateMenu();

			return 0;
		}
		LRESULT	OnEraseBkGnd(HDC)
		{
			return 0;
		}
		void	OnRMouseDown(UINT, CPoint)
		{
			if (m_Menu.IsMenu() && (m_dwStyle & PICTUREBOX_MENU))
			{
				POINT ptPoint;
				GetCursorPos(&ptPoint);
				m_Menu.TrackPopupMenu(TPM_RIGHTALIGN, ptPoint.x, ptPoint.y, m_hWnd);
			}
		}
		void	OnCenterImage(UINT, int, HWND)
		{
			if (m_dwStyle & PICTUREBOX_CENTER)
				CenterPicture(false);
			else
				CenterPicture(true);
		}
		void	OnStretchImage(UINT, int, HWND)
		{
			if (m_dwStyle & PICTUREBOX_STRETCH)
				StretchPicture(false);
			else
				StretchPicture(true);
		}
		// Overloaded from CScrollWindowImpl<>
		void	DoPaint(CDCHandle hDC)
		{
			CMemDC		mDC(hDC.m_hDC);
			WTL::CRect	rcClient;
			POINT		ptScrollOffset;

			GetClientRect(&rcClient);
			GetScrollOffset(ptScrollOffset);
			rcClient.OffsetRect(ptScrollOffset);

			mDC.FillRect(&rcClient, AtlGetStockBrush(WHITE_BRUSH));

			if (!m_Image.IsNull())
			{
				if (m_dwStyle & PICTUREBOX_STRETCH)
				{
					CDCHandle dcBitmap(m_Image.GetDC());
					mDC.StretchBlt(rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), dcBitmap, 0, 0, m_Image.GetWidth(), m_Image.GetHeight(), SRCCOPY);
					m_Image.ReleaseDC();
				}
				else
				{
					CDCHandle	dcBitmap(m_Image.GetDC());
					CRect		mRect(0, 0, m_Image.GetWidth(), m_Image.GetHeight());

					if (m_dwStyle & PICTUREBOX_CENTER)
					{
						int x = rcClient.Width() / 2 - m_Image.GetWidth() / 2;
						int y = rcClient.Height() / 2 - m_Image.GetHeight() / 2;

						if (x < 0)
							x = 0;
						if (y < 0)
							y = 0;

						mRect.right = x + mRect.Width();
						mRect.bottom = y + mRect.Height();
						mRect.left = x;
						mRect.top = y;
					}
					mDC.BitBlt(mRect.left, mRect.top, mRect.Width(), mRect.Height(), dcBitmap, 0, 0, SRCCOPY);
					m_Image.ReleaseDC();
				}
			}
		}
		int		ScrollWindowEx(int, int, UINT)
		{
			Invalidate();
			return 0;
		}
		// Picture functions
		HBITMAP	GetBitmap()
		{
			m_dwStyle &= ~PICTUREBOX_OWNER;
			return m_Image.Detach();
		}
		HBITMAP	GetSaveBitmap()const
		{
			return m_Image;
		}
		void	SetBitmap(HBITMAP hBitmap, bool bOwner = true)
		{
			if (!m_Image.IsNull())
			{
				if (m_dwStyle & PICTUREBOX_OWNER)
					m_Image.Destroy();
				else
					ATLTRACE(_T("Old bitmap not destroyed, resource leak"));
			}

			if (bOwner)
				m_dwStyle |= PICTUREBOX_OWNER;
			else
				m_dwStyle &= PICTUREBOX_OWNER;

			m_Image.Attach(hBitmap);

			if (IsWindow())
			{
				UpdateScrollView();
				Invalidate();
			}
		}
		bool	LoadBitmapFromFile(LPCTSTR pszFileName)
		{
			if (!m_Image.IsNull())
			{
				if (m_dwStyle & PICTUREBOX_OWNER)
					m_Image.Destroy();
				else
					ATLTRACE(_T("Old bitmap not destroyed, resource leak"));
			}
			if (m_Image.Load(pszFileName) == S_OK)
			{
				m_dwStyle |= PICTUREBOX_OWNER;
				if (IsWindow())
				{
					UpdateScrollView();
					Invalidate();
				}
				return true;
			}
			return false;
		}
		bool	LoadBitmapFromID(HINSTANCE hInstance, UINT uiIDResource)
		{
			if (!m_Image.IsNull())
			{
				if (m_dwStyle & PICTUREBOX_OWNER)
					m_Image.Destroy();
				else
					ATLTRACE(_T("Old bitmap not destroyed, resource leak"));
			}
			m_Image.LoadFromResource(hInstance, uiIDResource);
			m_dwStyle |= PICTUREBOX_OWNER;
			if (IsWindow())
			{
				UpdateScrollView();
				Invalidate();
			}
			return true;
		}
		void	CenterPicture(bool bCenter)
		{
			if (bCenter)
				m_dwStyle |= PICTUREBOX_CENTER;
			else
				m_dwStyle &= ~PICTUREBOX_CENTER;

			UpdateMenu();
			Invalidate();
		}
		void	StretchPicture(bool bStretch)
		{
			if (bStretch)
				m_dwStyle |= PICTUREBOX_STRETCH;
			else
				m_dwStyle &= ~PICTUREBOX_STRETCH;

			UpdateMenu();
			UpdateScrollView();
			Invalidate();
		}
		// Misc functions
		void	UseMenu(bool bUseMenu)
		{
			if (bUseMenu)
				m_dwStyle |= PICTUREBOX_MENU;
			else
				m_dwStyle &= ~PICTUREBOX_MENU;
		}
	private:
		// Misc functions
		void	UpdateScrollView()
		{
			int iWidth = 0;
			int iHeight = 0;
			if (!m_Image.IsNull())
			{
				iWidth = m_Image.GetWidth();
				iHeight = m_Image.GetHeight();
			}
			if (m_dwStyle & PICTUREBOX_STRETCH)
			{
				iWidth = 0;
				iHeight = 0;
			}

			if (iWidth < 1)
				iWidth = 1;
			if (iHeight < 1)
				iHeight = 1;

			RECT	rcClient;
			POINT	ptScrollOffset;
			SIZE	szSize;

			GetClientRect(&rcClient);
			GetScrollOffset(ptScrollOffset);

			// Calculate the new virtual size
			SetScrollSize(iWidth, iHeight);
			SetScrollLine(100, 100);
			SetScrollPage(rcClient.right - rcClient.left, rcClient.bottom - rcClient.top);

			// Prevent the window to be larger then the virtual view
			GetScrollSize(szSize);
			if (ptScrollOffset.y + rcClient.bottom - rcClient.top > szSize.cy)
			{
				if (rcClient.bottom - rcClient.top > szSize.cy)
					ptScrollOffset.y = 0;
				else
					ptScrollOffset.y = szSize.cy - rcClient.bottom - rcClient.top;
			}
			if (ptScrollOffset.x + rcClient.right - rcClient.left > szSize.cx)
			{
				if (rcClient.right - rcClient.left > szSize.cx)
					ptScrollOffset.x = 0;
				else
					ptScrollOffset.x = szSize.cx - rcClient.right - rcClient.left;
			}	
			SetScrollOffset(ptScrollOffset);
		}
		void	UpdateMenu()
		{
			if (m_Menu.IsMenu())
			{
				if (m_dwStyle & PICTUREBOX_CENTER)
					m_Menu.CheckMenuItem(PICTUREBOX_MENU_CENTER, MF_BYCOMMAND|MF_CHECKED);
				else
					m_Menu.CheckMenuItem(PICTUREBOX_MENU_CENTER, MF_BYCOMMAND|MF_UNCHECKED);

				if (m_dwStyle & PICTUREBOX_STRETCH)
				{
					m_Menu.CheckMenuItem(PICTUREBOX_MENU_STRETCH, MF_BYCOMMAND|MF_CHECKED);
					m_Menu.EnableMenuItem(PICTUREBOX_MENU_CENTER, MF_BYCOMMAND|MF_GRAYED);
				}
				else
				{
					m_Menu.CheckMenuItem(PICTUREBOX_MENU_STRETCH, MF_BYCOMMAND|MF_UNCHECKED);
					m_Menu.EnableMenuItem(PICTUREBOX_MENU_CENTER, MF_BYCOMMAND|MF_ENABLED);
				}
			}
		}
		// Variables
		DWORD		m_dwStyle;
		ATL::CImage	m_Image;
		WTL::CMenu	m_Menu;
	};
}
#endif