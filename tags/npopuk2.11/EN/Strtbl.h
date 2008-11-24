/*
 * nPOP
 *
 * Strtbl.h (EN)
 *
 * Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved.
 *		http://www.nakka.com/
 *		nakka@nakka.com
 *
 * nPOPuk code additions copyright (C) 2006-2008 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopuk.org.uk
 */

#ifndef _INC_STR_TBL_H
#define _INC_STR_TBL_H

// General
#ifdef _WIN32_WCE
#define STR_DEFAULT_FONT			TEXT("")
#else
#define STR_DEFAULT_FONT			TEXT("")
#endif
#define STR_DEFAULT_FONTCHARSET		DEFAULT_CHARSET

#define STR_DEFAULT_BURA			TEXT("")
#define STR_DEFAULT_OIDA			TEXT("")

#define STR_DEFAULT_HEAD_CHARSET	TEXT("")
#define STR_DEFAULT_HEAD_ENCODE		0		// 0-7bit 1-8bit 2-base64 3-quoted-printable
#define STR_DEFAULT_BODY_CHARSET	TEXT("")
#define STR_DEFAULT_BODY_ENCODE		0		// 0-7bit 1-8bit 2-base64 3-quoted-printable

#define STR_DEFAULT_TIMEZONE		TEXT("")
#define STR_DEFAULT_DATEFORMAT		TEXT("dd/MM/yyyy")
#define STR_DEFAULT_TIMEFORMAT		TEXT("HH:mm")

#define STR_FILE_FILTER				TEXT("All Files (*.*)\0*.*\0\0")
#define STR_TEXT_FILTER				TEXT("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0")
#define STR_WAVE_FILTER				TEXT("Sound Files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0\0")
#define STR_MBOX_FILTER				TEXT("MBOX Files (*.mbx)\0*.mbx\0EML Files (*.eml)\0*.eml\0nPOP Files (*.dat)\0*.dat\0All Files (*.*)\0*.*\0\0")

// Error
#define STR_ERR_MEMALLOC			TEXT("Memory Allocation error")
#define STR_ERR_INIT				TEXT("Initialisation error")
#define STR_ERR_INIFILE				TEXT("Cannot find INI file: %s")
#define STR_ERR_READONLY			TEXT("Application directory is read-only")
#define STR_ERR_NODATADIR			TEXT("DataFileDir is not a directory")
#define STR_ERR_DATAREADONLY		TEXT("DataFileDir is read-only")
#define STR_ERR_BACKUP_APPDIR		TEXT("Cannot back-up to Application directory")
#define STR_ERR_BACKUP_DATADIR		TEXT("Cannot back-up to DataFileDir")
#define STR_ERR_OPENMAILBOX			TEXT("Error opening %s")
#define STR_ERR_OPEN				TEXT("Open File error")
#define STR_ERR_SAVEEND				TEXT("Save File error \nContinue?")
#define STR_ERR_SAVE				TEXT("Save File error")
#define STR_ERR_PARTIAL_ATTACH		TEXT("Attachments were not fully downloaded")
#define STR_ERR_FILENAME_TOO_LONG	TEXT("Filename or path too long")
#define STR_ERR_ADD					TEXT("Address not added")
#define STR_ERR_POOM				TEXT("POOM failure; check \\Windows\\pimstore.dll")
#define STR_ERR_VIEW				TEXT("Display failed")
#define STR_ERR_TOOMANYMAILBOXES	TEXT("Too many mailboxes")
#define STR_ERR_TOOMANYFILES		TEXT("Too many files selected; try again.")
#define STR_ERR_SELECTMAILBOX		TEXT("No account specified")
#define STR_ERR_SELECTMAILADDR		TEXT("No address selected")
#define STR_ERR_SETMAILADDR			TEXT("No mail address set")
#define STR_ERR_INPUTMAILADDR		TEXT("No mail address entered")
#define STR_ERR_COPYFAIL			TEXT("Copy failed")
#define STR_ERR_SAVECOPY			TEXT("Copy to Savebox failed")
#define STR_ERR_NOITEM1				TEXT("Item 1 not set")
#define STR_ERR_NOITEM2				TEXT("Item 2 needed for UNLESS")
#define STR_ERR_NOSAVEBOX			TEXT("No Savebox selected")
#define STR_ERR_NOPRIORITY			TEXT("No Priority selected")
#define STR_ERR_NOSAVEBOXES			TEXT("No Savebox found")
#define STR_ERR_NOFWDADDRESS		TEXT("No forwarding address entered")
#define STR_ERR_INPUTFINDSTRING		TEXT("No 'Find' string entered")
#define STR_ERR_FINDISREPLACE		TEXT("'Find' and 'Replace' strings identical!")
#define STR_ERR_NOMAIL				TEXT("No Mail in the list")
#define STR_ERR_SENDLOCK			TEXT("Sending barred! Transmission in progress")
#define STR_ERR_FILTBOX				TEXT("A filter for %s has been disabled because it refers to an invalid SaveBox")
#define STR_ERR_GBLFILTBOX			TEXT("A global filter has been disabled because it refers to an invalid SaveBox")
#define STR_ERR_CANTDELETE			TEXT("Error deleting old backup file %s; please delete manually.")
#define STR_ERR_LOADMAILBOX			TEXT("Error loading %s")
#define STR_ERR_FILEEXIST			TEXT("Can't find file %s")
#define STR_ERR_FILE_TOO_LARGE		TEXT("File %s too large")
#define STR_ERR_FILENAME			TEXT("Invalid character in filename")
#define STR_ERR_FILECONFLICT		TEXT("\"%s\" conflicts with filename for %s")
#define STR_ERR_NOFILENAME			TEXT("No file name specified")
#define STR_ERR_NOSBOXNAME			TEXT("No name specified for savebox")
#define STR_ERR_NOUIDLNOFILL		TEXT("Cannot fill-in messages because 'NoUIDL=1' for this account")
#define STR_ERR_DUPPARAM			TEXT("Duplicate parameter '%s'")

