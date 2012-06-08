/*
 * nPOP
 *
 * ListView.c
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2012 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

/* Include Files */
#include "General.h"
#include "Memory.h"
#include "String.h"

/* Define */
#define ICONCOUNT			12

/* Global Variables */
// 外部参照
extern OPTION op;
extern TCHAR *AppDir;
extern HINSTANCE hInst;
extern int SelBox;
extern MAILBOX *MailBox;

/* Local Function Prototypes */
static void ListView_GetDispItem(LV_ITEM *hLVItem);
static int GetIconSortStatus(MAILITEM *tpMailItem);

/*
 * ImageListIconAdd - of the item information which In image list adding idea contest
 */
#ifdef LOAD_USER_IMAGES
static int ImageListIconAdd(HIMAGELIST IconList, int Index, TCHAR *Filename)
#else
static int ImageListIconAdd(HIMAGELIST IconList, int Index)
#endif
{
	HICON hIcon = NULL;
	int ret;

#ifdef LOAD_USER_IMAGES
	if (op.HasResourceDir) {
		// loading user-supplied icons
		TCHAR fpath[BUF_SIZE];
		wsprintf(fpath, TEXT("%sResource\\%s"), AppDir, Filename);
		hIcon = LoadImage(NULL, fpath, IMAGE_ICON, SICONSIZE, SICONSIZE, LR_LOADFROMFILE);
		if (hIcon == NULL && op.SocLog > 1) {
			TCHAR msg[MSG_SIZE];
			DWORD err = GetLastError();
			wsprintf(msg, TEXT("Failed to load image %s (err=%X)\r\n"), fpath, err);
			log_save(msg);
			if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, msg, MSG_SIZE-1, NULL)) {
				log_save(msg);
			}
		}
	}
	if (hIcon == NULL)
#endif
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
 * CreateListView - リストビューの作成と初期化
 */
HWND CreateListView(HWND hWnd, int Top, int Bottom, int Left, int Right)
{
	HIMAGELIST IconList;
	HWND hListView;

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
		Left, Top, Right, Bottom,
		hWnd, (HMENU)IDC_LISTVIEW, hInst, NULL);
	if (hListView == NULL) {
		return NULL;
	}

	//Extended style
#ifdef _WIN32_WCE_PPC
	ListView_SetExtendedListViewStyle(hListView, op.LvStyleEx | LVS_EX_ONECLICKACTIVATE);
#else
	ListView_SetExtendedListViewStyle(hListView, op.LvStyleEx);
#endif

	// ヘッダの設定
	if (str_cmp_ni_t(op.LvColumnOrder, TEXT("FSDZ"), 4) == 0) {
		ListView_AddColumn(hListView, LVCFMT_LEFT, *(op.LvColSize + 0), STR_LIST_LVHEAD_FROM, 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, *(op.LvColSize + 1), STR_LIST_LVHEAD_SUBJECT, 1);
	} else {
		ListView_AddColumn(hListView, LVCFMT_LEFT, *(op.LvColSize + 0), STR_LIST_LVHEAD_SUBJECT, 0);
		ListView_AddColumn(hListView, LVCFMT_LEFT, *(op.LvColSize + 1), STR_LIST_LVHEAD_FROM, 1);
	}
	ListView_AddColumn(hListView, LVCFMT_LEFT, *(op.LvColSize + 2), STR_LIST_LVHEAD_DATE, 2);
	ListView_AddColumn(hListView, LVCFMT_RIGHT, *(op.LvColSize + 3), STR_LIST_LVHEAD_SIZE, 3);
	if (lstrlen(op.LvColumnOrder) > 4 && *(op.LvColumnOrder+4) == TEXT('N')) {
		// add No as last column
		ListView_AddColumn(hListView, LVCFMT_RIGHT, 30, STR_LIST_LVHEAD_NO, 4);
	}

	//of header Setting
