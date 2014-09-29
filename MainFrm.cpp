// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

//#pragma comment(lib,"plate_dection.lib")

#include "aboutdlg.h"
#include "MainFrm.h"

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	return FALSE;
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	m_plateRecognize = new CPlateRecognize();
	m_plateRecognize->LoadSVM("model/svm.xml");
	m_plateRecognize->LoadANN("model/ann.xml");

	m_plateRecognize->setGaussianBlurSize(5);
	m_plateRecognize->setMorphSizeWidth(17);

	m_plateRecognize->setVerifyMin(3);
	m_plateRecognize->setVerifyMax(20);

	m_plateRecognize->setLiuDingSize(7);
	m_plateRecognize->setColorThreshold(150);

	m_fileName = "res/plate_recognize.jpg";
	m_matInput = imread(m_fileName);

	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar(m_fileName.c_str());
	//DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP;
	//UINT nID = ATL_IDW_STATUS_BAR;
	//HWND m_hWndStatusBar = CreateStatusWindow(dwStyle, _T("hehe2"), m_hWnd, nID);

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// [Added]
	m_PictureBox.LoadBitmapFromFile(m_fileName.c_str());
	m_hWndClient = m_PictureBox.Create(m_hWnd, rcDefault, NULL, WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE);
	// [/Added]

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFilterString	mFilter(IDS_FILEFILTER);
	CFileDialog		mDlg(TRUE, NULL, _T("*.jpg"), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, mFilter);

	if (mDlg.DoModal() == IDOK)
	{
		m_PictureBox.LoadBitmapFromFile(mDlg.m_szFileName);
		m_fileName = mDlg.m_szFileName;
		m_matInput = imread(m_fileName);
	}

	return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	static BOOL bVisible = TRUE;	// initially visible
	bVisible = !bVisible;
	CReBarCtrl rebar = m_hWndToolBar;
	int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
	rebar.ShowBand(nBandIndex, bVisible);
	UISetCheck(ID_VIEW_TOOLBAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
	::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
	UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
	UpdateLayout();
	return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}

LRESULT CMainFrame::OnPlateDetection(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加命令处理程序代码
	MessageBox(_T("开始定位车牌"));

	vector<Mat> resultVec;

	int result = m_plateRecognize->plateDetect(m_matInput, resultVec);
	if (result == 0)
	{
		int num = resultVec.size();
		for (int j = 0; j < num; j++)
		{
			Mat resultMat = resultVec[j];
			m_matPlate = resultMat;
			imshow("test", resultMat);
			waitKey(0);
		}
	}

	MessageBox(_T("搜索车牌结束"));

	return 0;
}


LRESULT CMainFrame::OnCharsRecognition(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	// TODO: 在此添加命令处理程序代码
	MessageBox(_T("开始识别字符"));

	if (!m_matPlate.data)
	{
		MessageBox(_T("没有处理好的车牌"));
		return -1;
	}

	string plate;

	int result = m_plateRecognize->charsRecognise(m_matPlate, plate);

	MessageBox(_T(plate.c_str()));

	MessageBox(_T("识别字符结束"));
	return 0;
}


LRESULT CMainFrame::OnPr(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	MessageBox(_T("开始进行车牌识别"));

	vector<string> stringVec;

	int result = m_plateRecognize->plateRecognize(m_matInput, stringVec);
	if (result == 0)
	{
		int num = stringVec.size();
		for (int j = 0; j < num; j++)
		{
			String s = stringVec[j];
			MessageBox(_T(s.c_str()));
		}
	}

	MessageBox(_T("车牌识别结束"));

	return 0;
}