// Socket error
#define STR_ERR_SOCK_SELECT			TEXT("Selection error")
#define STR_ERR_SOCK_DISCONNECT		TEXT("Server disconnected")
#define STR_ERR_SOCK_CONNECT		TEXT("Connection failed")
#define STR_ERR_SOCK_SENDRECV		TEXT("Send/Recv error")
#define STR_ERR_SOCK_EVENT			TEXT("Event setting error")
#define STR_ERR_SOCK_NOSERVER		TEXT("Server name not set")
#define STR_ERR_SOCK_GETIPADDR		TEXT("Unable to find email server")
#define STR_ERR_SOCK_CREATESOCKET	TEXT("Socket creation error")
#define STR_ERR_SOCK_TIMEOUT		TEXT("Connection timed-out")
#define STR_ERR_SOCK_SEND			TEXT("Send error")
#define STR_ERR_SOCK_RESPONSE		TEXT("Response not analysed\n\n")
#define STR_ERR_SOCK_GETITEMINFO	TEXT("Item information not acquired")
#define STR_ERR_SOCK_MAILSYNC 		TEXT("Mail numbers not synchronised\n\n")\
									TEXT("New and existing numbers must agree")
#define STR_ERR_SOCK_NOMESSAGEID	TEXT("Message-Id not acquired")
#define STR_ERR_SOCK_NOUSERID		TEXT("No username set")
#define STR_ERR_SOCK_NOPASSWORD		TEXT("No password set")
#define STR_ERR_SOCK_BADPASSWORD	TEXT("Username or password error\n\n")
#define STR_ERR_SOCK_ACCOUNT		TEXT("Account was not accepted\n\n")
#define STR_ERR_SOCK_NOAPOP			TEXT("Server does not accept APOP")
#define STR_ERR_SOCK_STAT			TEXT("STAT failed\n\n")
#define STR_ERR_SOCK_TOP			TEXT("TOP failed\n\n")
#define STR_ERR_SOCK_RETR			TEXT("RETR failed\n\n")
#define STR_ERR_SOCK_DELE			TEXT("Deletion failed\n\n")
#define STR_ERR_SOCK_NOATTACH		TEXT("Attached file was not found")
#define STR_ERR_SOCK_BADFROM		TEXT("Address error: Check SMTP account settings")
#define STR_ERR_SOCK_HELO 			TEXT("HELO failed\n")\
									TEXT("Please check address format\n\n")
