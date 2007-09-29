/*
 * nPOP
 *
 * Strtbl.h (JP)
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 */

#ifndef _INC_STR_TBL_H
#define _INC_STR_TBL_H

// General
#define STR_DEFAULT_FONT			TEXT("ＭＳ ゴシック")
#define STR_DEFAULT_FONTCHARSET		SHIFTJIS_CHARSET

#define STR_DEFAULT_BURA			TEXT("、。，．？！゛゜…‥・’”：；）〕］｝〉》」』】≫々ゝゞぁぃぅぇぉっゃゅょァィゥェォッャュョー")
#define STR_DEFAULT_OIDA			TEXT("‘“￥＄（［｛「〔《『【〈≪")

#define STR_DEFAULT_HEAD_CHARSET	TEXT("ISO-2022-JP")
#define STR_DEFAULT_HEAD_ENCODE		2		// 0-7bit 1-8bit 2-base64 3-quoted-printable
#define STR_DEFAULT_BODY_CHARSET	TEXT("ISO-2022-JP")
#define STR_DEFAULT_BODY_ENCODE		0		// 0-7bit 1-8bit 2-base64 3-quoted-printable

#define STR_DEFAULT_DATEFORMAT		TEXT("yyyy/MM/dd")
#define STR_DEFAULT_TIMEFORMAT		TEXT("HH:mm")

#define STR_FILE_FILTER				TEXT("すべてのﾌｧｲﾙ (*.*)\0*.*\0\0")
#define STR_TEXT_FILTER				TEXT("ﾃｷｽﾄ ﾌｧｲﾙ (*.txt)\0*.txt\0すべてのﾌｧｲﾙ (*.*)\0*.*\0\0")
#define STR_WAVE_FILTER				TEXT("ｻｳﾝﾄﾞ ﾌｧｲﾙ (*.wav)\0*.wav\0すべてのﾌｧｲﾙ (*.*)\0*.*\0\0")

// Error
#define STR_ERR_MEMALLOC			TEXT("ﾒﾓﾘ確保に失敗しました")
#define STR_ERR_INIT				TEXT("初期化に失敗しました")
#define STR_ERR_OPEN				TEXT("ﾌｧｲﾙｵｰﾌﾟﾝに失敗しました")
#define STR_ERR_SAVEEND				TEXT("保存に失敗しました\n終了処理を続行しますか？")
#define STR_ERR_SAVE				TEXT("保存に失敗しました")
#define STR_ERR_ADD					TEXT("追加に失敗しました")
#define STR_ERR_VIEW				TEXT("表示に失敗しました")
#define STR_ERR_SELECTMAILBOX		TEXT("ｱｶｳﾝﾄが指定されていません")
#define STR_ERR_SELECTMAILADDR		TEXT("ﾒｰﾙｱﾄﾞﾚｽが選択されていません")
#define STR_ERR_SETMAILADDR			TEXT("ﾒｰﾙｱﾄﾞﾚｽが設定されていません")
#define STR_ERR_INPUTMAILADDR		TEXT("ﾒｰﾙｱﾄﾞﾚｽが入力されていません")
#define STR_ERR_CREATECOPY			TEXT("ｺﾋﾟｰ作成に失敗しました")
#define STR_ERR_SAVECOPY			TEXT("保存箱へのｺﾋﾟｰに失敗しました")
#define STR_ERR_NOITEM1				TEXT("項目1が設定されていません")
#define STR_ERR_INPUTFINDSTRING		TEXT("検索文字列が入力されていません")
#define STR_ERR_NOMAIL				TEXT("一覧からﾒｰﾙが見つかりません")
#define STR_ERR_SENDLOCK			TEXT("他の送受信を行っている時は送信できません")