#ifdef _WIN32_WCE
	IconList = ImageList_Create(SICONSIZE, SICONSIZE, ILC_COLOR | ILC_MASK, ICONCOUNT, ICONCOUNT);
#else
	IconList = ImageList_Create(SICONSIZE, SICONSIZE, ILC_COLOR32 | ILC_MASK, ICONCOUNT, ICONCOUNT);
#endif

#ifdef LOAD_USER_IMAGES
	ImageListIconAdd(IconList, IDI_ICON_NON, TEXT("ico_none.ico"));
	ImageListIconAdd(IconList, IDI_ICON_MAIN, TEXT("ico_main.ico"));
	ImageListIconAdd(IconList, IDI_ICON_READ, TEXT("ico_read.ico"));
	ImageListIconAdd(IconList, IDI_ICON_DOWN, TEXT("ico_down.ico"));
	ImageListIconAdd(IconList, IDI_ICON_DEL, TEXT("icon_del.ico"));
	ImageListIconAdd(IconList, IDI_ICON_SENTMAIL, TEXT("ico_sent.ico"));
	ImageListIconAdd(IconList, IDI_ICON_SEND, TEXT("ico_send.ico"));
	ImageListIconAdd(IconList, IDI_ICON_ERROR, TEXT("icon_err.ico"));
	ImageListIconAdd(IconList, IDI_ICON_FLAG, TEXT("ico_flag.ico"));

	//Overlay
	ImageListIconAdd(IconList, IDI_ICON_NEW, TEXT("icon_new.ico"));
	ImageList_SetOverlayImage(IconList, 9, ICON_NEW_MASK);

	// GJC overlays for replied, forwarded
	ImageListIconAdd(IconList, IDI_ICON_REPL, TEXT("ico_repl.ico"));
	ImageList_SetOverlayImage(IconList, 10, ICON_REPL_MASK);
	ImageListIconAdd(IconList, IDI_ICON_FWD, TEXT("icon_fwd.ico"));
	ImageList_SetOverlayImage(IconList, 11, ICON_FWD_MASK);
	// could do this with ImageList_Merge
	ImageListIconAdd(IconList, IDI_ICON_REPLFWD, TEXT("ico_refw.ico"));
	ImageList_SetOverlayImage(IconList, 12, (ICON_REPL_MASK | ICON_FWD_MASK));

	ListView_SetImageList(hListView, IconList, LVSIL_SMALL);

	// State icons
	// state = (multipart*3) + priority(0=normal,1=high,2=low)
	IconList = ImageList_Create(SICONSIZE, SICONSIZE, ILC_COLOR | ILC_MASK, 8, 8);
	ImageListIconAdd(IconList, IDI_ICON_HIGH, TEXT("ico_high.ico"));      // 1
	ImageListIconAdd(IconList, IDI_ICON_LOW, TEXT("icon_low.ico"));       // 2
	ImageListIconAdd(IconList, IDI_ICON_HTML, TEXT("ico_html.ico"));      // 3
	ImageListIconAdd(IconList, IDI_ICON_HTML_HIGH, TEXT("ico_h_hi.ico")); // 4
	ImageListIconAdd(IconList, IDI_ICON_HTML_LOW, TEXT("ico_h_lo.ico"));  // 5
	ImageListIconAdd(IconList, IDI_ICON_CLIP, TEXT("icon_cli.ico"));      // 6
	ImageListIconAdd(IconList, IDI_ICON_CLIP_HIGH, TEXT("ico_clhi.ico")); // 7
	ImageListIconAdd(IconList, IDI_ICON_CLIP_LOW, TEXT("ico_cllo.ico"));  // 8
	
	ListView_SetImageList(hListView, IconList, LVSIL_STATE);