#define STR_ERR_SOCK_SMTPAUTH		TEXT("Login to SMTP server failed\n\n")
#define STR_ERR_SOCK_RSET			TEXT("RSET failed\n\n")
#define STR_ERR_SOCK_MAILFROM		TEXT("MAIL FROM failed\n")\
									TEXT("Please check address format\n\n")
#define STR_ERR_SOCK_NOTO			TEXT("Destination not set")
#define STR_ERR_SOCK_RCPTTO			TEXT("RCPT TO failed\n")\
									TEXT("Please check destination address\n\n")
#define STR_ERR_SOCK_DATA			TEXT("DATA failed\n\n")
#define STR_ERR_SOCK_MAILSEND		TEXT("Mail not sent\n\n")
#define STR_ERR_SOCK_SSL_INIT		TEXT("Initialisation of SSL failed\n%s")
#define STR_ERR_SOCK_SSL_VERIFY		TEXT("Verify of SSL failed\n%s")
#define STR_ERR_SOCK_NOSSL			TEXT("Initialisation failure of 'npopssl.dll'")
	
// Ras error
#define STR_ERR_RAS_NOSET			TEXT("No dial-up setting available")
#define STR_ERR_RAS_CONNECT			TEXT("Dial-up connection failed")
#define STR_ERR_RAS_DISCONNECT		TEXT("Dial-up was cut or line dropped")

// Warning
#define STR_WARN_ATTACH_MEM			TEXT("Attachments may be too large to encode\nin available memory")
#define STR_WARN_BACKUPDIR			TEXT("Previous backup directory no longer exists")

// Question
#define STR_Q_DELETE				TEXT("Delete it?")
#define STR_Q_DELSERVERMAIL			TEXT("Warning! Messages are marked to\ndelete from server. Delete them?")
#define STR_Q_DELLISTMAIL			TEXT("Delete %d mail from the list?%s")
#define STR_Q_DELLISTMAIL_NOSERVER	TEXT("\n(Is not deleted from the server)")
#define STR_Q_DEL_FWDHOLD			TEXT("Message is held to forward attachment. Delete anyway?")
#define STR_Q_DEL_FWDHOLD_ACCT		TEXT("Message(s) in account \"%s\" are held to forward attachment.\nDelete anyway?")
#define STR_Q_DELSERVERNOHOLD		TEXT("Delete messages not being held for forwarding?")
#define STR_Q_DELMAILBOX			TEXT("Delete mailbox \"%s\"?")
#define STR_Q_DELMAILBOXES			TEXT("Delete %d mailboxes?")
#define STR_Q_DELATTACH				TEXT("Delete attached files?")
#define STR_Q_DELSBOXFILE			TEXT("Delete existing file \"%s%s\" from DataFileDir?")
#define STR_Q_OVERWRITE				TEXT("\"%s\" \nMessage already in Savebox\nOverwrite?")
#define STR_Q_COPY					TEXT("Copy %d mail to %s?")
#define STR_Q_MOVE					TEXT("Move %d mail to %s?")
#define STR_Q_DEPENDENCE			TEXT("There is a character depending on the model.  Proceed?")
#define STR_Q_UNLINKATTACH			TEXT("Release the link to the attached file?")
#define STR_Q_ADDADDRESS			TEXT("Edit addresses (%d) when adding to the address book?")
#ifdef _WIN32_WCE
#define STR_Q_EDITADDPOOM			TEXT("Edit addresses (%d) when adding to PocketContacts?")
#define STR_Q_ADDPOOM				TEXT("Add address to PocketContacts?\n(Else lost on program exit)")
#endif
#define STR_Q_NEXTFIND				TEXT("Search completed!\nRedo from the start?")
#define STR_Q_EDITCANCEL			TEXT("Cancel the edit?")
#define STR_Q_SENDMAIL				TEXT("Send it?")
#define STR_Q_ATTACH				TEXT("Open this file?")
#define STR_Q_FORWARDMAIL			TEXT("The message you are forwarding may be incomplete.\nProceed to forward anyway?")
#define STR_Q_RASDISCON				TEXT("Disconnect dial-up connection?")
#define STR_Q_QUEUEDMAIL_EXIT		TEXT("Messages marked for sending haven't been sent.\nExit anyway?")
#define STR_Q_EXITSAVE				TEXT("Save files?")
#define STR_Q_UPGRADE				TEXT("Import settings from nPOP to nPOPuk?")
#define STR_Q_USEPOOM				TEXT("Using PocketContacts will replace existing items in Address book.\nProceed anyway?")
#define STR_Q_LOADMAILBOX			TEXT("Load mailbox \"%s\"?")
#define STR_Q_LOADASMBOX			TEXT("Load %s as MBOX (%d messages) instead of nPOP format (1 message)?")
#define STR_Q_SCRAMBLE_SAVE			TEXT("Save all mailboxes with new settings?")
#define STR_Q_NOBODYDELATT			TEXT("Message has no text part; deleting attachments\nwill leave nothing.  Proceed?")
#define STR_Q_SAVE_EMBEDDED			TEXT("Convert embedded image names?")
#define STR_Q_ATT_SAME_NAME			TEXT("A later attachment has the same name, but is larger.\nSkip this one?")
#define STR_Q_CREATE_INIFILE		TEXT("Create INIFILE at '%s'?")

