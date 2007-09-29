/*
 * nPOP
 *
 * ListView.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "String.h"

/* Define */
#define ICONCOUNT			9

/* Global Variables */
// �O���Q��
extern OPTION op;

extern HINSTANCE hInst;
extern int SelBox;

/* Local Function Prototypes */
static int ImageListIconAdd(HIMAGELIST IconList, int Index);
static void ListView_GetDispItem(LV_ITEM *hLVItem);
static int GetIconSortStatus(MAILITEM *tpMailItem);

/*
 * ImageListIconAdd - of the item information which In image list adding idea contest
 */
static int ImageListIconAdd(HIMAGELIST IconList, int Index)
{
	HICON hIcon = NULL;
	int ret;

	hIcon = (HICON)LoadImage(hInst, MAKEINTRESOURCE(Index), IMAGE_ICON,
		SICONSIZE, SICONSIZE, 0);

	//In image list idea contest additional
	ret = ImageList_AddIcon(IconList, hIcon);

	DestroyIcon(hIcon);
	return ret;
}

/*
 * ListView_SetColumn - In list view adding column
 */
void ListView_AddColumn(HWND hListView, int fmt, int cx, TCHAR *buf, int iSubItem)
{
	LV_COLUMN lvc;

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	lvc.fmt = fmt;
	lvc.cx = cx;
	lvc.pszText = buf;
	lvc.iSubItem = iSubItem;
	ListView_InsertColumn(hListView, iSubItem, &lvc);
}

/*
 * CreateListView - ���X�g�r���[�̍쐬�Ə�����
 */
HWND CreateListView(HWND hWnd, int Top, int bottom)
{
	HIMAGELIST IconList;
	HIMAGELIST TmpIconList;
	HICON TmpIcon;
	RECT rcClient;
	HWND hListView;

	GetClientRect(hWnd, &rcClient);

#ifdef _WIN32_WCE
#define WS_EX_STYLE		0
#else
#define WS_EX_STYLE		WS_EX_NOPARENTNOTIFY | WS_EX_CLIENTEDGE
#endif
	hListView = CreateWindowEx(WS_EX_STYLE, WC_LISTVIEW, TEXT(""),
#ifdef _WIN32_WCE_LAGENDA
		WS_BORDER | WS_VISIBLE | WS_CHILD | WS_TABSTOP | LVS_EDITLABELS | op.LvStyle,
#else	// _WIN32_WCE_LAGENDA
#ifdef _WIN32_WCE
		WS_BORDER | WS_VISIBLE | WS_CHILD | WS_TABSTOP | op.LvStyle,
#else	// _WIN32_WCE
		WS_VISIBLE | WS_CHILD | WS_TABSTOP | op.LvStyle,
#endif	// _WIN32_WCE
#endif	// _WIN32_WCE_LAGENDA
		0, Top,
		rcClient.right, rcClient.bottom - Top - bottom, hWnd,
		(HMENU)IDC_LISTVIEW, hInst, NULL);
	if (hListView == NULL) {
		return NULL;
	}

	//Extended style
#ifdef _WIN32_WCE_PPC
	ListView_SetExtendedListViewStyle(hListView, op.LvStyleEx | LVS_EX_ONECLICKACTIVATE);
#else
	ListView_SetExtendedListViewStyle(hListView, op.LvStyleEx);
#endif

	// �w�b�_�̐ݒ�
	ListView_AddColumn(hListView, LVCFMT_LEFT, *(op.LvColSize + 0), STR_LIST_LVHEAD_SUBJECT, 0);
	ListView_AddColumn(hListView, LVCFMT_LEFT, *(op.LvColSize + 1), STR_LIST_LVHEAD_FROM, 1);
	ListView_AddColumn(hListView, LVCFMT_LEFT, *(op.LvColSize + 2), STR_LIST_LVHEAD_DATE, 2);
	ListView_AddColumn(hListView, LVCFMT_RIGHT, *(op.LvColSize + 3), STR_LIST_LVHEAD_SIZE, 3);

	//of header Setting
	IconList = ImageList_Create(SICONSIZE, SICONSIZE, ILC_COLOR | ILC_MASK, ICONCOUNT, ICONCOUNT);
	ImageListIconAdd(IconList, IDI_ICON_NON);
	ImageListIconAdd(IconList, IDI_ICON_MAIN);
	ImageListIconAdd(IconList, IDI_ICON_READ);
	ImageListIconAdd(IconList, IDI_ICON_DOWN);
	ImageListIconAdd(IconList, IDI_ICON_DEL);
	ImageListIconAdd(IconList, IDI_ICON_SENDMAIL);
	ImageListIconAdd(IconList, IDI_ICON_SEND);

	//of image list Mail of transmission error compilation (synthesis of transmission being completed mail and deletion mark)
	TmpIconList = ImageList_Merge(IconList, ICON_SENDMAIL, IconList, ICON_DEL, 0, 0);
	TmpIcon = ImageList_GetIcon(TmpIconList, 0, ILD_NORMAL);
	ImageList_AddIcon(IconList, TmpIcon);
	DestroyIcon(TmpIcon);
	ImageList_Destroy((void *)TmpIconList);

	//Overlay
	ImageListIconAdd(IconList, IDI_ICON_NEW);
	ImageList_SetOverlayImage(IconList, ICON_NEW, 1);

	ListView_SetImageList(hListView, IconList, LVSIL_SMALL);

	///////////// MRP /////////////////////
	IconList = ImageList_Create(SICONSIZE, SICONSIZE, ILC_COLOR | ILC_MASK, 5, 5);  // MRP was 1,1
	///////////// --- /////////////////////
	//Status idea contest
	ImageListIconAdd(IconList, IDI_ICON_CLIP);
	///////////// MRP /////////////////////
	ImageListIconAdd(IconList, IDI_ICON_CLIP_HIGH);
	ImageListIconAdd(IconList, IDI_ICON_CLIP_LOW);
	ImageListIconAdd(IconList, IDI_ICON_HIGH);
	ImageListIconAdd(IconList, IDI_ICON_LOW);
	///////////// --- /////////////////////
	ListView_SetImageList(hListView, IconList, LVSIL_STATE);

	return hListView;
}