#else
	ImageListIconAdd(IconList, IDI_ICON_NON);
	ImageListIconAdd(IconList, IDI_ICON_MAIN);
	ImageListIconAdd(IconList, IDI_ICON_READ);
	ImageListIconAdd(IconList, IDI_ICON_DOWN);
	ImageListIconAdd(IconList, IDI_ICON_DEL);
	ImageListIconAdd(IconList, IDI_ICON_SENTMAIL);
	ImageListIconAdd(IconList, IDI_ICON_SEND);
	ImageListIconAdd(IconList, IDI_ICON_ERROR);
	ImageListIconAdd(IconList, IDI_ICON_FLAG);

	//Overlay
	ImageListIconAdd(IconList, IDI_ICON_NEW);
	ImageList_SetOverlayImage(IconList, 9, ICON_NEW_MASK);

	// GJC overlays for replied, forwarded
	ImageListIconAdd(IconList, IDI_ICON_REPL);
	ImageList_SetOverlayImage(IconList, 10, ICON_REPL_MASK);
	ImageListIconAdd(IconList, IDI_ICON_FWD);
	ImageList_SetOverlayImage(IconList, 11, ICON_FWD_MASK);
	// could do this with ImageList_Merge
	ImageListIconAdd(IconList, IDI_ICON_REPLFWD);
	ImageList_SetOverlayImage(IconList, 12, (ICON_REPL_MASK | ICON_FWD_MASK));

	ListView_SetImageList(hListView, IconList, LVSIL_SMALL);

	// State icons
	// state = (multipart*3) + priority(0=normal,1=high,2=low)
	IconList = ImageList_Create(SICONSIZE, SICONSIZE, ILC_COLOR | ILC_MASK, 8, 8);
	ImageListIconAdd(IconList, IDI_ICON_HIGH);      // 1
	ImageListIconAdd(IconList, IDI_ICON_LOW);       // 2
	ImageListIconAdd(IconList, IDI_ICON_HTML);      // 3
	ImageListIconAdd(IconList, IDI_ICON_HTML_HIGH); // 4
	ImageListIconAdd(IconList, IDI_ICON_HTML_LOW);  // 5
	ImageListIconAdd(IconList, IDI_ICON_CLIP);      // 6
	ImageListIconAdd(IconList, IDI_ICON_CLIP_HIGH); // 7
	ImageListIconAdd(IconList, IDI_ICON_CLIP_LOW);  // 8
	
	ListView_SetImageList(hListView, IconList, LVSIL_STATE);
#endif

	return hListView;
}

/*
 * ListView_SetRedraw - リストビューの描画切り替え
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
	int ItemCnt, i = 0;
	long Lptr;

	if (ColCnt > 9) {
		ColCnt = 9;
#ifdef _DEBUG
		ErrorMessage(NULL, TEXT("Programming Error!"));
#endif
	}

	ItemCnt = ListView_GetItemCount(hListView) - 1;
	for (i = 0; i < ColCnt; i++) {
		*(*(buf + i)) = TEXT('\0');
		ListView_GetItemText(hListView, SelectItem, i, *(buf + i), BUF_SIZE - 1);
	}
	Lptr = ListView_GetlParam(hListView, SelectItem);
	ListView_DeleteItem(hListView, SelectItem);

	SelectItem = SelectItem + Move;
	if (SelectItem < 0) {
		SelectItem = 0;
	} else if (SelectItem >= ItemCnt) {
		SelectItem = ItemCnt;
	}

	ListView_InsertItemEx(hListView, *buf, BUF_SIZE, 0, Lptr, SelectItem);
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
		ListView_GetItemText(hListView, SelectItem, 1, buf, BUF_SIZE - 1);
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
		ListView_GetItemText(hListView, SelectItem, 1, p, BUF_SIZE - 1);
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
		if (tpMailItem->Mark == ICON_DEL) {
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
		if (tpMailItem == NULL || tpMailItem->Body == NULL
			|| (tpMailItem->Mark == ICON_DEL && op.ViewSkipDeleted == 1)) {
				continue;
		}
		return i;
	}
	return -1;
}

/*
 * ListView_GetPrevMailItem - 指定インデックスより前の本文を持つメールアイテムのインデックスを取得
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
		if (tpMailItem == NULL || tpMailItem->Body == NULL
			|| (tpMailItem->Mark == ICON_DEL && op.ViewSkipDeleted == 1)) {
			continue;
		}
		return i;
	}
	return -1;
}

/*
 * ListView_GetNextUnreadItem - 未開封のメールアイテムのインデックスを取得
 */
