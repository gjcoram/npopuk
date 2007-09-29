/**************************************************************************

	nPOP

	Multipart.c

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

/**************************************************************************
	Include Files
**************************************************************************/

#include "General.h"


/**************************************************************************
	Define
**************************************************************************/

/**************************************************************************
	Global Variables
**************************************************************************/

extern int EncodeType;


/**************************************************************************
	Local Function Prototypes
**************************************************************************/

static BOOL GetContentValue(TCHAR *Content, TCHAR *Attribute, TCHAR *ret);
static TCHAR *NextPartPos(TCHAR *buf, TCHAR *Boundary);
static void GetContent(TCHAR *buf, TCHAR *str, TCHAR **ret);
static TCHAR *GetFilenameEx(TCHAR *Content);


/******************************************************************************

	AddMultiPartInfo

	Part���̒ǉ�

******************************************************************************/


struct TPMULTIPART *AddMultiPartInfo(struct TPMULTIPART ***tpMultiPart, int cnt)
{
	struct TPMULTIPART **TmpMultiPart;

	TmpMultiPart = (struct TPMULTIPART **)LocalAlloc(LPTR, sizeof(struct TPMULTIPART *) * (cnt + 1));
	if(TmpMultiPart == NULL){
		return NULL;
	}
	if(*tpMultiPart != NULL){
		tCopyMemory(TmpMultiPart, *tpMultiPart,
			sizeof(struct TPMULTIPART *) * cnt);
		LocalFree(*tpMultiPart);
	}
	*(TmpMultiPart + cnt) = (struct TPMULTIPART *)LocalAlloc(LPTR, sizeof(struct TPMULTIPART));
	*tpMultiPart = TmpMultiPart;
	return *(TmpMultiPart + cnt);
}


/******************************************************************************

	FreeMultipartInfo

	Part���̉��

******************************************************************************/

void FreeMultipartInfo(struct TPMULTIPART ***tpMultiPart, int cnt)
{
	int i;

	//Part���̉��
	for(i = 0; i < cnt; i++){
		if(*(*tpMultiPart + i) == NULL){
			continue;
		}
		NULLCHECK_FREE((*(*tpMultiPart + i))->ContentType);
		NULLCHECK_FREE((*(*tpMultiPart + i))->Filename);
		NULLCHECK_FREE((*(*tpMultiPart + i))->Encoding);

		LocalFree(*(*tpMultiPart + i));
	}
	NULLCHECK_FREE(*tpMultiPart);
}


/******************************************************************************

	GetContentValue

	�R���e���c���̎w�肵���l���擾

******************************************************************************/

static BOOL GetContentValue(TCHAR *Content, TCHAR *Attribute, TCHAR *ret)
{
	TCHAR *p = Content;
	TCHAR *r = ret;

	for(; *p == TEXT(' ') || *p == TEXT('\t'); p++);

	while(*p != TEXT('\0')){
		//attribute���`�F�b�N
		if(TStrCmpNI(p, Attribute, lstrlen(Attribute)) != 0){
			//����parameter�Ɉړ�
			for(; *p != TEXT('\0') && *p != TEXT(';'); p++);
			if(*p == TEXT(';')) p++;
			for(; *p == TEXT(' ') || *p == TEXT('\t'); p++);
			continue;
		}
		p += lstrlen(Attribute);
		for(; *p == TEXT(' ') || *p == TEXT('\t'); p++);
		if(*p == TEXT('=')){
			p++;
		}
		for(; *p == TEXT(' ') || *p == TEXT('\t'); p++);
		if(*p == TEXT('\"')){
			p++;
		}
		//value�̎擾
		for(; *p != TEXT('\0') && *p != TEXT('\"') && *p != TEXT(';'); p++, r++){
			*r = *p;
		}
		*r = TEXT('\0');
		return TRUE;
	}
	return FALSE;
}


/******************************************************************************

	NextPartPos

	����Part�̈ʒu���擾

******************************************************************************/

static TCHAR *NextPartPos(TCHAR *buf, TCHAR *Boundary)
{
	TCHAR *p = buf;

	if(*p == TEXT('-') && *(p + 1) == TEXT('-') && TStrCmpNI(p + 2, Boundary, lstrlen(Boundary)) == 0){
		return p;
	}
	while(1){
		for(; !(*p == TEXT('\r') && *(p + 1) == TEXT('\n')) && *p != TEXT('\0'); p++);
		if(*p == TEXT('\0')){
			break;
		}
		p += 2;
		if(!(*p == TEXT('-') && *(p + 1) == TEXT('-'))){
			continue;
		}
		if(TStrCmpNI(p + 2, Boundary, lstrlen(Boundary)) != 0){
			continue;
		}
		break;
	}
	return p;
}


