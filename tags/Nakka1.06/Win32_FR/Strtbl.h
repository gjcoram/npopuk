/**************************************************************************

	nPOP

	Strtbl.h

	Copyright (C) 1996-2002 by Tomoaki Nakashima. All rights reserved.
		http://www.nakka.com/
		nakka@nakka.com

**************************************************************************/

#ifndef _INC_STR_TBL_H
#define _INC_STR_TBL_H

#pragma setlocale("french")

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
#define STR_ERR_MEMALLOC			TEXT("Erreur d'allocation m��oire")
#define STR_ERR_INIT				TEXT("Erreur d'initialisation")
#define STR_ERR_OPEN				TEXT("Erreur durant l'ouverture du fichier")
#define STR_ERR_SAVEEND				TEXT("Erreur de pr��ervation\nLa proc��ure d'arr�� continue-t-elle?")
#define STR_ERR_SAVE				TEXT("Erreur de pr��ervation")
#define STR_ERR_ADD					TEXT("Erreur dans l'ajout")
#define STR_ERR_VIEW				TEXT("Erreur d'affichage")
#define STR_ERR_SELECTMAILBOX		TEXT("Le compte n'est pas sp��ifi�)
#define STR_ERR_SELECTMAILADDR		TEXT("L'adresse email n'a pas �t� s�lectionn�e")
#define STR_ERR_SETMAILADDR			TEXT("L'adresse email n'est pas d��inie")
#define STR_ERR_INPUTMAILADDR		TEXT("L'adresse email n'a pas �t� saisie")
#define STR_ERR_CREATECOPY			TEXT("Erreur durant la copie")
#define STR_ERR_SAVECOPY			TEXT("Erreur durant la copie du dossier de sauvegarde")
#define STR_ERR_NOITEM1				TEXT("Item1 n'est pas d��ini")
#define STR_ERR_INPUTFINDSTRING		TEXT("Le caract�re de r�cup�ration n'a pas �t� saisi")
#define STR_ERR_NOMAIL				TEXT("Mail n'a pas �t� trouv� dans la liste")
#define STR_ERR_SENDLOCK			TEXT("Impossible de transmettre")

//Socket error
#define STR_ERR_SOCK_SELECT			TEXT("Erreur de s��ection")
#define STR_ERR_SOCK_DISCONNECT		TEXT("D�connect� du serveur")
#define STR_ERR_SOCK_CONNECT		TEXT("Impossible de se connecter au serveur")
#define STR_ERR_SOCK_SENDRECV		TEXT("L'erreur s'est produite durant l'��ission et la r��eption")
#define STR_ERR_SOCK_EVENT			TEXT("Erreur de d��inition d'����ement")
#define STR_ERR_SOCK_NOSERVER		TEXT("Le nom du serveur n'est pas d��ini")
#define STR_ERR_SOCK_GETIPADDR		TEXT("Erreur durant l'acquisition de l'adresse IP")
#define STR_ERR_SOCK_CREATESOCKET	TEXT("Erreur d'ouverture de socket")
#define STR_ERR_SOCK_TIMEOUT		TEXT("Le connexion a d�pass� le d�lai")
#define STR_ERR_SOCK_SEND			TEXT("Erreur durant l'��ission")
#define STR_ERR_SOCK_RESPONSE		TEXT("La r��onse n'a pas pu ��re analys��\n\n")
#define STR_ERR_SOCK_GETITEMINFO	TEXT("Les informations n'ont pas pu ��re r��up����s pour cet ����ent")
#define STR_ERR_SOCK_MAILSYNC 		TEXT("Impossible de r��up��er le nombre de messages durant la synchronisation\n\n")\
									TEXT("N�cessit� de v�rifier les messages r�cent et de synchroniser le nombre de messages")
#define STR_ERR_SOCK_NOMESSAGEID	TEXT("Impossible de r��up��er l'identifiant du message")
#define STR_ERR_SOCK_NOUSERID		TEXT("Le nom d'utilisateur n'est pas d��ini")
#define STR_ERR_SOCK_NOPASSWORD		TEXT("Le mot de passe n'est pas d��ini")
#define STR_ERR_SOCK_BADPASSWORD	TEXT("Le nom d'utilisateur ou le mot de passe n'est pas valide\n\n")
#define STR_ERR_SOCK_ACCOUNT		TEXT("Le compte n'a pas �t� accept�\n\n")
#define STR_ERR_SOCK_NOAPOP			TEXT("Ce serveur ne correspond pas � un serveur POP")
#define STR_ERR_SOCK_STAT			TEXT("STAT n'a pas �t� accept�\n\n")
#define STR_ERR_SOCK_RETR			TEXT("RETR n'a pas �t� accept�\n\n")
#define STR_ERR_SOCK_DELE			TEXT("Erreur durant la suppression\n\n")
#define STR_ERR_SOCK_NOATTACH		TEXT("Le fichier attach� n'a pas pu �tre trouv�")
#define STR_ERR_SOCK_BADFROM		TEXT("La transmission de l'information de l'��etteur n'est pas d��inie correctement")
#define STR_ERR_SOCK_HELO 			TEXT("Erreur durant HELO\n")\
									TEXT("Merci de v��ifier que l'email de l'��etteur est correctement d��ini\n\n")
