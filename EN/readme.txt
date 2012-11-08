nPOPuk v3.00                                     readme.txt
                                                 1 Nov 2012
-----------------------------------------------------------


Contents
--------
1. Introduction
2. Installing nPOPuk
3. Upgrading from an earlier version of nPOP
4. What's New


-----------------------------------------------------------
1. Introduction

Thank you for downloading this release of nPOPuk.  Versions
for each of the supported platforms, significanly enhanced
from the source code of nPOP originally written by Tomoaki
Nakashima, have been created by English speaking users of
the program.

The latest public release of the nPOPuk, both source and 
executable code, are available at
http://npopuk.org.uk/3.00/downloads.html
Pre-release versions of future versions are available via 
the forum at:
http://npopuk.org.uk/3.00/forum.html


Thanks must go to Geoffrey Coram, Werner Furlan, Paul 
Holmes-Higgin, Bruce Jackson, Glenn Linderman, Amy 
Millenson, Matthew R. Pattman, Gerard Samija and all other 
members of the development/test team.

Greg Chapman
http://npopuk.org.uk


-----------------------------------------------------------
2. Installing nPOPuk

nPOPuk needs no special installation procedure.  Simply
copy the nPOPuk.exe file to a suitable location and run the
program.

NOTES:
------

*  If you need help configuring nPOPuk see:
   http://npopuk.org.uk/3.00/quickstart.html

*  A full installation guide is at:
   http://npopuk.org.uk/3.00/install.html

*  If upgrading from an earlier version of nPOP, either
   from Tomoaki's site or one of the a "UK Fixes", then pay
   attention to the important notes in Section 3 of this
   file.


-----------------------------------------------------------
3. Upgrading from an earlier version of nPOP

When first starting nPOPuk, the program looks for its
settings in a file nPOPuk.ini in the same directory as the
executable.  If no such file is found, nPOPuk will then
look for nPOP.ini, created by a Tomoaki Nakashima version
of nPOP or an earlier "nPOP-uk" version.  If the file is
found, you will be prompted whether to import those 
settings or not.

If you choose to import them, then SaveBox.dat will be
copied into a new "savebox-type" mailbox.  nPOPuk does not
use SaveBox.dat after this point, and you may delete the
file if you do not intend to return to a previous version
of the program.  Note that nPOPuk and nPOP both use the
MailBox?.dat files to store incoming messages and
SendBox.dat to store outgoing.  If you create a new
account/mailbox in either program, the other program will
be unaware, and data may be lost.

One of the features of nPOPuk is that it corrects the
spelling of four entries that appear in the nPOP.ini file
created by versions of the program obtained from Tomoaki
Nakashima's site.  If you import nPOP.ini, the misspelled
versions are automatically replaced by the correct settings
in nPOPuk.ini.

  Original entry        Replaced by
  --------------------------------------------
  StertPass             StartPass
  MstchCase             MatchCase
  ShowNewMailMessgae    ShowNewMailMessage
  ActiveNewMailMessgae  ActiveNewMailMessage

Additionally, the unused entries sBura and sOida (believed
to be related to word-wrapping for Japanese text) are
automatically deleted from nPOPuk.ini.

nPOPuk also changes the way messages are stored on disk,
saving some disk space and adding some functionality at the
cost of making the mailbox files incompatible Tomoaki's
releases.  (The message text will still be readable, but
information about whether the message was downloaded
or marked for downloading/deletion/sending will not be
correct.)

Compared with Tomoaki's nPOP, nPOPuk incorporates a new,
more sophisticated, date handling routine that enables
the correct sorting of mail from across different time
zones.

However, depending on the date format used (a setting found
in the nPOPuk.ini file) and the settings for Mail List
Sorting (Found on the Other tab of the Global Options
dialogue), on initial upgrade dates may not, initially,
display correctly in the main window.

To correct any problem, simply open the Mailbox Menu and
select "Initialise".  On the dialogue that appears, under
the section "When checking mail download:" set the radio
button to "Mail from item number:" and in the associated
box enter the value "1" (without quotes).

When you next check you mail, all mail will be read again
and the list re-populated, now showing in the desired
format and correct order.

Finally, some previous versions of nPOP set the TimeZone
entry in nPOP.ini to "+0000" (GMT).  Generally, the entry
should be blank, i.e.:
TimeZone=
so that nPOP uses the time zone set by the Windows
operating system (and tracks changes if you travel with
your laptop and change the time zone).  If you import
nPOP.ini, you may want to check nPOPuk.ini for this
setting.

-----------------------------------------------------------
4. What's New

Program Code:

*  Added: For Windows Mobile and WindowsCE devices, the 
   ability to automatically activate the Wi-Fi connection
   to check mail.

*  Changed: Improved ability to handle pasted text with 
   "LF" line ends (often found in files posted by iPhone
   users).

*  Changed: Improved efficiency and accuracy of character
   conversion for non-ASCII characters.

Program Interface Improvements:

*  Changed: Additional and re-ordered buttons on the Main
   and Mail View windows.
 
*  Changed: For sent mail, the external editor now shows
   header information (Useful when printing sent mail)

*  Added: The Mailbox Pane now indicates which Mailboxes
   have flagged mail.

*  Added: Header area in Sent Mail window now shows the
   date.

*  Added: On the Global Options Recv tab, an option to
   fetch messages in reverse order.

*  Added: On the Global Options Other tab, an option for
   password to configure program.

*  added: On the Mailbox Initialise dialogue, an option to
   delete all messages on the server.

*  Added: On the File > Connection menu, an option to view
   the Server Log file.

*  Added: Setup Files now provide for user input via
   dialogues.

For a complete history of the development of nPOPuk refer 
to:
http://npopuk.org.uk/3.00/changes.html

-----------------------------------------------------------
EOF