int ListView_GetNextUnreadItem(HWND hListView, int Index, int endindex)
{
	MAILITEM *tpMailItem;
	int i, ItemCnt;

	ItemCnt = ListView_GetItemCount(hListView);
	for (i = ((Index <= -1) ? 0 : Index); i < endindex; i++) {
		tpMailItem = (MAILITEM *)ListView_GetlParam(hListView, i);
		if (tpMailItem == NULL || tpMailItem->Body == NULL
			|| (tpMailItem->Mark == ICON_DEL && op.ViewSkipDeleted != 0)) {
				continue;
		}
		if (tpMailItem->MailStatus == ICON_MAIL) {
			return i;
		}
	}
	return -1;
}

/*
 * ListView_GetNewItem - 新着のインデックスを取得
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
 * ListView_ShowItem - リストビューにアイテムを表示する
 */
BOOL ListView_ShowItem(HWND hListView, MAILBOX *tpMailBox, BOOL AddLast)
{
	MAILITEM *tpMailItem;
	LV_ITEM lvi;
	int i, j, state, sort = SORT_NO + 1;
	int start_index, index = -1;

	ListView_SetRedraw(hListView, FALSE);

	// GJC
	if (AddLast == TRUE) {
		// just inserting the last item
		start_index = tpMailBox->MailItemCnt - 1;
	} else {
		// delete all and build from scratch
		ListView_DeleteAllItems(hListView);
		start_index = 0;
	}
	// end GJC

	ListView_SetItemCount(hListView, tpMailBox->MailItemCnt);

	//Additional
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;
	lvi.iSubItem = 0;
	lvi.stateMask = LVIS_OVERLAYMASK | LVIS_STATEIMAGEMASK | LVIS_CUT;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.cchTextMax = 0;
	lvi.iImage = I_IMAGECALLBACK;

	for (i = start_index, j = i-1; i < tpMailBox->MailItemCnt; i++) {
		if ((tpMailItem = (*(tpMailBox->tpMailItem + i))) == NULL) {
			continue;
		}
		if (tpMailItem->New == TRUE) {
			lvi.state = INDEXTOOVERLAYMASK(ICON_NEW_MASK);
		} else {
			lvi.state = INDEXTOOVERLAYMASK(tpMailItem->ReFwd & ICON_REFWD_MASK); // GJC
		}
		state = ListView_ComputeState(tpMailItem->Priority, tpMailItem->Multipart);
		lvi.state |= INDEXTOSTATEIMAGEMASK(state);

		if (tpMailItem->Download == FALSE && tpMailItem->Mark != ICON_DOWN &&
			tpMailItem->Mark != ICON_DEL && MAILBOX_SEND != SelBox) {
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
		// スレッド表示
		item_create_thread(tpMailBox);
		ListView_SortItems(hListView, CompareFunc, SORT_THREAD + 1);
		sort = SORT_THREAD + 1;
	} else if (((MailBox+SelBox)->Type == MAILBOX_TYPE_SAVE && op.LvAutoSort == 1) || op.LvAutoSort == 2) {
		//Automatic sort
		ListView_SortItems(hListView, CompareFunc, op.LvSortItem);
		sort = op.LvSortItem;
	}
#ifndef _WIN32_WCE
	if (AddLast == FALSE) {
		ListViewSortMenuCheck(sort);
	}
#endif
	ListView_SetRedraw(hListView, FALSE);

	//The item is put in selective state the
	if ((j = ListView_GetItemCount(hListView)) > 0) {
		int st = LVIS_FOCUSED;
		ListView_EnsureVisible(hListView, j - 1, TRUE);
		if (op.PreviewPaneHeight <= 0 || op.AutoPreview) {
			st |= LVIS_SELECTED;
		}
		//Acquisition
		i = ListView_GetNextUnreadItem(hListView, -1, ListView_GetItemCount(hListView));
		if (SelBox < MAILBOX_USER || i == -1) {
			//of not yet opening position The last mail is selected the
			index = (op.LvDefSelectPos == 0) ? 0 : j - 1;
			ListView_SetItemState(hListView, index, st, st);
		} else {
			//The not yet opening mail is selected the
			ListView_SetItemState(hListView, i, st, st);
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
 * ListView_GetDispItem - リストビューに表示するアイテム情報の設定
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
		hLVItem->iImage = tpMailItem->Mark;
	}

	//Text
	if (hLVItem->mask & LVIF_TEXT) {
		int col = hLVItem->iSubItem;
		if ((col == 0 || col == 1) && str_cmp_ni_t(op.LvColumnOrder, TEXT("FSDZ"), 4) == 0) {
			col = (col == 0) ? 1 : 0;
		}
		switch (col) {
		// 件名
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
				// 件名の追加
				str_cpy_n_t(r, p, BUF_SIZE - (r - hLVItem->pszText) - 1);
				return;
			}
			break;

		//of subject When the sender (transmission box it is, addressee)
		case 1:
			if (MAILBOX_SEND == SelBox) {
				if (tpMailItem->RedirectTo != NULL)
					p = tpMailItem->RedirectTo;
				else
					p = tpMailItem->To;
			} else if (tpMailItem->MailStatus == ICON_SENTMAIL || tpMailItem->MailStatus == ICON_SEND) {
				r = hLVItem->pszText;
				*(r++) = TEXT('T');
				*(r++) = TEXT('o');
				*(r++) = TEXT(':');
				*(r++) = TEXT(' ');
				p = tpMailItem->To;
				str_cpy_n_t(r, p, BUF_SIZE - (r - hLVItem->pszText) - 1);
				return;
			} else {
				p = tpMailItem->From;
			}
			break;

		//Date
		case 2:
			p = tpMailItem->FmtDate;
			break;

		//Size
		case 3:
			p = tpMailItem->Size;
			break;

		//No
		case 4:
			p = hLVItem->pszText;
			wsprintf(p, TEXT("%d"), tpMailItem->No);
			break;

		default:
			return;
		}
		if (p != hLVItem->pszText) {
			str_cpy_n_t(hLVItem->pszText, (p != NULL) ? p : TEXT("\0"), BUF_SIZE - 1);
		}
	}
}

/*
 * GetIconSortStatus - ソート用のステータスの取得
 */
static int GetIconSortStatus(MAILITEM *tpMailItem)
{
	int retval = 0;
	switch (tpMailItem->Mark) {
		case ICON_FLAG:		retval = 100; break;
		case ICON_MAIL:     retval = 200; break;
		case ICON_READ:     retval = 300; break;
		case ICON_SENTMAIL:	retval = 400; break;
		case ICON_ERROR:	retval = 500; break;
		case ICON_DOWN:		retval = 600; break;
		case ICON_DEL:		retval = 700; break;
		case ICON_SEND:		retval = 800; break;
		case ICON_NON:		retval = 900; break;
		default:            retval = 0; break;
	}
	retval += tpMailItem->Priority * 1000;
	retval += tpMailItem->Download * 10;
	retval += tpMailItem->ReFwd & ICON_REFWD_MASK;
	return retval;
}

/*
 * CompareFunc - ソートフラグに従って文字列の比較を行う
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

	if (lParam1 == 0 || lParam2 == 0) {
		return 0;
	}

	sfg = (lParamSort < 0) ? 1 : 0;	//Ascending order / descending order
	ghed = ABS(lParamSort) - 1;		//Sorts the header

	wbuf1 = TEXT("\0");
	wbuf2 = TEXT("\0");

	switch (ghed) {
	// 件名
	case SORT_SUBJ:
		if (((MAILITEM *)lParam1)->Subject != NULL)
			wbuf1 = ((MAILITEM *)lParam1)->Subject;
		if (((MAILITEM *)lParam2)->Subject != NULL)
			wbuf2 = ((MAILITEM *)lParam2)->Subject;
		if (op.ReFwdPrefixes && *op.ReFwdPrefixes != TEXT('\0')) {
			// remove Re: and Fwd: for sorting
			TCHAR *p, *q, *pfx; 
			p = op.ReFwdPrefixes, q = wbuf1;
			pfx = (TCHAR *)mem_alloc(sizeof(TCHAR) * (lstrlen(op.ReFwdPrefixes) + 1));
			if (!pfx) break;
			while (*q != TEXT('\0') && *p != TEXT('\0')) {
				p = str_cpy_f_t(pfx, p, TEXT(','));
				len1 = lstrlen(pfx);
				if (str_cmp_ni_t(q, pfx, len1) == 0) {
					q += len1;
					if (*q == TEXT(' ')) q++;
					p = op.ReFwdPrefixes; // start over (Re: Re: Re:)
				}
			}
			if (*q != TEXT('\0')) {
				// if we didn't take everything away
				wbuf1 = q;
			}
			p = op.ReFwdPrefixes, q = wbuf2;
			while (*q != TEXT('\0') && *p != TEXT('\0')) {
				p = str_cpy_f_t(pfx, p, TEXT(','));
				len1 = lstrlen(pfx);
				if (str_cmp_ni_t(q, pfx, len1) == 0) {
					q += len1;
					if (*q == TEXT(' ')) q++;
					p = op.ReFwdPrefixes; // start over (Re: Re: Re:)
				}
			}
			if (*q != TEXT('\0')) {
				// if we didn't take everything away
				wbuf2 = q;
			}
			mem_free(&pfx);
		}
		break;

	//When the sender (transmission box it is, addressee)
	case SORT_FROM:
		if (MAILBOX_SEND == SelBox) {
			if (((MAILITEM *)lParam1)->To != NULL)
				wbuf1 = ((MAILITEM *)lParam1)->To;
			if (((MAILITEM *)lParam2)->To != NULL)
				wbuf2 = ((MAILITEM *)lParam2)->To;
		} else {
			int ms = ((MAILITEM *)lParam1)->MailStatus;
			if (ms == ICON_SENTMAIL || ms == ICON_SEND) {
				if (((MAILITEM *)lParam1)->To != NULL)
					wbuf1 = ((MAILITEM *)lParam1)->To;
			} else {
				if (((MAILITEM *)lParam1)->From != NULL)
					wbuf1 = ((MAILITEM *)lParam1)->From;
			}
			ms = ((MAILITEM *)lParam2)->MailStatus;
			if (ms == ICON_SENTMAIL || ms == ICON_SEND) {
				if (((MAILITEM *)lParam2)->To != NULL)
					wbuf2 = ((MAILITEM *)lParam2)->To;
			} else {
				if (((MAILITEM *)lParam2)->From != NULL)
					wbuf2 = ((MAILITEM *)lParam2)->From;
			}
			if (op.LvSortFromAddressOnly) {
				if (((MAILITEM *)lParam1)->From_email != NULL)
						wbuf1 = ((MAILITEM *)lParam1)->From_email;
				if (((MAILITEM *)lParam2)->From_email != NULL)
					wbuf2 = ((MAILITEM *)lParam2)->From_email;
			}
		}
		while (*wbuf1 == TEXT('"') || *wbuf1 == TEXT('<')) wbuf1++;
		while (*wbuf2 == TEXT('"') || *wbuf2 == TEXT('<')) wbuf2++;
		break;

	//Date
	case SORT_DATE:
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
	case SORT_SIZE:
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
	case SORT_ICON:
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
 * AddrCompareFunc - address list sort comparison
 */
int CALLBACK AddrCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TCHAR *wbuf1, *wbuf2;
	int ret, sfg, ghed;
	int len1 = 0, len2 = 0;
	BOOL NumFlag = FALSE;

	sfg = (lParamSort < 0) ? 1 : 0;	//Ascending order / descending order
	ghed = ABS(lParamSort);		//Sorts the header

	if (lParam1 == 0 || lParam2 == 0) {
		return 0;
	}

	wbuf1 = TEXT("\0");
	wbuf2 = TEXT("\0");

	switch (ghed) {
	case 1:
		if (((ADDRESSITEM *)lParam1)->MailAddress != NULL) {
			wbuf1 = ((ADDRESSITEM *)lParam1)->MailAddress;
			if ((*wbuf1 == TEXT('<') || *wbuf1 == TEXT('"')) && *(wbuf1+1) != TEXT('\0')) {
				wbuf1++;
			}
		}
		if (((ADDRESSITEM *)lParam2)->MailAddress != NULL) {
			wbuf2 = ((ADDRESSITEM *)lParam2)->MailAddress;
			if ((*wbuf2 == TEXT('<') || *wbuf2 == TEXT('"')) && *(wbuf2+1) != TEXT('\0')) {
				wbuf2++;
			}
		}
		break;

	case 2:
		if (((ADDRESSITEM *)lParam1)->Comment != NULL)
			wbuf1 = ((ADDRESSITEM *)lParam1)->Comment;
		if (((ADDRESSITEM *)lParam2)->Comment != NULL)
			wbuf2 = ((ADDRESSITEM *)lParam2)->Comment;
		break;

	case 3:
		if (((ADDRESSITEM *)lParam1)->Group != NULL)
			wbuf1 = ((ADDRESSITEM *)lParam1)->Group;
		if (((ADDRESSITEM *)lParam2)->Group != NULL)
			wbuf2 = ((ADDRESSITEM *)lParam2)->Group;
		break;

	//Number
	case 0:
		len1 = ((ADDRESSITEM *)lParam1)->Num;
		len2 = ((ADDRESSITEM *)lParam2)->Num;
		NumFlag = TRUE;
		break;
	}

	ret = (NumFlag == FALSE) ? lstrcmpi(wbuf1, wbuf2) : (len1 - len2);
	return (((ret < 0 && sfg == 1) || (ret > 0 && sfg == 0)) ? 1 : -1);
}