// Socket error
#define STR_ERR_SOCK_SELECT			TEXT("selectに失敗しました")
#define STR_ERR_SOCK_DISCONNECT		TEXT("ｻｰﾊﾞから切断しました")
#define STR_ERR_SOCK_CONNECT		TEXT("ｻｰﾊﾞ接続に失敗しました")
#define STR_ERR_SOCK_SENDRECV		TEXT("送受信時にｴﾗｰが発生しました")
#define STR_ERR_SOCK_EVENT			TEXT("ｲﾍﾞﾝﾄ設定に失敗しました")
#define STR_ERR_SOCK_NOSERVER		TEXT("ｻｰﾊﾞ名が設定されていません")
#define STR_ERR_SOCK_GETIPADDR		TEXT("IPｱﾄﾞﾚｽ取得に失敗しました\nｻｰﾊﾞ名を確認してください")
#define STR_ERR_SOCK_CREATESOCKET	TEXT("ｿｹｯﾄ作成に失敗しました")
#define STR_ERR_SOCK_TIMEOUT		TEXT("ﾀｲﾑｱｳﾄしました")
#define STR_ERR_SOCK_SEND			TEXT("送信に失敗しました")
#define STR_ERR_SOCK_RESPONSE		TEXT("ﾚｽﾎﾟﾝｽが解析できませんでした\n\n")
#define STR_ERR_SOCK_GETITEMINFO	TEXT("ｱｲﾃﾑ情報を取得できませんでした")
#define STR_ERR_SOCK_MAILSYNC		TEXT("ﾒｰﾙ番号の同期が取れませんでした\n\n")\
									TEXT("新着ﾁｪｯｸを行いﾒｰﾙ番号を同期させてください")
#define STR_ERR_SOCK_NOMESSAGEID	TEXT("Message-Idを取得出来ませんでした")
#define STR_ERR_SOCK_NOUSERID		TEXT("ﾕｰｻﾞ名が設定されていません")
#define STR_ERR_SOCK_NOPASSWORD		TEXT("ﾊﾟｽﾜｰﾄﾞが設定されていません")
#define STR_ERR_SOCK_BADPASSWORD	TEXT("ﾕｰｻﾞ名かﾊﾟｽﾜｰﾄﾞが間違っています\n\n")
#define STR_ERR_SOCK_ACCOUNT		TEXT("ｱｶｳﾝﾄが受け付けられませんでした\n\n")
#define STR_ERR_SOCK_NOAPOP			TEXT("APOPに対応していないｻｰﾊﾞです")
#define STR_ERR_SOCK_STAT			TEXT("STAT に失敗しました\n\n")
#define STR_ERR_SOCK_TOP			TEXT("TOP に失敗しました\n\n")
#define STR_ERR_SOCK_RETR			TEXT("RETR に失敗しました\n\n")
#define STR_ERR_SOCK_DELE			TEXT("削除に失敗しました\n\n")
#define STR_ERR_SOCK_NOATTACH		TEXT("添付ﾌｧｲﾙが見つかりませんでした")
#define STR_ERR_SOCK_BADFROM		TEXT("送信元ﾒｰﾙｱﾄﾞﾚｽが正しく設定されていません")
#define STR_ERR_SOCK_HELO			TEXT("HELO に失敗しました\n")\
									TEXT("送信元ﾒｰﾙｱﾄﾞﾚｽが正しく設定されているか確認してください\n\n")
#define STR_ERR_SOCK_SMTPAUTH		TEXT("SMTPｻｰﾊﾞのﾛｸﾞｲﾝに失敗しました\n\n")
#define STR_ERR_SOCK_RSET			TEXT("RSET に失敗しました\n\n")
#define STR_ERR_SOCK_MAILFROM		TEXT("MAIL FROM に失敗しました\n")\
									TEXT("送信元ﾒｰﾙｱﾄﾞﾚｽが正しく設定されているか確認してください\n\n")
#define STR_ERR_SOCK_NOTO			TEXT("送信先が設定されていません")
#define STR_ERR_SOCK_RCPTTO			TEXT("RCPT TO に失敗しました\n")\
									TEXT("送信先ﾒｰﾙｱﾄﾞﾚｽが正しく設定されているか確認してください\n\n")
