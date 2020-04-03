// JwwClass.h: CJwwXXX クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define	JWWVER_300	300
#define	JWWVER_351	351		// ～Ver4.2x
#define	JWWVER_420	420		// Ver5.0～
#define	JWWVER		JWWVER_420

//////////////////////////////////////////////////////////////////////
//	ﾍｯﾀﾞｰ情報

class CJwwHead : public CObject  
{
	char	m_szJWID[8];		// 識別用
	CString	m_strComment;		// ｺﾒﾝﾄ文字列
	DWORD	m_dwPaper;			// 図面ｻｲｽﾞ

public:
	CJwwHead() {}
	virtual	void	Serialize(CArchive&);

	DECLARE_SERIAL(CJwwHead)
};

//////////////////////////////////////////////////////////////////////
//	ﾚｲﾔｸﾞﾙｰﾌﾟ, ﾚｲﾔ状態

class CJwwLayerInfo : public CObject  
{
	DWORD	m_dwGLay,
			m_dwWriteLay,
			m_dwGLayProtect;
	double	m_dScale;
	struct	{
		DWORD	m_dwLay,
				m_dwLatProtect;
	} m_Lay[JW_LAYERGRPOUP];

public:
	double	GetScale(void) {
		return m_dScale;
	}

public:
	CJwwLayerInfo() {}
	virtual void	Serialize(CArchive&);

	DECLARE_SERIAL(CJwwLayerInfo)
};

//////////////////////////////////////////////////////////////////////
//	各ﾃﾞｰﾀﾍﾞｰｽｸﾗｽ

class CDataBlock;
class CData;
class CDataList;
typedef	CTypedPtrList<CObList, CData*>		CJwwData;
typedef	CTypedPtrList<CObList, CDataList*>	CJwwBlock;

class CData : public CObject  
{
protected:
	DWORD		m_dwGroup;
	BYTE		m_nPenStyle;
	WORD		m_nPenColor,
				m_nPenWidth;	// Ver3.51～
	WORD		m_nLayer, m_nGLayer;
	WORD		m_bFlg;

	UINT	GetLayer(void) {
		return m_nGLayer * JW_LAYERGRPOUP + m_nLayer;
	}

public:
	CData() {}
	virtual void	Serialize(CArchive&);

	virtual	BOOL	JWWtoNCVCdata(NCVCHANDLE, const CJwwBlock&, const JWLAYER[]) {
		return TRUE;
	}
	virtual	CData*	CopyObject(void) {
		return NULL;
	}
	virtual	void	OrgTuning(CDataBlock*);

	DECLARE_SERIAL(CData)
};

//////////////////////////////////////////////////////////////////////
//	線ﾃﾞｰﾀｸﾗｽ

class CDataSen : public CData
{
	CPointD	m_pts, m_pte;

public:
	CDataSen() {}
	CDataSen(const CDataSen* pData) {
		m_pts = pData->m_pts;
		m_pte = pData->m_pte;
	}
	virtual void	Serialize(CArchive&);

	virtual	BOOL	JWWtoNCVCdata(NCVCHANDLE, const CJwwBlock&, const JWLAYER[]);
	virtual	CData*	CopyObject(void) {
		CDataSen* pData = new CDataSen(this);
		return pData;
	}
	virtual	void	OrgTuning(CDataBlock*);

	DECLARE_SERIAL(CDataSen)
};

//////////////////////////////////////////////////////////////////////
//	円ﾃﾞｰﾀｸﾗｽ

class CDataEnko : public CData
{
	CPointD	m_ptc;
	double	m_r, m_sq, m_eq, m_lq, m_flat;
	DWORD	m_bCircle;

public:
	CDataEnko() {}
	CDataEnko(const CDataEnko* pData) {
		m_ptc = pData->m_ptc;	m_r  = pData->m_r;
		m_sq = pData->m_sq;		m_eq = pData->m_eq;
		m_lq = pData->m_lq;		m_flat = pData->m_flat;
		m_bCircle = pData->m_bCircle;
	}
	virtual void	Serialize(CArchive&);

	virtual	BOOL	JWWtoNCVCdata(NCVCHANDLE, const CJwwBlock&, const JWLAYER[]);
	virtual	CData*	CopyObject(void) {
		CDataEnko* pData = new CDataEnko(this);
		return pData;
	}
	virtual	void	OrgTuning(CDataBlock*);

	DECLARE_SERIAL(CDataEnko)
};

//////////////////////////////////////////////////////////////////////
//	点ﾃﾞｰﾀｸﾗｽ

class CDataTen : public CData
{
	CPointD	m_ptc;
	DWORD	m_bVirtual;

public:
	CDataTen() {}
	CDataTen(const CDataTen* pData) {
		m_ptc = pData->m_ptc;
		m_bVirtual = pData->m_bVirtual;
	}
	virtual void	Serialize(CArchive&);