// Message
#define STR_MSG_NOMARK				TEXT("There is no marked mail")
#define STR_MSG_MARK_HELD			TEXT("Marked message is held for forwarding")
#define STR_MSG_ATT_HELD			TEXT("Attachments of this message are held for forwarding")
#define STR_MSG_NOATT				TEXT("Attached file not found; verify attachment list")
#define STR_MSG_NOFWD				TEXT("Original message not found to forward attachments")
#define STR_MSG_NOBODY				TEXT("Message body not downloaded. Open anyway?")
#define STR_MSG_NONEWMAIL			TEXT("No new mail!")
#define STR_MSG_NOFIND				TEXT("\"%s\" not found")
#define STR_REPLACED_N				TEXT("Replaced %d instances")
#define STR_MSG_NEWVERSION			TEXT("%s was created with a newer version of %s.\nSome incompatibilities may exist.\nProceed anyway?")
#define STR_MSG_NOTEXTPART			TEXT("[Message has no text part]")
#define STR_MSG_ADDR_ALREADY		TEXT("All addresses already in address book")

// Window title
#define STR_TITLE_NEWMAILBOX		TEXT("%s - [Mailboxes with new mail: %d]")
#define STR_TITLE_MAILEDIT			TEXT("Mail Edit")
#define STR_TITLE_MAILSENT			TEXT("Sent Mail")
#define STR_TITLE_MAILVIEW			TEXT("Mail View")
#define STR_TITLE_MAILVIEW_COUNT	TEXT(" - [No.%d]")

// Message title
#define STR_TITLE_EXEC				TEXT("Update account")
#define STR_TITLE_ALLEXEC			TEXT("Update all accounts")
#define STR_TITLE_SEND				TEXT("Send now")
#define STR_TITLE_OPEN				TEXT("Open")
#define STR_TITLE_SAVE				TEXT("Save")
#define STR_TITLE_COPY				TEXT("Copy")
#define STR_TITLE_MOVE				TEXT("Move")
#define STR_TITLE_DELETE			TEXT("Delete")
#define STR_TITLE_ERROR				TEXT("Error")
#define STR_TITLE_SETMAILBOX		TEXT("Account Settings")
#define STR_TITLE_OPTION			TEXT("Global Options")
#define STR_TITLE_STARTPASSWORD		TEXT("Startup")
#define STR_TITLE_SHOWPASSWORD		TEXT("Show")
#define STR_TITLE_FIND				TEXT("Find")
#define STR_TITLE_REPLACE			TEXT("Replace")
#define STR_TITLE_ALLFIND			TEXT("Look up \"%s\"")
#define STR_TITLE_ATTACHED			TEXT("Attachment")
#define STR_TITLE_COPY2				TEXT("Copy to:")
#define STR_TITLE_MOVE2				TEXT("Move to:")
#define STR_TITLE_RENAMESBOX		TEXT("Rename SaveBox")
#define STR_RADIO_RENAMESBOX		TEXT("Rename &savebox")
#define STR_TITLE_ADDSBOX			TEXT("Add SaveBox")
#define STR_TITLE_ADD_ADDR			TEXT("Add Addresses")