/******************************************************************************

	GetContent

	�R���e���c�̎擾

******************************************************************************/

static void GetContent(TCHAR *buf, TCHAR *str, TCHAR **ret)
{
	TCHAR *p;
	int len;

	//�ʒu�̎擾
	p = GetHeaderStringPointT(buf, str);
	if(p == NULL){
		*ret = NULL;
		return;
	}
	//�T�C�Y�̎擾
	len = GetHeaderStringSizeT(p, FALSE);
	*ret = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(*ret == NULL){
		return;
	}
	GetHeaderStringT(p, *ret, FALSE);
}


/******************************************************************************

	GetFileNameEx

	�t�@�C�����̎擾 (RFC 2231)

******************************************************************************/

static TCHAR *GetFilenameEx(TCHAR *buf)
{
	TCHAR *p = buf, *r;
	TCHAR *tmp;
	TCHAR *ret;
	TCHAR **Names = NULL, **NamesTmp;
	int No;
	int AllocNo = 0;
	int len;
	BOOL EncFlag = FALSE;

	if(buf == NULL){
		return NULL;
	}

	while(*p != TEXT('\0')){
		//attribute���`�F�b�N
		if(TStrCmpNI(p, TEXT("filename*"), lstrlen(TEXT("filename*"))) != 0){
			//����parameter�Ɉړ�
			for(; *p != TEXT('\0') && *p != TEXT(';'); p++);
			if(*p == TEXT(';')) p++;
			for(; *p == TEXT(' ') || *p == TEXT('\t'); p++);
			continue;
		}
		p += lstrlen(TEXT("filename"));
		if(*(p + 1) == TEXT('=')){
			//�P��̏ꍇ
			No = 0;
		}else{
			//�A�Ԃ��t���Ă���ꍇ
			p++;
			No = _ttoi(p);
		}
		if(AllocNo <= No){
			//�t�@�C�����̃��X�g�̊m��
			NamesTmp = (TCHAR **)LocalAlloc(LPTR, sizeof(TCHAR *) * (No + 1));
			if(NamesTmp == NULL){
				break;
			}
			if(Names != NULL){
				tCopyMemory(NamesTmp, Names, sizeof(TCHAR *) * AllocNo);
				LocalFree(Names);
			}
			Names = NamesTmp;
			AllocNo = No + 1;
		}
		for(; *p != TEXT('\0') && *p != TEXT('*') && *p != TEXT('='); p++);
		if(No == 0){
			EncFlag = (*p == TEXT('*')) ? TRUE : FALSE;
		}
		for(; *p == TEXT('*') || *p == TEXT('=') || *p == TEXT('\"'); p++);

		//value�̎擾
		for(r = p; *p != TEXT('\0') && *p != TEXT('\"') && *p != TEXT(';'); p++);
		tmp = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (p - r + 1));
		if(tmp == NULL){
			break;
		}
		TStrCpyN(tmp, r, p - r + 1);

		*(Names + No) = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(tmp) + 1));
		if(*(Names + No) == NULL){
			LocalFree(tmp);
			break;
		}
		lstrcpy(*(Names + No), tmp);
		LocalFree(tmp);
	}

	if(Names == NULL){
		return NULL;
	}

	//�t�@�C�����̒������v�Z
	for(No = 0, len = 0; No < AllocNo; No++){
		if(*(Names + No) != NULL){
			len += lstrlen(*(Names + No));
		}
	}
	ret = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(ret != NULL){
		//�t�@�C������A��
		for(No = 0, r = ret; No < AllocNo; No++){
			if(*(Names + No) != NULL){
				r = TStrCpy(r, *(Names + No));
			}
		}
		//�f�R�[�h
		if(EncFlag == TRUE){
			tmp = ExtendedDecode(ret);
			if(tmp != NULL){
				LocalFree(ret);
				ret = tmp;
			}
		}
	}
	//�t�@�C�����̃��X�g�̉��
	for(No = 0; No < AllocNo; No++){
		if(*(Names + No) != NULL){
			LocalFree(*(Names + No));
		}
	}
	LocalFree(Names);
	return ret;
}


