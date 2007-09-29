nPOP v1.0.8 UK fix-5                      readme-108uk5.txt
                                                10 Oct 2006
-----------------------------------------------------------


Contents
--------
1. Introduction
2. Installing the Fix
3. What's New
4. What's Planned
5. Upgrading from a Tomoaki Nakashima version
6. Upgrading from an earlier version of nPOP


-----------------------------------------------------------
1. Introduction

Thank you for downloading this release of nPOP.  Versions
for each of the supported platforms, based on the
source code originally written by Tomoaki Nakashima, have
been created by English speaking users of the program.

The latest public release of the "UK versions" of nPOP,
both source and executable code, are available at
http://www.npopsupport.fsworld.co.uk/downloads.htm.
Pre-release versions are available to subscribers of the
npopsupport mail list at:
http://groups.yahoo.com/groups/npopsupport/files


Thanks must go to programmers Amy Millenson and Bruce
Jackson who worked on this release and especially Geoffrey
Coram, who appeared to be working 24 hours a day, and did
the vast bulk of the work.

Greg Chapman
Webmaster
http://www.npopsupport.fsworld.co.uk


-----------------------------------------------------------
2. Installing nPOP

nPOP needs no special installation procedure.  Simply copy
the nPOP.exe file to a suitable location and run the
program.

NOTES:
------

*  If you need help configuring nPOP see:
   http://www.npopsupport.fsworld.co.uk/install.htm 

*  If upgrading from a version obtained from Tomoaki's
   site, see the note in Section 5 of this file.

*  If upgrading from an earlier UK version of nPOP, then
   see the note in Section 6 of this file.


-----------------------------------------------------------
3. What's New

This version extends the original nPOP v1.0.8 version to
include the following features:

* Overlays in the main mail list window to indicate whether
  messages have been forwarded and/or replied to.

* Improvements to word-wrapping for English text as well as
  text in most European character sets (the original nPOP
  is for Japanese Unicode).  Linebreaks no longer happen
  at punctuation or non-English characters, extra spaces at
  the beginning of lines are removed, and quoted lines are
  broken before having the quotation mark ("> ") added.

* Correct indication of whether a message has been fully
  downloaded, even if nPOP is set to download only the
  first N lines.  (Previously, nPOP would indicate partial
  download when set to download only N lines, even if
  the message was in fact shorter than N lines.)

* The size column in the OutBox includes the attachments,
  so one can better estimate the upload time.

* The message composition (Edit) window now has separate
  icons and menu items for "Send now", "Save to Outbox",
  and "Save and Mark" to send later (on the next update
  account); this replaces the AutoMarkSend global option
  found in previous UK versions of nPOP.

* One may quote only portions of a message when replying
  or forwarding by selecting the desired text in the Mail
  View window before initiating the reply/forward.

* Dates now display correctly, regardless of the time zone
  of the sender and recipient.  The date and time format
  can be set in the Global Options dialog "Other" tab.

* When the Global Option "Include header lines" is checked,
  the View source option is used to view the header lines.
  (Previously, in text-only messages, the headers intruded
  into the message body.)

* Sent mail can no longer be edited.

* The Reply-To address can be selected from a drop-down
  list.

* Filters are now correctly moved with accounts when
  reordering (Move up/down).

* Messages that include lines starting with "from"
  (regardless of case) are handled correctly.

* The Help/About dialogue now includes a reference to the
  nPOP UK Support web site.

* New icons and toolbar button images.

* A Mail Forwarding facility
  (Further improved over nPOPw v1.0.1.4 Beta 2, with
  additional Global Option settings, such as different
  headers for Reply and Forward, and whether to add the
  signature to forwarded mail.)

* Global Options to allow:
    Disabling of Warning Popup Boxes
    Control of Mail List Sorting

* An option to Save all working files without exiting. 

* Support for mail priorities (High, Normal and Low). 

* Support for read and delivery receipts.

* In new installations, the quotation string for replies
  and forwarded mail is "> " rather than ">".

* Many revised translations and corrected spellings in
  menus and dialogues throughout the program.  (See also
  Section 5.)


-----------------------------------------------------------
4. What's Planned

A small group of programmers are working to improve nPOP.
If you have programming skills and wish to help you may
contact them through the npopsupport mail list.
(See: http://groups.yahoo.com/groups/npopsupport/)


-----------------------------------------------------------
5. Upgrading from a Tomoaki Nakashima version

One of the features of UK versions of nPOP is that they
correct the spelling of four entries that appear in the
nPOP.ini file created by versions of the program obtained
from Tomoaki Nakashima's site.  Update of the original
settings is automatic, and the misspelled versions are
automatically deleted.

  Original entry        Replaced by
  --------------------------------------------
  StertPass             StartPass
  MstchCase             MatchCase
  ShowNewMailMessgae    ShowNewMailMessage
  ActiveNewMailMessgae  ActiveNewMailMessage

Additionally, the unused entries sBura and sOida (believed
to be related to word-wrapping for Japanese text) are
automatically deleted from nPOP.ini.

-----------------------------------------------------------
6. Upgrading from an earlier version of nPOP

This release of nPOP incorporates a new, more
sophisticated, date handling routine that enables the
correct sorting of mail from across different time zones.

However, depending on the date format used (a setting found
in the nPOP.ini file) and the settings for Mail List
Sorting (Found on the Other tab of the Global Options
dialogue), on initial upgrade dates may not, initially,
display correctly in the main window.

To correct any problem, simply open the Account Menu and
select "Initialise".  On the dialogue that appears, under
the section "When checking mail download:" set the radio
button to "Mail from item number:" and in the associated
box enter the value "1" (without quotes).

When you next check you mail, all mail will be read again
and the list re-populated, now showing in the desired
format and correct order.

This release of nPOP also changes the way messages are
stored on disk, saving some disk space at the cost of
making the mailbox files incompatible with previous
releases.  (The message text will still be readable, but
information about whether the message was downloaded
or marked for downloading/deletion/sending will not be
correct.)

Finally, some previous versions of nPOP set the TimeZone
entry in nPOP.ini to "+0000" (GMT).  Generally, the entry
should be blank, i.e.:
TimeZone=
so that nPOP uses the time zone set by the Windows
operating system (and tracks changes if you travel with
your laptop and change the time zone).


-----------------------------------------------------------
EOF
