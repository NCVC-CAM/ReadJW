// JwwClass.cpp: CJwwXXX クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "NCVCaddin.h"
#include "ReadJW.h"
#include "JwwClass.h"

#include <memory.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_SERIAL(CJwwHead, CObject, 1)
IMPLEMENT_SERIAL(CJwwLayerInfo, CObject, 1)
//
IMPLEMENT_SERIAL(CData, CObject, JWWVER|VERSIONABLE_SCHEMA)
IMPLEMENT_SERIAL(CDataSen, CData, JWWVER|VERSIONABLE_SCHEMA)
IMPLEMENT_SERIAL(CDataEnko, CData, JWWVER|VERSIONABLE_SCHEMA)
IMPLEMENT_SERIAL(CDataTen, CData, JWWVER|VERSIONABLE_SCHEMA)
IMPLEMENT_SERIAL(CDataMoji, CData, JWWVER|VERSIONABLE_SCHEMA)
IMPLEMENT_SERIAL(CDataSunpou, CData, JWWVER|VERSIONABLE_SCHEMA)
IMPLEMENT_SERIAL(CDataSolid, CData, JWWVER|VERSIONABLE_SCHEMA)
IMPLEMENT_SERIAL(CDataBlock, CData, JWWVER|VERSIONABLE_SCHEMA)
IMPLEMENT_SERIAL(CDataList, CData, JWWVER|VERSIONABLE_SCHEMA)
//
extern	DWORD	g_dwVersion = 0;	// Ver.No.

//////////////////////////////////////////////////////////////////////
//	Serialize(読み込み専用なので，ar.IsStoring() のﾁｪｯｸはしない)

void CJwwHead::Serialize(CArchive& ar)
{
	CString	strMsg;

	// 識別子
	ar.Read(m_szJWID, sizeof(m_szJWID));
	if ( memcmp(m_szJWID, "JwwData.", sizeof(m_szJWID)) != 0 ) {
		strMsg.Format(IDS_ERR_JW, ar.GetFile()->GetFilePath());
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		AfxThrowUserException();
	}
	// ﾊﾞｰｼﾞｮﾝNo.
	ar >> g_dwVersion;
	if ( g_dwVersion < JWWVER_300 ) {
		strMsg.Format(IDS_ERR_JWVER, ar.GetFile()->GetFilePath());
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		AfxThrowUserException();
	}
	// ｺﾒﾝﾄ
	ar >> m_strComment;
	// 図面No.
	ar >> m_dwPaper;
}

void CJwwLayerInfo::Serialize(CArchive& ar)
{
	ar >> m_dwGLay >> m_dwWriteLay >>
		m_dScale >> m_dwGLayProtect;
	for ( int i=0; i<JW_LAYERGRPOUP; i++ )
		ar >> m_Lay[i].m_dwLay >> m_Lay[i].m_dwLatProtect;
}

//////////////////////////////////////////////////////////////////////

void CData::Serialize(CArchive& ar)
{
	ar >> m_dwGroup >> m_nPenStyle >> m_nPenColor;
	if ( g_dwVersion >= JWWVER_351 )	// Ver3.51～
		ar >> m_nPenWidth;
	else
		m_nPenWidth = 0;
	ar >> m_nLayer >> m_nGLayer >> m_bFlg;
}

void CDataSen::Serialize(CArchive& ar)
{
	CData::Serialize(ar);
	ar >> m_pts.x >> m_pts.y >> m_pte.x >> m_pte.y;
}

void CDataEnko::Serialize(CArchive& ar)
{
	CData::Serialize(ar);
	ar >> m_ptc.x >> m_ptc.y >> m_r >> m_sq >> m_eq >> m_lq >> m_flat >>
		m_bCircle;
}

void CDataTen::Serialize(CArchive& ar)
{
	CData::Serialize(ar);
	ar >> m_ptc.x >> m_ptc.y >> m_bVirtual;
	if ( m_nPenStyle == 100 ) {
		DWORD	m_nCode;
		double	m_radKaitenKaku, m_dBairitsu;
		ar >> m_nCode >> m_radKaitenKaku >> m_dBairitsu;
	}
}

void CDataMoji::Serialize(CArchive& ar)
{
	CData::Serialize(ar);
	ar >> m_pts.x >> m_pts.y >> m_pte.x >> m_pte.y >> m_nFont >>
		m_dSizeX >> m_dSizeY >> m_dSpace >> m_dAngle >>
		m_strFont >> m_strText;
}

