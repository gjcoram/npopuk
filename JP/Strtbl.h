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
#define STR_DEFAULT_FONT			TEXT("�l�r �S�V�b�N")
#define STR_DEFAULT_FONTCHARSET		SHIFTJIS_CHARSET

#define STR_DEFAULT_BURA			TEXT("�A�B�C�D�H�I�J�K�c�d�E�f�h�F�G�j�l�n�p�r�t�v�x�z��X�T�U����������������@�B�D�F�H�b�������[")
#define STR_DEFAULT_OIDA			TEXT("�e�g�����i�m�o�u�k�s�w�y�q��")

#define STR_DEFAULT_HEAD_CHARSET	TEXT("ISO-2022-JP")
#define STR_DEFAULT_HEAD_ENCODE		2		// 0-7bit 1-8bit 2-base64 3-quoted-printable
#define STR_DEFAULT_BODY_CHARSET	TEXT("ISO-2022-JP")
#define STR_DEFAULT_BODY_ENCODE		0		// 0-7bit 1-8bit 2-base64 3-quoted-printable

#define STR_DEFAULT_DATEFORMAT		TEXT("yyyy/MM/dd")
#define STR_DEFAULT_TIMEFORMAT		TEXT("HH:mm")

#define STR_FILE_FILTER				TEXT("���ׂĂ�̧�� (*.*)\0*.*\0\0")
#define STR_TEXT_FILTER				TEXT("÷�� ̧�� (*.txt)\0*.txt\0���ׂĂ�̧�� (*.*)\0*.*\0\0")
#define STR_WAVE_FILTER				TEXT("����� ̧�� (*.wav)\0*.wav\0���ׂĂ�̧�� (*.*)\0*.*\0\0")

// Error
#define STR_ERR_MEMALLOC			TEXT("��؊m�ۂɎ��s���܂���")
#define STR_ERR_INIT				TEXT("�������Ɏ��s���܂���")
#define STR_ERR_OPEN				TEXT("̧�ٵ���݂Ɏ��s���܂���")
#define STR_ERR_SAVEEND				TEXT("�ۑ��Ɏ��s���܂���\n�I�������𑱍s���܂����H")
#define STR_ERR_SAVE				TEXT("�ۑ��Ɏ��s���܂���")
#define STR_ERR_ADD					TEXT("�ǉ��Ɏ��s���܂���")
#define STR_ERR_VIEW				TEXT("�\���Ɏ��s���܂���")
#define STR_ERR_SELECTMAILBOX		TEXT("����Ă��w�肳��Ă��܂���")
#define STR_ERR_SELECTMAILADDR		TEXT("Ұٱ��ڽ���I������Ă��܂���")
#define STR_ERR_SETMAILADDR			TEXT("Ұٱ��ڽ���ݒ肳��Ă��܂���")
#define STR_ERR_INPUTMAILADDR		TEXT("Ұٱ��ڽ�����͂���Ă��܂���")
#define STR_ERR_CREATECOPY			TEXT("��߰�쐬�Ɏ��s���܂���")
#define STR_ERR_SAVECOPY			TEXT("�ۑ����ւ̺�߰�Ɏ��s���܂���")
#define STR_ERR_NOITEM1				TEXT("����1���ݒ肳��Ă��܂���")
#define STR_ERR_INPUTFINDSTRING		TEXT("���������񂪓��͂���Ă��܂���")
#define STR_ERR_NOMAIL				TEXT("�ꗗ����Ұق�������܂���")
#define STR_ERR_SENDLOCK			TEXT("���̑���M���s���Ă��鎞�͑��M�ł��܂���")