/*
 * ListView_SetRedraw - ���X�g�r���[�̕`��؂�ւ�
 */
void ListView_SetRedraw(HWND hListView, BOOL DrawFlag)
{
	switch (DrawFlag) {
	case FALSE:
		//for attachment file It does not draw the
		SendMessage(hListView, WM_SETREDRAW, (WPARAM)FALSE, 0);
		break;

	case TRUE:
		//It draws the
		SendMessage(hListView, WM_SETREDRAW, (WPARAM)TRUE, 0);
		UpdateWindow(hListView);
		break;
	}
}

/*
 * ListView_InsertItemEx - of list view The item is added to list view
 */
int ListView_InsertItemEx(HWND hListView, TCHAR *buf, int len, int Img, long lp, int iItem)
{
	LV_ITEM lvi;

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	lvi.pszText = buf;
	lvi.cchTextMax = len;
	lvi.iImage = Img;
	lvi.lParam = lp;

	//The item is added to list view the
	return ListView_InsertItem(hListView, &lvi);
}

/*
 * ListView_MoveItem - Movement item of list view
 */
void ListView_MoveItem(HWND hListView, int SelectItem, int Move, int ColCnt)
{
	TCHAR buf[10][BUF_SIZE];
	int i = 0;

	for (i = 0; i < ColCnt; i++) {
		*(*(buf + i)) = TEXT('\0');
		ListView_GetItemText(hListView, SelectItem, i, *(buf + i), BUF_SIZE - 1);
	}
	ListView_DeleteItem(hListView, SelectItem);

	SelectItem = SelectItem + Move;

	ListView_InsertItemEx(hListView, *buf, BUF_SIZE, 0, 0, SelectItem);
	for (i = 1; i < ColCnt; i++) {
		ListView_SetItemText(hListView, SelectItem, i, *(buf + i));
	}
	ListView_SetItemState(hListView, SelectItem,
		LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	ListView_EnsureVisible(hListView, SelectItem, TRUE);
}

/*
 * ListView_GetSelStringList - Compilation title list of selective item
 */
TCHAR *ListView_GetSelStringList(HWND hListView)
{
	TCHAR buf[BUF_SIZE];
	TCHAR *StrAddr, *p;
	int SelectItem;
	int len;

	SelectItem = -1;
	len = 0;
	while ((SelectItem = ListView_GetNextItem(hListView, SelectItem, LVNI_SELECTED)) != -1) {
		*buf = TEXT('\0');
		ListView_GetItemText(hListView, SelectItem, 0, buf, BUF_SIZE - 1);
		len += lstrlen(buf) + 2;
	}

	p = StrAddr = (TCHAR *)mem_calloc(sizeof(TCHAR) * (len + 1));
	if (StrAddr == NULL) {
		return NULL;
	}

	SelectItem = -1;
	while ((SelectItem = ListView_GetNextItem(hListView, SelectItem, LVNI_SELECTED)) != -1) {
		if (p != StrAddr) {
			*(p++) = TEXT(',');
			*(p++) = TEXT(' ');
		}
		ListView_GetItemText(hListView, SelectItem, 0, p, BUF_SIZE - 1);
		p += lstrlen(p);
	}
	*p = TEXT('\0');
	return StrAddr;
}

/*
 * ListView_GetlParam - Acquisition LPARAM of item
 */
long ListView_GetlParam(HWND hListView, int i)
{
	LV_ITEM lvi;

	lvi.mask = LVIF_PARAM;
	lvi.iItem = i;
	lvi.iSubItem = 0;
	lvi.lParam = 0;
	ListView_GetItem(hListView, &lvi);
	return lvi.lParam;
}

/*
 * ListView_GetMemToItem - From mail item acquisition index of list view
 */
int ListView_GetMemToItem(HWND hListView, MAILITEM *tpMemMailItem)
{
	MAILITEM *tpMailItem;
	int i, ItemCnt;

	ItemCnt = ListView_GetItemCount(hListView);
	for (i = 0; i < ItemCnt; i++) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem == tpMemMailItem) {
			return i;
		}
	}
	return -1;
}

