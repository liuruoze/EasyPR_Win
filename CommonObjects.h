//////////////////////////////////////////////////////////////////////////
// Name:	    Common Objects Header
// Date:	    1-7-2004
// Author:	    Trilobyte
// Copyright:   (c) Trilobyte-Solutions 2004-2008
// Desciption:
// The following objects are declared:
// CButtonCtrl - Implements a CButton
// CFilterString - Load an ID and converts it into a CFileDialog filter string
// CPictureCtrl - Display a picture
// CMemDC - OffScreen buffer
//////////////////////////////////////////////////////////////////////////
#pragma once
#include "stdafx.h"

//********************************************************************************************
class CButtonCtrl: public CWindowImpl<CButtonCtrl, CButton>
{
public:
	BEGIN_MSG_MAP(CButtonCtrl)
	END_MSG_MAP()
};
//********************************************************************************************
class CFilterString
{
public:
	CFilterString(UINT uiStringID)
	{
		if (m_strFilter.LoadString(uiStringID) && !m_strFilter.IsEmpty())
		{
			// Get a pointer to the string buffer
			LPTSTR psz = m_strFilter.GetBuffer(0);
			// Replace '|' with '\0'
			while ((psz = _tcschr(psz, '|')) != NULL)
				*psz++ = '\0';
		}
	}
	inline operator LPCTSTR() const { return m_strFilter;}
	CString m_strFilter;
};
//********************************************************************************************
class CPictureCtrl: public CWindowImpl<CPictureCtrl, CStatic>
{
public:
	BEGIN_MSG_MAP(CPictureCtrl)
		MSG_WM_SIZE(OnSize)
	END_MSG_MAP()

	void OnSize(UINT, CSize)
	{
		Invalidate();
		SetMsgHandled(false);
	}
};
//********************************************************************************************
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
//********************************************************************************************