/******************************************************************************

	GetFilename

	�t�@�C�����̎擾

******************************************************************************/

TCHAR *GetFilename(TCHAR *buf, TCHAR *Attribute)
{
	TCHAR *p = buf;
	TCHAR *fname, *dname;
#ifdef UNICODE
	char *cfname;
	char *cdname;
#endif

	if(buf == NULL){
		return NULL;
	}

	fname = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(buf) + 1));
	if(fname == NULL){
		return NULL;
	}
	if(GetContentValue(buf, Attribute, fname) == FALSE){
		LocalFree(fname);
		return NULL;
	}

#ifdef UNICODE
	//TCHAR ���� char �ɕϊ�
	cfname = AllocTcharToChar(fname);
	LocalFree(fname);
	if(cfname == NULL){
		return NULL;
	}
	cdname = (char *)LocalAlloc(LMEM_FIXED, tstrlen(cfname) + 1);
	if(cdname == NULL){
		return NULL;
	}
	MIMEdecode(cfname, cdname);
	LocalFree(cfname);
	//char ���� TCHAR �ɕϊ�
	dname = AllocCharToTchar(cdname);
	LocalFree(cdname);
#else
	dname = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(buf) + 1));
	if(dname == NULL){
		LocalFree(fname);
		return NULL;
	}
	MIMEdecode(fname, dname);
	LocalFree(fname);
#endif
	return dname;
}


/******************************************************************************

	MultiPart_Parse

	Part����͂��� (RFC 2046)

******************************************************************************/

int MultiPart_Parse(TCHAR *ContentType, TCHAR *buf, struct TPMULTIPART ***tpMultiPart, int cnt)
{
	struct TPMULTIPART *tpMultiPartItem;
	TCHAR *Boundary;
	TCHAR *p;
	TCHAR *Content, *sPos;

	if(ContentType == NULL || buf == NULL){
		return 0;
	}

	//boundary�̎擾
	Boundary = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(ContentType) + 1));
	if(Boundary == NULL){
		return 0;
	}
	if(GetContentValue(ContentType, TEXT("boundary"), Boundary) == FALSE){
		LocalFree(Boundary);
		return 0;
	}

	//part�̈ʒu�̎擾
	p = NextPartPos(buf, Boundary);

	while(1){
		if(*p == TEXT('\0')){
			break;
		}
		p += (2 + lstrlen(Boundary));
		//�p�[�c�̏I��肩�`�F�b�N
		if(*p == TEXT('-') && *(p + 1) == TEXT('-')){
			break;
		}

		GetContent(p, TEXT(HEAD_CONTENTTYPE), &Content);
		if(Content != NULL &&
			TStrCmpNI(Content, TEXT("multipart"), lstrlen(TEXT("multipart"))) == 0){
			//�K�w�ɂȂ��Ă���ꍇ�͍ċA����
			sPos = GetBodyPointaT(p);
			cnt = MultiPart_Parse(Content, sPos, tpMultiPart, cnt);
			LocalFree(Content);
			p = NextPartPos(sPos, Boundary);
			continue;
		}

		//�}���`�p�[�g���̒ǉ�
		if((tpMultiPartItem = AddMultiPartInfo(tpMultiPart, cnt)) == NULL){
			break;
		}
		cnt++;

		//�w�b�_���擾
		tpMultiPartItem->ContentType = Content;
		GetContent(p, TEXT(HEAD_ENCODING), &tpMultiPartItem->Encoding);
		GetContent(p, TEXT(HEAD_DISPOSITION), &Content);

		//�t�@�C�����̎擾
		if((tpMultiPartItem->Filename = GetFilenameEx(Content)) == NULL &&
			(tpMultiPartItem->Filename = GetFilename(Content, TEXT("filename"))) == NULL &&
			(tpMultiPartItem->Filename = GetFilename(tpMultiPartItem->ContentType, TEXT("name"))) == NULL){
		}
		NULLCHECK_FREE(Content);

		//�{���̈ʒu�̎擾
		tpMultiPartItem->sPos = GetBodyPointaT(p);
		if(tpMultiPartItem->sPos == NULL){
			break;
		}
		//����part�̈ʒu���擾
		p = NextPartPos(tpMultiPartItem->sPos, Boundary);
		if(*p != TEXT('\0')){
			tpMultiPartItem->ePos = p;
		}
	}
	LocalFree(Boundary);
	return cnt;
}


