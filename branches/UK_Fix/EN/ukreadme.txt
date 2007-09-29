nPOP v1.0.8 UK fix-4                           ukreadme.txt
                                                 2 Aug 2006
-----------------------------------------------------------


Contents
--------
1. Introduction
2. Installing the Fix
3. What's New
4. What's Planned


-----------------------------------------------------------
1. Introduction

Thank you for downloading this release of nPOP.  Versions
for each of the supported platforms, based on the
source code originally written by Tomoaki Nakashima, have
been created by English speaking users of the program.

The latest public release of the "UK versions" of nPOP, both
source and executable code, are available at
http://www.npopsupport.fsworld.co.uk.  Pre-release versions
are available to subscribers of the npopsupport mail list
(http://groups.yahoo.com/groups/npopsupport/files).

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

If you need help configuring nPOP see:
http://www.npopsupport.fsworld.co.uk/install.htm 

If upgrading a previous installation, you may simply replace
the original nPOP.exe with this one.  One of the features of
this release is that it corrects the spelling of four of the
entries that appear in the nPOP.ini file.  However update of
the original settings is not automatic and you will need to
check these manually.

When you run the new version for the first time, check the
following dialogues to make sure all settings are as you
wish.

Global Options/Check  (Main window File/Global Options...)
Global Options/Other  (Main window File/Global Options...)
Find                  (Mail View window Edit/Find...)

Technical Details
-----------------

For those upgrading from a previous release:
--------------------------------------------
This release of the program creates the following new
entries in the nPOP.ini file.

StartPass=
MatchCase=
ShowNewMailMessage=
ActiveNewMailMessage=

These replace the functionality of the earlier entries:

StertPass=            Setting on Option/Other Dialogue
MstchCase=            Setting on Find Dialogue
ShowNewMailMessgae    Setting on Option/Check Dialogue
ActiveNewMailMessgae  Setting on Option/Check Dialogue

Once your original settings have been copied to the new,
correctly spelt, versions, you may safely delete the mis-
spelled versions of the nPOP.ini entries.


-----------------------------------------------------------
3. What's New

This version extends the original nPOP v1.0.8 version to
include the following features:

* Support for read and delivery receipts.

* Support for mail priorities (High, Normal and Low). 

* An option to Save all working files without exiting. 

* Global Options to allow:
    Automatically Marking Mail to be Sent
    Disabling of Warning Popup Boxes
    Control of Mail List Sorting

* A Mail Forwarding Facility.

UK fix-4 improves on nPOP v1.0.8 in the following ways:

* Sent mail can no longer be edited.

* Many revised translations in menus and dialogues
  throughout the program.

* The Mail View window, Edit menu, now includes options
  which correctly toggle between "View source" and "View
  text" options.

* Mis-spellings corrected in nPOP.ini file settings. (See
  notes above)

* Mail Bug: Messages that include lines starting with "from"
  (regardless of case) are handled correctly (only those that
  start with "From " need to have ">" prepended, due to a
  limitation in the Internet mail protocol).

* Address book column widths are now saved to the nPOP.ini
  file. (See: AddColSize-0= and AddColSize-1=)

* The Help/About dialogue now includes a reference to the
  nPOP UK Support web site.

* In new installations, the quotation character will default
  to  "> " instead of ">".

* Filters are correctly moved with accounts when reordering
  (move up/down).

* The Reply-To address can be selected from a drop-down list.

* When "include header lines" is checked, headers are no longer
  visible in a text-only message, unless View Source is performed.

* Word-wrap is improved for English text (the original nPOP is
  for Japanese Unicode).

-----------------------------------------------------------
4. What's Planned

A small group of programmers are working to improve nPOP.
If you have programming skills and wish to help you may
contact them through the npopsupport mail list.
(See: http://groups.yahoo.com/groups/npopsupport/)

-----------------------------------------------------------
EOF