// Socket error
#define STR_ERR_SOCK_SELECT			TEXT("select�Ɏ��s���܂���")
#define STR_ERR_SOCK_DISCONNECT		TEXT("���ނ���ؒf���܂���")
#define STR_ERR_SOCK_CONNECT		TEXT("���ސڑ��Ɏ��s���܂���")
#define STR_ERR_SOCK_SENDRECV		TEXT("����M���ɴװ���������܂���")
#define STR_ERR_SOCK_EVENT			TEXT("����Đݒ�Ɏ��s���܂���")
#define STR_ERR_SOCK_NOSERVER		TEXT("���ޖ����ݒ肳��Ă��܂���")
#define STR_ERR_SOCK_GETIPADDR		TEXT("IP���ڽ�擾�Ɏ��s���܂���\n���ޖ����m�F���Ă�������")
#define STR_ERR_SOCK_CREATESOCKET	TEXT("���č쐬�Ɏ��s���܂���")
#define STR_ERR_SOCK_TIMEOUT		TEXT("��ѱ�Ă��܂���")
#define STR_ERR_SOCK_SEND			TEXT("���M�Ɏ��s���܂���")
#define STR_ERR_SOCK_RESPONSE		TEXT("ڽ��ݽ����͂ł��܂���ł���\n\n")
#define STR_ERR_SOCK_GETITEMINFO	TEXT("���я����擾�ł��܂���ł���")
#define STR_ERR_SOCK_MAILSYNC		TEXT("Ұٔԍ��̓��������܂���ł���\n\n")\
									TEXT("�V���������s��Ұٔԍ��𓯊������Ă�������")
#define STR_ERR_SOCK_NOMESSAGEID	TEXT("Message-Id���擾�o���܂���ł���")
#define STR_ERR_SOCK_NOUSERID		TEXT("հ�ޖ����ݒ肳��Ă��܂���")
#define STR_ERR_SOCK_NOPASSWORD		TEXT("�߽ܰ�ނ��ݒ肳��Ă��܂���")
#define STR_ERR_SOCK_BADPASSWORD	TEXT("հ�ޖ����߽ܰ�ނ��Ԉ���Ă��܂�\n\n")
#define STR_ERR_SOCK_ACCOUNT		TEXT("����Ă��󂯕t�����܂���ł���\n\n")
#define STR_ERR_SOCK_NOAPOP			TEXT("APOP�ɑΉ����Ă��Ȃ����ނł�")
#define STR_ERR_SOCK_STAT			TEXT("STAT �Ɏ��s���܂���\n\n")
#define STR_ERR_SOCK_TOP			TEXT("TOP �Ɏ��s���܂���\n\n")
#define STR_ERR_SOCK_RETR			TEXT("RETR �Ɏ��s���܂���\n\n")
#define STR_ERR_SOCK_DELE			TEXT("�폜�Ɏ��s���܂���\n\n")
#define STR_ERR_SOCK_NOATTACH		TEXT("�Y�ţ�ق�������܂���ł���")
#define STR_ERR_SOCK_BADFROM		TEXT("���M��Ұٱ��ڽ���������ݒ肳��Ă��܂���")
#define STR_ERR_SOCK_HELO			TEXT("HELO �Ɏ��s���܂���\n")\
									TEXT("���M��Ұٱ��ڽ���������ݒ肳��Ă��邩�m�F���Ă�������\n\n")
#define STR_ERR_SOCK_SMTPAUTH		TEXT("SMTP���ނ�۸޲݂Ɏ��s���܂���\n\n")
#define STR_ERR_SOCK_RSET			TEXT("RSET �Ɏ��s���܂���\n\n")
#define STR_ERR_SOCK_MAILFROM		TEXT("MAIL FROM �Ɏ��s���܂���\n")\
									TEXT("���M��Ұٱ��ڽ���������ݒ肳��Ă��邩�m�F���Ă�������\n\n")
#define STR_ERR_SOCK_NOTO			TEXT("���M�悪�ݒ肳��Ă��܂���")
#define STR_ERR_SOCK_RCPTTO			TEXT("RCPT TO �Ɏ��s���܂���\n")\
									TEXT("���M��Ұٱ��ڽ���������ݒ肳��Ă��邩�m�F���Ă�������\n\n")