void CDataSunpou::Serialize(CArchive& ar)
{
	CData::Serialize(ar);
	m_Line.Serialize(ar);
	m_Text.Serialize(ar);
	if ( g_dwVersion >= JWWVER_420 ) {	// Ver4.20～
		ar >> m_bSxfMode;
		m_SenHo1.Serialize(ar);
		m_SenHo2.Serialize(ar);
		m_Ten1.Serialize(ar);
		m_Ten2.Serialize(ar);
		m_TenHo1.Serialize(ar);
		m_TenHo2.Serialize(ar);
	}
}

void CDataSolid::Serialize(CArchive& ar)
{
	CData::Serialize(ar);
	ar >> m_pts.x >> m_pts.y >> m_pte.x >> m_pte.y >>
		m_pt2.x >> m_pt2.y >> m_pt3.x >> m_pt3.y;
	if ( m_nPenColor == 10 )
		ar >> m_dwColor;
}

void CDataBlock::Serialize(CArchive& ar)
{
	CData::Serialize(ar);
	ar >> m_oxy.x >> m_oxy.y >> m_mxy.x >> m_mxy.y >> m_ro >>
		m_dwID;

	m_sin = sin(m_ro);
	m_cos = cos(m_ro);
}

void CDataList::Serialize(CArchive& ar)
{
	CData::Serialize(ar);
	ar >> m_dwID >> m_dwReffered >> m_time >>
		m_strBlock;
	m_lstMember.Serialize(ar);
}

/////////////////////////////////////////////////////////////////////////////
//	各種ﾃﾞｰﾀ解析関数

BOOL CDataSen::JWWtoNCVCdata
	(NCVCHANDLE hDoc, const CJwwBlock&, const JWLAYER jwl[])
{
	BOOL	bResult = TRUE;
	UINT	wLayer = GetLayer();
	int		nLayer = CheckDataLayer(jwl, wLayer);
	double	dScale;

	if ( nLayer == DXFORGLAYER ) {
		DPOINT	pts, pte;
		dScale = jwl[wLayer].dScale;
		pts = m_pts * dScale;
		pte = m_pte * dScale;
		NCVC_SetDXFLatheLine(hDoc, &pts, &pte);
	}
	else if ( nLayer>=DXFCAMLAYER && nLayer<=DXFMOVLAYER ) {
		DXFDATA	dxf;
		dScale = jwl[wLayer].dScale;
		dxf.dwSize = sizeof(DXFDATA);
		dxf.enType = DXFLINEDATA;
		dxf.ptS    = m_pts * dScale;
		dxf.de.ptE = m_pte * dScale;
		dxf.nLayer = nLayer;
		lstrcpyn(dxf.szLayer, jwl[wLayer].strLayer,
			min(sizeof(dxf.szLayer), jwl[wLayer].strLayer.GetLength()+1));
		bResult = NCVC_AddDXFData(hDoc, &dxf);
	}

	return bResult;
}