// Window status
#ifdef _WIN32_WCE
#define STR_STATUS_VIEWINFO			TEXT("%d View (%s) ")
#define STR_STATUS_MAILBOXINFO		TEXT("%d/%d View/Server")
#define STR_STATUS_MAILINFO			TEXT("%d/%d New/Unread")
#define STR_STATUS_MAILINFO_U		TEXT("%d/%d/%d New/Unread/Unsent")
#define STR_STATUS_UNSENT			TEXT("%d Unsent")
#else
#define STR_STATUS_VIEWINFO			TEXT("View %d (%s) ")
#define STR_STATUS_MAILBOXINFO		TEXT("View %d (%s) / Server %d (%s)")
#define STR_STATUS_MAILINFO			TEXT("New %d, Unread %d")
#define STR_STATUS_MAILINFO_U		TEXT("New %d, Unread %d, Unsent %d")
#define STR_STATUS_UNSENT			TEXT("Unsent %d")
#endif
#define STR_STATUS_MAILSIZE_B		TEXT("%d B")
#define STR_STATUS_MAILSIZE_KB_d	TEXT("%d KB")
#define STR_STATUS_MAILSIZE_KB		TEXT("%s KB")
#define STR_STATUS_MAILSIZE_MB		TEXT("%s MB")
#define STR_STATUS_MAILSIZE_GB		TEXT("%s GB")

// Socket status
#define STR_STATUS_GETHOSTBYNAME	TEXT("Finding Host...")
#define STR_STATUS_CONNECT			TEXT("Connecting...")
#define STR_STATUS_SSL				TEXT("SSL connect...")
#define STR_STATUS_RECV				TEXT("Receiving...")
#define STR_STATUS_SENDBODY			TEXT("Sending body...")
#define STR_STATUS_SEND_ATT			TEXT("Sending attachment %d...")
#define STR_STATUS_ATT_END			TEXT("Done sending attachments")
#ifdef WSAASYNC
#define STR_STATUS_SOCKINFO			TEXT("[....................] %d bytes %s")
#else
#define STR_STATUS_SOCKINFO			TEXT("%d bytes %s")
#endif
#define STR_STATUS_SOCKINFO_RECV	TEXT("Recv")
#define STR_STATUS_RECVDONE			TEXT("Recv done.")
#define STR_STATUS_SOCKINFO_SEND	TEXT("Sent")
#define STR_STATUS_SEND_USER		TEXT("Send username")
#define STR_STATUS_SEND_PASS		TEXT("Send password")

// Ras status
#define STR_STATUS_RAS_START		TEXT("Starting Dial-up")
#define STR_STATUS_RAS_PORTOPEN		TEXT("Opening port...")
#define STR_STATUS_RAS_DEVICE		TEXT("Connecting device...")
#define STR_STATUS_RAS_AUTH			TEXT("Verifying user...")
#define STR_STATUS_RAS_CONNECT		TEXT("Dial-up connected")
#define STR_STATUS_RAS_DISCONNECT	TEXT("Dial-up disconnected")

//Initialise status
#define STR_STATUS_INIT_MAILCNT		TEXT("%d")
#define STR_STATUS_INIT_MAILSIZE_B	TEXT("%s bytes")
#define STR_STATUS_INIT_MAILSIZE_KB	TEXT("%s KB")

// Mail list
#define STR_SAVEBOX_NAME			TEXT("[Savebox]")
#define STR_SAVEBOX_NONAME			TEXT("Savebox-%d")
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
#define STR_LIST_MENU_REPLY			TEXT("&Reply...\tCtrl+R")
#define STR_LIST_MENU_REPLYALL		TEXT("Repl&y to all...\tCtrl+Alt+R")
#define STR_LIST_MENU_SENDMARK		TEXT("&Mark to send\tCtrl+D")
#define STR_LIST_MENU_DELMARK		TEXT("Mark to &delete\tCtrl+Del")
#define STR_LIST_MENU_CREATECOPY	TEXT("Create cop&y\tCtrl+C")
#define STR_LIST_MENU_DELATTACH		TEXT("Delete attac&hments")
#define STR_LIST_MENU_RECVMARK		TEXT("&Mark to download\tCtrl+D")
#define STR_LIST_MENU_DELLIST		TEXT("Delete from &list\tShift+Del")
#define STR_LIST_MENU_COPYSBOX		TEXT("C&opy to")
#define STR_LIST_MENU_MOVESBOX		TEXT("Mo&ve to")
#define STR_LIST_MENU_COPYSBOX1		TEXT("&Copy to")
#define STR_LIST_MENU_MOVESBOX1		TEXT("&Move to")
#define STR_LIST_MENU_NEW			TEXT("(New)")