#define STR_ERR_SOCK_DATA			TEXT("DATA に失敗しました\n\n")
#define STR_ERR_SOCK_MAILSEND		TEXT("ﾒｰﾙ送信に失敗しました\n\n")
#define STR_ERR_SOCK_SSL_INIT		TEXT("SSLの初期化に失敗しました\n%s")
#define STR_ERR_SOCK_SSL_VERIFY		TEXT("サーバ証明書の検証に失敗しました\n%s")
#define STR_ERR_SOCK_NOSSL			TEXT("'npopssl.dll' の初期化に失敗しました")

// Ras error
#define STR_ERR_RAS_NOSET			TEXT("ﾀﾞｲﾔﾙｱｯﾌﾟの設定がされていません")
#define STR_ERR_RAS_CONNECT			TEXT("ﾀﾞｲﾔﾙｱｯﾌﾟ接続に失敗しました")
#define STR_ERR_RAS_DISCONNECT		TEXT("ﾀﾞｲﾔﾙｱｯﾌﾟ切断したか、接続に失敗しました")

// Question
#define STR_Q_DELETE				TEXT("削除してもよろしいですか？")
#define STR_Q_DELSERVERMAIL			TEXT("ｻｰﾊﾞから削除されるﾒｰﾙがありますが実行しますか？")
#define STR_Q_DELLISTMAIL			TEXT("%d 件のﾒｰﾙを一覧から削除してもよろしいですか？%s")
#define STR_Q_DELLISTMAIL_NOSERVER	TEXT("\n(ｻｰﾊﾞからは削除されません)")
#define STR_Q_DELMAILBOX			TEXT("現在表示されているｱｶｳﾝﾄを削除してよろしいですか？")
#define STR_Q_DELATTACH				TEXT("添付ﾌｧｲﾙを削除してもよろしいですか？")
#define STR_Q_COPY					TEXT("%d 件のﾒｰﾙを保存箱へｺﾋﾟｰしてもよろしいですか？")
#define STR_Q_DEPENDENCE			TEXT("機種依存文字が存在しますがよろしいですか？")
#define STR_Q_UNLINKATTACH			TEXT("添付ﾌｧｲﾙへのﾘﾝｸを解除してもよろしいですか？")
#define STR_Q_ADDADDRESS			TEXT("%d 件のﾒｰﾙｱﾄﾞﾚｽをｱﾄﾞﾚｽ帳に追加してもよろしいですか？")
#define STR_Q_NEXTFIND				TEXT("最後まで検索しました\n初めから検索しなおしますか？")
#define STR_Q_EDITCANSEL			TEXT("編集をｷｬﾝｾﾙしてもよろしいですか？")
#define STR_Q_SENDMAIL				TEXT("送信してもよろしいですか？")
#define STR_Q_ATTACH				TEXT("添付ﾌｧｲﾙにｳｨﾙｽなどが含まれている場合、\n")\
									TEXT("ｺﾝﾋﾟｭｰﾀに被害を及ぼす可能性があります。\n\n")\
									TEXT("実行してよろしいですか？")

// Message
#define STR_MSG_NOMARK				TEXT("ﾏｰｸされたﾒｰﾙがありません")
#define STR_MSG_NOBODY				TEXT("本文が取得されていないため開くことができません\n\n")\
									TEXT("'受信用にﾏｰｸ'を付けて'ﾏｰｸを実行'して本文を取得してください")
#define STR_MSG_NONEWMAIL			TEXT("新着ﾒｰﾙはありません")
#define STR_MSG_NOFIND				TEXT("\"%s\" が見つかりません")

// Window title
#define STR_TITLE_NOREADMAILBOX		TEXT("%s - [未読ｱｶｳﾝﾄ: %d]")
#define STR_TITLE_MAILEDIT			TEXT("ﾒｰﾙ編集")
#define STR_TITLE_MAILVIEW			TEXT("ﾒｰﾙ表示")
#define STR_TITLE_MAILVIEW_COUNT	TEXT(" - [%d 件目]")