BOOL CDataEnko::JWWtoNCVCdata
	(NCVCHANDLE hDoc, const CJwwBlock&, const JWLAYER jwl[])
{
	DXFDATA	dxf;
	DPOINT	pt;
	UINT	wLayer = GetLayer();

	dxf.dwSize = sizeof(DXFDATA);
	double	dScale = jwl[wLayer].dScale;

	switch ( CheckDataLayer(jwl, wLayer) ) {
	case DXFORGLAYER:
		// 真円の時だけ原点登録
		if ( m_bCircle ) {
			pt = m_ptc * dScale;
			NCVC_SetDXFCutterOrigin(hDoc, &pt, m_r*dScale, FALSE);
		}
		break;
	case DXFCAMLAYER:
		dxf.ptS = m_ptc * dScale;
		dxf.nLayer = DXFCAMLAYER;
		lstrcpyn(dxf.szLayer, jwl[wLayer].strLayer,
			min(sizeof(dxf.szLayer), jwl[wLayer].strLayer.GetLength()+1));
		if ( m_flat == 1.0 ) {
			if ( m_bCircle ) {
				// 真円ﾃﾞｰﾀ
				dxf.enType = DXFCIRCLEDATA;
				dxf.de.dR = m_r * dScale;
			}
			else {
				// 円弧(角度単位は度)
				dxf.enType = DXFARCDATA;
				dxf.de.arc.r = m_r * dScale;
				dxf.de.arc.sq = DEG(m_sq + m_lq);
				dxf.de.arc.eq = DEG(m_sq + m_eq + m_lq);
				// 円弧角がﾏｲﾅｽの時は始点終点の入れ替え
				if ( m_eq < 0 )
					std::swap(dxf.de.arc.sq, dxf.de.arc.eq);
			}
		}
		else {
			// 楕円(角度単位はﾗｼﾞｱﾝ)
			// 楕円弧
			dxf.enType = DXFELLIPSEDATA;
			dxf.de.elli.sq = m_sq;
			dxf.de.elli.eq = m_sq + m_eq;
			// 円弧角がﾏｲﾅｽの時は始点終点の入れ替え
			if ( m_eq < 0 )
				std::swap(dxf.de.elli.sq, dxf.de.elli.eq);
			// 長軸は原点からの相対座標
			dxf.de.elli.ptL.x = m_r * cos(m_lq) * dScale;
			dxf.de.elli.ptL.y = m_r * sin(m_lq) * dScale;
			dxf.de.elli.s = m_flat;
		}
		if ( !NCVC_AddDXFData(hDoc, &dxf) )
			return FALSE;
		break;
	case DXFSTRLAYER:
		// 真円の時だけ加工開始位置登録
		if ( m_bCircle ) {
			dxf.enType = DXFCIRCLEDATA;
			dxf.ptS = m_ptc * dScale;
			dxf.nLayer = DXFSTRLAYER;
			lstrcpyn(dxf.szLayer, jwl[wLayer].strLayer,
				min(sizeof(dxf.szLayer), jwl[wLayer].strLayer.GetLength()+1));
			dxf.de.dR = m_r * dScale;
			if ( !NCVC_AddDXFData(hDoc, &dxf) )
				return FALSE;
		}
		break;
	}

	return TRUE;
}

BOOL CDataTen::JWWtoNCVCdata
	(NCVCHANDLE hDoc, const CJwwBlock&, const JWLAYER jwl[])
{
	UINT	wLayer = GetLayer();

	if ( !m_bVirtual &&
			CheckDataLayer(jwl, wLayer) == DXFCAMLAYER ) {
		DXFDATA	dxf;
		double	dScale = jwl[wLayer].dScale;
		dxf.dwSize = sizeof(DXFDATA);
		dxf.enType = DXFPOINTDATA;
		dxf.ptS    = m_ptc * dScale;
		dxf.nLayer = DXFCAMLAYER;
		lstrcpyn(dxf.szLayer, jwl[wLayer].strLayer,
			min(sizeof(dxf.szLayer), jwl[wLayer].strLayer.GetLength()+1));
		if ( !NCVC_AddDXFData(hDoc, &dxf) )
			return FALSE;
	}

	return TRUE;
}

BOOL CDataMoji::JWWtoNCVCdata
	(NCVCHANDLE hDoc, const CJwwBlock&, const JWLAYER jwl[])
{
	static	CString		strIgnore[] = {
		"Printer_Orientation =",
		"Printer_PaperSize =",
		"Printer_D2dBMP =",
		"Printer_BmpZENTAI =",
		"View_Direct2d =",
		"Draw_BmpTOUKA ="
	};
	static	CPointD		ptIgnore(0, -1000);

	BOOL	bResult = TRUE;
	UINT	wLayer = GetLayer();
	int		nLayer = CheckDataLayer(jwl, wLayer);

	if ( nLayer>=DXFCAMLAYER && nLayer<=DXFCOMLAYER ) {
		if ( m_pts==ptIgnore && m_pte==ptIgnore ) {
			for ( int i=0; i<SIZEOF(strIgnore); i++ ) {
				if ( strIgnore[i] == m_strText.Left(strIgnore[i].GetLength()) )
					return bResult;
			}
		}
		DXFDATA	dxf;
		double	dScale = jwl[wLayer].dScale;
		dxf.dwSize = sizeof(DXFDATA);
		dxf.enType = DXFTEXTDATA;
		dxf.ptS    = m_pts * dScale;
		dxf.nLayer = nLayer;
		lstrcpyn(dxf.szLayer, jwl[wLayer].strLayer,
				min(sizeof(dxf.szLayer), jwl[wLayer].strLayer.GetLength()+1));
		lstrcpyn(dxf.de.szText, m_strText,
				min(sizeof(dxf.de.szText), m_strText.GetLength()+1));
		bResult = NCVC_AddDXFData(hDoc, &dxf);
	}

	return bResult;
}

