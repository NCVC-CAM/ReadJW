#include "stdafx.h"
#include "resource.h"
#include "NCVCaddin.h"
#include "ReadJW.h"

#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern AFX_EXTENSION_MODULE ReadJWDLL;

/////////////////////////////////////////////////////////////////////////////
//	JWC用 構造体定義．!!!1byteｱﾗｲﾒﾝﾄ!!!

#pragma	pack(1)
// ﾍｯﾀﾞｰ情報(200byte x 4)
typedef	struct	tagJWCHEAD {
	// ﾍｯﾀﾞｰ#1
	char	szJWID1[20];	// 識別用
	char	szJWID2[20];
	char	szDummy1[160];
	// ﾍｯﾀﾞｰ#2
	char	szDataCnt[200];	// 線，円，文字列，実点，仮点のﾃﾞｰﾀ数がCSV形式で格納
	// ﾍｯﾀﾞｰ#3
	char	szDummy2[200];
	// ﾍｯﾀﾞｰ#4
	char	szTextAddress[200];	// 文字列ﾊﾞｯﾌｧのｱﾄﾞﾚｽ
} JWCHEAD;

// ﾃﾞｰﾀ部(読み飛ばし)情報
typedef	struct	tagJWCDATA {
	char	szDummy1[909];	// 仮点情報(404+404+101)
	char	szDummy2[88];	// 文字情報(22+22+22+22)
} JWCDATA;	// 定義のみ

// ﾃﾞｰﾀ部(ﾚｲﾔ)情報
typedef	struct	tagJWCLAYERINFO1 {
	float	dScale[JW_LAYERGRPOUP];		// ｽｹｰﾙ(実数4byte)
	char	szDummy[560];		// ﾚｲﾔ他情報(16+256+16+256+16)
} JWCLAYERINFO1;
typedef	struct	tagJWCLAYERINFO2 {
	short	nScale[JW_LAYERGRPOUP];		// ｽｹｰﾙ(整数2byte)
	char	szDummy[560];
} JWCLAYERINFO2;

// ﾃﾞｰﾀ部
typedef	struct	tagJWCLINE {
	CPointF	pts, pte;			// 始点終点
	unsigned char	sType, sColor, sLayer;	// 線種線色，ﾚｲﾔ
	char	szDummy[3];
} JWCLINE;
typedef	struct	tagJWCCIRCLE {
	CPointF	ptc;				// 中心XY
	float	r;					// 半径
	short	flat;				// 扁平率x10000
	long	sq, eq, lq;			// 開始終了角度x65536(度), 傾きx65536
	unsigned char	sType, sColor, sLayer;	// 線種線色，ﾚｲﾔ
	char	szDummy[3];
} JWCCIRCLE;
typedef	struct	tagJWCTEXT {
	CPointF	pts, pte;			// 始点終点
	LPVOID	lpText;				// 文字列へのｱﾄﾞﾚｽ
	char	szDummy1;
	unsigned char	sLayer;		// ﾚｲﾔ
	char	szDummy2[2];
} JWCTEXT;
typedef	struct	tagJWCPOINT {
	CPointF	pt;					// 点座標
	unsigned char	sLayer;		// ﾚｲﾔ
	char	szDummy[3];
} JWCPOINT;
#pragma	pack()

/////////////////////////////////////////////////////////////////////////////
//	JWC用 解析関数ﾌﾟﾛﾄﾀｲﾌﾟ

// ﾍｯﾀﾞｰ解析
static	BOOL	CheckHeader(CFile&, BOOL&, int[], float&, int&);
// ﾃﾞｰﾀ解析１
static	BOOL	CheckDataIntro(CFile&, BOOL, JWLAYER[]);
// ﾚｲﾔ名取得
static	BOOL	GetLayerName(CFile&, JWLAYER[]);
// 各種ﾃﾞｰﾀ処理
static	BOOL	GetJwcLine(NCVCHANDLE, CFile&, int[], float, const JWLAYER[]);
static	BOOL	GetJwcCircle(NCVCHANDLE, CFile&, int[], float, const JWLAYER[]);
static	BOOL	GetJwcText(NCVCHANDLE, CFile&, int[], float, const JWLAYER[], const char*);
static	BOOL	GetJwcPoint(NCVCHANDLE, CFile&, int[], float, const JWLAYER[]);

