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
#define STR_ERR_MEMALLOC			TEXT("Erreur d'allocation méíoire")
#define STR_ERR_INIT				TEXT("Erreur d'initialisation")
#define STR_ERR_OPEN				TEXT("Erreur durant l'ouverture du fichier")
#define STR_ERR_SAVEEND				TEXT("Erreur de préóervation\nLa procéäure d'arrêô continue-t-elle?")
#define STR_ERR_SAVE				TEXT("Erreur de préóervation")
#define STR_ERR_ADD					TEXT("Erreur dans l'ajout")
#define STR_ERR_VIEW				TEXT("Erreur d'affichage")
#define STR_ERR_SELECTMAILBOX		TEXT("Le compte n'est pas spéãifié¢)
#define STR_ERR_SELECTMAILADDR		TEXT("L'adresse email n'a pas été sélectionnée")
#define STR_ERR_SETMAILADDR			TEXT("L'adresse email n'est pas déæinie")
#define STR_ERR_INPUTMAILADDR		TEXT("L'adresse email n'a pas été saisie")
#define STR_ERR_CREATECOPY			TEXT("Erreur durant la copie")
#define STR_ERR_SAVECOPY			TEXT("Erreur durant la copie du dossier de sauvegarde")
#define STR_ERR_NOITEM1				TEXT("Item1 n'est pas déæini")
#define STR_ERR_INPUTFINDSTRING		TEXT("Le caractère de récupération n'a pas été saisi")
#define STR_ERR_NOMAIL				TEXT("Mail n'a pas été trouvé dans la liste")
#define STR_ERR_SENDLOCK			TEXT("Impossible de transmettre")

//Socket error
#define STR_ERR_SOCK_SELECT			TEXT("Erreur de séìection")
#define STR_ERR_SOCK_DISCONNECT		TEXT("Déconnecté du serveur")
#define STR_ERR_SOCK_CONNECT		TEXT("Impossible de se connecter au serveur")
#define STR_ERR_SOCK_SENDRECV		TEXT("L'erreur s'est produite durant l'éíission et la réãeption")
#define STR_ERR_SOCK_EVENT			TEXT("Erreur de déæinition d'éöéîement")
#define STR_ERR_SOCK_NOSERVER		TEXT("Le nom du serveur n'est pas déæini")
#define STR_ERR_SOCK_GETIPADDR		TEXT("Erreur durant l'acquisition de l'adresse IP")
#define STR_ERR_SOCK_CREATESOCKET	TEXT("Erreur d'ouverture de socket")
#define STR_ERR_SOCK_TIMEOUT		TEXT("Le connexion a dépassé le délai")
#define STR_ERR_SOCK_SEND			TEXT("Erreur durant l'éíission")
#define STR_ERR_SOCK_RESPONSE		TEXT("La réðonse n'a pas pu êôre analyséå\n\n")
#define STR_ERR_SOCK_GETITEMINFO	TEXT("Les informations n'ont pas pu êôre réãupéòéås pour cet éìéíent")
#define STR_ERR_SOCK_MAILSYNC 		TEXT("Impossible de réãupéòer le nombre de messages durant la synchronisation\n\n")\
									TEXT("Nécessité de vérifier les messages récent et de synchroniser le nombre de messages")
#define STR_ERR_SOCK_NOMESSAGEID	TEXT("Impossible de réãupéòer l'identifiant du message")
#define STR_ERR_SOCK_NOUSERID		TEXT("Le nom d'utilisateur n'est pas déæini")
#define STR_ERR_SOCK_NOPASSWORD		TEXT("Le mot de passe n'est pas déæini")
#define STR_ERR_SOCK_BADPASSWORD	TEXT("Le nom d'utilisateur ou le mot de passe n'est pas valide\n\n")
#define STR_ERR_SOCK_ACCOUNT		TEXT("Le compte n'a pas été accepté\n\n")
#define STR_ERR_SOCK_NOAPOP			TEXT("Ce serveur ne correspond pas à un serveur POP")
#define STR_ERR_SOCK_STAT			TEXT("STAT n'a pas été accepté\n\n")
#define STR_ERR_SOCK_RETR			TEXT("RETR n'a pas été accepté\n\n")
#define STR_ERR_SOCK_DELE			TEXT("Erreur durant la suppression\n\n")
#define STR_ERR_SOCK_NOATTACH		TEXT("Le fichier attaché n'a pas pu être trouvé")
#define STR_ERR_SOCK_BADFROM		TEXT("La transmission de l'information de l'éíetteur n'est pas déæinie correctement")
#define STR_ERR_SOCK_HELO 			TEXT("Erreur durant HELO\n")\
									TEXT("Merci de véòifier que l'email de l'éíetteur est correctement déæini\n\n")