/******************************************************************************

	CreateMultipart

	�}���`�p�[�g���쐬���� (RFC 2046, RFC 2183)

******************************************************************************/

int CreateMultipart(TCHAR *Filename, TCHAR *ContentType, TCHAR *Encoding, TCHAR **RetContentType, TCHAR *body, TCHAR **RetBody)
{
#define BREAK_LEN			76
#define	CTYPE_MULTIPART		TEXT("multipart/mixed;\r\n boundary=\"")
#define CONTENT_DIPPOS		TEXT("Content-Disposition: attachment;")
#define ENCODING_BASE64		TEXT("Content-Transfer-Encoding: base64")
	SYSTEMTIME st;
	TCHAR *Boundary, *ctype;
	TCHAR *fpath, *fname;
	TCHAR *buf, *ret, *tmp;
	TCHAR *p, *f, *ef;
	TCHAR date[15];
	char *cBuf;
	char *b64str;
	char *cp, *cr;
	unsigned char digest[16];
	long FileSize;
	int i, len;

	if(Filename == NULL || *Filename == TEXT('\0')){
		return MP_NO_ATTACH;
	}

	//�o�E���_���̐���
	GetLocalTime(&st);
	wsprintf(date, TEXT("%04d%02d%02d%02d%02d%02d"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

#ifdef UNICODE
	cp = AllocTcharToChar(Filename);
	cr = AllocTcharToChar(date);
	HMAC_MD5(cp, tstrlen(cp), cr, tstrlen(cr), digest);
	NULLCHECK_FREE(cp);
	NULLCHECK_FREE(cr);
#else
	HMAC_MD5(Filename, lstrlen(Filename), date, lstrlen(date), digest);
#endif

	Boundary = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * ((16 * 2) + 17 + 1));
	if(Boundary == NULL){
		return MP_ERROR_ALLOC;
	}
	p = TStrCpy(Boundary, TEXT("-----_MULTIPART_"));
	for(i = 0; i < 16; i++){
		wsprintf(p, TEXT("%02X"), digest[i]);
		p += 2;
	}
	TStrCpy(p, TEXT("_"));

	//�}���`�p�[�g�̍쐬
	len = 2 + lstrlen(Boundary) + 2 +
		lstrlen(TEXT(HEAD_CONTENTTYPE)) + 1 + lstrlen(ContentType) + 2 +
		lstrlen(TEXT(HEAD_ENCODING)) + 1 + lstrlen(Encoding) + 4 +
		lstrlen(body) + 4;
	ret = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(ret == NULL){
		LocalFree(Boundary);
		return MP_ERROR_ALLOC;
	}
	TStrJoin(ret, TEXT("--"), Boundary, TEXT("\r\n"),
		TEXT(HEAD_CONTENTTYPE), TEXT(" "), ContentType, TEXT("\r\n"),
		TEXT(HEAD_ENCODING), TEXT(" "), Encoding, TEXT("\r\n\r\n"),
		body, TEXT("\r\n\r\n"), (TCHAR *)-1);

	fpath = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (lstrlen(Filename) + 1));
	if(fpath == NULL){
		LocalFree(Boundary);
		LocalFree(ret);
		return MP_ERROR_ALLOC;
	}

	f = Filename;
	while(*f != TEXT('\0')){
		f = TStrCpyF(fpath, f, TEXT(','));
		fname = GetFileNameString(fpath);

		//�t�@�C����ǂݍ���
		FileSize = GetFileSerchSize(fpath);
		if(FileSize < 0 || (cBuf = ReadFileBuf(fpath, FileSize)) == NULL){
			LocalFree(Boundary);
			LocalFree(ret);
			LocalFree(fpath);
			return MP_ERROR_FILE;
		}

		//�G���R�[�h
		b64str = (char *)LocalAlloc(LMEM_FIXED, FileSize * 2 + 4);
		if(b64str == NULL){
			LocalFree(Boundary);
			LocalFree(ret);
			LocalFree(fpath);
			LocalFree(cBuf);
			return MP_ERROR_ALLOC;
		}
		Base64Encode(cBuf, b64str, FileSize);
		LocalFree(cBuf);

		//�܂�Ԃ�
		cBuf = (char *)LocalAlloc(LMEM_FIXED, tstrlen(b64str) + (tstrlen(b64str) / BREAK_LEN * 2) + 1);
		if(cBuf == NULL){
			LocalFree(Boundary);
			LocalFree(ret);
			LocalFree(fpath);
			LocalFree(b64str);
			return MP_ERROR_ALLOC;
		}
		for(cp = b64str, cr = cBuf, i = 0; *cp != '\0'; cp++, i++){
			if(i >= BREAK_LEN){
				i = 0;
				*(cr++) = '\r';
				*(cr++) = '\n';
			}
			*(cr++) = *cp;
		}
		*cr = '\0';
		LocalFree(b64str);

#ifdef UNICODE
		buf = AllocCharToTchar(cBuf);
		LocalFree(cBuf);
		if(buf == NULL){
			LocalFree(Boundary);
			LocalFree(ret);
			LocalFree(fpath);
			return MP_ERROR_ALLOC;
		}
#else
		buf = cBuf;
#endif

		//MIME type�̎擾
		ctype = GetMIME2Extension(NULL, fname);
		if(ctype == NULL){
			LocalFree(Boundary);
			LocalFree(ret);
			LocalFree(fpath);
			LocalFree(buf);
			return MP_ERROR_ALLOC;
		}

		//�t�@�C�����̃G���R�[�h
		p = NULL;
		if(EncodeType == 1){
			p = MIMEencode(fname, FALSE);
			if(p == NULL){
				LocalFree(Boundary);
				LocalFree(ret);
				LocalFree(fpath);
				LocalFree(buf);
				LocalFree(ctype);
				NULLCHECK_FREE(p);
				return MP_ERROR_ALLOC;
			}
			fname = p;
		}

		ef = ExtendedEncode(fname);
		if(ef == NULL){
			LocalFree(Boundary);
			LocalFree(ret);
			LocalFree(fpath);
			LocalFree(buf);
			LocalFree(ctype);
			NULLCHECK_FREE(p);
			return MP_ERROR_ALLOC;
		}
		NULLCHECK_FREE(p);

		//Part�̒ǉ�
		len += (2 + lstrlen(Boundary) + 2 +
			lstrlen(TEXT(HEAD_CONTENTTYPE)) + 1 + lstrlen(ctype) + 2 +
			lstrlen(CONTENT_DIPPOS) + lstrlen(ef) + 2 +
			lstrlen(ENCODING_BASE64) + 4 +
			lstrlen(buf) + 4);
		tmp = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
		if(tmp == NULL){
			LocalFree(Boundary);
			LocalFree(ret);
			LocalFree(fpath);
			LocalFree(buf);
			LocalFree(ctype);
			LocalFree(ef);
			return MP_ERROR_ALLOC;
		}
		TStrJoin(tmp, ret, TEXT("--"), Boundary, TEXT("\r\n"),
			TEXT(HEAD_CONTENTTYPE), TEXT(" "), ctype, TEXT("\r\n"),
			CONTENT_DIPPOS, ef, TEXT("\r\n"),
			ENCODING_BASE64, TEXT("\r\n\r\n"),
			buf, TEXT("\r\n\r\n"), (TCHAR *)-1);

		LocalFree(ef);
		LocalFree(ctype);
		LocalFree(buf);
		LocalFree(ret);
		ret = tmp;
	}
	LocalFree(fpath);

	//�}���`�p�[�g�̏I���
	len += (2 + lstrlen(Boundary) + 2 + 2);
	*RetBody = (TCHAR *)LocalAlloc(LMEM_FIXED, sizeof(TCHAR) * (len + 1));
	if(*RetBody == NULL){
		LocalFree(Boundary);
		LocalFree(ret);
		return MP_ERROR_ALLOC;
	}
	TStrJoin(*RetBody, ret, TEXT("--"), Boundary, TEXT("--\r\n"), (TCHAR *)-1);
	LocalFree(ret);

	//Content type�̐���
	*RetContentType = (TCHAR *)LocalAlloc(LMEM_FIXED,
		sizeof(TCHAR) * (lstrlen(CTYPE_MULTIPART) + lstrlen(Boundary) + 2));
	if(*RetContentType == NULL){
		LocalFree(Boundary);
		LocalFree(*RetBody);
		return MP_ERROR_ALLOC;
	}
	TStrJoin(*RetContentType, CTYPE_MULTIPART, Boundary, TEXT("\""), (TCHAR *)-1);
	LocalFree(Boundary);
	return MP_ATTACH;
}
/* End of source */