BOOL CDataBlock::JWWtoNCVCdata
	(NCVCHANDLE hDoc, const CJwwBlock& lstBlk, const JWLAYER jwl[])
{
	CData*		pData;
	CDataList*	pList;
	int			nLayer = CheckDataLayer(jwl, GetLayer());
	POSITION	pos1, pos2;

	if ( nLayer >= 0 ) {
		for ( pos1=lstBlk.GetHeadPosition(); pos1; ) {
			pList = lstBlk.GetNext(pos1);
			if ( m_dwID == pList->m_dwID ) {
				for ( pos2=pList->m_lstMember.GetHeadPosition(); pos2; ) {
					pData = pList->m_lstMember.GetNext(pos2);
					// ｺﾋﾟｰｵﾌﾞｼﾞｪｸﾄの生成
					pData = pData->CopyObject();
					if ( pData ) {
						// 回転，原点調整
						pData->OrgTuning(this);
						// ﾌﾞﾛｯｸﾘｽﾄ登録ﾒﾝﾊﾞをNCVCに登録
						if ( !pData->JWWtoNCVCdata(hDoc, lstBlk, jwl) ) {
							delete	pData;
							return FALSE;
						}
						delete	pData;
					}
				}
				break;
			}
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	ﾌﾞﾛｯｸ図形の原点調整

void CData::OrgTuning(CDataBlock* pBlk)
{
	// ﾌﾞﾛｯｸのﾚｲﾔ情報をｾｯﾄ
	m_nLayer = pBlk->m_nLayer;
	m_nGLayer = pBlk->m_nGLayer;
}

void CDataSen::OrgTuning(CDataBlock* pBlk)
{
	// ﾌﾞﾛｯｸのﾚｲﾔ情報をｾｯﾄ
	CData::OrgTuning(pBlk);

	while ( pBlk ) {
		// 倍率変更
		if ( pBlk->m_mxy != 0.0 ) {
			m_pts *= pBlk->m_mxy;
			m_pte *= pBlk->m_mxy;
		}
		// 回転
		pBlk->RoundPoint(m_pts);
		pBlk->RoundPoint(m_pte);
		// 原点移動
		m_pts += pBlk->m_oxy;	m_pte += pBlk->m_oxy;
		// 階層ﾌﾞﾛｯｸｾｯﾄ
		pBlk = pBlk->m_pBlk;
	}
}

void CDataEnko::OrgTuning(CDataBlock* pBlk)
{
	// ﾌﾞﾛｯｸのﾚｲﾔ情報をｾｯﾄ
	CData::OrgTuning(pBlk);

	double	a, b;
	CPointD	pts, pte;

	while ( pBlk ) {

		// 準備
		a = m_r;			// 長軸
		b = a * m_flat;		// 短軸

		// 倍率変更
		if ( pBlk->m_mxy != 0.0 ) {
			if ( pBlk->m_mxy.x != pBlk->m_mxy.y ) {
				// JW_CAD final for DOS 徹底解説 リファレンス編
				// -- JW_CADの内側 -- 参照
				double	coslq = cos(m_lq),// + pBlk->m_cos,
						sinlq = sin(m_lq),// + pBlk->m_sin,
						ac = a * pBlk->m_mxy.x,	ad = a * pBlk->m_mxy.y,
						bc = b * pBlk->m_mxy.x,	bd = b * pBlk->m_mxy.y;
				double	r1 = atan2( ( ad + bc ) * sinlq,
									( ac + bd ) * coslq ),
						r2 = atan2( (-ad + bc ) * sinlq,
									( ac - bd ) * coslq );
				double	d1 = (r1+r2)/2,
						d2 = (r1-r2)/2;		// 新たな傾き
				// 新しい長軸と短軸
				double	l1 = ( ac + bd ) * coslq / cos(d1+d2),
						l2 = ( ac - bd ) * coslq / cos(d1-d2);
				double	ll = (l1+l2)/2,		// 長軸 → 半径
						ls = (l1-l2)/2;
				//
				// （単位円としての）始点終点
				pts.x = cos(m_sq);
				pts.y = sin(m_sq);
				pte.x = cos(m_sq+m_eq);
				pte.y = sin(m_sq+m_eq);
				// 傾き
				pts.RoundPoint(d1);
				pte.RoundPoint(d1);
				// 倍率変更
				pts.x *= ll;	pts.y *= ls;
				pte.x *= ll;	pte.y *= ls;
				// 楕円変換後の傾き
				pts.RoundPoint(d2);
				pte.RoundPoint(d2);
				//
				m_r    = ll;
				m_lq   = d2;
				m_flat = ls / ll;
				m_ptc *= pBlk->m_mxy;
				pts   += m_ptc;
				pte   += m_ptc;
			}
			else {
				// 始点終点
				pts.x = a * cos(m_sq);
				pts.y = b * sin(m_sq);
				pte.x = a * cos(m_sq+m_eq);
				pte.y = b * sin(m_sq+m_eq);
				// 円の傾き(始点終点の回転)
				pts.RoundPoint(m_lq);
				pte.RoundPoint(m_lq);
				// 倍率変更
				m_r   *= pBlk->m_mxy.x;
				m_ptc *= pBlk->m_mxy;
				pts    = pts * pBlk->m_mxy + m_ptc;
				pte    = pte * pBlk->m_mxy + m_ptc;
			}
		}
		// ﾌﾞﾛｯｸの回転
		if ( pBlk->m_ro != 0.0 ) {
			pBlk->RoundPoint(m_ptc);
			pBlk->RoundPoint(pts);
			pBlk->RoundPoint(pte);
			m_lq += pBlk->m_ro;
		}
		// 新しい開始角度と円弧角
		if ( !m_bCircle ) {
			pts -= m_ptc;			pte -= m_ptc;
			pts.RoundPoint(-m_lq);
			pte.RoundPoint(-m_lq);
			a = pts.x / m_r;	// acos()前の範囲補正
			a = _copysign(min(fabs(a), 1.0), a);
			m_sq = _copysign(acos(a), pts.y);
			// 円弧角の計算
			a = pte.x / m_r;
			a = _copysign(min(fabs(a), 1.0), a);
			double eq = _copysign(acos(a), pte.y) - m_sq;
			if ( m_eq > 0 ) {
				// 反時計回り
				if ( eq < 0 )
					eq += RAD(360.0);
			}
			else {
				// 時計回り
				if ( eq > 0 )
					eq -= RAD(360.0);
			}
			m_eq = eq;
		}
		// 原点移動
		m_ptc += pBlk->m_oxy;
		// 階層ﾌﾞﾛｯｸｾｯﾄ
		pBlk = pBlk->m_pBlk;

	}
}

void CDataTen::OrgTuning(CDataBlock* pBlk)
{
	// ﾌﾞﾛｯｸのﾚｲﾔ情報をｾｯﾄ
	CData::OrgTuning(pBlk);

	while ( pBlk ) {
		// 倍率変更
		if ( pBlk->m_mxy != 0.0 )
			m_ptc *= pBlk->m_mxy;
		// 回転
		pBlk->RoundPoint(m_ptc);
		// 原点移動
		m_ptc += pBlk->m_oxy;
		// 階層ﾌﾞﾛｯｸｾｯﾄ
		pBlk = pBlk->m_pBlk;
	}
}

void CDataMoji::OrgTuning(CDataBlock* pBlk)
{
	// ﾌﾞﾛｯｸのﾚｲﾔ情報をｾｯﾄ
	CData::OrgTuning(pBlk);

	while ( pBlk ) {
		// 倍率変更(使うのは始点のみ)
		if ( pBlk->m_mxy != 0.0 )
			m_pts *= pBlk->m_mxy;
		// 回転
		pBlk->RoundPoint(m_pts);
		// 原点移動
		m_pts += pBlk->m_oxy;
		// 階層ﾌﾞﾛｯｸｾｯﾄ
		pBlk = pBlk->m_pBlk;
	}
}

void CDataBlock::OrgTuning(CDataBlock* pBlk)
{
	// 階層ﾌﾞﾛｯｸｾｯﾄ
	m_pBlk = pBlk;
}