#define STR_ERR_SOCK_SMTPAUTH		TEXT("Erreur durant l'identification sur le serveur SMTP\n\n")
#define STR_ERR_SOCK_RSET			TEXT("Erreur durant RSET\n\n")
#define STR_ERR_SOCK_MAILFROM		TEXT("Failed in MAIL FROM\n")\
									TEXT("Merci de véòifier que l'email de l'éíetteur est correctement déæini\n\n")
#define STR_ERR_SOCK_NOTO			TEXT("Le destinataire n'est pas déæini")
#define STR_ERR_SOCK_RCPTTO			TEXT("Erreur durant RCPT TO\n")\
									TEXT("Merci de véòifier que l'email du destinataire est correctement déæini\n\n")
#define STR_ERR_SOCK_DATA			TEXT("Erreur dans DATA\n\n")
#define STR_ERR_SOCK_MAILSEND		TEXT("Erreur durant l'envoi des messages\n\n")

//Ras error
#define STR_ERR_RAS_NOSET			TEXT("Aucune connexion internet n'est déæinie")
#define STR_ERR_RAS_CONNECT			TEXT("Impossible d'éôablir une connexion")
#define STR_ERR_RAS_DISCONNECT		TEXT("La connexion a été interrompue ou a échoué")

//Question
#define STR_Q_DELETE				TEXT("Voulez-vous supprimer ?")
#define STR_Q_DELSERVERMAIL			TEXT("Voulez-vous vraiment supprimer les messages sur le serveur ?")
#define STR_Q_DELLISTMAIL			TEXT("Supprimer  %d messages de la liste ?%s")
#define STR_Q_DELLISTMAIL_NOSERVER	TEXT("\n(N'est pas supprimé sur le serveur)")
#define STR_Q_DELMAILBOX			TEXT("Supprimer compte?")
#define STR_Q_DELATTACH				TEXT("Supprimer pièães jointes ?")
#define STR_Q_OVERWRITE				TEXT("\"%s\" \n\na déjà reçu un message\nVoulez-vous l'écraser ?")
#define STR_Q_COPY					TEXT("Copier %d messages dans l'Archive?")
#define STR_Q_DEPENDENCE			TEXT("Cela vous convient-il bien qu'un caractèòe déðende du modèìe ?")
#define STR_Q_UNLINKATTACH			TEXT("Rompre la relation avec le fichier attaché ?")
#define STR_Q_ADDADDRESS			TEXT("Ajouter l'adresse email %d au carnet d'adresses ?")
#define STR_Q_NEXTFIND				TEXT("dernière occurence trouvée\nVoulez-vous recommencer à partir du début ?")
#define STR_Q_EDITCANSEL			TEXT("Annuler l'éäition ?")
#define STR_Q_SENDMAIL				TEXT("Voulez-vous envoyer le message ?")

//Message
#define STR_MSG_NOMARK				TEXT("Aucun message n'est marqué¢)
#define STR_MSG_NOBODY				TEXT("Impossible d'ouvrir\n\n")\
									TEXT("'Marqués pour la réception' est attaché, 'Mettre à jour les messages marqués' is performed and the text is acquired.")
#define STR_MSG_NONEWMAIL			TEXT("Aucun nouveau message")
#define STR_MSG_NOFIND				TEXT("\"%s\" n'a pas été trouvé")

//Window title
#define STR_TITLE_NOREADMAILBOX		TEXT("%s - [Compte non lu: %d]")
#define STR_TITLE_MAILEDIT			TEXT("Edition de message")
#define STR_TITLE_MAILVIEW			TEXT("Consultation de message")
#define STR_TITLE_MAILVIEW_COUNT	TEXT(" - [No.%d]")

