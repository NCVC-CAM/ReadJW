// JwwClass.cpp: CJwwXXX �N���X�̃C���v�������e�[�V����
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
//	Serialize(�ǂݍ��ݐ�p�Ȃ̂ŁCar.IsStoring() �������͂��Ȃ�)

void CJwwHead::Serialize(CArchive& ar)
{
	CString	strMsg;

	// ���ʎq
	ar.Read(m_szJWID, sizeof(m_szJWID));
	if ( memcmp(m_szJWID, "JwwData.", sizeof(m_szJWID)) != 0 ) {
		strMsg.Format(IDS_ERR_JW, ar.GetFile()->GetFilePath());
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		AfxThrowUserException();
	}
	// �ް�ޮ�No.
	ar >> g_dwVersion;
	if ( g_dwVersion < JWWVER_300 ) {
		strMsg.Format(IDS_ERR_JWVER, ar.GetFile()->GetFilePath());
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		AfxThrowUserException();
	}
	// ����
	ar >> m_strComment;
	// �}��No.
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
	if ( g_dwVersion >= JWWVER_351 )	// Ver3.51�`
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
	if ( g_dwVersion >= JWWVER_420 ) {	// Ver4.20�`
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
//	�e���ް���͊֐�

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
		// �^�~�̎��������_�o�^
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
				// �^�~�ް�
				dxf.enType = DXFCIRCLEDATA;
				dxf.de.dR = m_r * dScale;
			}
			else {
				// �~��(�p�x�P�ʂ͓x)
				dxf.enType = DXFARCDATA;
				dxf.de.arc.r = m_r * dScale;
				dxf.de.arc.sq = DEG(m_sq + m_lq);
				dxf.de.arc.eq = DEG(m_sq + m_eq + m_lq);
				// �~�ʊp��ϲŽ�̎��͎n�_�I�_�̓���ւ�
				if ( m_eq < 0 )
					std::swap(dxf.de.arc.sq, dxf.de.arc.eq);
			}
		}
		else {
			// �ȉ~(�p�x�P�ʂ�׼ޱ�)
			// �ȉ~��
			dxf.enType = DXFELLIPSEDATA;
			dxf.de.elli.sq = m_sq;
			dxf.de.elli.eq = m_sq + m_eq;
			// �~�ʊp��ϲŽ�̎��͎n�_�I�_�̓���ւ�
			if ( m_eq < 0 )
				std::swap(dxf.de.elli.sq, dxf.de.elli.eq);
			// �����͌��_����̑��΍��W
			dxf.de.elli.ptL.x = m_r * cos(m_lq) * dScale;
			dxf.de.elli.ptL.y = m_r * sin(m_lq) * dScale;
			dxf.de.elli.s = m_flat;
		}
		if ( !NCVC_AddDXFData(hDoc, &dxf) )
			return FALSE;
		break;
	case DXFSTRLAYER:
		// �^�~�̎��������H�J�n�ʒu�o�^
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
					// ��߰��޼ު�Ă̐���
					pData = pData->CopyObject();
					if ( pData ) {
						// ��]�C���_����
						pData->OrgTuning(this);
						// ��ۯ�ؽēo�^���ނ�NCVC�ɓo�^
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
//	��ۯ��}�`�̌��_����

void CData::OrgTuning(CDataBlock* pBlk)
{
	// ��ۯ���ڲԏ����
	m_nLayer = pBlk->m_nLayer;
	m_nGLayer = pBlk->m_nGLayer;
}

void CDataSen::OrgTuning(CDataBlock* pBlk)
{
	// ��ۯ���ڲԏ����
	CData::OrgTuning(pBlk);

	while ( pBlk ) {
		// �{���ύX
		if ( pBlk->m_mxy != 0.0 ) {
			m_pts *= pBlk->m_mxy;
			m_pte *= pBlk->m_mxy;
		}
		// ��]
		pBlk->RoundPoint(m_pts);
		pBlk->RoundPoint(m_pte);
		// ���_�ړ�
		m_pts += pBlk->m_oxy;	m_pte += pBlk->m_oxy;
		// �K�w��ۯ����
		pBlk = pBlk->m_pBlk;
	}
}

void CDataEnko::OrgTuning(CDataBlock* pBlk)
{
	// ��ۯ���ڲԏ����
	CData::OrgTuning(pBlk);

	double	a, b;
	CPointD	pts, pte;

	while ( pBlk ) {

		// ����
		a = m_r;			// ����
		b = a * m_flat;		// �Z��

		// �{���ύX
		if ( pBlk->m_mxy != 0.0 ) {
			if ( pBlk->m_mxy.x != pBlk->m_mxy.y ) {
				// JW_CAD final for DOS �O���� ���t�@�����X��
				// -- JW_CAD�̓��� -- �Q��
				double	coslq = cos(m_lq),// + pBlk->m_cos,
						sinlq = sin(m_lq),// + pBlk->m_sin,
						ac = a * pBlk->m_mxy.x,	ad = a * pBlk->m_mxy.y,
						bc = b * pBlk->m_mxy.x,	bd = b * pBlk->m_mxy.y;
				double	r1 = atan2( ( ad + bc ) * sinlq,
									( ac + bd ) * coslq ),
						r2 = atan2( (-ad + bc ) * sinlq,
									( ac - bd ) * coslq );
				double	d1 = (r1+r2)/2,
						d2 = (r1-r2)/2;		// �V���ȌX��
				// �V���������ƒZ��
				double	l1 = ( ac + bd ) * coslq / cos(d1+d2),
						l2 = ( ac - bd ) * coslq / cos(d1-d2);
				double	ll = (l1+l2)/2,		// ���� �� ���a
						ls = (l1-l2)/2;
				//
				// �i�P�ʉ~�Ƃ��Ắj�n�_�I�_
				pts.x = cos(m_sq);
				pts.y = sin(m_sq);
				pte.x = cos(m_sq+m_eq);
				pte.y = sin(m_sq+m_eq);
				// �X��
				pts.RoundPoint(d1);
				pte.RoundPoint(d1);
				// �{���ύX
				pts.x *= ll;	pts.y *= ls;
				pte.x *= ll;	pte.y *= ls;
				// �ȉ~�ϊ���̌X��
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
				// �n�_�I�_
				pts.x = a * cos(m_sq);
				pts.y = b * sin(m_sq);
				pte.x = a * cos(m_sq+m_eq);
				pte.y = b * sin(m_sq+m_eq);
				// �~�̌X��(�n�_�I�_�̉�])
				pts.RoundPoint(m_lq);
				pte.RoundPoint(m_lq);
				// �{���ύX
				m_r   *= pBlk->m_mxy.x;
				m_ptc *= pBlk->m_mxy;
				pts    = pts * pBlk->m_mxy + m_ptc;
				pte    = pte * pBlk->m_mxy + m_ptc;
			}
		}
		// ��ۯ��̉�]
		if ( pBlk->m_ro != 0.0 ) {
			pBlk->RoundPoint(m_ptc);
			pBlk->RoundPoint(pts);
			pBlk->RoundPoint(pte);
			m_lq += pBlk->m_ro;
		}
		// �V�����J�n�p�x�Ɖ~�ʊp
		if ( !m_bCircle ) {
			pts -= m_ptc;			pte -= m_ptc;
			pts.RoundPoint(-m_lq);
			pte.RoundPoint(-m_lq);
			a = pts.x / m_r;	// acos()�O�͈͕̔␳
			a = _copysign(min(fabs(a), 1.0), a);
			m_sq = _copysign(acos(a), pts.y);
			// �~�ʊp�̌v�Z
			a = pte.x / m_r;
			a = _copysign(min(fabs(a), 1.0), a);
			double eq = _copysign(acos(a), pte.y) - m_sq;
			if ( m_eq > 0 ) {
				// �����v���
				if ( eq < 0 )
					eq += RAD(360.0);
			}
			else {
				// ���v���
				if ( eq > 0 )
					eq -= RAD(360.0);
			}
			m_eq = eq;
		}
		// ���_�ړ�
		m_ptc += pBlk->m_oxy;
		// �K�w��ۯ����
		pBlk = pBlk->m_pBlk;

	}
}

void CDataTen::OrgTuning(CDataBlock* pBlk)
{
	// ��ۯ���ڲԏ����
	CData::OrgTuning(pBlk);

	while ( pBlk ) {
		// �{���ύX
		if ( pBlk->m_mxy != 0.0 )
			m_ptc *= pBlk->m_mxy;
		// ��]
		pBlk->RoundPoint(m_ptc);
		// ���_�ړ�
		m_ptc += pBlk->m_oxy;
		// �K�w��ۯ����
		pBlk = pBlk->m_pBlk;
	}
}

void CDataMoji::OrgTuning(CDataBlock* pBlk)
{
	// ��ۯ���ڲԏ����
	CData::OrgTuning(pBlk);

	while ( pBlk ) {
		// �{���ύX(�g���͎̂n�_�̂�)
		if ( pBlk->m_mxy != 0.0 )
			m_pts *= pBlk->m_mxy;
		// ��]
		pBlk->RoundPoint(m_pts);
		// ���_�ړ�
		m_pts += pBlk->m_oxy;
		// �K�w��ۯ����
		pBlk = pBlk->m_pBlk;
	}
}

void CDataBlock::OrgTuning(CDataBlock* pBlk)
{
	// �K�w��ۯ����
	m_pBlk = pBlk;
}
