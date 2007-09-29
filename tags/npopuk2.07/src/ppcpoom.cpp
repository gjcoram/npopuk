/* nPOPuk 
 * ppcpoom.cpp
 * Code originally from Matthew R. Pattman (?)
 * Updates copyright (C) 2006-2007 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopsupport.org.uk
 */

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "pimstore.h"
#include "ppcpoom.h"

void UpdateAddressBook(unsigned short * szFileName, int NameOrder, int NameIsComment)
{
	IPOutlookApp	*pOutlook;

	// Start by initializing COM
	CoInitializeEx(NULL, 0);

	__try
	{
		if (CoCreateInstance(__uuidof(Application), NULL, CLSCTX_INPROC_SERVER, __uuidof(IPOutlookApp), (LPVOID *) &pOutlook) == S_OK)
		{
			if (pOutlook->Logon(NULL) == S_OK)	// Logon to Pocket Outlook
			{
				IFolder			*pFolder;
				IPOutlookItemCollection *pContacts;
				IContact		*pContact;
				BSTR			bstrTitle, bstrFname, bstrLname, bstrMname;
				BSTR			bstrEmail1, bstrEmail2, bstrEmail3;
				BSTR			bstrCategories;
				WCHAR			wFileName[MAX_PATH];
				char			szOutputData[MAX_PATH * 4], userName[100], categories[MAX_PATH];

				// Backup last file
				wsprintf(wFileName, L"%s.bak", szFileName);
				MoveFile((LPCWSTR)szFileName, wFileName);

				pOutlook->GetDefaultFolder(olFolderContacts, &pFolder);
				if (pFolder->get_Items(&pContacts) == S_OK && pContacts)
				{
					int numItems,i;
					if (NameOrder == 1) {
						pContacts->Sort(L"[FirstName]", FALSE);
					}
					// else default order is by last name
					pContacts->get_Count(&numItems);

					HANDLE hFile = CreateFile((LPCWSTR)szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hFile != INVALID_HANDLE_VALUE)
					{
						DWORD bytesWritten = 0;

						for (i = 1; i <= numItems; i++)
						{
							pContacts->Item(i, (IDispatch **)&pContact);
							if (pContact)
							{
								pContact->get_Title(&bstrTitle);
								pContact->get_FirstName(&bstrFname);
								pContact->get_LastName(&bstrLname);
								pContact->get_MiddleName(&bstrMname);
								pContact->get_Email1Address(&bstrEmail1);
								pContact->get_Email2Address(&bstrEmail2);
								pContact->get_Email3Address(&bstrEmail3);
								pContact->get_Categories(&bstrCategories); // GJC
								RELEASE_OBJ(pContact);

								// Write data to file
								int offset = 0;
								if (NameOrder == 1) {
									sprintf(userName, "%S%s%S", bstrFname, SysStringLen(bstrFname)>0?" ":"", bstrLname);
								} else if (NameIsComment == 1) {
									sprintf(userName, "%S%s%S", bstrLname, SysStringLen(bstrLname)>0?", ":"", bstrFname);
								} else {
									sprintf(userName, "\"%S%s%S\"", bstrLname, SysStringLen(bstrLname)>0?", ":"", bstrFname);
								}
								if (SysStringLen(bstrCategories)>0) {
									if (NameIsComment == 1) {
										sprintf(categories, "\x09%S", bstrCategories);
									} else {
										sprintf(categories, "\x09\x09%S", bstrCategories);
									}
								} else {
									categories[0] = '\0';
								}

								if (SysStringLen(bstrEmail1) > 0) {
									if (NameIsComment == 1) {
										sprintf(szOutputData, "%S\x09%s%s\x0d\x0a", bstrEmail1, userName, categories);
									} else {
										sprintf(szOutputData, "%s <%S>%s\x0d\x0a", userName, bstrEmail1, categories);
									}
									WriteFile(hFile, szOutputData, strlen(szOutputData), &bytesWritten, NULL);
								}
								if (SysStringLen(bstrEmail2) > 0) {
									if (NameIsComment == 1) {
										sprintf(szOutputData, "%S\x09%s%s\x0d\x0a", bstrEmail2, userName, categories);
									} else {
										sprintf(szOutputData, "%s <%S>%s\x0d\x0a", userName, bstrEmail2, categories);
									}
									WriteFile(hFile, szOutputData, strlen(szOutputData), &bytesWritten, NULL);
								}
								if (SysStringLen(bstrEmail3) > 0) {
									if (NameIsComment == 1) {
										sprintf(szOutputData, "%S\x09%s%s\x0d\x0a", bstrEmail3, userName, categories);
									} else {
										sprintf(szOutputData, "%s <%S>%s\x0d\x0a", userName, bstrEmail3, categories);
									}
									WriteFile(hFile, szOutputData, strlen(szOutputData), &bytesWritten, NULL);
								}

								SysFreeString(bstrTitle);
								SysFreeString(bstrFname);
								SysFreeString(bstrLname);
								SysFreeString(bstrMname);
								SysFreeString(bstrEmail1);
								SysFreeString(bstrEmail2);
								SysFreeString(bstrEmail3);
								SysFreeString(bstrCategories);
							}
						}
						CloseHandle(hFile);
					}
					else	// Couldn't create file - Restore backup file
						MoveFile(wFileName, (LPCWSTR)szFileName);

					RELEASE_OBJ(pContacts);
				}
				RELEASE_OBJ(pFolder);
			}
		}
		pOutlook->Logoff();
		RELEASE_OBJ(pOutlook);
	}
	__except(1)
	{
		; // Do nothing for now!
	}
	CoUninitialize();
}