/*
 * ListView_GetNextDeleteItem - Acquisition index of item of deletion mark
 */
int ListView_GetNextDeleteItem(HWND hListView, int Index)
{
	MAILITEM *tpMailItem;
	int i, ItemCnt;

	ItemCnt = ListView_GetItemCount(hListView);
	for (i = Index + 1; i < ItemCnt; i++) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->Status == ICON_DEL) {
			return i;
		}
	}
	return -1;
}

/*
 * ListView_GetNextMailItem - From the designated index acquisition the index of the mail item which has the text of rear
 */
int ListView_GetNextMailItem(HWND hListView, int Index)
{
	MAILITEM *tpMailItem;
	int i, ItemCnt;

	ItemCnt = ListView_GetItemCount(hListView);
	for (i = Index + 1; i < ItemCnt; i++) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem == NULL || tpMailItem->Body == NULL) {
			continue;
		}
		return i;
	}
	return -1;
}

/*
 * ListView_GetPrevMailItem - �w��C���f�b�N�X���O�̖{���������[���A�C�e���̃C���f�b�N�X���擾
 */
int ListView_GetPrevMailItem(HWND hListView, int Index)
{
	MAILITEM *tpMailItem;
	int i;

	if (Index == -1) {
		Index = ListView_GetItemCount(hListView);
	}
	for (i = Index - 1; i >= 0; i--) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem == NULL || tpMailItem->Body == NULL) {
			continue;
		}
		return i;
	}
	return -1;
}

/*
 * ListView_GetNextNoReadItem - ���J���̃��[���A�C�e���̃C���f�b�N�X���擾
 */
int ListView_GetNextNoReadItem(HWND hListView, int Index, int endindex)
{
	MAILITEM *tpMailItem;
	int i, ItemCnt;

	ItemCnt = ListView_GetItemCount(hListView);
	for (i = ((Index <= -1) ? 0 : Index); i < endindex; i++) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem == NULL || tpMailItem->Body == NULL) {
			continue;
		}
		if (tpMailItem->MailStatus == ICON_MAIL) {
			return i;
		}
	}
	return -1;
}

/*
 * ListView_GetNewItem - �V���̃C���f�b�N�X���擾
 */
int ListView_GetNewItem(HWND hListView, MAILBOX *tpMailBox)
{
	MAILITEM *tpMailItem;
	int i;

	for (i = 0; i < tpMailBox->MailItemCnt; i++) {
		tpMailItem = *(tpMailBox->tpMailItem + i);
		if (tpMailItem == NULL) {
			continue;
		}
		if (tpMailItem->New == TRUE) {
			return ListView_GetMemToItem(hListView, tpMailItem);
		}
	}
	return -1;
}