	virtual	BOOL	JWWtoNCVCdata(NCVCHANDLE, const CJwwBlock&, const JWLAYER[]);
	virtual	CData*	CopyObject(void) {
		CDataTen* pData = new CDataTen(this);
		return pData;
	}
	virtual	void	OrgTuning(CDataBlock*);

	DECLARE_SERIAL(CDataTen)
};

//////////////////////////////////////////////////////////////////////
//	文字ﾃﾞｰﾀｸﾗｽ

class CDataMoji : public CData
{
	CPointD	m_pts, m_pte;
	double	m_dSizeX, m_dSizeY, m_dSpace,
			m_dAngle;
	DWORD	m_nFont;
	CString	m_strFont, m_strText;

public:
	CDataMoji() {}
	CDataMoji(const CDataMoji* pData) {
		m_pts = pData->m_pts;
		m_pte = pData->m_pte;
		m_dSizeX = pData->m_dSizeX;
		m_dSizeY = pData->m_dSizeY;
		m_dSpace = pData->m_dSpace;
		m_dAngle = pData->m_dAngle;
		m_nFont  = pData->m_nFont;
		m_strFont = pData->m_strFont;
		m_strText = pData->m_strText;
	}
	virtual void	Serialize(CArchive&);

	virtual	BOOL	JWWtoNCVCdata(NCVCHANDLE, const CJwwBlock&, const JWLAYER[]);
	virtual	CData*	CopyObject(void) {
		CDataMoji* pData = new CDataMoji(this);
		return pData;
	}
	virtual	void	OrgTuning(CDataBlock*);

	DECLARE_SERIAL(CDataMoji)
};

//////////////////////////////////////////////////////////////////////
//	寸法ﾃﾞｰﾀｸﾗｽ

class CDataSunpou : public CData
{
	CDataSen	m_Line;
	CDataMoji	m_Text;
	// Ver4.20～
	WORD		m_bSxfMode;
	CDataSen	m_SenHo1, m_SenHo2;
	CDataTen	m_Ten1, m_Ten2, m_TenHo1, m_TenHo2;

public:
	CDataSunpou() {}
	virtual void	Serialize(CArchive&);

	DECLARE_SERIAL(CDataSunpou)
};

//////////////////////////////////////////////////////////////////////
//	ｿﾘｯﾄﾞﾃﾞｰﾀｸﾗｽ

class CDataSolid : public CData
{
	CPointD	m_pts, m_pte, m_pt2, m_pt3;
	DWORD	m_dwColor;

public:
	CDataSolid() {}
	virtual void	Serialize(CArchive&);

	DECLARE_SERIAL(CDataSolid)
};

//////////////////////////////////////////////////////////////////////
//	ﾌﾞﾛｯｸﾃﾞｰﾀｸﾗｽ

class CDataBlock : public CData
{
	friend	class	CDataSen;
	friend	class	CDataEnko;
	friend	class	CDataTen;
	friend	class	CDataMoji;

	CPointD	m_oxy, m_mxy;
	double	m_ro;
	DWORD	m_dwID;

	double	m_sin, m_cos;	// m_ro の sin(), cos()
	CDataBlock*		m_pBlk;	// 階層ﾌﾞﾛｯｸ

public:
	CDataBlock() {
		m_pBlk = NULL;
	}
	CDataBlock(const CDataBlock* pData) {
		m_oxy = pData->m_oxy;	m_mxy = pData->m_mxy;
		m_ro = pData->m_ro;		m_dwID = pData->m_dwID;
		m_sin = pData->m_sin;	m_cos = pData->m_cos;
		m_pBlk = pData->m_pBlk;
	}
	virtual void	Serialize(CArchive&);

	virtual	BOOL	JWWtoNCVCdata(NCVCHANDLE, const CJwwBlock&, const JWLAYER[]);
	virtual	CData*	CopyObject(void) {
		CDataBlock* pData = new CDataBlock(this);
		return pData;
	}
	virtual	void	OrgTuning(CDataBlock*);
	//
	void	RoundPoint(CPointD& pt) {
		double xx = pt.x, yy = pt.y;
		pt.x = xx * m_cos - yy * m_sin;
		pt.y = xx * m_sin + yy * m_cos;
	}

	DECLARE_SERIAL(CDataBlock)
};

//////////////////////////////////////////////////////////////////////
//	ﾌﾞﾛｯｸ定義ｸﾗｽ

class CDataList : public CData
{
	friend	class	CDataBlock;

	DWORD		m_dwID, m_dwReffered;
	CTime		m_time;
	CString		m_strBlock;
	// ---
	CJwwData	m_lstMember;	// ﾌﾞﾛｯｸを構成するﾒﾝﾊﾞ

public:
	CDataList() {}
	virtual ~CDataList() {
		for ( POSITION pos=m_lstMember.GetHeadPosition(); pos; )
			delete	m_lstMember.GetNext(pos);
	}
	virtual void	Serialize(CArchive&);

	DECLARE_SERIAL(CDataList)
};