#define STR_ERR_SOCK_DATA			TEXT("DATA �Ɏ��s���܂���\n\n")
#define STR_ERR_SOCK_MAILSEND		TEXT("Ұّ��M�Ɏ��s���܂���\n\n")
#define STR_ERR_SOCK_SSL_INIT		TEXT("SSL�̏������Ɏ��s���܂���\n%s")
#define STR_ERR_SOCK_SSL_VERIFY		TEXT("�T�[�o�ؖ����̌��؂Ɏ��s���܂���\n%s")
#define STR_ERR_SOCK_NOSSL			TEXT("'npopssl.dll' �̏������Ɏ��s���܂���")

// Ras error
#define STR_ERR_RAS_NOSET			TEXT("�޲�ٱ��߂̐ݒ肪����Ă��܂���")
#define STR_ERR_RAS_CONNECT			TEXT("�޲�ٱ��ߐڑ��Ɏ��s���܂���")
#define STR_ERR_RAS_DISCONNECT		TEXT("�޲�ٱ��ߐؒf�������A�ڑ��Ɏ��s���܂���")

// Question
#define STR_Q_DELETE				TEXT("�폜���Ă���낵���ł����H")
#define STR_Q_DELSERVERMAIL			TEXT("���ނ���폜�����Ұق�����܂������s���܂����H")
#define STR_Q_DELLISTMAIL			TEXT("%d ����Ұق��ꗗ����폜���Ă���낵���ł����H%s")
#define STR_Q_DELLISTMAIL_NOSERVER	TEXT("\n(���ނ���͍폜����܂���)")
#define STR_Q_DELMAILBOX			TEXT("���ݕ\������Ă��鱶��Ă��폜���Ă�낵���ł����H")
#define STR_Q_DELATTACH				TEXT("�Y�ţ�ق��폜���Ă���낵���ł����H")
#define STR_Q_COPY					TEXT("%d ����Ұق�ۑ����ֺ�߰���Ă���낵���ł����H")
#define STR_Q_DEPENDENCE			TEXT("�@��ˑ����������݂��܂�����낵���ł����H")
#define STR_Q_UNLINKATTACH			TEXT("�Y�ţ�قւ��ݸ���������Ă���낵���ł����H")
#define STR_Q_ADDADDRESS			TEXT("%d ����Ұٱ��ڽ����ڽ���ɒǉ����Ă���낵���ł����H")
#define STR_Q_NEXTFIND				TEXT("�Ō�܂Ō������܂���\n���߂��猟�����Ȃ����܂����H")
#define STR_Q_EDITCANSEL			TEXT("�ҏW��ݾق��Ă���낵���ł����H")
#define STR_Q_SENDMAIL				TEXT("���M���Ă���낵���ł����H")
#define STR_Q_ATTACH				TEXT("�Y�ţ�قɳ�ٽ�Ȃǂ��܂܂�Ă���ꍇ�A\n")\
									TEXT("���߭���ɔ�Q���y�ڂ��\��������܂��B\n\n")\
									TEXT("���s���Ă�낵���ł����H")

// Message
#define STR_MSG_NOMARK				TEXT("ϰ����ꂽҰق�����܂���")
#define STR_MSG_NOBODY				TEXT("�{�����擾����Ă��Ȃ����ߊJ�����Ƃ��ł��܂���\n\n")\
									TEXT("'��M�p��ϰ�'��t����'ϰ������s'���Ė{�����擾���Ă�������")
#define STR_MSG_NONEWMAIL			TEXT("�V��Ұق͂���܂���")
#define STR_MSG_NOFIND				TEXT("\"%s\" ��������܂���")

// Window title
#define STR_TITLE_NOREADMAILBOX		TEXT("%s - [���Ǳ����: %d]")
#define STR_TITLE_MAILEDIT			TEXT("ҰٕҏW")
#define STR_TITLE_MAILVIEW			TEXT("Ұٕ\��")
#define STR_TITLE_MAILVIEW_COUNT	TEXT(" - [%d ����]")