// Mail view
#define STR_VIEW_HEAD_FROM			TEXT("From: ")
#define STR_VIEW_HEAD_SUBJECT		TEXT("\r\nSubject: ")
#define STR_VIEW_HEAD_DATE			TEXT("\r\nDate: ")

#define STR_VIEW_MENU_ATTACH		TEXT("&View text")
#define STR_VIEW_MENU_SOURCE		TEXT("&View source")
#define STR_VIEW_MENU_SAVEATTACH	TEXT("&Save attach")
#define STR_VIEW_MENU_DELATTACH		TEXT("&Delete attach")
#define STR_VIEW_RETURN				TEXT("&Return to original")

#define STR_HTML_CONV				TEXT("[HTML tags removed by nPOPuk]\r\n")
#define STR_NPOPUK_FILES			TEXT("nPOPuk files")

// Mail edit
#define STR_EDIT_HEAD_MAILBOX		TEXT("Account: ")
#define STR_EDIT_HEAD_TO			TEXT("\r\nTo: ")
#define STR_EDIT_HEAD_SUBJECT		TEXT("\r\nSubject: ")
#define STR_REPLACE_DONE			TEXT("Done")

// SSL, SMTP-AUTH
#define STR_SSL_AUTO				TEXT("Auto")
#define STR_SSL_TLS10				TEXT("TLS 1.0")
#define STR_SSL_SSL30				TEXT("SSL 3.0")
#define STR_SSL_SSL20				TEXT("SSL 2.0")
#define STR_SSL_STARTTLS			TEXT("STARTTLS")
#define STR_SMTPAUTH_CRAM_MD5		TEXT("CRAM-MD5")
#define STR_SMTPAUTH_LOGIN			TEXT("LOGIN")
#define STR_SMTPAUTH_PLAIN			TEXT("PLAIN")

// Filter
#define STR_FILTER_USE				TEXT("Use")
#define STR_FILTER_NOUSE			TEXT("Unused")
#define STR_FILTER_STATUS			TEXT("Status")
#define STR_FILTER_ACTION			TEXT("Action")
#define STR_FILTER_ITEM1			TEXT("Item 1")
#define STR_FILTER_CONTENT1			TEXT("Content 1")
#define STR_FILTER_BOOLEAN			TEXT("Op")
#define STR_FILTER_ITEM2			TEXT("Item 2")
#define STR_FILTER_CONTENT2			TEXT("Content 2")
#define STR_FILTER_UNRECV			TEXT("Reject")
#define STR_FILTER_RECV				TEXT("Accept")
#define STR_FILTER_DOWNLOADMARK		TEXT("Mark to download")
#define STR_FILTER_DELETEMARK		TEXT("Mark to delete")
#define STR_FILTER_READICON			TEXT("Mark as read")
#define STR_FILTER_COPY				TEXT("Copy to:")
#define STR_FILTER_MOVE				TEXT("Move to:")
#define STR_FILTER_PRIORITY			TEXT("Set priority:")
#define STR_FILTER_FORWARD			TEXT("Forward to:")
#define STR_FILTER_SAVEBOX			TEXT("&Savebox:")
#define STR_FILTER_FWDADDR			TEXT("A&ddress:")
#define STR_FILTER_PRIOLVL			TEXT("&Level:")
#define STR_FILTER_AND				TEXT("&&")
#define STR_FILTER_OR				TEXT("||")
#define STR_FILTER_UNLESS			TEXT("&& !")
#define STR_LAZYLOAD_STARTUP		TEXT("All at startup")
#define STR_LAZYLOAD_AUTO			TEXT("Automatically as needed")
#define STR_LAZYLOAD_AUTO_NOSEARCH	TEXT("Auto, but not for find/next unread")
#define STR_LAZYLOAD_PROMPT			TEXT("Ask before loading for find/next unread")