#define STR_ERR_SOCK_SMTPAUTH		TEXT("Erreur durant l'identification sur le serveur SMTP\n\n")
#define STR_ERR_SOCK_RSET			TEXT("Erreur durant RSET\n\n")
#define STR_ERR_SOCK_MAILFROM		TEXT("Failed in MAIL FROM\n")\
									TEXT("Merci de v��ifier que l'email de l'��etteur est correctement d��ini\n\n")
#define STR_ERR_SOCK_NOTO			TEXT("Le destinataire n'est pas d��ini")
#define STR_ERR_SOCK_RCPTTO			TEXT("Erreur durant RCPT TO\n")\
									TEXT("Merci de v��ifier que l'email du destinataire est correctement d��ini\n\n")
#define STR_ERR_SOCK_DATA			TEXT("Erreur dans DATA\n\n")
#define STR_ERR_SOCK_MAILSEND		TEXT("Erreur durant l'envoi des messages\n\n")

//Ras error
#define STR_ERR_RAS_NOSET			TEXT("Aucune connexion internet n'est d��inie")
#define STR_ERR_RAS_CONNECT			TEXT("Impossible d'��ablir une connexion")
#define STR_ERR_RAS_DISCONNECT		TEXT("La connexion a �t� interrompue ou a �chou�")

//Question
#define STR_Q_DELETE				TEXT("Voulez-vous supprimer ?")
#define STR_Q_DELSERVERMAIL			TEXT("Voulez-vous vraiment supprimer les messages sur le serveur ?")
#define STR_Q_DELLISTMAIL			TEXT("Supprimer  %d messages de la liste ?%s")
#define STR_Q_DELLISTMAIL_NOSERVER	TEXT("\n(N'est pas supprim� sur le serveur)")
#define STR_Q_DELMAILBOX			TEXT("Supprimer compte?")
#define STR_Q_DELATTACH				TEXT("Supprimer pi��es jointes ?")
#define STR_Q_OVERWRITE				TEXT("\"%s\" \n\na d�j� re�u un message\nVoulez-vous l'�craser ?")
#define STR_Q_COPY					TEXT("Copier %d messages dans l'Archive?")
#define STR_Q_DEPENDENCE			TEXT("Cela vous convient-il bien qu'un caract��e d��ende du mod��e ?")
#define STR_Q_UNLINKATTACH			TEXT("Rompre la relation avec le fichier attach� ?")
#define STR_Q_ADDADDRESS			TEXT("Ajouter l'adresse email %d au carnet d'adresses ?")
#define STR_Q_NEXTFIND				TEXT("derni�re occurence trouv�e\nVoulez-vous recommencer � partir du d�but ?")
#define STR_Q_EDITCANSEL			TEXT("Annuler l'��ition ?")
#define STR_Q_SENDMAIL				TEXT("Voulez-vous envoyer le message ?")

//Message
#define STR_MSG_NOMARK				TEXT("Aucun message n'est marqu�)
#define STR_MSG_NOBODY				TEXT("Impossible d'ouvrir\n\n")\
									TEXT("'Marqu�s pour la r�ception' est attach�, 'Mettre � jour les messages marqu�s' is performed and the text is acquired.")
#define STR_MSG_NONEWMAIL			TEXT("Aucun nouveau message")
#define STR_MSG_NOFIND				TEXT("\"%s\" n'a pas �t� trouv�")

//Window title
#define STR_TITLE_NOREADMAILBOX		TEXT("%s - [Compte non lu: %d]")
#define STR_TITLE_MAILEDIT			TEXT("Edition de message")
#define STR_TITLE_MAILVIEW			TEXT("Consultation de message")
#define STR_TITLE_MAILVIEW_COUNT	TEXT(" - [No.%d]")

//Message title
#define STR_TITLE_EXEC				TEXT("Mettre � jour les comptes marqu�s")
#define STR_TITLE_ALLEXEC			TEXT("Mettre � jour tous les comptes")
#define STR_TITLE_SEND				TEXT("Envoyer maintenant")
#define STR_TITLE_OPEN				TEXT("Ouvrir")
#define STR_TITLE_SAVE				TEXT("Enregistrer")
#define STR_TITLE_COPY				TEXT("Copier")
#define STR_TITLE_DELETE			TEXT("Supprimer")
#define STR_TITLE_ERROR				TEXT("Erreur")
#define STR_TITLE_SETMAILBOX		TEXT("D��inir compte")
#define STR_TITLE_OPTION			TEXT("Option")
#ifndef _WIN32_WCE
#define STR_TITLE_STARTPASSWORD		TEXT("D��arrage")
#define STR_TITLE_SHOWPASSWORD		TEXT("Afficher")
#endif
#define STR_TITLE_FIND				TEXT("Chercher")
#define STR_TITLE_ALLFIND			TEXT("Chercher tout \"%s\"")