// Message title
#define STR_TITLE_EXEC				TEXT("���s")
#define STR_TITLE_ALLEXEC			TEXT("������s")
#define STR_TITLE_SEND				TEXT("�����ɑ��M")
#define STR_TITLE_OPEN				TEXT("�J��")
#define STR_TITLE_SAVE				TEXT("�ۑ�")
#define STR_TITLE_COPY				TEXT("��߰")
#define STR_TITLE_DELETE			TEXT("�폜")
#define STR_TITLE_ERROR				TEXT("�װ")
#define STR_TITLE_SETMAILBOX		TEXT("����Đݒ�")
#define STR_TITLE_OPTION			TEXT("��߼��")
#ifndef _WIN32_WCE
#define STR_TITLE_STARTPASSWORD		TEXT("�N��")
#define STR_TITLE_SHOWPASSWORD		TEXT("�\��")
#endif
#define STR_TITLE_FIND				TEXT("����")
#define STR_TITLE_ALLFIND			TEXT("\"%s\" �̌���")
#define STR_TITLE_ATTACH_MSG		TEXT("�J��")

// Window status
#define STR_STATUS_VIEWINFO			TEXT("�\�� %d ��")
#define STR_STATUS_MAILBOXINFO		TEXT("�\�� %d/ ���� %d")
#define STR_STATUS_MAILINFO			TEXT("�V�� %d, ���J�� %d")

// Socket status
#define STR_STATUS_GETHOSTBYNAME	TEXT("gethostbyname...")
#define STR_STATUS_CONNECT			TEXT("connect...")
#define STR_STATUS_RECV				TEXT("��M��...")
#define STR_STATUS_SENDBODY			TEXT("�{�����M��...")
#define STR_STATUS_SOCKINFO			TEXT("%d �޲�%s")
#define STR_STATUS_SOCKINFO_RECV	TEXT("��M")
#define STR_STATUS_SOCKINFO_SEND	TEXT("���M")
#define STR_STATUS_SSL				TEXT("SSL connect...")

// Ras status
#define STR_STATUS_RAS_START		TEXT("�޲�ٱ��ߊJ�n")
#define STR_STATUS_RAS_PORTOPEN		TEXT("�߰Ă��J���Ă��܂�...")
#define STR_STATUS_RAS_DEVICE		TEXT("���޲��ɐڑ���...")
#define STR_STATUS_RAS_AUTH			TEXT("հ�ޔF�ؒ�...")
#define STR_STATUS_RAS_CONNECT		TEXT("�޲�ٱ��ߐڑ����܂���")
#define STR_STATUS_RAS_DISCONNECT	TEXT("�޲�ٱ��ߐؒf���܂���")

// Initialize status
#define STR_STATUS_INIT_MAILCNT		TEXT("%d ��")
#define STR_STATUS_INIT_MAILSIZE_B	TEXT("%s �޲�")
#define STR_STATUS_INIT_MAILSIZE_KB	TEXT("%s KB")

// Mail list
#define STR_SAVEBOX_NAME			TEXT("[�ۑ���]")
#define STR_SENDBOX_NAME			TEXT("[���M��]")
#define STR_MAILBOX_NONAME			TEXT("���̖��ݒ�")
#define STR_LIST_LVHEAD_SUBJECT		TEXT("����")
#define STR_LIST_LVHEAD_FROM		TEXT("���o�l")
#define STR_LIST_LVHEAD_TO			TEXT("���l")
#define STR_LIST_LVHEAD_DATE		TEXT("���t")
#define STR_LIST_LVHEAD_SIZE		TEXT("����")
#define STR_LIST_NOSUBJECT			TEXT("(no subject)")
#define STR_LIST_THREADSTR			TEXT("  + ")