// Cc list
#define STR_CCLIST_TYPE				TEXT("Type")
#define STR_CCLIST_MAILADDRESS		TEXT("Mail address")

// Set send
#define STR_SETSEND_BTN_CC			TEXT("&Recipients")
#define STR_SETSEND_BTN_ATTACH		TEXT("Attac&h")
#define STR_FWDATT_PREFIX			TEXT("Fwd:")
#define STR_FWD_ORIG_MSG			TEXT("Fwd: (original message)")
#define STR_OMIT_REPLYTO			TEXT("(Omit)")

// Mail Prop
#define STR_MAILPROP_HEADER			TEXT("Header")
#define STR_MAILPROP_MAILADDRESS	TEXT("Mail address")

// Mailbox list
#define STR_MAILBOXES_NAME			TEXT("Name")
#define STR_MAILBOXES_MAILCOUNT		TEXT("New/Unread/Total")
#define STR_MAILBOXES_MAILCNT		TEXT("N/U/T")
#define STR_MAILBOXES_DISKSIZE		TEXT("Disk Size")
#define STR_MAILBOXES_NEEDS_SAVE	TEXT("Changed")
#define STR_MAILBOXES_LOADED		TEXT("Loaded")
#define STR_MAILBOXES_FILENAME		TEXT("Filename")

// Address list
#define STR_ADDRESSLIST_MAILADDRESS	TEXT("Mail address")
#define STR_ADDRESS_SELECT			TEXT("&Select")
#define STR_ADDRESSLIST_COMMENT		TEXT("Comment")
#ifdef _WIN32_WCE
#define STR_ADDRESSLIST_GROUP		TEXT("Categories")
#define STR_ADDRESSLIST_ALLGROUP	TEXT("[All]")
#define STR_ADDRESSLIST_NOGROUP		TEXT("[No cat.]")
#define STR_ADDRESSBOOK_ADDGROUP	TEXT("Change category")
#define STR_ADDREDIT_NAME			TEXT("Name")
#else
#define STR_ADDRESSLIST_GROUP		TEXT("Group")
#define STR_ADDRESSLIST_ALLGROUP	TEXT("[All]")
#define STR_ADDRESSLIST_NOGROUP		TEXT("[No group]")
#define STR_ADDRESSBOOK_ADDGROUP	TEXT("Change group")
#endif
#define STR_MULTIPLE_ADDRESSES		TEXT("[Multiple addresses]")
#define STR_EXAMPLE_ADDRESS			TEXT("\"Name\" <username@domain.com>")
#define STR_AUTO_ADDED_ADDRESS		TEXT("Replied-to")

// WindowsCE
#ifdef _WIN32_WCE
#define STR_CMDBAR_RECV				TEXT("Check account")
#define STR_CMDBAR_ALLCHECK			TEXT("Check all accounts")
#define STR_CMDBAR_EXEC				TEXT("Update account")
#define STR_CMDBAR_ALLEXEC			TEXT("Update all accounts")
#define STR_CMDBAR_STOP				TEXT("Stop")
#define STR_CMDBAR_NEWMAIL			TEXT("New message")
#define STR_CMDBAR_RAS_CONNECT		TEXT("Dial-up connect")
#define STR_CMDBAR_RAS_DISCONNECT	TEXT("Dial-up disconnect")

#define STR_CMDBAR_PREVMAIL			TEXT("Prev mail")
#define STR_CMDBAR_NEXTMAIL			TEXT("Next mail")
#define STR_CMDBAR_NEXTUNREAD		TEXT("Next unread mail")
#define STR_CMDBAR_REMESSEGE		TEXT("Reply")
#define STR_CMDBAR_ALLREMESSEGE		TEXT("Reply to all")
#define STR_CMDBAR_DOWNMARK			TEXT("Mark to download")
#define STR_CMDBAR_SENDMARK			TEXT("Mark to send")
#define STR_CMDBAR_DELMARK			TEXT("Mark to delete")
#define STR_CMDBAR_DELETE			TEXT("Delete from list")
#define STR_CMDBAR_UNREADMARK		TEXT("Mark as read")
#define STR_CMDBAR_FLAGMARK			TEXT("Flag for follow-up")
#define STR_CMDBAR_FORWARD			TEXT("Forward")
#define STR_CMDBAR_NEXTFIND			TEXT("Find next")

