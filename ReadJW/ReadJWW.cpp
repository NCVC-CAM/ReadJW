#include "stdafx.h"
#include "resource.h"
#include "NCVCaddin.h"
#include "ReadJW.h"
#include "JwwClass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern AFX_EXTENSION_MODULE ReadJWDLL;

/////////////////////////////////////////////////////////////////////////////
//	JWC用 解析関数ﾌﾟﾛﾄﾀｲﾌﾟ

// ﾍｯﾀﾞｰ解析
static	void	ReadJWWinfo(CArchive&, JWLAYER[]);
// ﾃﾞｰﾀ登録
static	BOOL	JWWtoNCVCdata(NCVCHANDLE, const CJwwData&, const CJwwBlock&, const JWLAYER[]);

/////////////////////////////////////////////////////////////////////////////
//	JWW 読み込み
//	NCVC_CreateDXFDocument() の第２引数で指定．
//	NCVC本体から自動的に呼ばれる

NCADDIN BOOL Read_JWW(NCVCHANDLE hDoc, LPCTSTR pszFile)
{
	// ﾘｿｰｽをDLLから呼び出す
	CNCVCExtAddin_ManageState	managestate(ReadJWDLL.hModule);

	POSITION	pos;
	BOOL		bResult = TRUE;
	JWLAYER		jwl[JW_LAYERGRPOUP*JW_LAYERGRPOUP];	// ﾚｲﾔ名
	CString		strMsg;
	CJwwData	lstJWW;
	CJwwBlock	lstBlk;
	CFile		fp;

	// ﾌｧｲﾙｵｰﾌﾟﾝ
	if ( !fp.Open(pszFile, CFile::modeRead | CFile::shareDenyWrite) ) {
		strMsg.Format(IDS_ERR_FILE, pszFile);
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		return FALSE;
	}

	try {
		// ｸﾞﾛｰｻｲｽﾞの設定
//		lstJWW.SetSize(0, 1024);
//		lstBlk.SetSize(0, 1024);
		// ｱｰｶｲﾌﾞ構築
		CArchive	ar(&fp, CArchive::load);
		// ﾍｯﾀﾞｰ読み込み
		ReadJWWinfo(ar, jwl);
		// ﾃﾞｰﾀ読み込み
		lstJWW.Serialize(ar);	// これ１行で読み込みとｸﾗｽｵﾌﾞｼﾞｪｸﾄの生成
		// ﾌﾞﾛｯｸ定義の読み込み
		lstBlk.Serialize(ar);	// CDataList情報

		// -- 画像ﾃﾞｰﾀ等は無視

		// NCVCへﾃﾞｰﾀ登録
		bResult = JWWtoNCVCdata(hDoc, lstJWW, lstBlk, jwl);
	}
	catch ( CUserException* e ) {	// from CJwwHead::Serialize()
		e->Delete();
		return FALSE;
	}
	catch ( CMemoryException* e ) {
		AfxMessageBox(IDS_ERR_OUTOFMEM, MB_OK|MB_ICONSTOP);
		e->Delete();
		bResult = FALSE;
	}
	catch ( CArchiveException* e ) {
		strMsg.Format(IDS_ERR_FILE, pszFile);
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		e->Delete();
		bResult = FALSE;
	}
	catch ( CFileException* e ) {
		strMsg.Format(IDS_ERR_FILE, pszFile);
		AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
		e->Delete();
		bResult = FALSE;
	}

	// CObArrayﾒﾝﾊﾞの削除
	for ( pos=lstJWW.GetHeadPosition(); pos; )
		delete	lstJWW.GetNext(pos);
	for ( pos=lstBlk.GetHeadPosition(); pos; )
		delete	lstBlk.GetNext(pos);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
//	解析関数

void ReadJWWinfo(CArchive& ar, JWLAYER jwl[])
{
	extern	DWORD	g_dwVersion;
	CJwwHead		jwh;
	CJwwLayerInfo	jwi[JW_LAYERGRPOUP];
	int				i, j;
	DWORD			dw;		// ﾀﾞﾐｰ読み込み
	double			dd;
	CString			ss;

	// ﾍｯﾀﾞｰ部読み込み
	jwh.Serialize(ar);
	// ｽｹｰﾙ，ﾚｲﾔ名等の読み込み
	ar >> dw;
	for ( i=0; i<JW_LAYERGRPOUP; i++ )
		jwi[i].Serialize(ar);
	// DWORD x14 -- ﾀﾞﾐｰ
	// DWORD x5  -- 寸法関係の設定
	// DWORD x1  -- ﾀﾞﾐｰ
	// DWORD x1  -- 線描画の最大幅
	for ( i=0; i<21; i++ )
		ar >> dw;
	// プリンタ出力範囲の原点(X,Y)
	// プリンタ出力倍率
	ar >> dd >> dd >> dd;
	// プリンタ90°回転出力、プリンタ出力基準点位置
	// 目盛設定モード
	// 目盛表示最小間隔ドット
	// 目盛表示間隔(X,Y)
	// 目盛基準点(X,Y)
	ar >> dw >> dw
		>> dd >> dd >> dd >> dd >> dd;
	// ﾚｲﾔ名(ﾚｲﾔｸﾞﾙｰﾌﾟのｽｹｰﾙもｾｯﾄ)
	for ( i=0; i<JW_LAYERGRPOUP; i++ ) {
		for ( j=0; j<JW_LAYERGRPOUP; j++ ) {
			ar >> jwl[i*JW_LAYERGRPOUP+j].strLayer;
			jwl[i*JW_LAYERGRPOUP+j].dScale = jwi[i].GetScale();
		}
	}
	// ﾚｲﾔｸﾞﾙｰﾌﾟ名
	for ( i=0; i<JW_LAYERGRPOUP; i++ )
		ar >> ss;

	// -- 以降の数値ﾃﾞｰﾀはNCVCでは読み飛ばし --

	// 日影計算の条件
	// 測定面高さ
	// 緯度
	// 9～15の測定の指定(DWORD)
	// 壁面日影測定面高さ
	ar >> dd >> dd >> dw >> dd;
	// 天空図の条件（Ver.3.00以降)
	// 測定面高さ
	// 天空図の半径×2
	if ( g_dwVersion >= JWWVER_300 )
		ar >> dd >> dd;
	// 2.5Dの計算単位(0以外はmm単位で計算)
	ar >> dw;
	// 保存時の画面倍率(読込むと前画面倍率になる)
	// 範囲記憶倍率と基準点(X,Y)
	ar >> dd >> dd >> dd >> dd >> dd >> dd;
	// マークジャンプ倍率、基準点(X,Y)およびレイヤグループ
	if ( g_dwVersion >= JWWVER_300 ) {
		for ( i=1; i<=8; i++ )
			ar >> dd >> dd >> dd >> dw;
		// 文字の描画状態(Ver.4.05以降）
		ar >> dd >> dd >> dd >> dw >> dd >> dd >> dd >> dw;
	}
	else {
		for ( i=1; i<=4; i++ )
			ar >> dd >> dd >> dd;
	}
	// 複線間隔
	for ( i=0; i<=9; i++ ) 
		ar >> dd;
	// 両側複線の留線出の寸法
	ar >> dd;
	// 色番号ごとの画面表示色、線幅
	for ( i=0; i<=9; i++ )
	    ar >> dw >> dw;
	// 色番号ごとのプリンタ出力色、線幅、実点半径
	for ( i=0; i<=9; i++ )
		ar >> dw >> dw >> dd;
	// 線種番号2から9までのパターン、1ユニットのドット数、ピッチ、プリンタ出力ピッチ
	for ( i=2; i<=9; i++ )
		ar >> dw >> dw >> dw >> dw;
	// ランダム線1から5までのパターン、画面表示振幅・ピッチ、プリンタ出力振幅・ピッチ
	for ( i=11; i<=15; i++ )
		ar >> dw >> dw >> dw >> dw >> dw;
	// 倍長線種番号6から9までのパターン、1ユニットのドット数、ピッチ、プリンタ出力ピッチ
	for ( i=16; i<=19; i++ )
	    ar >> dw >> dw >> dw >> dw;
	// 実点を画面描画時の指定半径で描画
	// 実点をプリンタ出力時、指定半径で書く
	// BitMap・ソリッドを最初に描画する
	// 逆描画, 逆サーチ, カラー印刷
	// レイヤ順の印刷, 色番号順の印刷
	// レイヤグループまたはレイヤごとのプリンタ連続出力指定
	// プリンタ共通レイヤ(表示のみレイヤ)のグレー出力指定
	// プリンタ出力時に表示のみレイヤは出力しない
	// 作図時間（Ver.2.23以降）
	// 2.5Dの始点位置が設定されている時のフラグ（Ver.2.23以降）
	// 2.5Dの透視図・鳥瞰図・アイソメ図の視点水平角（Ver.2.23以降）
	ar >> dw >> dw >> dw >> dw >> dw
		>> dw >> dw >> dw >> dw >> dw
		>> dw >> dw >> dw >> dw >> dw >> dw;
	// 2.5Dの透視図の視点高さ・視点離れ（Ver.2.23以降）
	// 2.5Dの鳥瞰図の視点高さ・視点離れ（Ver.2.23以降）
	// 2.5Dのアイソメ図の視点垂直角（Ver.2.23以降）
	// 線の長さ指定の最終値（Ver.2.25以降）
	// 矩形寸法横寸法・縦寸法指定の最終値（Ver.2.25以降）
	// 円の半径指定の最終値（Ver.2.25以降）
	// ソリッドを任意色で書くフラグ、ソリッドの任意色の既定値（Ver.2.30以降）
	ar >> dd >> dd >> dd >> dd >> dd
		>> dd >> dd >> dd >> dd >> dw >> dw;

	// SXF対応拡張線色定義（Ver.4.20以降）
	if ( g_dwVersion >= JWWVER_420 ) {
		for ( i=0; i<=256; i++ )
			ar >> dw >> dw;
		for ( i=0; i<=256; i++ )
			ar >> ss >> dw >> dw >> dd;
		for ( i=0; i<=32; i++ )
			ar >> dw >> dw >> dw >> dw;
		for ( i=0; i<=32; i++ ) {
			ar >> ss >> dw;
			for ( j=1; j<=10; j++ )
				ar >> dd;
		}
	}

	// 文字種1から10までの文字幅、高さ、間隔、色番号
	for ( i=1; i<=10; i++ )
		ar >> dd >> dd >> dd >> dw;
	// 書込み文字の文字幅、高さ、間隔、色番号、文字番号
	// 文字位置整理の行間、文字数
	// 文字基準点のずれ位置使用のフラグ
	// 文字基準点の横方向のずれ位置左、中、右
	// 文字基準点の縦方向のずれ位置下、中、上
	ar >> dd >> dd >> dd >> dw >> dw
		>> dd >> dd >> dw >> dd >> dd >> dd
		>> dd >> dd >> dd;
}

/////////////////////////////////////////////////////////////////////////////
//	各種ﾃﾞｰﾀ解析関数

BOOL JWWtoNCVCdata
	(NCVCHANDLE hDoc, const CJwwData& lstJWW, const CJwwBlock& lstBlk, const JWLAYER jwl[])
{
	POSITION	pos;
	int			i;
	CData*	pData;
	CString	strMsg;

	// ﾒｲﾝﾌﾚｰﾑのﾌﾟﾛｸﾞﾚｽﾊﾞｰ初期化
	NCVC_MainfrmProgressRange(0, lstJWW.GetCount());

	for ( i=0, pos=lstJWW.GetHeadPosition(); pos; i++ ) {
		pData = lstJWW.GetNext(pos);
		if ( !pData->JWWtoNCVCdata(hDoc, lstBlk, jwl) ) {
			strMsg.Format(IDS_ERR_ADDNCVC, -1, i+1);	// 第２引数は省略
			AfxMessageBox(strMsg, MB_OK|MB_ICONSTOP);
			return FALSE;
		}
		// ﾌﾟﾛｸﾞﾚｽﾊﾞｰ更新(64回おき)
		if ( (i & 0x003f) == 0 )
			NCVC_MainfrmProgressPos(i);
	}

	return TRUE;
}