#define STR_LIST_MENU_SENDINFO		TEXT("���M���(&R)...")
#define STR_LIST_MENU_REPLY			TEXT("�ԐM(&R)...")
#define STR_LIST_MENU_SENDMARK		TEXT("���M�p��ϰ�(&M)\tCtrl+D")
#define STR_LIST_MENU_CREATECOPY	TEXT("��߰�̍쐬(&C)\tCtrl+C")
#define STR_LIST_MENU_RECVMARK		TEXT("��M�p��ϰ�(&M)\tCtrl+D")
#define STR_LIST_MENU_SAVEBOXCOPY	TEXT("�ۑ����ֺ�߰(&C)\tCtrl+C")

// Mail view
#define STR_VIEW_HEAD_FROM			TEXT("���o�l: ")
#define STR_VIEW_HEAD_SUBJECT		TEXT("\r\n����: ")
#define STR_VIEW_HEAD_DATE			TEXT("\r\n���t: ")

#define STR_VIEW_MENU_ATTACH		TEXT("�Y�t�\��(&M)")
#define STR_VIEW_MENU_SOURCE		TEXT("����\��(&S)")
#define STR_VIEW_MENU_DELATTACH		TEXT("�Y�t�폜(&T)")

// Mail edit
#define STR_EDIT_HEAD_MAILBOX		TEXT("�����: ")
#define STR_EDIT_HEAD_TO			TEXT("\r\n����: ")
#define STR_EDIT_HEAD_SUBJECT		TEXT("\r\n����: ")

// SSL
#define STR_SSL_AUTO				TEXT("����")
#define STR_SSL_TLS10				TEXT("TLS 1.0")
#define STR_SSL_SSL30				TEXT("SSL 3.0")
#define STR_SSL_SSL20				TEXT("SSL 2.0")
#define STR_SSL_STARTTLS			TEXT("STARTTLS")

// Filter
#define STR_FILTER_USE				TEXT("�g�p")
#define STR_FILTER_NOUSE			TEXT("���g�p")
#define STR_FILTER_STATUS			TEXT("���")
#define STR_FILTER_ACTION			TEXT("����")
#define STR_FILTER_ITEM1			TEXT("����1")
#define STR_FILTER_CONTENT1			TEXT("���e1")
#define STR_FILTER_ITEM2			TEXT("����2")
#define STR_FILTER_CONTENT2			TEXT("���e2")

#define STR_FILTER_UNRECV			TEXT("��M���Ȃ�")
#define STR_FILTER_RECV				TEXT("��M����")
#define STR_FILTER_DOWNLOADMARK		TEXT("��M�p��ϰ�")
#define STR_FILTER_DELETEMARK		TEXT("�폜�p��ϰ�")
#define STR_FILTER_READICON			TEXT("�J���ς݂ɂ���")
#define STR_FILTER_SAVE				TEXT("�ۑ����ֺ�߰")

// Cc list
#define STR_CCLIST_TYPE				TEXT("���")
#define STR_CCLIST_MAILADDRESS		TEXT("Ұٱ��ڽ")

// Set send
#define STR_SETSEND_BTN_CC			TEXT("Cc, Bcc")
#define STR_SETSEND_BTN_ATTACH		TEXT("�Y�ţ��")
#define STR_SETSEND_BTN_ETC			TEXT("���̑�")

// Mail Prop
#define STR_MAILPROP_HEADER			TEXT("ͯ��")
#define STR_MAILPROP_MAILADDRESS	TEXT("Ұٱ��ڽ")

// Address list
#define STR_ADDRESSLIST_MAILADDRESS	TEXT("Ұٱ��ڽ")
#define STR_ADDRESSLIST_COMMENT		TEXT("����")

// WindowsCE
#ifdef _WIN32_WCE
#define STR_CMDBAR_RECV				TEXT("�V������")
#define STR_CMDBAR_ALLCHECK			TEXT("��������")
#define STR_CMDBAR_EXEC				TEXT("ϰ������s")
#define STR_CMDBAR_ALLEXEC			TEXT("������s")
#define STR_CMDBAR_STOP				TEXT("���~")
#define STR_CMDBAR_NEWMAIL			TEXT("ү���ނ̍쐬")
#define STR_CMDBAR_RAS_CONNECT		TEXT("�޲�ٱ��ߐڑ�")
#define STR_CMDBAR_RAS_DISCONNECT	TEXT("�޲�ٱ��ߐؒf")

