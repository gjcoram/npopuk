nPOP Ver 1.0.2
--

[Introduzione]

Questo software ti permette di accedere alle e-mail attraverso un server POP3 direttamente dal tuo Pocket PC or PC.
Svolge le normali funzioni di accesso remoto alle e-mail.
Le e-mail visualizzate da nPOP sono solo le mail presenti sul server. Pertanto , se una mail è cancellata dal server scomparirà dalla lista, anche se in quel momento è visualizzata nella lista.

Quando ti connetti utilizzando nPOP, le mail sono elencate e puoi vedere solo l'intestazione.
Dall'elenco tu puoi selezionare la mail che vuoi ricevere sul tuo Pocket PC or PC; questo è molto più efficiente che scaricare tutte le mail sul Pocket PC or PC.

Può essere usato per la gestione della posta sul server etc. o come metodo per controllare l'arrivo di nuova posta.


[Funzioni]

- Recevere E-Mail (POP3 e APOP)
- Inviare E-Mail (SMTP e SMTP-AUTH e POP prima di SMTP)
- L'utente può specificare le porte per POP3 e SMTP
- Supporta multi-account (è possibile una ricezione a 360°)
- Controllo automatico per nuove mail
- La posta è salva nella casella di posta.
- Visualizzazione dei 'thread' della lista delle mail
- Supporto per allegati quando invii e ricevi e-mail (RFC 2231)
- Filtro per Routing delle mail ricevute
- Semplice rubrica
- Gestione Dial-up


[Ambiente]

- Win32 - Windows95, 98, Me, NT4.0, 2000, XP

- Pocket PC 2002
    + nPOP.arm.CAB   - CPU : StrongARM, XScale

- Pocket PC
    + nPOP.mips.CAB  - CPU : MIPS
    + nPOP.sh3.CAB   - CPU : SH-3
    + nPOP.arm.CAB   - CPU : StrongARM, XScale

    Copia i CAB file nel Pocket PC utilizzando ActiveSync, e cliccaci sopra per installare nPOP.

- Windows CE 2.11 e successivi  - StrongARM, MIPS, SH-3, SH4

- Windows CE 2.0 e successivi - MIPS, SH-3


[Languaggio di modifica delle mail]

Chiudi "nPOP.exe" e modifica "nPOP.ini"

"HeadCharset=" e "BodyCharset="   - Set dei caratteri
"HeadEncoding=" e "BodyEncoding=" - Codifica (0-7bit 1-8bit 2-BASE64 3-quoted-printable)


### ISO-8859-1 (Latin1)
HeadCharset="ISO-8859-1"
HeadEncoding=3
BodyCharset="ISO-8859-1"
BodyEncoding=3

### ISO-2022-JP (Japanese)
FontName="MS Gothic"
FontCharset=128
LvFontName="MS UI Gothic"
LvFontCharset=128
HeadCharset="ISO-2022-JP"
HeadEncoding=2
BodyCharset="ISO-2022-JP"
BodyEncoding=0

### EUC-KR (Korean)
FontName="MS Gothic"
FontCharset=129
LvFontName="MS UI Gothic"
LvFontCharset=129
HeadCharset="EUC-KR"
HeadEncoding=2
BodyCharset="EUC-KR"
BodyEncoding=3

### ISO-2022-KR (Korean)
FontName="MS Gothic"
FontCharset=129
LvFontName="MS UI Gothic"
LvFontCharset=129
HeadCharset="EUC-KR"
HeadEncoding=2
BodyCharset="ISO-2022-KR"
BodyEncoding=0

### UTF-8 (UNICODE) [Windows CE: Non supportato]
HeadCharset="UTF-8"
HeadEncoding=2
BodyCharset="UTF-8"
BodyEncoding=2

--

Ringraziamenti Speciali:
  Manabu & Naomi Anzai
  Chris De Herrera
  Marlof Bregonje
  Takehiko Hanada
  Antoine
  Greg Chapman

--

Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
	http://www.nakka.com/

02/11/2005