//Window status
#define STR_STATUS_VIEWINFO			TEXT("Voir %d")
#define STR_STATUS_MAILBOXINFO		TEXT("Voir %d/ Serveur %d")
#define STR_STATUS_MAILINFO			TEXT("Nouveau(x) %d, Non lu(s) %d")

//Socket status
#define STR_STATUS_GETHOSTBYNAME	TEXT("gethostbyname...")
#define STR_STATUS_CONNECT			TEXT("connexion...")
#define STR_STATUS_RECV				TEXT("r��eption...")
#define STR_STATUS_SENDBODY			TEXT("envoi du corps du message...")
#define STR_STATUS_SOCKINFO			TEXT("%d octet %s")
#define STR_STATUS_SOCKINFO_RECV	TEXT("recv")
#define STR_STATUS_SOCKINFO_SEND	TEXT("send")

//Ras status
#define STR_STATUS_RAS_START		TEXT("Num��otation")
#define STR_STATUS_RAS_PORTOPEN		TEXT("Ouverture de port...")
#define STR_STATUS_RAS_DEVICE		TEXT("Connnexion en cours...")
#define STR_STATUS_RAS_AUTH			TEXT("Identification...")
#define STR_STATUS_RAS_CONNECT		TEXT("La connexion a �t� �tablie")
#define STR_STATUS_RAS_DISCONNECT	TEXT("La connexion a �t� rompue")

//Initialize status
#define STR_STATUS_INIT_MAILCNT		TEXT("%d")
#define STR_STATUS_INIT_MAILSIZE_B	TEXT("%s octets")
#define STR_STATUS_INIT_MAILSIZE_KB	TEXT("%s ko")

//Mail list
#define STR_SAVEBOX_NAME			TEXT("[Archive]")
#define STR_SENDBOX_NAME			TEXT("[Messages envoy��]")
#define STR_MAILBOX_NONAME			TEXT("Sans titre")
#define STR_LIST_LVHEAD_SUBJECT		TEXT("Subjet")
#define STR_LIST_LVHEAD_FROM		TEXT("De")
#define STR_LIST_LVHEAD_TO			TEXT("A")
#define STR_LIST_LVHEAD_DATE		TEXT("Date")
#define STR_LIST_LVHEAD_SIZE		TEXT("Taille")
#define STR_LIST_NOSUBJECT			TEXT("(Aucun sujet)")
#define STR_LIST_THREADSTR			TEXT("  + ")

#define STR_LIST_MENU_SENDINFO		TEXT("&Propri����...")
#define STR_LIST_MENU_REPLY			TEXT("&R��ondre...")
#define STR_LIST_MENU_SENDMARK		TEXT("&Marquer pour envoyer\tCtrl+D")
#define STR_LIST_MENU_CREATECOPY	TEXT("Copi&er\tCtrl+C")
#define STR_LIST_MENU_RECVMARK		TEXT("&Marquer pour recevoir\tCtrl+D")
#define STR_LIST_MENU_SAVEBOXCOPY	TEXT("Copier dans l'&Archive\tCtrl+C")

//Mail view
#define STR_VIEW_HEAD_FROM			TEXT("De: ")
#define STR_VIEW_HEAD_SUBJECT		TEXT("\r\nSujet: ")
#define STR_VIEW_HEAD_DATE			TEXT("\r\nDate: ")

#define STR_VIEW_MENU_ATTACH		TEXT("&Afficher pi��e jointe")
#define STR_VIEW_MENU_SOURCE		TEXT("&Afficher source")
#define STR_VIEW_MENU_DELATTACH		TEXT("&Supprimer pi��e jointe")

//Mail edit
#define STR_EDIT_HEAD_MAILBOX		TEXT("Compte: ")
#define STR_EDIT_HEAD_TO			TEXT("\r\nA: ")
#define STR_EDIT_HEAD_SUBJECT		TEXT("\r\nSujet: ")

//Filter
#define STR_FILTER_USE				TEXT("Utilis�)
#define STR_FILTER_NOUSE			TEXT("Non utilis�)
#define STR_FILTER_STATUS			TEXT("Etat")
#define STR_FILTER_ACTION			TEXT("Action")
#define STR_FILTER_ITEM1			TEXT("El��ent1")
#define STR_FILTER_CONTENT1			TEXT("Contenu1")
#define STR_FILTER_ITEM2			TEXT("El��ent2")
#define STR_FILTER_CONTENT2			TEXT("Contenu2")