#define STR_CMDBAR_PREVMAIL			TEXT("�O��Ұ�")
#define STR_CMDBAR_NEXTMAIL			TEXT("����Ұ�")
#define STR_CMDBAR_NEXTNOREAD		TEXT("���̖��J��Ұ�")
#define STR_CMDBAR_REMESSEGE		TEXT("�ԐM")
#define STR_CMDBAR_ALLREMESSEGE		TEXT("�S���ɕԐM")
#define STR_CMDBAR_DOWNMARK			TEXT("��M�p��ϰ�")
#define STR_CMDBAR_DELMARK			TEXT("�폜�p��ϰ�")

#define STR_CMDBAR_SEND				TEXT("�����ɑ��M")
#define STR_CMDBAR_SENDBOX			TEXT("���M���ɕۑ�")
#define STR_CMDBAR_SENDINFO			TEXT("���M���")

#define STR_LIST_PPCMENU_SENDINFO	TEXT("���M���...")
#define STR_LIST_PPCMENU_REPLY		TEXT("�ԐM...")
#define STR_LIST_PPCMENU_SENDMARK	TEXT("���M�p��ϰ�")
#define STR_LIST_PPCMENU_CREATECOPY	TEXT("��߰�̍쐬")
#define STR_LIST_PPCMENU_RECVMARK	TEXT("��M�p��ϰ�")
#define STR_LIST_PPCMENU_SAVEBOXCOPY	TEXT("�ۑ����ֺ�߰")

#define STR_VIEW_PPCMENU_ATTACH		TEXT("�Y�t�\��")
#define STR_VIEW_PPCMENU_SOURCE		TEXT("����\��")
#define STR_VIEW_PPCMENU_DELATTACH	TEXT("�Y�t�폜")

#ifdef _WIN32_WCE_PPC
#define STR_TITLE_SMTPAUTH			TEXT("SMTP�F�� - �ݒ�")
#define STR_TITLE_SETSSL			TEXT("SSL�ݒ�")
#define STR_TITLE_FILTER			TEXT("̨���ݒ�")
#define STR_TITLE_INITMAILBOX		TEXT("������")
#define STR_TITLE_CCBCC				TEXT("Cc, Bcc")
#define STR_TITLE_ATTACH			TEXT("�Y�ţ��")
#define STR_TITLE_ETCHEADER			TEXT("���̑���ͯ��")
#define STR_TITLE_SENDINFO			TEXT("���M���")
#define STR_TITLE_ADDRESSINFO		TEXT("���ڽ���")
#define STR_TITLE_EDITADDRESS		TEXT("���ڽ")
#define STR_TITLE_ADDRESSLIST		TEXT("���ڽ��")
#define STR_TITLE_FIND				TEXT("����")

#define STR_SF_TITLE				TEXT("̧�ّI��")
#define STR_SF_LV_NAME				TEXT("���O")
#define STR_SF_LV_TYPE				TEXT("���")
#define STR_SF_LV_SIZE				TEXT("����")
#define STR_SF_LV_DATE				TEXT("�X�V����")
#define STR_SF_Q_OVERWRITE			TEXT("�㏑�����܂����H")
#endif	//_WIN32_WCE_PPC

#ifdef _WIN32_WCE_LAGENDA
#define STR_MENU_FILE				TEXT("̧��")
#define STR_MENU_MAIL				TEXT("Ұ�")
#define STR_MENU_EDIT				TEXT("�ҏW")
#endif	//_WIN32_WCE_LAGENDA
#endif	//_WIN32_WCE

#endif	//_INC_STR_TBL_H
/* End of source */