// Message title
#define STR_TITLE_EXEC				TEXT("実行")
#define STR_TITLE_ALLEXEC			TEXT("巡回実行")
#define STR_TITLE_SEND				TEXT("直ちに送信")
#define STR_TITLE_OPEN				TEXT("開く")
#define STR_TITLE_SAVE				TEXT("保存")
#define STR_TITLE_COPY				TEXT("ｺﾋﾟｰ")
#define STR_TITLE_DELETE			TEXT("削除")
#define STR_TITLE_ERROR				TEXT("ｴﾗｰ")
#define STR_TITLE_SETMAILBOX		TEXT("ｱｶｳﾝﾄ設定")
#define STR_TITLE_OPTION			TEXT("ｵﾌﾟｼｮﾝ")
#ifndef _WIN32_WCE
#define STR_TITLE_STARTPASSWORD		TEXT("起動")
#define STR_TITLE_SHOWPASSWORD		TEXT("表示")
#endif
#define STR_TITLE_FIND				TEXT("検索")
#define STR_TITLE_ALLFIND			TEXT("\"%s\" の検索")
#define STR_TITLE_ATTACH_MSG		TEXT("開く")

// Window status
#define STR_STATUS_VIEWINFO			TEXT("表示 %d 件")
#define STR_STATUS_MAILBOXINFO		TEXT("表示 %d/ ｻｰﾊﾞ %d")
#define STR_STATUS_MAILINFO			TEXT("新着 %d, 未開封 %d")

// Socket status
#define STR_STATUS_GETHOSTBYNAME	TEXT("gethostbyname...")
#define STR_STATUS_CONNECT			TEXT("connect...")
#define STR_STATUS_RECV				TEXT("受信中...")
#define STR_STATUS_SENDBODY			TEXT("本文送信中...")
#define STR_STATUS_SOCKINFO			TEXT("%d ﾊﾞｲﾄ%s")
#define STR_STATUS_SOCKINFO_RECV	TEXT("受信")
#define STR_STATUS_SOCKINFO_SEND	TEXT("送信")
#define STR_STATUS_SSL				TEXT("SSL connect...")

// Ras status
#define STR_STATUS_RAS_START		TEXT("ﾀﾞｲﾔﾙｱｯﾌﾟ開始")
#define STR_STATUS_RAS_PORTOPEN		TEXT("ﾎﾟｰﾄを開いています...")
#define STR_STATUS_RAS_DEVICE		TEXT("ﾃﾞﾊﾞｲｽに接続中...")
#define STR_STATUS_RAS_AUTH			TEXT("ﾕｰｻﾞ認証中...")
#define STR_STATUS_RAS_CONNECT		TEXT("ﾀﾞｲﾔﾙｱｯﾌﾟ接続しました")
#define STR_STATUS_RAS_DISCONNECT	TEXT("ﾀﾞｲﾔﾙｱｯﾌﾟ切断しました")

// Initialize status
#define STR_STATUS_INIT_MAILCNT		TEXT("%d 通")
#define STR_STATUS_INIT_MAILSIZE_B	TEXT("%s ﾊﾞｲﾄ")
#define STR_STATUS_INIT_MAILSIZE_KB	TEXT("%s KB")

// Mail list
#define STR_SAVEBOX_NAME			TEXT("[保存箱]")
#define STR_SENDBOX_NAME			TEXT("[送信箱]")
#define STR_MAILBOX_NONAME			TEXT("名称未設定")
#define STR_LIST_LVHEAD_SUBJECT		TEXT("件名")
#define STR_LIST_LVHEAD_FROM		TEXT("差出人")
#define STR_LIST_LVHEAD_TO			TEXT("受取人")
#define STR_LIST_LVHEAD_DATE		TEXT("日付")
#define STR_LIST_LVHEAD_SIZE		TEXT("ｻｲｽﾞ")
#define STR_LIST_NOSUBJECT			TEXT("(no subject)")
#define STR_LIST_THREADSTR			TEXT("  + ")

#define STR_LIST_MENU_SENDINFO		TEXT("送信情報(&R)...")
#define STR_LIST_MENU_REPLY			TEXT("返信(&R)...")
#define STR_LIST_MENU_SENDMARK		TEXT("送信用にﾏｰｸ(&M)\tCtrl+D")
#define STR_LIST_MENU_CREATECOPY	TEXT("ｺﾋﾟｰの作成(&C)\tCtrl+C")
#define STR_LIST_MENU_RECVMARK		TEXT("受信用にﾏｰｸ(&M)\tCtrl+D")
#define STR_LIST_MENU_SAVEBOXCOPY	TEXT("保存箱へｺﾋﾟｰ(&C)\tCtrl+C")