//Message title
#define STR_TITLE_EXEC				TEXT("Mettre à jour les comptes marqués")
#define STR_TITLE_ALLEXEC			TEXT("Mettre à jour tous les comptes")
#define STR_TITLE_SEND				TEXT("Envoyer maintenant")
#define STR_TITLE_OPEN				TEXT("Ouvrir")
#define STR_TITLE_SAVE				TEXT("Enregistrer")
#define STR_TITLE_COPY				TEXT("Copier")
#define STR_TITLE_DELETE			TEXT("Supprimer")
#define STR_TITLE_ERROR				TEXT("Erreur")
#define STR_TITLE_SETMAILBOX		TEXT("Déæinir compte")
#define STR_TITLE_OPTION			TEXT("Option")
#ifndef _WIN32_WCE
#define STR_TITLE_STARTPASSWORD		TEXT("Déíarrage")
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
#define STR_STATUS_RECV				TEXT("réãeption...")
#define STR_STATUS_SENDBODY			TEXT("envoi du corps du message...")
#define STR_STATUS_SOCKINFO			TEXT("%d octet %s")
#define STR_STATUS_SOCKINFO_RECV	TEXT("recv")
#define STR_STATUS_SOCKINFO_SEND	TEXT("send")

//Ras status
#define STR_STATUS_RAS_START		TEXT("Numéòotation")
#define STR_STATUS_RAS_PORTOPEN		TEXT("Ouverture de port...")
#define STR_STATUS_RAS_DEVICE		TEXT("Connnexion en cours...")
#define STR_STATUS_RAS_AUTH			TEXT("Identification...")
#define STR_STATUS_RAS_CONNECT		TEXT("La connexion a été établie")
#define STR_STATUS_RAS_DISCONNECT	TEXT("La connexion a été rompue")

//Initialize status
#define STR_STATUS_INIT_MAILCNT		TEXT("%d")
#define STR_STATUS_INIT_MAILSIZE_B	TEXT("%s octets")
#define STR_STATUS_INIT_MAILSIZE_KB	TEXT("%s ko")

//Mail list
#define STR_SAVEBOX_NAME			TEXT("[Archive]")
#define STR_SENDBOX_NAME			TEXT("[Messages envoyéó]")
#define STR_MAILBOX_NONAME			TEXT("Sans titre")
#define STR_LIST_LVHEAD_SUBJECT		TEXT("Subjet")
#define STR_LIST_LVHEAD_FROM		TEXT("De")
#define STR_LIST_LVHEAD_TO			TEXT("A")
#define STR_LIST_LVHEAD_DATE		TEXT("Date")
#define STR_LIST_LVHEAD_SIZE		TEXT("Taille")
#define STR_LIST_NOSUBJECT			TEXT("(Aucun sujet)")
#define STR_LIST_THREADSTR			TEXT("  + ")

#define STR_LIST_MENU_SENDINFO		TEXT("&Propriéôéó...")
#define STR_LIST_MENU_REPLY			TEXT("&Réðondre...")
#define STR_LIST_MENU_SENDMARK		TEXT("&Marquer pour envoyer\tCtrl+D")
#define STR_LIST_MENU_CREATECOPY	TEXT("Copi&er\tCtrl+C")
#define STR_LIST_MENU_RECVMARK		TEXT("&Marquer pour recevoir\tCtrl+D")
#define STR_LIST_MENU_SAVEBOXCOPY	TEXT("Copier dans l'&Archive\tCtrl+C")

//Mail view
#define STR_VIEW_HEAD_FROM			TEXT("De: ")
#define STR_VIEW_HEAD_SUBJECT		TEXT("\r\nSujet: ")
#define STR_VIEW_HEAD_DATE			TEXT("\r\nDate: ")

#define STR_VIEW_MENU_ATTACH		TEXT("&Afficher pièãe jointe")
#define STR_VIEW_MENU_SOURCE		TEXT("&Afficher source")
#define STR_VIEW_MENU_DELATTACH		TEXT("&Supprimer pièãe jointe")

//Mail edit
#define STR_EDIT_HEAD_MAILBOX		TEXT("Compte: ")
#define STR_EDIT_HEAD_TO			TEXT("\r\nA: ")
#define STR_EDIT_HEAD_SUBJECT		TEXT("\r\nSujet: ")

