/**************************************************************************

	nPOP

	Strtbl.h

	Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
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

#define STR_DEFAULT_BURA			TEXT("")
#define STR_DEFAULT_OIDA			TEXT("")

#define STR_DEFAULT_HEAD_CHARSET	TEXT("ISO-8859-1")
#define STR_DEFAULT_HEAD_ENCODE		3		// 0-7bit 1-8bit 2-base64 3-quoted-printable
#define STR_DEFAULT_BODY_CHARSET	TEXT("ISO-8859-1")
#define STR_DEFAULT_BODY_ENCODE		3		// 0-7bit 1-8bit 2-base64 3-quoted-printable

#define STR_DEFAULT_DATEFORMAT		TEXT("dd/MM/yyyy")
#define STR_DEFAULT_TIMEFORMAT		TEXT("HH:mm")

#define STR_FILE_FILTER				TEXT("Tutti i files (*.*)\0*.*\0\0")
#define STR_TEXT_FILTER				TEXT("Files di testo (*.txt)\0*.txt\0Tutti i files (*.*)\0*.*\0\0")
#define STR_WAVE_FILTER				TEXT("Suoni (*.wav)\0*.wav\0Tutti i files (*.*)\0*.*\0\0")

//Error
#define STR_ERR_MEMALLOC			TEXT("Errore di allocazione di memoria")
#define STR_ERR_INIT				TEXT("Errore di inizializzazione")
#define STR_ERR_OPEN				TEXT("Errore apertura file")
#define STR_ERR_SAVEEND				TEXT("Errore salvataggio file \nContinuare?")
#define STR_ERR_SAVE				TEXT("Errore salvataggio file")
#define STR_ERR_ADD					TEXT("L'indirizzo non è stato aggiunto")
#define STR_ERR_VIEW				TEXT("Visualizzazione fallita")
#define STR_ERR_SELECTMAILBOX		TEXT("Nessun account specificato")
#define STR_ERR_SELECTMAILADDR		TEXT("Nessun indirizzo selezionato")
#define STR_ERR_SETMAILADDR			TEXT("Nessun indirizzo mail impostato")
#define STR_ERR_INPUTMAILADDR		TEXT("Nessun indirizzo mail inserito")
#define STR_ERR_CREATECOPY			TEXT("Copia fallita")
#define STR_ERR_SAVECOPY			TEXT("Copia in 'Posta Salvata' fallita")
#define STR_ERR_NOITEM1				TEXT("Item 1 non impostato")
#define STR_ERR_INPUTFINDSTRING		TEXT("Non 'Trovo' il testo immesso")
#define STR_ERR_NOMAIL				TEXT("Nessuna mail in lista")
#define STR_ERR_SENDLOCK			TEXT("Invio escluso!  Trasmissione in progresso")

//Socket error
#define STR_ERR_SOCK_SELECT			TEXT("Errore di selezione")
#define STR_ERR_SOCK_DISCONNECT		TEXT("Server disconnesso")
#define STR_ERR_SOCK_CONNECT		TEXT("Connessione fallita")
#define STR_ERR_SOCK_SENDRECV		TEXT("Errore invio/ricezione")
#define STR_ERR_SOCK_EVENT			TEXT("Errore 'Event setting'")
#define STR_ERR_SOCK_NOSERVER		TEXT("Nome server non impostato")
#define STR_ERR_SOCK_GETIPADDR		TEXT("Impossibile trovare email server")
#define STR_ERR_SOCK_CREATESOCKET	TEXT("Errore creazione socket")
#define STR_ERR_SOCK_TIMEOUT		TEXT("Connessione timed-out")
#define STR_ERR_SOCK_SEND			TEXT("Errore di invio")
#define STR_ERR_SOCK_RESPONSE		TEXT("Risposta non analizzata\n\n")
#define STR_ERR_SOCK_GETITEMINFO	TEXT("Informazioni 'Item' non ricevute")
#define STR_ERR_SOCK_MAILSYNC 		TEXT("Numero mail non sincronizzato\n\n")\
									TEXT("Numeri nuovi ed esistenti devono essere accettati")
#define STR_ERR_SOCK_NOMESSAGEID	TEXT("ID messaggio non acquisito")
#define STR_ERR_SOCK_NOUSERID		TEXT("Username non settato")
#define STR_ERR_SOCK_NOPASSWORD		TEXT("Password non settata")
#define STR_ERR_SOCK_BADPASSWORD	TEXT("Errore username o password\n\n")
#define STR_ERR_SOCK_ACCOUNT		TEXT("Account non accettato\n\n")
#define STR_ERR_SOCK_NOAPOP			TEXT("Server non accetta 'APOP'")
#define STR_ERR_SOCK_STAT			TEXT("'STAT' non accettato\n\n")
#define STR_ERR_SOCK_RETR			TEXT("'RETR' non accettato\n\n")
#define STR_ERR_SOCK_DELE			TEXT("Eliminazione fallita\n\n")
#define STR_ERR_SOCK_NOATTACH		TEXT("Allegato non trovato")
#define STR_ERR_SOCK_BADFROM		TEXT("Errore formato indirizzo")
#define STR_ERR_SOCK_HELO 			TEXT("'HELO' fallito\n")\
									TEXT("Controllare formato indirizzo\n\n")
#define STR_ERR_SOCK_SMTPAUTH		TEXT("Login a 'SMTP server' fallito\n\n")
#define STR_ERR_SOCK_RSET			TEXT("'RSET' fallito\n\n")
#define STR_ERR_SOCK_MAILFROM		TEXT("'MAIL FROM' fallito\n")\
									TEXT("Controllare formato indirizzo\n\n")
#define STR_ERR_SOCK_NOTO			TEXT("Destinazione non settata")
#define STR_ERR_SOCK_RCPTTO			TEXT("'RCPT TO' fallita\n")\
									TEXT("Controllare indirizzo destinazione\n\n")
#define STR_ERR_SOCK_DATA			TEXT("'DATA' fallita\n\n")
#define STR_ERR_SOCK_MAILSEND		TEXT("Mail non inviata\n\n")

//Ras error
#define STR_ERR_RAS_NOSET			TEXT("Nessuna impostazione di connessione 'Dial-up'")
#define STR_ERR_RAS_CONNECT			TEXT("Connessione 'Dial-up' fallita")
#define STR_ERR_RAS_DISCONNECT		TEXT("Connessione 'Dial-up' interrotta o linea occupata")

//Question
#define STR_Q_DELETE				TEXT("Cancellarlo?")
#define STR_Q_DELSERVERMAIL			TEXT("Attenzione! Questa operazione eliminerà le mail dal server")
#define STR_Q_DELLISTMAIL			TEXT("Cancellare %d mail dalla lista?%s")
#define STR_Q_DELLISTMAIL_NOSERVER	TEXT("\n(Non sono state eliminate dal server)")
#define STR_Q_DELMAILBOX			TEXT("Eliminare account?")
#define STR_Q_DELATTACH				TEXT("Eliminare allegato?")
#define STR_Q_OVERWRITE				TEXT("\"%s\" \n\ngià ricevuta\nSovrascrivere?")
#define STR_Q_COPY					TEXT("Copia %d mail in Savebox?")
#define STR_Q_DEPENDENCE			TEXT("Un carattere dipende dal modello.  Procedere?")
#define STR_Q_UNLINKATTACH			TEXT("Chiudere il link al file allegato?")
#define STR_Q_ADDADDRESS			TEXT("Aggiungi %d indirizzo mail alla rubrica?")
#define STR_Q_NEXTFIND				TEXT("Ricerca completata!\nRicominciare dall'inizio?")
#define STR_Q_EDITCANSEL			TEXT("Cancella la modifica?")
#define STR_Q_SENDMAIL				TEXT("Invio?")

//Message
#define STR_MSG_NOMARK				TEXT("Non ci sono mail selezionate")
#define STR_MSG_NOBODY				TEXT("Corpo del messaggio non scaricato. Non può essere aperto\n\n")\
									TEXT("Seleziona e aggiorna server per scaricare il corpo")
#define STR_MSG_NONEWMAIL			TEXT("Nessuna nuova mail!")
#define STR_MSG_NOFIND				TEXT("\"%s\" non trovato")

//Window title
#define STR_TITLE_NOREADMAILBOX		TEXT("%s - [Accounts con mail non lette: %d]")
#define STR_TITLE_MAILEDIT			TEXT("Modifica mail")
#define STR_TITLE_MAILVIEW			TEXT("Visualizza mail")
#define STR_TITLE_MAILVIEW_COUNT	TEXT(" - [No.%d]")

//Message title
#define STR_TITLE_EXEC				TEXT("Aggiorna account")
#define STR_TITLE_ALLEXEC			TEXT("Aggiorna tutti gli accounts")
#define STR_TITLE_SEND				TEXT("Invia ora")
#define STR_TITLE_OPEN				TEXT("Apri")
#define STR_TITLE_SAVE				TEXT("Salva")
#define STR_TITLE_COPY				TEXT("Copia")
#define STR_TITLE_DELETE			TEXT("Elimina")
#define STR_TITLE_ERROR				TEXT("Errore")
#define STR_TITLE_SETMAILBOX		TEXT("Settaggio account")
#define STR_TITLE_OPTION			TEXT("Opzioni")
#ifndef _WIN32_WCE
#define STR_TITLE_STARTPASSWORD		TEXT("Avvio")
#define STR_TITLE_SHOWPASSWORD		TEXT("Mostra")
#endif
#define STR_TITLE_FIND				TEXT("Trova")
#define STR_TITLE_ALLFIND			TEXT("Trovati \"%s\"")

//Window status
#define STR_STATUS_VIEWINFO			TEXT("Visualizza %d")
#define STR_STATUS_MAILBOXINFO		TEXT("Visualizzate %d/ Server %d")
#define STR_STATUS_MAILINFO			TEXT("Nuovi %d, Non letti %d")

//Socket status
#define STR_STATUS_GETHOSTBYNAME	TEXT("Trova Host...")
#define STR_STATUS_CONNECT			TEXT("Connetti...")
#define STR_STATUS_RECV				TEXT("Recevuto...")
#define STR_STATUS_SENDBODY			TEXT("Corpo inviato...")
#define STR_STATUS_SOCKINFO			TEXT("%d byte %s")
#define STR_STATUS_SOCKINFO_RECV	TEXT("Ricevuto")
#define STR_STATUS_SOCKINFO_SEND	TEXT("Inviato")

//Ras status
#define STR_STATUS_RAS_START		TEXT("Avvia 'Dial-up'")
#define STR_STATUS_RAS_PORTOPEN		TEXT("Apri porta...")
#define STR_STATUS_RAS_DEVICE		TEXT("Connetti periferica...")
#define STR_STATUS_RAS_AUTH			TEXT("Verifica user...")
#define STR_STATUS_RAS_CONNECT		TEXT("'Dial-up' connesso")
#define STR_STATUS_RAS_DISCONNECT	TEXT("'Dial-up' disconnesso")

//Initialize status
#define STR_STATUS_INIT_MAILCNT		TEXT("%d")
#define STR_STATUS_INIT_MAILSIZE_B	TEXT("%s bytes")
#define STR_STATUS_INIT_MAILSIZE_KB	TEXT("%s KB")

//Mail list
#define STR_SAVEBOX_NAME			TEXT("[Posta Salvata]")
#define STR_SENDBOX_NAME			TEXT("[Posta Inviata]")
#define STR_MAILBOX_NONAME			TEXT("Senza titolo")
#define STR_LIST_LVHEAD_SUBJECT		TEXT("Oggetto")
#define STR_LIST_LVHEAD_FROM		TEXT("Da")
#define STR_LIST_LVHEAD_TO			TEXT("a")
#define STR_LIST_LVHEAD_DATE		TEXT("Data")
#define STR_LIST_LVHEAD_SIZE		TEXT("Dimensione")
#define STR_LIST_NOSUBJECT			TEXT("(nessun oggetto)")
#define STR_LIST_THREADSTR			TEXT("  + ")

#define STR_LIST_MENU_SENDINFO		TEXT("&Proprietà...")
#define STR_LIST_MENU_REPLY			TEXT("&Rispondi...")
#define STR_LIST_MENU_SENDMARK		TEXT("&Marca per invio\tCtrl+D")
#define STR_LIST_MENU_CREATECOPY	TEXT("Crea copia\tCtrl+C")
#define STR_LIST_MENU_RECVMARK		TEXT("&Marca come ricevuto\tCtrl+D")
#define STR_LIST_MENU_SAVEBOXCOPY	TEXT("Copia in Posta salvata\tCtrl+C")

//Mail view
#define STR_VIEW_HEAD_FROM			TEXT("Da: ")
#define STR_VIEW_HEAD_SUBJECT		TEXT("\r\nOggetto: ")
#define STR_VIEW_HEAD_DATE			TEXT("\r\nData: ")

#define STR_VIEW_MENU_ATTACH		TEXT("&Mostra allegati")
#define STR_VIEW_MENU_SOURCE		TEXT("&Visualizza codice")
#define STR_VIEW_MENU_DELATTACH		TEXT("&Elimina allegato")

//Mail edit
#define STR_EDIT_HEAD_MAILBOX		TEXT("Account: ")
#define STR_EDIT_HEAD_TO			TEXT("\r\nA: ")
#define STR_EDIT_HEAD_SUBJECT		TEXT("\r\nOggetto: ")

//Filter
#define STR_FILTER_USE				TEXT("Usato")
#define STR_FILTER_NOUSE			TEXT("Inusato")
#define STR_FILTER_STATUS			TEXT("Stato")
#define STR_FILTER_ACTION			TEXT("Azione")
#define STR_FILTER_ITEM1			TEXT("Item 1")
#define STR_FILTER_CONTENT1			TEXT("Contenuto 1")
#define STR_FILTER_ITEM2			TEXT("Item 2")
#define STR_FILTER_CONTENT2			TEXT("Contenuto 2")

#define STR_FILTER_UNRECV			TEXT("Non scaricato")
#define STR_FILTER_RECV				TEXT("Scaricato")
#define STR_FILTER_DOWNLOADMARK		TEXT("Selezionato per il download")
#define STR_FILTER_DELETEMARK		TEXT("Selezionato per l'eliminazione")
#define STR_FILTER_READICON			TEXT("Selezionato come letto")
#define STR_FILTER_SAVE				TEXT("Copia in [Posta salvata]")

//Cc list
#define STR_CCLIST_TYPE				TEXT("Tipo")
#define STR_CCLIST_MAILADDRESS		TEXT("Indirizzo mail")

//Set send
#define STR_SETSEND_BTN_CC			TEXT("Cc, Ccn")
#define STR_SETSEND_BTN_ATTACH		TEXT("Allegato")
#define STR_SETSEND_BTN_ETC			TEXT("Altro")

//Mail Prop
#define STR_MAILPROP_HEADER			TEXT("Intestazione")
#define STR_MAILPROP_MAILADDRESS	TEXT("Indirizzo mail")

//Address list
#define STR_ADDRESSLIST_MAILADDRESS	TEXT("Indirizzo mail")
#define STR_ADDRESSLIST_COMMENT		TEXT("Commento")

//WindowsCE
#ifdef _WIN32_WCE
#define STR_CMDBAR_RECV				TEXT("Controlla nuove mail")
#define STR_CMDBAR_ALLCHECK			TEXT("Controlla tutti gli accounts")
#define STR_CMDBAR_EXEC				TEXT("Aggiorna i selezionati")
#define STR_CMDBAR_ALLEXEC			TEXT("Aggiorna tutti gli accounts")
#define STR_CMDBAR_STOP				TEXT("Stop")
#define STR_CMDBAR_NEWMAIL			TEXT("Nuovo messaggio")
#define STR_CMDBAR_RAS_CONNECT		TEXT("'Dial-up' connesso")
#define STR_CMDBAR_RAS_DISCONNECT	TEXT("'Dial-up' disconnesso")

#define STR_CMDBAR_PREVMAIL			TEXT("Mail precedente")
#define STR_CMDBAR_NEXTMAIL			TEXT("Mail successiva")
#define STR_CMDBAR_NEXTNOREAD		TEXT("Successiva mail non letta")
#define STR_CMDBAR_REMESSEGE		TEXT("Rispondi")
#define STR_CMDBAR_ALLREMESSEGE		TEXT("Rispondi a tutti")

#define STR_CMDBAR_SEND				TEXT("Invia ora")
#define STR_CMDBAR_SENDBOX			TEXT("Salva in [Posta Inviata]")
#define STR_CMDBAR_SENDINFO			TEXT("Proprietà")

#define STR_LIST_PPCMENU_SENDINFO	TEXT("&Proprietà")
#define STR_LIST_PPCMENU_REPLY		TEXT("&Rispondi")
#define STR_LIST_PPCMENU_SENDMARK	TEXT("&Seleziona per invio")
#define STR_LIST_PPCMENU_CREATECOPY	TEXT("Crea copia")
#define STR_LIST_PPCMENU_RECVMARK	TEXT("&Seleziona per ricezione")
#define STR_LIST_PPCMENU_SAVEBOXCOPY	TEXT("Copia in &[Posta Salvata]")

#define STR_VIEW_PPCMENU_ATTACH		TEXT("&Mostra allegati")
#define STR_VIEW_PPCMENU_SOURCE		TEXT("&Visualizza codice")
#define STR_VIEW_PPCMENU_DELATTACH	TEXT("&Elimina allegato")

#ifdef _WIN32_WCE_PPC
#define STR_TITLE_SMTPAUTH			TEXT("'SMTP-AUTH'")
#define STR_TITLE_FILTER			TEXT("Setta filtro")
#define STR_TITLE_INITMAILBOX		TEXT("Inizializza")
#define STR_TITLE_CCBCC				TEXT("Cc, Ccn")
#define STR_TITLE_ATTACH			TEXT("Allegati")
#define STR_TITLE_ETCHEADER			TEXT("Altre intestazioni")
#define STR_TITLE_SENDINFO			TEXT("Proprietà")
#define STR_TITLE_ADDRESSINFO		TEXT("Informazioni indirizzo")
#define STR_TITLE_EDITADDRESS		TEXT("Indirizzo")
#define STR_TITLE_ADDRESSLIST		TEXT("Rubrica")
#define STR_TITLE_FIND				TEXT("Trova")
#endif
#endif

#endif
/* End of source */