// Mail view
#define STR_VIEW_HEAD_FROM			TEXT("差出人: ")
#define STR_VIEW_HEAD_SUBJECT		TEXT("\r\n件名: ")
#define STR_VIEW_HEAD_DATE			TEXT("\r\n日付: ")

#define STR_VIEW_MENU_ATTACH		TEXT("添付表示(&M)")
#define STR_VIEW_MENU_SOURCE		TEXT("ｿｰｽ表示(&S)")
#define STR_VIEW_MENU_DELATTACH		TEXT("添付削除(&T)")

// Mail edit
#define STR_EDIT_HEAD_MAILBOX		TEXT("ｱｶｳﾝﾄ: ")
#define STR_EDIT_HEAD_TO			TEXT("\r\n宛先: ")
#define STR_EDIT_HEAD_SUBJECT		TEXT("\r\n件名: ")

// SSL
#define STR_SSL_AUTO				TEXT("自動")
#define STR_SSL_TLS10				TEXT("TLS 1.0")
#define STR_SSL_SSL30				TEXT("SSL 3.0")
#define STR_SSL_SSL20				TEXT("SSL 2.0")
#define STR_SSL_STARTTLS			TEXT("STARTTLS")

// Filter
#define STR_FILTER_USE				TEXT("使用")
#define STR_FILTER_NOUSE			TEXT("未使用")
#define STR_FILTER_STATUS			TEXT("状態")
#define STR_FILTER_ACTION			TEXT("動作")
#define STR_FILTER_ITEM1			TEXT("項目1")
#define STR_FILTER_CONTENT1			TEXT("内容1")
#define STR_FILTER_ITEM2			TEXT("項目2")
#define STR_FILTER_CONTENT2			TEXT("内容2")

#define STR_FILTER_UNRECV			TEXT("受信しない")
#define STR_FILTER_RECV				TEXT("受信する")
#define STR_FILTER_DOWNLOADMARK		TEXT("受信用にﾏｰｸ")
#define STR_FILTER_DELETEMARK		TEXT("削除用にﾏｰｸ")
#define STR_FILTER_READICON			TEXT("開封済みにする")
#define STR_FILTER_SAVE				TEXT("保存箱へｺﾋﾟｰ")

// Cc list
#define STR_CCLIST_TYPE				TEXT("種別")
#define STR_CCLIST_MAILADDRESS		TEXT("ﾒｰﾙｱﾄﾞﾚｽ")

// Set send
#define STR_SETSEND_BTN_CC			TEXT("Cc, Bcc")
#define STR_SETSEND_BTN_ATTACH		TEXT("添付ﾌｧｲﾙ")
#define STR_SETSEND_BTN_ETC			TEXT("その他")

// Mail Prop
#define STR_MAILPROP_HEADER			TEXT("ﾍｯﾀﾞ")
#define STR_MAILPROP_MAILADDRESS	TEXT("ﾒｰﾙｱﾄﾞﾚｽ")

// Address list
#define STR_ADDRESSLIST_MAILADDRESS	TEXT("ﾒｰﾙｱﾄﾞﾚｽ")
#define STR_ADDRESSLIST_COMMENT		TEXT("ｺﾒﾝﾄ")

// WindowsCE
#ifdef _WIN32_WCE
#define STR_CMDBAR_RECV				TEXT("新着ﾁｪｯｸ")
#define STR_CMDBAR_ALLCHECK			TEXT("巡回ﾁｪｯｸ")
#define STR_CMDBAR_EXEC				TEXT("ﾏｰｸを実行")
#define STR_CMDBAR_ALLEXEC			TEXT("巡回実行")
#define STR_CMDBAR_STOP				TEXT("中止")
#define STR_CMDBAR_NEWMAIL			TEXT("ﾒｯｾｰｼﾞの作成")
#define STR_CMDBAR_RAS_CONNECT		TEXT("ﾀﾞｲﾔﾙｱｯﾌﾟ接続")
#define STR_CMDBAR_RAS_DISCONNECT	TEXT("ﾀﾞｲﾔﾙｱｯﾌﾟ切断")