//Filter
#define STR_FILTER_USE				TEXT("Utilisé¢)
#define STR_FILTER_NOUSE			TEXT("Non utilisé¢)
#define STR_FILTER_STATUS			TEXT("Etat")
#define STR_FILTER_ACTION			TEXT("Action")
#define STR_FILTER_ITEM1			TEXT("Eléíent1")
#define STR_FILTER_CONTENT1			TEXT("Contenu1")
#define STR_FILTER_ITEM2			TEXT("Eléíent2")
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
#define STR_MAILPROP_HEADER			TEXT("En-têôe")
#define STR_MAILPROP_MAILADDRESS	TEXT("Adresse Email")

//Address list
#define STR_ADDRESSLIST_MAILADDRESS	TEXT("Adresse Email")
#define STR_ADDRESSLIST_COMMENT		TEXT("Commentaire")

//WindowsCE
#ifdef _WIN32_WCE
#define STR_CMDBAR_RECV				TEXT("Véòifier l'existence de nouveaux messages")
#define STR_CMDBAR_ALLCHECK			TEXT("Véòifier tous les comptes")
#define STR_CMDBAR_EXEC				TEXT("Mettre à jour les comptes marqués")
#define STR_CMDBAR_ALLEXEC			TEXT("Mettre à jour tous les comptes")
#define STR_CMDBAR_STOP				TEXT("Stop")
#define STR_CMDBAR_NEWMAIL			TEXT("Nouveau message")
#define STR_CMDBAR_RAS_CONNECT		TEXT("Se connecter")
#define STR_CMDBAR_RAS_DISCONNECT	TEXT("Se déãonnecter")

#define STR_CMDBAR_PREVMAIL			TEXT("Message préãéäent")
#define STR_CMDBAR_NEXTMAIL			TEXT("Message suivant")
#define STR_CMDBAR_NEXTNOREAD		TEXT("Message suivant non lul")
#define STR_CMDBAR_REMESSEGE		TEXT("Réðondre")
#define STR_CMDBAR_ALLREMESSEGE		TEXT("Répondre à tous")

#define STR_CMDBAR_SEND				TEXT("Envoyer maintenant")
#define STR_CMDBAR_SENDBOX			TEXT("Copier dans l'Archive")
#define STR_CMDBAR_SENDINFO			TEXT("Propriéôéó")

#define STR_LIST_PPCMENU_SENDINFO	TEXT("&Propriéôéó")
#define STR_LIST_PPCMENU_REPLY		TEXT("&Réðondre")
#define STR_LIST_PPCMENU_SENDMARK	TEXT("&Marquer pour envoyer")
#define STR_LIST_PPCMENU_CREATECOPY	TEXT("Créår une cop&ie")
#define STR_LIST_PPCMENU_RECVMARK	TEXT("&Marquer pour recevoir")
#define STR_LIST_PPCMENU_SAVEBOXCOPY	TEXT("Copier dans l'&Archive")

#define STR_VIEW_PPCMENU_ATTACH		TEXT("&Afficher pièãe jointe")
#define STR_VIEW_PPCMENU_SOURCE		TEXT("A&fficher source")
#define STR_VIEW_PPCMENU_DELATTACH	TEXT("&Supprimer pièãe jointe")

#ifdef _WIN32_WCE_PPC
#define STR_TITLE_SMTPAUTH			TEXT("SMTP-AUTH")
#define STR_TITLE_FILTER			TEXT("Déæinir filtres")
#define STR_TITLE_INITMAILBOX		TEXT("Initialiser")
#define STR_TITLE_CCBCC				TEXT("Cc, Bcc")
#define STR_TITLE_ATTACH			TEXT("Attacher pièães jointes")
#define STR_TITLE_ETCHEADER			TEXT("Autres en-têôes")
#define STR_TITLE_SENDINFO			TEXT("Propriéôéó")
#define STR_TITLE_ADDRESSINFO		TEXT("Addresse - Information")
#define STR_TITLE_EDITADDRESS		TEXT("Addresse")
#define STR_TITLE_ADDRESSLIST		TEXT("Carnet d'adresses")
#define STR_TITLE_FIND				TEXT("Chercher")
#endif
#endif

#endif
/* End of source */