/////////////////////////////////////////////////////////////////////////////
//	JWC 読み込み
//	NCVC_CreateDXFDocument() の第２引数で指定．
//	NCVC本体から自動的に呼ばれる

NCADDIN BOOL Read_JWC(NCVCHANDLE hDoc, LPCTSTR pszFile)
{
	// ﾘｿｰｽをDLLから呼び出す
	CNCVCExtAddin_ManageState	managestate(ReadJWDLL.hModule);

	BOOL		bScale;		// ｽｹｰﾙが実数(TRUE)か否(FALSE)か
	int			nData[JW_MAXDATA],	// 線，円，文字列，実点，仮点の数
				nText;				// 文字列領域の長さ
	char*		pszText = NULL;		// 文字情報
	float		dUnit;				// 単位ﾕﾆｯﾄ
	JWLAYER		jwl[JW_LAYERGRPOUP*JW_LAYERGRPOUP];	// ﾚｲﾔ名
	CString		strMsg;

	/*	--- #pragma	pack(1) が必要なことがわかった
#ifdef _DEBUG
	printf("sizeof(JWCLINE)=%d\n", sizeof(JWCLINE));
	printf("sizeof(JWCCIRCLE)=%d\n", sizeof(JWCCIRCLE));
	printf("sizeof(JWCTEXT)=%d\n", sizeof(JWCTEXT));
	printf("sizeof(JWCPOINT)=%d\n", sizeof(JWCPOINT));
#endif
*/
	try {
		// ﾌｧｲﾙｵｰﾌﾟﾝ
		CFile	fp(pszFile, CFile::modeRead | CFile::shareDenyWrite);
		// ﾍｯﾀﾞｰﾁｪｯｸ
		if ( !CheckHeader(fp, bScale, nData, dUnit, nText) )
			return FALSE;
#ifdef _DEBUG
		printf("Read_JWC() bScale=%d nText=%d\n", bScale, nText);
		printf("           Cnt l=%d c=%d t=%d p=%d\n",
			nData[JW_LINE], nData[JW_CIRCLE], nData[JW_TEXT], nData[JW_POINT]);
#endif
		// ﾃﾞｰﾀ部序章
		if ( !CheckDataIntro(fp, bScale, jwl) )
			return FALSE;
		// 本ﾃﾞｰﾀ...の前にﾃｷｽﾄ情報とﾚｲﾔ名を取得(ﾃﾞｰﾀの読み飛ばし)
		fp.Seek(
			sizeof(JWCLINE)   * nData[JW_LINE] +
			sizeof(JWCCIRCLE) * nData[JW_CIRCLE] +
			sizeof(JWCTEXT)   * nData[JW_TEXT],
			CFile::current);
		// 文字列情報
		if ( nText > 0 ) {
			pszText = new char[nText];
			if ( fp.Read(pszText, nText) != (UINT)nText ) {
				delete	pszText;
				strMsg.Format(IDS_ERR_JW, pszFile);
				AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
				return FALSE;
			}
		}
		fp.Seek(sizeof(JWCPOINT) * nData[JW_POINT], CFile::current);
		// ﾚｲﾔ情報
		if ( !GetLayerName(fp, jwl) ) {
			if ( pszText ) delete pszText;
			return FALSE;
		}
		fp.Seek(	// 本ﾃﾞｰﾀの位置まで戻す
			sizeof(JWCHEAD) + sizeof(JWCDATA) +
			(bScale ? sizeof(JWCLAYERINFO1) : sizeof(JWCLAYERINFO2)),
			CFile::begin);
		// ﾒｲﾝﾌﾚｰﾑのﾌﾟﾛｸﾞﾚｽﾊﾞｰ初期化
		NCVC_MainfrmProgressRange(0,
			nData[JW_LINE]+nData[JW_CIRCLE]+nData[JW_TEXT]+nData[JW_POINT]-1);
		// 本ﾃﾞｰﾀ取得処理
		if ( !GetJwcLine(hDoc, fp, nData, dUnit, jwl) ) {
			if ( pszText ) delete pszText;
			return FALSE;
		}
		if ( !GetJwcCircle(hDoc, fp, nData, dUnit, jwl) ) {
			if ( pszText ) delete pszText;
			return FALSE;
		}
		if ( !GetJwcText(hDoc, fp, nData, dUnit, jwl, pszText) ) {
			if ( pszText ) delete pszText;
			return FALSE;
		}
		if ( pszText ) delete pszText;
		fp.Seek(nText, CFile::current);	// 文字列ﾊﾞｯﾌｧ分読み飛ばし
		if ( !GetJwcPoint(hDoc, fp, nData, dUnit, jwl) )
			return FALSE;
	}
	catch ( CMemoryException* e ) {
		if ( pszText ) delete pszText;
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		e->Delete();
		return FALSE;
	}
	catch ( CFileException* e ) {
		if ( pszText ) delete pszText;
		strMsg.Format(IDS_ERR_FILE, pszFile);
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		e->Delete();
		return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	解析関数

BOOL CheckHeader(CFile& fp, BOOL& bScale, int nData[], float& dUnit, int& nTextLength)
{
	static	const	char	szJWID[] = "jw_cad(c)data.......";
	static	const	char	szDelimiter[] = ",";
	static	const	int		nPaperSize[] = {	// A0, A1, A2, A3, A4
		1189, 841, 594, 420, 297
	};
	int			i;
	char*		pszContext;
	char*		pszCnt;				// ﾃﾞｰﾀ数
	char*		pszTextAddress[2];	// 開始終了ｱﾄﾞﾚｽ
	int			nTextAddress[2],	// ｱﾄﾞﾚｽ値
				nPaper = 3,			// 用紙(A3)
				nWindow = 518;		// 作図ｳｨﾝﾄﾞｳ(640ﾄﾞｯﾄ表示)
	JWCHEAD		jwh;
	CString		strMsg;

	// ﾍｯﾀﾞｰ部読み込みﾁｪｯｸ
	if ( fp.Read(&jwh, sizeof(JWCHEAD)) != sizeof(JWCHEAD) ||
			memcmp(szJWID, jwh.szJWID1, sizeof(jwh.szJWID1)) != 0 ) {
		strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	// ｽｹｰﾙ(実数か否か)
	bScale = jwh.szJWID2[2] == 'f' ? TRUE : FALSE;
	// ﾃﾞｰﾀ数の取得
	for ( i=0; i<JW_MAXDATA; nData[i++] = 0 );
	i = 0;
	pszCnt = strtok_s(jwh.szDataCnt, szDelimiter, &pszContext);
	while ( pszCnt && i<JW_MAXDATA ) {
		nData[i++] = atoi(pszCnt);
		pszCnt = strtok_s(NULL, szDelimiter, &pszContext);
	}
	// 用紙ｻｲｽﾞ，作図ｳｨﾝﾄﾞｳの幅を取得
	while ( pszCnt && i<JW_MAXDATA+7) {
		pszCnt = strtok_s(NULL, szDelimiter, &pszContext);
		i++;
	}
	if ( pszCnt )
		nPaper = atoi(pszCnt);
	while ( pszCnt && i<JW_MAXDATA+26) {
		pszCnt = strtok_s(NULL, szDelimiter, &pszContext);
		i++;
	}
	if ( pszCnt )
		nWindow = atoi(pszCnt);
	// ﾕﾆｯﾄ計算
	if ( nPaper<0 || nPaper>=SIZEOF(nPaperSize) ) {
		AfxMessageBox(IDS_ERR_JWPAPER, MB_OK|MB_ICONSTOP);
		return FALSE;
	}
	dUnit = (float)nPaperSize[nPaper] / nWindow;
	// 文字列領域の長さ計算
	pszTextAddress[0] = strtok_s(jwh.szTextAddress, szDelimiter, &pszContext);
	pszTextAddress[1] = strtok_s(NULL, szDelimiter, &pszContext);
	for ( i=0; i<2; i++ ) {
		sscanf_s(strtok_s(pszTextAddress[i], ":", &pszContext), "%4x", &nTextAddress[0]);
		sscanf_s(strtok_s(NULL, ":", &pszContext), "%4x", &nTextAddress[1]);
		pszTextAddress[i] = (char *)(nTextAddress[0] * 16 + nTextAddress[1]);
	}
	nTextLength = pszTextAddress[1] - pszTextAddress[0];

	return TRUE;
}

BOOL CheckDataIntro(CFile& fp, BOOL bScale, JWLAYER jwl[])
{
	int			i, j;
	JWCLAYERINFO1	jwl1;
	JWCLAYERINFO2	jwl2;
	CString		strMsg;

	// ﾃﾞｰﾀ部(序章)の読み飛ばし
	fp.Seek(sizeof(JWCDATA), CFile::current);
	// 実数ｽｹｰﾙか否かで読み込む構造体を変更
	if ( bScale ) {
		if ( fp.Read(&jwl1, sizeof(JWCLAYERINFO1)) != sizeof(JWCLAYERINFO1) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		for ( i=0; i<JW_LAYERGRPOUP; i++ ) {
			for ( j=0; j<JW_LAYERGRPOUP; j++ ) {
				jwl[i*JW_LAYERGRPOUP+j].dScale = jwl1.dScale[i];
			}
		}
	}
	else {
		if ( fp.Read(&jwl2, sizeof(JWCLAYERINFO2)) != sizeof(JWCLAYERINFO2) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		for ( i=0; i<JW_LAYERGRPOUP; i++ ) {
			for ( j=0; j<JW_LAYERGRPOUP; j++ ) {
				jwl[i*JW_LAYERGRPOUP+j].dScale = (double)jwl2.nScale[i];
			}
		}
	}

	return TRUE;
}

BOOL GetLayerName(CFile& fp, JWLAYER jwl[])
{
	char		szLayerArray[JW_LAYERGRPOUP*JW_LAYERGRPOUP][JW_LAYERLENGTH],
				szLayer[JW_LAYERLENGTH+1];
	CString		strMsg;

	if ( fp.Read(szLayerArray, sizeof(szLayerArray)) != sizeof(szLayerArray) ) {
		strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	// ﾚｲﾔ情報をそれぞれに分割
	for ( int i=0; i<JW_LAYERGRPOUP*JW_LAYERGRPOUP; i++ ) {
		ZeroMemory(szLayer, sizeof(szLayer));
		memcpy(szLayer, szLayerArray[i], JW_LAYERLENGTH);
		jwl[i].strLayer = szLayer;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	各種ﾃﾞｰﾀ解析関数(ｻﾌﾞ)

inline int CheckCircle(const JWCCIRCLE* jw)
{
//	return code 0:真円，1:円弧，2:楕円，3:楕円弧
	int	nResult;
	if ( jw->flat / 10000.0 == 1.0 )	// 扁平率
		nResult = ( jw->sq == jw->eq ) ? 0 : 1;
	else
		nResult = ( jw->sq == jw->eq ) ? 2 : 3;

	return nResult;
}

/////////////////////////////////////////////////////////////////////////////
//	各種ﾃﾞｰﾀ解析関数

BOOL GetJwcLine
	(NCVCHANDLE hDoc, CFile& fp, int nCnt[], float dUnit, const JWLAYER jwl[])
{
	BOOL		bResult;
	int			nLayer;
	float		dScale;
	JWCLINE		jw;
	DXFDATA		dxf;
	DPOINT		pts, pte;
	CString		strMsg;

	dxf.dwSize  = sizeof(DXFDATA);
	dxf.enType  = DXFLINEDATA;

	for ( int i=0; i<nCnt[JW_LINE]; i++ ) {
		if ( fp.Read(&jw, sizeof(JWCLINE)) != sizeof(JWCLINE) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		// ﾚｲﾔﾁｪｯｸ
		nLayer = CheckDataLayer(jwl, jw.sLayer);
		if ( nLayer == DXFORGLAYER ) {
			dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
			pts = jw.pts * dScale;
			pte = jw.pte * dScale;
			NCVC_SetDXFLatheLine(hDoc, &pts, &pte);
		}
		if ( nLayer>=DXFCAMLAYER && nLayer<=DXFMOVLAYER ) {
			dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
			dxf.ptS    = jw.pts * dScale;
			dxf.de.ptE = jw.pte * dScale;
			dxf.nLayer = nLayer;
			lstrcpyn(dxf.szLayer, jwl[jw.sLayer].strLayer,
				min(sizeof(dxf.szLayer), jwl[jw.sLayer].strLayer.GetLength()+1));
			bResult = NCVC_AddDXFData(hDoc, &dxf);
			if ( !bResult ) {
				strMsg.Format(IDS_ERR_ADDNCVC, DXFLINEDATA, i);
				AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
				return FALSE;
			}
		}
		// ﾌﾟﾛｸﾞﾚｽﾊﾞｰ更新(64回おき)
		if ( (i & 0x003f) == 0 )
			NCVC_MainfrmProgressPos(i);
	}

	return TRUE;
}

BOOL GetJwcCircle
	(NCVCHANDLE hDoc, CFile& fp, int nCnt[], float dUnit, const JWLAYER jwl[])
{
	int			nPos;
	float		dScale;
	JWCCIRCLE	jw;
	DXFDATA		dxf;
	DPOINT		pt;
	double		lq;
	CString		strMsg;

	dxf.dwSize  = sizeof(DXFDATA);

	for ( int i=0; i<nCnt[JW_CIRCLE]; i++ ) {
		if ( fp.Read(&jw, sizeof(JWCCIRCLE)) != sizeof(JWCCIRCLE) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		// ﾚｲﾔﾁｪｯｸ
		switch ( CheckDataLayer(jwl, jw.sLayer) ) {
		case DXFORGLAYER:
			// 真円の時だけ原点登録
			if ( CheckCircle(&jw) == 0 ) {
				dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
				pt = jw.ptc * dScale;
				NCVC_SetDXFCutterOrigin(hDoc, &pt, jw.r*dScale, FALSE);
			}
			break;
		case DXFCAMLAYER:
			dxf.nLayer = DXFCAMLAYER;
			lstrcpyn(dxf.szLayer, jwl[jw.sLayer].strLayer,
				min(sizeof(dxf.szLayer), jwl[jw.sLayer].strLayer.GetLength()+1));
			dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
			dxf.ptS = jw.ptc * dScale;
			switch ( CheckCircle(&jw) ) {
			case 0:		// 真円
				dxf.enType = DXFCIRCLEDATA;
				dxf.de.dR = jw.r * dScale;
				break;
			case 1:		// 円弧(角度単位は度)
				dxf.enType = DXFARCDATA;
				dxf.de.arc.r = jw.r * dScale;
				dxf.de.arc.sq = jw.sq / 65536.0;
				dxf.de.arc.eq = jw.eq / 65536.0;
				break;
			case 2:		// 楕円(角度単位はﾗｼﾞｱﾝ)
			case 3:		// 楕円弧
				dxf.enType = DXFELLIPSEDATA;
				dxf.de.elli.sq = RAD(jw.sq / 65536.0);
				dxf.de.elli.eq = RAD(jw.eq / 65536.0);
				lq = RAD(jw.lq / 65536.0);
				dxf.de.elli.ptL.x = jw.r * cos(lq) * dScale;	// 長軸は原点からの相対座標
				dxf.de.elli.ptL.y = jw.r * sin(lq) * dScale;
				dxf.de.elli.s = jw.flat / 10000.0;
				break;
			}
			if ( !NCVC_AddDXFData(hDoc, &dxf) ) {
				strMsg.Format(IDS_ERR_ADDNCVC, DXFCIRCLEDATA, i);
				AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
				return FALSE;
			}
			break;
		case DXFSTRLAYER:
			// 真円の時だけ加工開始位置登録
			if ( CheckCircle(&jw) == 0 ) {
				dxf.nLayer = DXFSTRLAYER;
				lstrcpyn(dxf.szLayer, jwl[jw.sLayer].strLayer,
					min(sizeof(dxf.szLayer), jwl[jw.sLayer].strLayer.GetLength()+1));
				dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
				dxf.ptS   = jw.ptc * dScale;
				dxf.de.dR = jw.r * dScale;
				if ( !NCVC_AddDXFData(hDoc, &dxf) ) {
					strMsg.Format(IDS_ERR_ADDNCVC, DXFCIRCLEDATA, i);
					AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
					return FALSE;
				}
			}
			break;
		}
		// ﾌﾟﾛｸﾞﾚｽﾊﾞｰ更新(64回おき)
		nPos = i + nCnt[JW_LINE];
		if ( (nPos & 0x003f) == 0 )
			NCVC_MainfrmProgressPos(nPos);
	}

	return TRUE;
}

BOOL GetJwcText
	(NCVCHANDLE hDoc, CFile& fp, int nCnt[], float dUnit, const JWLAYER jwl[], const char* pszTextInfo)
{
	BOOL		bResult;
	int			nLayer, nPos, nBase = nCnt[JW_LINE] + nCnt[JW_CIRCLE];
	float		dScale;
	char*		pszText;
	JWCTEXT		jw;
	DXFDATA		dxf;
	CString		strMsg;

	dxf.dwSize  = sizeof(DXFDATA);
	dxf.enType  = DXFTEXTDATA;

	for ( int i=0; i<nCnt[JW_TEXT]; i++ ) {
		if ( fp.Read(&jw, sizeof(JWCTEXT)) != sizeof(JWCTEXT) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		// ﾚｲﾔﾁｪｯｸ
		nLayer = CheckDataLayer(jwl, jw.sLayer);
		if ( nLayer>=DXFCAMLAYER && nLayer<=DXFCOMLAYER ) {
			dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
			dxf.ptS = jw.pts * dScale;
			dxf.nLayer = nLayer;
			pszText = (char *)jw.lpText - 0x40000000 + (unsigned int)pszTextInfo;
			lstrcpyn(dxf.szLayer, jwl[jw.sLayer].strLayer,
				min(sizeof(dxf.szLayer), jwl[jw.sLayer].strLayer.GetLength()+1));
			lstrcpyn(dxf.de.szText, pszText,
				min(sizeof(dxf.de.szText), lstrlen(pszText)+1));
			bResult = NCVC_AddDXFData(hDoc, &dxf);
			if ( !bResult ) {
				strMsg.Format(IDS_ERR_ADDNCVC, DXFTEXTDATA, i);
				AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
				return FALSE;
			}
		}
		// ﾌﾟﾛｸﾞﾚｽﾊﾞｰ更新(64回おき)
		nPos = i + nBase;
		if ( (nPos & 0x003f) == 0 )
			NCVC_MainfrmProgressPos(nPos);
	}

	return TRUE;
}

BOOL GetJwcPoint
	(NCVCHANDLE hDoc, CFile& fp, int nCnt[], float dUnit, const JWLAYER jwl[])
{
	int			nPos, nBase = nCnt[JW_LINE] + nCnt[JW_CIRCLE] + nCnt[JW_TEXT];
	float		dScale;
	JWCPOINT	jw;
	DXFDATA		dxf;
	CString		strMsg;

	dxf.dwSize  = sizeof(DXFDATA);
	dxf.enType  = DXFPOINTDATA;

	for ( int i=0; i<nCnt[JW_POINT]; i++ ) {
		if ( fp.Read(&jw, sizeof(JWCPOINT)) != sizeof(JWCPOINT) ) {
			strMsg.Format(IDS_ERR_JW, fp.GetFilePath());
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		// ﾚｲﾔﾁｪｯｸ
		if ( CheckDataLayer(jwl, jw.sLayer) == DXFCAMLAYER ) {
			lstrcpyn(dxf.szLayer, jwl[jw.sLayer].strLayer,
				min(sizeof(dxf.szLayer), jwl[jw.sLayer].strLayer.GetLength()+1));
			dScale = (float)(dUnit * jwl[jw.sLayer].dScale);
			dxf.ptS = jw.pt * dScale;
			dxf.nLayer = DXFCAMLAYER;
			if ( !NCVC_AddDXFData(hDoc, &dxf) ) {
				strMsg.Format(IDS_ERR_ADDNCVC, DXFPOINTDATA, i);
				AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
				return FALSE;
			}
		}
		// ﾌﾟﾛｸﾞﾚｽﾊﾞｰ更新(64回おき)
		nPos = i + nBase;
		if ( (nPos & 0x003f) == 0 )
			NCVC_MainfrmProgressPos(nPos);
	}

	return TRUE;
}