/*
 * ListView_NotifyProc - リストビューイベント
 */
LRESULT ListView_NotifyProc(HWND hWnd, LPARAM lParam)
{
	LV_DISPINFO *plv = (LV_DISPINFO *)lParam;
	NMHDR *CForm = (NMHDR *)lParam;

	switch (plv->hdr.code) {
	case LVN_GETDISPINFO:		// 表示アイテムの要求
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
	case LVN_ITEMCHANGED:		// アイテムの選択状態の変更
		return SendMessage(hWnd, WM_LV_EVENT, plv->hdr.code, lParam);

#if defined(_WIN32_WCE_PPC) || defined(_WIN32_WCE_LAGENDA)
	case LVN_BEGINDRAG:
		//of selective state of item With drag extended
		return LVBD_DRAGSELECT;
#endif
	}

	switch (CForm->code) {
	case NM_DBLCLK:				// ダブルクリック
	case NM_CLICK:				// クリック
	case NM_RCLICK:				// 右クリック
		return SendMessage(hWnd, WM_LV_EVENT, CForm->code, lParam);
	}
	return FALSE;
}

int ListView_ComputeState(int Priority, int Multipart)
{
	int state;
	switch (Priority) {
		case 4:  // LOW
		case 5:
			state = 2;
			break;
		case 1:  // HIGH
		case 2:
			state = 1;
			break;
		case 3:  // NORMAL
		default:
			state = 0;
			break;
	}
	if (Multipart == MULTIPART_HTML) {
		state += 3;
	} else if (Multipart != MULTIPART_NONE) {
		state += 6;
	}
	return state;
}
/* End of source */