#define STR_CMDBAR_PREVMAIL			TEXT("前のﾒｰﾙ")
#define STR_CMDBAR_NEXTMAIL			TEXT("次のﾒｰﾙ")
#define STR_CMDBAR_NEXTNOREAD		TEXT("次の未開封ﾒｰﾙ")
#define STR_CMDBAR_REMESSEGE		TEXT("返信")
#define STR_CMDBAR_ALLREMESSEGE		TEXT("全員に返信")
#define STR_CMDBAR_DOWNMARK			TEXT("受信用にﾏｰｸ")
#define STR_CMDBAR_DELMARK			TEXT("削除用にﾏｰｸ")

#define STR_CMDBAR_SEND				TEXT("直ちに送信")
#define STR_CMDBAR_SENDBOX			TEXT("送信箱に保存")
#define STR_CMDBAR_SENDINFO			TEXT("送信情報")

#define STR_LIST_PPCMENU_SENDINFO	TEXT("送信情報...")
#define STR_LIST_PPCMENU_REPLY		TEXT("返信...")
#define STR_LIST_PPCMENU_SENDMARK	TEXT("送信用にﾏｰｸ")
#define STR_LIST_PPCMENU_CREATECOPY	TEXT("ｺﾋﾟｰの作成")
#define STR_LIST_PPCMENU_RECVMARK	TEXT("受信用にﾏｰｸ")
#define STR_LIST_PPCMENU_SAVEBOXCOPY	TEXT("保存箱へｺﾋﾟｰ")

#define STR_VIEW_PPCMENU_ATTACH		TEXT("添付表示")
#define STR_VIEW_PPCMENU_SOURCE		TEXT("ｿｰｽ表示")
#define STR_VIEW_PPCMENU_DELATTACH	TEXT("添付削除")

#ifdef _WIN32_WCE_PPC
#define STR_TITLE_SMTPAUTH			TEXT("SMTP認証 - 設定")
#define STR_TITLE_SETSSL			TEXT("SSL設定")
#define STR_TITLE_FILTER			TEXT("ﾌｨﾙﾀ設定")
#define STR_TITLE_INITMAILBOX		TEXT("初期化")
#define STR_TITLE_CCBCC				TEXT("Cc, Bcc")
#define STR_TITLE_ATTACH			TEXT("添付ﾌｧｲﾙ")
#define STR_TITLE_ETCHEADER			TEXT("その他のﾍｯﾀﾞ")
#define STR_TITLE_SENDINFO			TEXT("送信情報")
#define STR_TITLE_ADDRESSINFO		TEXT("ｱﾄﾞﾚｽ情報")
#define STR_TITLE_EDITADDRESS		TEXT("ｱﾄﾞﾚｽ")
#define STR_TITLE_ADDRESSLIST		TEXT("ｱﾄﾞﾚｽ帳")
#define STR_TITLE_FIND				TEXT("検索")

#define STR_SF_TITLE				TEXT("ﾌｧｲﾙ選択")
#define STR_SF_LV_NAME				TEXT("名前")
#define STR_SF_LV_TYPE				TEXT("種類")
#define STR_SF_LV_SIZE				TEXT("ｻｲｽﾞ")
#define STR_SF_LV_DATE				TEXT("更新日時")
#define STR_SF_Q_OVERWRITE			TEXT("上書きしますか？")
#endif	//_WIN32_WCE_PPC

#ifdef _WIN32_WCE_LAGENDA
#define STR_MENU_FILE				TEXT("ﾌｧｲﾙ")
#define STR_MENU_MAIL				TEXT("ﾒｰﾙ")
#define STR_MENU_EDIT				TEXT("編集")
#endif	//_WIN32_WCE_LAGENDA
#endif	//_WIN32_WCE

#endif	//_INC_STR_TBL_H
/* End of source */