#define STR_CMDBAR_SEND				TEXT("Send now")
#define STR_CMDBAR_SBOXMARK			TEXT("Save and Mark")
#define STR_CMDBAR_SENDBOX			TEXT("Save to Outbox")
#define STR_CMDBAR_SENDINFO			TEXT("Property")

#define STR_LIST_PPCMENU_SENDINFO	TEXT("&Property")
#define STR_LIST_PPCMENU_REPLY		TEXT("&Reply")
#define STR_LIST_PPCMENU_REPLYALL		TEXT("Repl&y to all")
#define STR_LIST_PPCMENU_FORWARD		TEXT("&Forward")
#define STR_LIST_PPCMENU_SENDMARK	TEXT("&Mark to send")
#define STR_LIST_PPCMENU_DELMARK	TEXT("&Mark to delete")
#define STR_LIST_PPCMENU_CREATECOPY	TEXT("Create cop&y")
#define STR_LIST_PPCMENU_RECVMARK	TEXT("&Mark to download")
#define STR_LIST_PPCMENU_DELLIST		TEXT("Delete from &list")

#ifdef _WIN32_WCE_PPC
#define STR_TITLE_SMTPAUTH			TEXT("SMTP-AUTH")
#define STR_TITLE_SETSSL			TEXT("Set SSL")
#define STR_TITLE_FILTER			TEXT("Set filter")
#define STR_TITLE_INITMAILBOX		TEXT("Initialise")
#define STR_TITLE_CCBCC				TEXT("Recipients")
#define STR_TITLE_ATTACH			TEXT("Attach files")
#define STR_TITLE_ETCHEADER			TEXT("Other headers")
#define STR_TITLE_SENDINFO			TEXT("Property")
#define STR_TITLE_ADDRESSINFO		TEXT("Address information")
#define STR_TITLE_EDITADDRESS		TEXT("Address")
#define STR_TITLE_ADDRESSLIST		TEXT("Address book")
#define STR_TITLE_MAILBOXLIST		TEXT("Mailboxes")
#define STR_TITLE_MAILBOXPANE		TEXT("Mailbox pane")
#define STR_TITLE_FIND				TEXT("Find")
#define STR_TITLE_ENCODE			TEXT("Encoding")
#define STR_TITLE_NEWMBOX			TEXT("Create mailbox")

#define STR_SF_TITLE				TEXT("Select file")
#define STR_SF_LV_NAME				TEXT("Name")
#define STR_SF_LV_TYPE				TEXT("Type")
#define STR_SF_LV_SIZE				TEXT("Size")
#define STR_SF_LV_DATE				TEXT("Date")
#define STR_SF_Q_OVERWRITE			TEXT("Replace existing file?")
#endif	//_WIN32_WCE_PPC

#ifdef _WIN32_WCE_LAGENDA
#define STR_MENU_FILE				TEXT("File")
#define STR_MENU_MAIL				TEXT("Mail")
#define STR_MENU_EDIT				TEXT("Edit")
#endif	//_WIN32_WCE_LAGENDA
#endif	//_WIN32_WCE

///////////// MRP /////////////////////
#define STR_TITLE_ABOUT		TEXT("About")
#define STR_WEB_ADDR		TEXT("http://www.npopuk.org.uk") // was www.npopsupport.org.uk
#define STR_ABOUT_TEXT		TEXT("Extended from nPOP Version 1.0.9\r\nCopyright © 1996-2006 by \r\nTomoaki Nakashima. All rights reserved.\r\n\r\nhttp://www.nakka.com/\r\nnakka@nakka.com\r\n\r\nUK Fix info at http://www.npopuk.org.uk\r\nContributions from Greg Chapman, Geoffrey Coram, Werner Furlan, Paul Holmes-Higgin, Bruce Jackson, Glenn Linderman, Amy Millenson, Matthew R. Pattman, and Gerard Samija.\r\n")
///////////// --- /////////////////////

#endif	//_INC_STR_TBL_H
/* End of source */
