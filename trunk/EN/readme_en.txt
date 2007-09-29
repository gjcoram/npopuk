nPOP Ver 1.0.4
--

[Introduction]

This software allows you to access e-mail on a POP3 mail server on your Pocket PC or PC. It
provides the function of common remote e-mail access.

The mail displayed by nPOP is only the mail on the present on the server. Therefore, if mail
is deleted from your server it will disappear from a list, even if the mail is displayed in
the list.

When you connect using nPOP, the mail is listed and you can see the header only.  From that
list you can select e-mail you want to receive on your Pocket PC or PC. This is more efficient
than downloading all the e-mail on your Pocket PC or PC.

It can be use for management of the mail on a server etc. as a method to check for the arrival
of new e-mail.


[Function]

- Receive E-Mail (POP3 and APOP)
- Send Mail (SMTP and SMTP-AUTH and POP before SMTP)
- User specified ports for POP3 and SMTP
- Multi-account support (round reception is possible)
- Automatic check for new mail
- Mail is saved in a message box.
- A thread display of the mail list
- Support for attachments when sending and receiving e-mail (RFC 2231)
- Filters for Routing Received E-Mail
- Simple address book
- Dial-up management
- SSL/STARTTLS ('npopssl.dll' and OpenSSL Library)


[Environment]

- Win32 - Windows95, 98, Me, NT4.0, 2000, XP

- Pocket PC 2002
    + nPOP.arm.CAB   - CPU : StrongARM, XScale

- Pocket PC
    + nPOP.mips.CAB  - CPU : MIPS
    + nPOP.sh3.CAB   - CPU : SH-3
    + nPOP.arm.CAB   - CPU : StrongARM, XScale

    Copy CAB file to Pocket PC using ActiveSync, and click on it to install nPOP.

- Windows CE 2.11 and later - StrongARM, MIPS, SH-3, SH4

- Windows CE 2.0 and later - MIPS, SH-3


[Language of mail edit]

Edit "nPOP.ini"

"HeadCharset=" and "BodyCharset="   - Character set
"HeadEncoding=" and "BodyEncoding=" - Encoding (0-7bit 1-8bit 2-BASE64 3-quoted-printable)


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

### UTF-8 (UNICODE) [Windows CE: Unsupported]
HeadCharset="UTF-8"
HeadEncoding=2
BodyCharset="UTF-8"
BodyEncoding=2

--

Special thanks:
  Manabu & Naomi Anzai
  Chris De Herrera
  Marlof Bregonje
  Takehiko Hanada
  Antoine
  Greg Chapman

--

Copyright (C) 1996-2005 by Nakashima Tomoaki. All rights reserved.
	http://www.nakka.com/

02/20/2006
