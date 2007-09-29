/**************************************************************************

	nPOP

	Strtbl.h

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_STR_TBL_H
#define _INC_STR_TBL_H

//General
#ifdef _WIN32_WCE
#define STR_DEFAULT_FONT			TEXT("Tahoma")
#else
#define STR_DEFAULT_FONT			TEXT("")
#endif
#define STR_DEFAULT_FONTCHARSET		DEFAULT_CHARSET

#define STR_DEFAULT_BURA			TEXT("")
#define STR_DEFAULT_OIDA			TEXT("")

#define STR_DEFAULT_HEAD_CHARSET	TEXT("ISO-8859-1")
#define STR_DEFAULT_HEAD_ENCODE		3		// 0-7bit 1-8bit 2-base64 3-quoted-printable
#define STR_DEFAULT_BODY_CHARSET	TEXT("ISO-8859-1")
#define STR_DEFAULT_BODY_ENCODE		3		// 0-7bit 1-8bit 2-base64 3-quoted-printable

#define STR_DEFAULT_DATEFORMAT		TEXT("MM/dd/yyyy")
#define STR_DEFAULT_TIMEFORMAT		TEXT("HH:mm")

#define STR_FILE_FILTER				TEXT("All Files (*.*)\0*.*\0\0")
#define STR_TEXT_FILTER				TEXT("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0")
#define STR_WAVE_FILTER				TEXT("Sound Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0\0")

//Error
#define STR_ERR_MEMALLOC			TEXT("Failed in the memory allocating")
#define STR_ERR_INIT				TEXT("Failed in initialization")
#define STR_ERR_OPEN				TEXT("Failed in the file opening")
#define STR_ERR_SAVEEND				TEXT("failed in preservation\nIs the termination continued?")
#define STR_ERR_SAVE				TEXT("Failed in preservation")
#define STR_ERR_ADD					TEXT("Failed in the addition")
#define STR_ERR_VIEW				TEXT("Failed in the display")
#define STR_ERR_SELECTMAILBOX		TEXT("The account is not specified")
#define STR_ERR_SELECTMAILADDR		TEXT("The mail address has not been selected")
#define STR_ERR_SETMAILADDR			TEXT("The mail address is not set")
#define STR_ERR_INPUTMAILADDR		TEXT("The mail address is not input")
#define STR_ERR_CREATECOPY			TEXT("Failed in the copy making")
#define STR_ERR_SAVECOPY			TEXT("Failed in the copy to the Savebox")
#define STR_ERR_NOITEM1				TEXT("Item1 is not set")
#define STR_ERR_INPUTFINDSTRING		TEXT("The retrieval character string is not input")
#define STR_ERR_NOMAIL				TEXT("Mail is not found from the list")
#define STR_ERR_SENDLOCK			TEXT("When the and others is transmitted and received, it is not possible to transmit")

//Socket error
#define STR_ERR_SOCK_SELECT			TEXT("Failed in select")
#define STR_ERR_SOCK_DISCONNECT		TEXT("Disconnected from the server")
#define STR_ERR_SOCK_CONNECT		TEXT("Failed in the server connection")
#define STR_ERR_SOCK_SENDRECV		TEXT("The error occurred when transmitting and receiving")
#define STR_ERR_SOCK_EVENT			TEXT("Failed in the event setting")
#define STR_ERR_SOCK_NOSERVER		TEXT("The server name is not set")
#define STR_ERR_SOCK_GETIPADDR		TEXT("Failed in the IP address acquisition")
#define STR_ERR_SOCK_CREATESOCKET	TEXT("Failed in the socket making")
#define STR_ERR_SOCK_TIMEOUT		TEXT("Connection is timeout")
#define STR_ERR_SOCK_SEND			TEXT("Failed in the transmission")
#define STR_ERR_SOCK_RESPONSE		TEXT("The response was not able to be analyzed\n\n")
#define STR_ERR_SOCK_GETITEMINFO	TEXT("Item information was not able to be acquired")
#define STR_ERR_SOCK_MAILSYNC 		TEXT("The synchronization of the mail number was not able to be taken\n\n")\
									TEXT("Must newly arrived check and the mail number must be synchronized")
#define STR_ERR_SOCK_NOMESSAGEID	TEXT("Message-Id was not able to be acquired")
#define STR_ERR_SOCK_NOUSERID		TEXT("Username is not set")
#define STR_ERR_SOCK_NOPASSWORD		TEXT("The password is not set")
#define STR_ERR_SOCK_BADPASSWORD	TEXT("The username or password is wrong\n\n")
#define STR_ERR_SOCK_ACCOUNT		TEXT("The account was not accepted\n\n")
#define STR_ERR_SOCK_NOAPOP			TEXT("It is a server which does not correspond to APOP")
#define STR_ERR_SOCK_STAT			TEXT("STAT was not accepted\n\n")
#define STR_ERR_SOCK_RETR			TEXT("RETR was not accepted\n\n")
#define STR_ERR_SOCK_DELE			TEXT("failed in the deletion\n\n")
#define STR_ERR_SOCK_NOATTACH		TEXT("The attached file was not found")
#define STR_ERR_SOCK_BADFROM		TEXT("The transmission former mail address is not correctly set")
#define STR_ERR_SOCK_HELO 			TEXT("Failed in HELO\n")\
									TEXT("Please confirm whether the transmission former mail address is correctly set\n\n")
#define STR_ERR_SOCK_SMTPAUTH		TEXT("Failed in the login of the SMTP server\n\n")
#define STR_ERR_SOCK_RSET			TEXT("Failed in RSET\n\n")
#define STR_ERR_SOCK_MAILFROM		TEXT("Failed in MAIL FROM\n")\
									TEXT("Please confirm whether the transmission former mail address is correctly set\n\n")
#define STR_ERR_SOCK_NOTO			TEXT("The destination is not set")
#define STR_ERR_SOCK_RCPTTO			TEXT("Failed in RCPT TO\n")\
									TEXT("Please confirm whether the destination mail address is correctly set\n\n")
#define STR_ERR_SOCK_DATA			TEXT("Failed in DATA\n\n")
#define STR_ERR_SOCK_MAILSEND		TEXT("Failed in Mail Sending\n\n")

//Ras error
#define STR_ERR_RAS_NOSET			TEXT("A dial-up setting is not done")
#define STR_ERR_RAS_CONNECT			TEXT("Failed in a dial-up connection")
#define STR_ERR_RAS_DISCONNECT		TEXT("Dial-up was cut or failed in the connection")

//Question
#define STR_Q_DELETE				TEXT("Delete it?")
#define STR_Q_DELSERVERMAIL			TEXT("Do you update it though are the mail deleted from the server?")
#define STR_Q_DELLISTMAIL			TEXT("Delete %d mail from the list?%s")
#define STR_Q_DELLISTMAIL_NOSERVER	TEXT("\n(Is not deleted from the server)")
#define STR_Q_DELMAILBOX			TEXT("Delete account?")
#define STR_Q_DELATTACH				TEXT("Delete attached files?")
#define STR_Q_OVERWRITE				TEXT("\"%s\" \n\nalready has received something\nDo you overwrite?")
#define STR_Q_COPY					TEXT("Copy %d mail to the Savebox?")
#define STR_Q_DEPENDENCE			TEXT("Is it all right although there is a character depending on the model?")
#define STR_Q_UNLINKATTACH			TEXT("Release the link to the attached file?")
#define STR_Q_ADDADDRESS			TEXT("Add %d mail addresses to the address book?")
#define STR_Q_NEXTFIND				TEXT("retrieved it to the last minute\nDo you retrieve it again at the time of beginning?")
#define STR_Q_EDITCANSEL			TEXT("Cancel the edit?")
#define STR_Q_SENDMAIL				TEXT("Send it?")

//Message
#define STR_MSG_NOMARK				TEXT("There is no marked mail")
#define STR_MSG_NOBODY				TEXT("Because the text is not acquired, it is not possible to open\n\n")\
									TEXT("'Mark for receive' is attached, 'Update to marked' is performed and the text is acquired.")
#define STR_MSG_NONEWMAIL			TEXT("There is no newly arrived mail")
#define STR_MSG_NOFIND				TEXT("\"%s\" is not found")

//Window title
#define STR_TITLE_NOREADMAILBOX		TEXT("%s - [Unread account: %d]")
#define STR_TITLE_MAILEDIT			TEXT("Mail edit")
#define STR_TITLE_MAILVIEW			TEXT("Mail view")
#define STR_TITLE_MAILVIEW_COUNT	TEXT(" - [No.%d]")

//Message title
#define STR_TITLE_EXEC				TEXT("Update to marked")
#define STR_TITLE_ALLEXEC			TEXT("Update all accounts")
#define STR_TITLE_SEND				TEXT("Send now")
#define STR_TITLE_OPEN				TEXT("Open")
#define STR_TITLE_SAVE				TEXT("Save")
#define STR_TITLE_COPY				TEXT("Copy")
#define STR_TITLE_DELETE			TEXT("Delete")
#define STR_TITLE_ERROR				TEXT("Error")
#define STR_TITLE_SETMAILBOX		TEXT("Set account")
#define STR_TITLE_OPTION			TEXT("Option")
#ifndef _WIN32_WCE
#define STR_TITLE_STARTPASSWORD		TEXT("Startup")
#define STR_TITLE_SHOWPASSWORD		TEXT("Show")
#endif
#define STR_TITLE_FIND				TEXT("Find")
#define STR_TITLE_ALLFIND			TEXT("Look up \"%s\"")

//Window status
#define STR_STATUS_VIEWINFO			TEXT("View %d")
#define STR_STATUS_MAILBOXINFO		TEXT("View %d/ Server %d")
#define STR_STATUS_MAILINFO			TEXT("New %d, Unread %d")

//Socket status
#define STR_STATUS_GETHOSTBYNAME	TEXT("gethostbyname...")
#define STR_STATUS_CONNECT			TEXT("connect...")
#define STR_STATUS_RECV				TEXT("receive...")
#define STR_STATUS_SENDBODY			TEXT("send body...")
#define STR_STATUS_SOCKINFO			TEXT("%d byte %s")
#define STR_STATUS_SOCKINFO_RECV	TEXT("recv")
#define STR_STATUS_SOCKINFO_SEND	TEXT("send")

//Ras status
#define STR_STATUS_RAS_START		TEXT("Dial-up beginning")
#define STR_STATUS_RAS_PORTOPEN		TEXT("Open port...")
#define STR_STATUS_RAS_DEVICE		TEXT("Connecting device...")
#define STR_STATUS_RAS_AUTH			TEXT("User attestation...")
#define STR_STATUS_RAS_CONNECT		TEXT("Dial-up was connected")
#define STR_STATUS_RAS_DISCONNECT	TEXT("Dial-up was disconnect")

//Initialize status
#define STR_STATUS_INIT_MAILCNT		TEXT("%d")
#define STR_STATUS_INIT_MAILSIZE_B	TEXT("%s bytes")
#define STR_STATUS_INIT_MAILSIZE_KB	TEXT("%s KB")

//Mail list
#define STR_SAVEBOX_NAME			TEXT("[Savebox]")
#define STR_SENDBOX_NAME			TEXT("[Outbox]")
#define STR_MAILBOX_NONAME			TEXT("Untitled")
#define STR_LIST_LVHEAD_SUBJECT		TEXT("Subject")
#define STR_LIST_LVHEAD_FROM		TEXT("From")
#define STR_LIST_LVHEAD_TO			TEXT("To")
#define STR_LIST_LVHEAD_DATE		TEXT("Date")
#define STR_LIST_LVHEAD_SIZE		TEXT("Size")
#define STR_LIST_NOSUBJECT			TEXT("(no subject)")
#define STR_LIST_THREADSTR			TEXT("  + ")

#define STR_LIST_MENU_SENDINFO		TEXT("&Property...")
#define STR_LIST_MENU_REPLY			TEXT("&Reply...")
#define STR_LIST_MENU_SENDMARK		TEXT("&Mark for send\tCtrl+D")
#define STR_LIST_MENU_CREATECOPY	TEXT("Create cop&y\tCtrl+C")
#define STR_LIST_MENU_RECVMARK		TEXT("&Mark for receive\tCtrl+D")
#define STR_LIST_MENU_SAVEBOXCOPY	TEXT("Copy to &Savebox\tCtrl+C")

//Mail view
#define STR_VIEW_HEAD_FROM			TEXT("From: ")
#define STR_VIEW_HEAD_SUBJECT		TEXT("\r\nSubject: ")
#define STR_VIEW_HEAD_DATE			TEXT("\r\nDate: ")

#define STR_VIEW_MENU_ATTACH		TEXT("&Show attach")
#define STR_VIEW_MENU_SOURCE		TEXT("&View source")
#define STR_VIEW_MENU_DELATTACH		TEXT("&Delete attach")

//Mail edit
#define STR_EDIT_HEAD_MAILBOX		TEXT("Account: ")
#define STR_EDIT_HEAD_TO			TEXT("\r\nTo: ")
#define STR_EDIT_HEAD_SUBJECT		TEXT("\r\nSubject: ")

//Filter
#define STR_FILTER_USE				TEXT("Use")
#define STR_FILTER_NOUSE			TEXT("Unused")
#define STR_FILTER_STATUS			TEXT("Status")
#define STR_FILTER_ACTION			TEXT("Action")
#define STR_FILTER_ITEM1			TEXT("Item1")
#define STR_FILTER_CONTENT1			TEXT("Content1")
#define STR_FILTER_ITEM2			TEXT("Item2")
#define STR_FILTER_CONTENT2			TEXT("Content2")

#define STR_FILTER_UNRECV			TEXT("Not Receive")
#define STR_FILTER_RECV				TEXT("Receive")
#define STR_FILTER_DOWNLOADMARK		TEXT("Mark for receive")
#define STR_FILTER_DELETEMARK		TEXT("Mark for delete")
#define STR_FILTER_READICON			TEXT("Mark as read")
#define STR_FILTER_SAVE				TEXT("Copy to Savebox")

//Cc list
#define STR_CCLIST_TYPE				TEXT("Type")
#define STR_CCLIST_MAILADDRESS		TEXT("Mail address")

//Set send
#define STR_SETSEND_BTN_CC			TEXT("Cc, Bcc")
#define STR_SETSEND_BTN_ATTACH		TEXT("Attach")
#define STR_SETSEND_BTN_ETC			TEXT("Other")

//Mail Prop
#define STR_MAILPROP_HEADER			TEXT("Header")
#define STR_MAILPROP_MAILADDRESS	TEXT("Mail address")

//Address list
#define STR_ADDRESSLIST_MAILADDRESS	TEXT("Mail address")
#define STR_ADDRESSLIST_COMMENT		TEXT("Comment")

//WindowsCE
#ifdef _WIN32_WCE
#define STR_CMDBAR_RECV				TEXT("Check for new mail")
#define STR_CMDBAR_ALLCHECK			TEXT("Check all accounts")
#define STR_CMDBAR_EXEC				TEXT("Update to marked")
#define STR_CMDBAR_ALLEXEC			TEXT("Update all accounts")
#define STR_CMDBAR_STOP				TEXT("Stop")
#define STR_CMDBAR_NEWMAIL			TEXT("New message")
#define STR_CMDBAR_RAS_CONNECT		TEXT("Dial-up connect")
#define STR_CMDBAR_RAS_DISCONNECT	TEXT("Dial-up disconnect")

#define STR_CMDBAR_PREVMAIL			TEXT("Prev mail")
#define STR_CMDBAR_NEXTMAIL			TEXT("Next mail")
#define STR_CMDBAR_NEXTNOREAD		TEXT("Next unread mail")
#define STR_CMDBAR_REMESSEGE		TEXT("Reply")
#define STR_CMDBAR_ALLREMESSEGE		TEXT("Reply all")

#define STR_CMDBAR_SEND				TEXT("Send now")
#define STR_CMDBAR_SENDBOX			TEXT("Save to Outbox")
#define STR_CMDBAR_SENDINFO			TEXT("Property")

#define STR_LIST_PPCMENU_SENDINFO	TEXT("&Property")
#define STR_LIST_PPCMENU_REPLY		TEXT("&Reply")
#define STR_LIST_PPCMENU_SENDMARK	TEXT("&Mark for send")
#define STR_LIST_PPCMENU_CREATECOPY	TEXT("Create cop&y")
#define STR_LIST_PPCMENU_RECVMARK	TEXT("&Mark for receive")
#define STR_LIST_PPCMENU_SAVEBOXCOPY	TEXT("Copy to &Savebox")

#define STR_VIEW_PPCMENU_ATTACH		TEXT("&Show attach")
#define STR_VIEW_PPCMENU_SOURCE		TEXT("&View source")
#define STR_VIEW_PPCMENU_DELATTACH	TEXT("&Delete attach")

#ifdef _WIN32_WCE_PPC
#define STR_TITLE_SMTPAUTH			TEXT("SMTP-AUTH")
#define STR_TITLE_FILTER			TEXT("Set filter")
#define STR_TITLE_INITMAILBOX		TEXT("Initialize")
#define STR_TITLE_CCBCC				TEXT("Cc, Bcc")
#define STR_TITLE_ATTACH			TEXT("Attach files")
#define STR_TITLE_ETCHEADER			TEXT("Other headers")
#define STR_TITLE_SENDINFO			TEXT("Property")
#define STR_TITLE_ADDRESSINFO		TEXT("Address information")
#define STR_TITLE_EDITADDRESS		TEXT("Address")
#define STR_TITLE_ADDRESSLIST		TEXT("Address book")
#define STR_TITLE_FIND				TEXT("Find")
#endif
#endif

#endif
/* End of source */
