/*
	Šg’£DLL‚Å‚ÍºİÊß²Ù´×°‚É‚È‚é 
		AFX_MANAGE_STATE(AfxGetStaticModuleState());
	‚Ì‘ã‚í‚è
	½º°Ìß‚É‚ ‚é‚Æ‚«‚¾‚¯ºİ½Ä×¸Àˆø”‚ÌØ¿°½ÊİÄŞÙ‚ğ—LŒø‚É‚·‚é
	’Êí AFX_EXTENSION_MODULE ‚Ì hModule ‚ğ“n‚·
*/
#pragma once

/////////////////////////////////////////////////////////////////////////////
//	CNCVCExtAddin_ManageState

class	CNCVCExtAddin_ManageState
{
	HINSTANCE	m_hInstance;

public:
	CNCVCExtAddin_ManageState(HMODULE hModule) {
		m_hInstance = ::AfxGetResourceHandle();
		::AfxSetResourceHandle(hModule);
	}
	~CNCVCExtAddin_ManageState() {
		::AfxSetResourceHandle(m_hInstance);
	}
};

/////////////////////////////////////////////////////////////////////////////
//	define

#define	JW_LINE				0
#define	JW_CIRCLE			1
#define	JW_TEXT				2
#define	JW_POINT			3
#define	JW_MAXDATA			4
#define	JW_LAYERGRPOUP		16
#define	JW_LAYERLENGTH		8

typedef	struct	tagJWLAYER {
	CString	strLayer;		// Ú²Ô–¼(MAX‚W•¶š)
	double	dScale;			// Ú²Ô(¸ŞÙ°Ìß)‚²‚Æ‚Ì½¹°Ù
} JWLAYER;

#include "PointTemplate.h"

/////////////////////////////////////////////////////////////////////////////
//	common function

// NCVC‚Ì“Ç‚İ‚İÚ²ÔÁª¯¸
int		CheckDataLayer(const JWLAYER jwl[], UINT nLayer);