/*
 * ListView_ShowItem - ���X�g�r���[�ɃA�C�e����\������
 */
BOOL ListView_ShowItem(HWND hListView, MAILBOX *tpMailBox)
{
	MAILITEM *tpMailItem;
	LV_ITEM lvi;
	int i, j;
	int index = -1;

	ListView_SetRedraw(hListView, FALSE);
	//All items deletion
	ListView_DeleteAllItems(hListView);

	ListView_SetItemCount(hListView, tpMailBox->MailItemCnt);

	//Additional
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvi.iSubItem = 0;
	lvi.stateMask = LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK | LVIS_CUT;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.cchTextMax = 0;
	lvi.iImage = I_IMAGECALLBACK;

	for (i = 0, j = -1; i < tpMailBox->MailItemCnt; i++) {
		if ((tpMailItem = (*(tpMailBox->tpMailItem + i))) == NULL) {
			continue;
		}
		lvi.state = (tpMailItem->New == TRUE) ? INDEXTOOVERLAYMASK(1) : 0;
		switch (tpMailItem->Priority)
		{
			case 5:  // LOW
			case 4:
				if(tpMailItem->Multipart == TRUE)
				{
					lvi.state |= INDEXTOSTATEIMAGEMASK(3);
				}
				else
				{
					lvi.state |= INDEXTOSTATEIMAGEMASK(5);
				}
				break;

			case 1:  // HIGH
			case 2:
				if(tpMailItem->Multipart == TRUE)
				{
					lvi.state |= INDEXTOSTATEIMAGEMASK(2);
				}
				else
				{
					lvi.state |= INDEXTOSTATEIMAGEMASK(4);
				}
				break;

			case 3:  // NORMAL
			default:
				if(tpMailItem->Multipart == TRUE)
				{
					lvi.state |= INDEXTOSTATEIMAGEMASK(1);
				}
				else
				{
					lvi.state |= 0;
				}
				break;

		}
		///////////// --- /////////////////////

		if (tpMailItem->Download == FALSE && tpMailItem->Status != ICON_DOWN &&
			tpMailItem->Status != ICON_DEL && MAILBOX_SEND != SelBox) {
			lvi.state |= LVIS_CUT;
		}
		lvi.iItem = j + 1;
		lvi.lParam = (LPARAM)tpMailItem;

		//of item The item is added to list view the
		j = ListView_InsertItem(hListView, &lvi);
		if (j == -1) {
			ListView_SetRedraw(hListView, TRUE);
			return FALSE;
		}
	}

	if (op.LvThreadView == 1) {
		// �X���b�h�\��
		item_create_thread(tpMailBox);
		ListView_SortItems(hListView, CompareFunc, SORT_THREAD + 1);
	} else if ((SelBox == MAILBOX_SAVE && op.LvAutoSort == 1) || op.LvAutoSort == 2) {
		//Automatic sort
		ListView_SortItems(hListView, CompareFunc, op.LvSortItem);
	}
	ListView_SetRedraw(hListView, FALSE);

	//The item is put in selective state the
	if ((j = ListView_GetItemCount(hListView)) > 0) {
		ListView_EnsureVisible(hListView, j - 1, TRUE);
		//Acquisition
		i = ListView_GetNextNoReadItem(hListView, -1, ListView_GetItemCount(hListView));
		if (SelBox < MAILBOX_USER || i == -1) {
			//of not yet opening position The last mail is selected the
			index = (op.LvDefSelectPos == 0) ? 0 : j - 1;
			ListView_SetItemState(hListView, index,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
		} else {
			//The not yet opening mail is selected the
			ListView_SetItemState(hListView, i,
				LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
			index = (i <= 0) ? 0 : (i - 1);
			ListView_EnsureVisible(hListView, index, TRUE);
		}
	}
	ListView_SetRedraw(hListView, TRUE);
	if (index != -1) {
		ListView_EnsureVisible(hListView, index, TRUE);
	}
	return TRUE;
}

/*
 * ListView_GetDispItem - ���X�g�r���[�ɕ\������A�C�e�����̐ݒ�
 */
static void ListView_GetDispItem(LV_ITEM *hLVItem)
{
	MAILITEM *tpMailItem = (MAILITEM *)hLVItem->lParam;
	TCHAR *p = NULL, *r;
	int i, j;

	if (tpMailItem == NULL) {
		return;
	}

	//Idea contest
	if (hLVItem->mask & LVIF_IMAGE) {
		hLVItem->iImage = tpMailItem->Status;
	}

	//Text
	if (hLVItem->mask & LVIF_TEXT) {
		switch (hLVItem->iSubItem) {
		// ����
		case 0:
			p = (tpMailItem->Subject != NULL && *tpMailItem->Subject != TEXT('\0')) ?
				tpMailItem->Subject : STR_LIST_NOSUBJECT;

			if (op.LvThreadView == 1 && tpMailItem->Indent != 0) {
				r = hLVItem->pszText;
				//The number of blanks which are added from the indent is calculated the
				j = ((tpMailItem->Indent * 4) > (hLVItem->cchTextMax - 1)) ?
					((hLVItem->cchTextMax - 1) / 4) : tpMailItem->Indent;
				j = (j - 1) * 4;
				//Additional
				for (i = 1; i < j; i++) {
					*(r++) = TEXT(' ');
				}
				//of blank Additional
				r = str_cpy_t(r, STR_LIST_THREADSTR);
				// �����̒ǉ�
				str_cpy_n_t(r, p, BUF_SIZE - (r - hLVItem->pszText) - 1);
				return;
			}
			break;

		//of subject When the sender (transmission box it is, addressee)
		case 1:
			p = (MAILBOX_SEND == SelBox) ? tpMailItem->To : tpMailItem->From;
			break;

		//Date
		case 2:
			p = tpMailItem->FmtDate;
			break;

		//Size
		case 3:
			p = tpMailItem->Size;
			break;

		default:
			return;
		}
		str_cpy_n_t(hLVItem->pszText, (p != NULL) ? p : TEXT("\0"), BUF_SIZE - 1);
	}
}

/*
 * GetIconSortStatus - �\�[�g�p�̃X�e�[�^�X�̎擾
 */
static int GetIconSortStatus(MAILITEM *tpMailItem)
{
	switch (tpMailItem->Status) {
	case ICON_MAIL:		return (tpMailItem->Download == TRUE) ? 1 : 2;
	case ICON_READ:		return (tpMailItem->Download == TRUE) ? 3 : 4;
	case ICON_SENDMAIL:	return 5;
	case ICON_ERROR:	return 6;
	case ICON_DOWN:		return 7;
	case ICON_DEL:		return 8;
	case ICON_SEND:		return 9;
	case ICON_NON:		return 10;
	}
	return 0;
}

/*
 * CompareFunc - �\�[�g�t���O�ɏ]���ĕ�����̔�r���s��
 */
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TCHAR *wbuf1, *wbuf2;
	TCHAR tbuf1[BUF_SIZE], tbuf2[BUF_SIZE];
#ifdef UNICODE
	char tmp1[BUF_SIZE], tmp2[BUF_SIZE];
#endif
	int ret, sfg, ghed;
	int len1 = 0, len2 = 0;
	BOOL NumFlag = FALSE;

	sfg = (lParamSort < 0) ? 1 : 0;	//Ascending order / descending order
	ghed = ABS(lParamSort) - 1;		//Sorts the header

	if (lParam1 == 0 || lParam2 == 0) {
		return 0;
	}

	wbuf1 = TEXT("\0");
	wbuf2 = TEXT("\0");

	switch (ghed) {
	// ����
	case 0:
		if (((MAILITEM *)lParam1)->Subject != NULL)
			wbuf1 = ((MAILITEM *)lParam1)->Subject;
		if (((MAILITEM *)lParam2)->Subject != NULL)
			wbuf2 = ((MAILITEM *)lParam2)->Subject;
		break;

	//When the sender (transmission box it is, addressee)
	case 1:
		if (MAILBOX_SEND == SelBox) {
			if (((MAILITEM *)lParam1)->To != NULL)
				wbuf1 = ((MAILITEM *)lParam1)->To;
			if (((MAILITEM *)lParam2)->To != NULL)
				wbuf2 = ((MAILITEM *)lParam2)->To;
		} else {
			if (((MAILITEM *)lParam1)->From != NULL)
				wbuf1 = ((MAILITEM *)lParam1)->From;
			if (((MAILITEM *)lParam2)->From != NULL)
				wbuf2 = ((MAILITEM *)lParam2)->From;
		}
		break;

	//Date
	case 2:
		if (((MAILITEM *)lParam1)->Date != NULL) {
			wbuf1 = ((MAILITEM *)lParam1)->Date;
#ifdef UNICODE
			tchar_to_char(wbuf1, tmp1, BUF_SIZE);
			DateConv(tmp1, tmp2, TRUE);
			char_to_tchar(tmp2, tbuf1, BUF_SIZE);
#else
			DateConv(wbuf1, tbuf1, TRUE);
#endif
		} else if (MAILBOX_SEND == SelBox) {
			// unsent mail sorts last
			wsprintf(tbuf1, TEXT("99999999 9999"));
		}
		if (((MAILITEM *)lParam2)->Date != NULL) {
			wbuf2 = ((MAILITEM *)lParam2)->Date;
#ifdef UNICODE
			tchar_to_char(wbuf2, tmp1, BUF_SIZE);
			DateConv(tmp1, tmp2, TRUE);
			char_to_tchar(tmp2, tbuf2, BUF_SIZE);
#else
			DateConv(wbuf2, tbuf2, TRUE);
#endif
		} else if (MAILBOX_SEND == SelBox) {
			// unsent mail sorts last
			wsprintf(tbuf2, TEXT("99999999 9999"));
		}
		wbuf1 = tbuf1;
		wbuf2 = tbuf2;
		break;

	//Size
	case 3:
		if (((MAILITEM *)lParam1)->Size != NULL)
			len1 = _ttoi(((MAILITEM *)lParam1)->Size);
		if (((MAILITEM *)lParam2)->Size != NULL)
			len2 = _ttoi(((MAILITEM *)lParam2)->Size);
		NumFlag = TRUE;
		break;

	//Number
	case SORT_NO:
		len1 = ((MAILITEM *)lParam1)->No;
		len2 = ((MAILITEM *)lParam2)->No;
		NumFlag = TRUE;
		break;

	//Idea contest
	case SORT_IOCN:
		len1 = GetIconSortStatus((MAILITEM *)lParam1);
		len2 = GetIconSortStatus((MAILITEM *)lParam2);
		NumFlag = TRUE;
		break;

	//Thread
	case SORT_THREAD:
		len1 = ((MAILITEM *)lParam1)->PrevNo;
		len2 = ((MAILITEM *)lParam2)->PrevNo;
		NumFlag = TRUE;
		break;
	}

	ret = (NumFlag == FALSE) ? lstrcmpi(wbuf1, wbuf2) : (len1 - len2);
	return (((ret < 0 && sfg == 1) || (ret > 0 && sfg == 0)) ? 1 : -1);
}

/*
 * ListView_NotifyProc - ���X�g�r���[�C�x���g
 */
LRESULT ListView_NotifyProc(HWND hWnd, LPARAM lParam)
{
	LV_DISPINFO *plv = (LV_DISPINFO *)lParam;
	NMHDR *CForm = (NMHDR *)lParam;

	switch (plv->hdr.code) {
	case LVN_GETDISPINFO:		// �\���A�C�e���̗v��
		ListView_GetDispItem(&(plv->item));
		return TRUE;
	case LVN_COLUMNCLICK: // bj: reload if date column
		if (plv->item.iItem == 2)
			return TRUE;
		else
			return FALSE;

#ifdef _WIN32_WCE_LAGENDA
	case LVN_BEGINLABELEDIT:
#endif
#ifdef _WIN32_WCE_PPC
	case LVN_ITEMACTIVATE:
#endif
	case LVN_ITEMCHANGED:		// �A�C�e���̑I����Ԃ̕ύX
		return SendMessage(hWnd, WM_LV_EVENT, plv->hdr.code, lParam);

#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	case LVN_BEGINDRAG:
		//of selective state of item With drag extended
		return LVBD_DRAGSELECT;
#endif
	}

	switch (CForm->code) {
#ifndef _WIN32_WCE_PPC
	case NM_DBLCLK:				// �_�u���N���b�N
#endif
	case NM_CLICK:				// �N���b�N
	case NM_RCLICK:				// �E�N���b�N
		return SendMessage(hWnd, WM_LV_EVENT, CForm->code, lParam);
	}
	return FALSE;
}
/* End of source */