#define STR_FILTER_UNRECV			TEXT("Ne pas recevoir")
#define STR_FILTER_RECV				TEXT("Recevoir")
#define STR_FILTER_DOWNLOADMARK		TEXT("Marquer pour recevoir")
#define STR_FILTER_DELETEMARK		TEXT("Marquer pour supprimer")
#define STR_FILTER_READICON			TEXT("Marquer comme lu")
#define STR_FILTER_SAVE				TEXT("Copier dans l'Archive")

//Cc list
#define STR_CCLIST_TYPE				TEXT("Type")
#define STR_CCLIST_MAILADDRESS		TEXT("Adresse Email")

//Set send
#define STR_SETSEND_BTN_CC			TEXT("Cc, Bcc")
#define STR_SETSEND_BTN_ATTACH		TEXT("Attacher")
#define STR_SETSEND_BTN_ETC			TEXT("Autres")

//Mail Prop
#define STR_MAILPROP_HEADER			TEXT("En-t��e")
#define STR_MAILPROP_MAILADDRESS	TEXT("Adresse Email")

//Address list
#define STR_ADDRESSLIST_MAILADDRESS	TEXT("Adresse Email")
#define STR_ADDRESSLIST_COMMENT		TEXT("Commentaire")

//WindowsCE
#ifdef _WIN32_WCE
#define STR_CMDBAR_RECV				TEXT("V��ifier l'existence de nouveaux messages")
#define STR_CMDBAR_ALLCHECK			TEXT("V��ifier tous les comptes")
#define STR_CMDBAR_EXEC				TEXT("Mettre � jour les comptes marqu�s")
#define STR_CMDBAR_ALLEXEC			TEXT("Mettre � jour tous les comptes")
#define STR_CMDBAR_STOP				TEXT("Stop")
#define STR_CMDBAR_NEWMAIL			TEXT("Nouveau message")
#define STR_CMDBAR_RAS_CONNECT		TEXT("Se connecter")
#define STR_CMDBAR_RAS_DISCONNECT	TEXT("Se d��onnecter")

#define STR_CMDBAR_PREVMAIL			TEXT("Message pr����ent")
#define STR_CMDBAR_NEXTMAIL			TEXT("Message suivant")
#define STR_CMDBAR_NEXTNOREAD		TEXT("Message suivant non lul")
#define STR_CMDBAR_REMESSEGE		TEXT("R��ondre")
#define STR_CMDBAR_ALLREMESSEGE		TEXT("R�pondre � tous")

#define STR_CMDBAR_SEND				TEXT("Envoyer maintenant")
#define STR_CMDBAR_SENDBOX			TEXT("Copier dans l'Archive")
#define STR_CMDBAR_SENDINFO			TEXT("Propri����")

#define STR_LIST_PPCMENU_SENDINFO	TEXT("&Propri����")
#define STR_LIST_PPCMENU_REPLY		TEXT("&R��ondre")
#define STR_LIST_PPCMENU_SENDMARK	TEXT("&Marquer pour envoyer")
#define STR_LIST_PPCMENU_CREATECOPY	TEXT("Cr��r une cop&ie")
#define STR_LIST_PPCMENU_RECVMARK	TEXT("&Marquer pour recevoir")
#define STR_LIST_PPCMENU_SAVEBOXCOPY	TEXT("Copier dans l'&Archive")

#define STR_VIEW_PPCMENU_ATTACH		TEXT("&Afficher pi��e jointe")
#define STR_VIEW_PPCMENU_SOURCE		TEXT("A&fficher source")
#define STR_VIEW_PPCMENU_DELATTACH	TEXT("&Supprimer pi��e jointe")

#ifdef _WIN32_WCE_PPC
#define STR_TITLE_SMTPAUTH			TEXT("SMTP-AUTH")
#define STR_TITLE_FILTER			TEXT("D��inir filtres")
#define STR_TITLE_INITMAILBOX		TEXT("Initialiser")
#define STR_TITLE_CCBCC				TEXT("Cc, Bcc")
#define STR_TITLE_ATTACH			TEXT("Attacher pi��es jointes")
#define STR_TITLE_ETCHEADER			TEXT("Autres en-t��es")
#define STR_TITLE_SENDINFO			TEXT("Propri����")
#define STR_TITLE_ADDRESSINFO		TEXT("Addresse - Information")
#define STR_TITLE_EDITADDRESS		TEXT("Addresse")
#define STR_TITLE_ADDRESSLIST		TEXT("Carnet d'adresses")
#define STR_TITLE_FIND				TEXT("Chercher")
#endif
#endif

#endif
/* End of source */
