// ReadJW.cpp : DLL 用の初期化処理の定義を行います。
//

#include "stdafx.h"
#include <afxdllx.h>
#include "resource.h"
#include "NCVCaddin.h"
#include "ReadJW.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern AFX_EXTENSION_MODULE ReadJWDLL = { NULL, NULL };

static	const	char*	g_szFileExt[] = {
	"jww", "jwc"
};
static	const	char*	g_szSerialFunc[] = {
	"Read_JWW", "Read_JWC"
};

/////////////////////////////////////////////////////////////////////////////
//	MFC拡張DLL ｴﾝﾄﾘ

extern "C" int APIENTRY
	DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH) {
		TRACE0("READJW.DLL Initializing!\n");
		// 拡張 DLL を１回だけ初期化します。
		if (!AfxInitExtensionModule(ReadJWDLL, hInstance))
			return 0;
		// この DLL をリソース チェインへ挿入します。
		new CDynLinkLibrary(ReadJWDLL);
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		TRACE0("READJW.DLL Terminating!\n");
		// デストラクタが呼び出される前にライブラリを終了します
		AfxTermExtensionModule(ReadJWDLL);
	}
	return 1;   // ok
}

/////////////////////////////////////////////////////////////////////////////
// NCVC ｱﾄﾞｲﾝ関数

NCADDIN BOOL NCVC_Initialize(NCVCINITIALIZE* nci)
{
	static	const	char	szMenuName[] = "JWﾃﾞｰﾀを開く...";
	static	const	char	szFuncName[] = "ReadJW";
	static	const	int		nMenu[] = {
		NCVCADIN_ARY_APPFILE, NCVCADIN_ARY_NCDFILE, NCVCADIN_ARY_DXFFILE
	};
	int		i;

#ifdef _DEBUG
	printf("ReadJW.dll\nNCVC_Initialize Start!!\n");
#endif

	// ｱﾄﾞｲﾝの必要情報
	nci->dwSize = sizeof(NCVCINITIALIZE);
	nci->dwType = NCVCADIN_FLG_APPFILE|NCVCADIN_FLG_NCDFILE|NCVCADIN_FLG_DXFFILE;
//	nci->nToolBar = 0;
	for ( i=0; i<SIZEOF(nMenu); i++ ) {
		nci->lpszMenuName[nMenu[i]] = szMenuName;
		nci->lpszFuncName[nMenu[i]] = szFuncName;
	}
	nci->lpszAddinName	= szFuncName;
	nci->lpszCopyright	= "MNCT-S K.Magara";
	nci->lpszSupport	= "http://s-gikan2.maizuru-ct.ac.jp/";

	// ｶｽﾀﾑ拡張子の登録．これをしておくとD&Dでもﾌｧｲﾙが開ける
	for ( i=0; i<SIZEOF(g_szFileExt); i++ )
		NCVC_AddDXFExtensionFunc(g_szFileExt[i], szFuncName, g_szSerialFunc[i]);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// NCVC ｲﾍﾞﾝﾄ関数

NCADDIN void ReadJW(void)
{
#ifdef _DEBUG
	printf("ReadJW.dll\nReadJW() Start!!\n");
#endif
	CFileDialog	dlg(TRUE, g_szFileExt[0], NULL, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,
		"jw files (*.jwc;*.jww)|*.jwc;*.jww|All Files (*.*)|*.*||");
	if ( dlg.DoModal() != IDOK )
		return;
#ifdef _DEBUG
	printf("File=%s\n", LPCTSTR(dlg.GetPathName()));
#endif
	// 新規ﾄﾞｷｭﾒﾝﾄの作成
	int	nSel = dlg.GetFileExt().CompareNoCase(g_szFileExt[0]) == 0 ? 0 : 1;
	NCVC_CreateDXFDocument(dlg.GetPathName(), g_szSerialFunc[nSel]);
/*
	// ﾌﾟﾚｰｽﾊﾞｰ表示のためSDK流に書き直し
	TCHAR			szFileName[_MAX_PATH];
	::ZeroMemory(szFileName, sizeof(_MAX_PATH));
	OPENFILENAME	ofn;
	::ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize	= sizeof(OPENFILENAME);
	ofn.hwndOwner	= NCVC_GetMainWnd();
	ofn.lpstrFilter	= "jw files (*.jwc;*.jww)\0*.jwc;*.jww\0All Files (*.*)\0*.*\0\0";
	ofn.lpstrFile	= szFileName;
	ofn.nMaxFile	= _MAX_PATH;
	ofn.Flags		= OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
	ofn.lpstrDefExt	= g_szFileExt[0];
	if ( ::GetOpenFileName(&ofn) ) {
		CString	strFile(ofn.lpstrFile);
#ifdef _DEBUG
		dbg.printf("File=%s", strFile);
#endif
		// 新規ﾄﾞｷｭﾒﾝﾄの作成
		int	nSel = strFile.Right(3).CompareNoCase(g_szFileExt[0]) == 0 ? 0 : 1;
		NCVC_CreateDXFDocument(strFile, g_szSerialFunc[nSel]);
	}
*/
}

/////////////////////////////////////////////////////////////////////////////
// ReadJW 共通関数

int CheckDataLayer(const JWLAYER jwl[], UINT nLayer)
{
	// ﾚｲﾔ文字列
	if ( nLayer >= JW_LAYERGRPOUP*JW_LAYERGRPOUP )
		return -1;
	if ( !jwl[nLayer].strLayer.IsEmpty() ) {
		// 切削ﾚｲﾔ
		if ( NCVC_IsCutterLayer(jwl[nLayer].strLayer) )
			return DXFCAMLAYER;		// 1
		// 移動指示ﾚｲﾔ
		if ( NCVC_IsMoveLayer(jwl[nLayer].strLayer) )
			return DXFMOVLAYER;		// 3
		// 加工開始位置指示ﾚｲﾔ
		if ( NCVC_IsStartLayer(jwl[nLayer].strLayer) )
			return DXFSTRLAYER;		// 2
		// ｺﾒﾝﾄﾚｲﾔ
		if ( NCVC_IsCommentLayer(jwl[nLayer].strLayer) )
			return DXFCOMLAYER;		// 4
		// 原点ﾚｲﾔ
		if ( NCVC_IsOriginLayer(jwl[nLayer].strLayer) )
			return DXFORGLAYER;		// 0
		// ↑ﾃﾞｰﾀ数の多い(であろう)順にﾁｪｯｸ
	}
	return -1;
}
