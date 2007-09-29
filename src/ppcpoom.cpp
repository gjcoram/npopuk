#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "pimstore.h"
#include "ppcpoom.h"

void UpdateAddressBook(unsigned short * szFileName)
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
				WCHAR			wFileName[MAX_PATH];
				char			szOutputData[MAX_PATH * 4], userName[100];

				// Backup last file
				wsprintf(wFileName, L"%s.bak", szFileName);
				MoveFile(szFileName, wFileName);

				pOutlook->GetDefaultFolder(olFolderContacts, &pFolder);
				if (pFolder->get_Items(&pContacts) == S_OK && pContacts)
				{
					int numItems,i;
					pContacts->get_Count(&numItems);

					HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
								RELEASE_OBJ(pContact);

								// Write data to file
								int offset = 0;
								sprintf(userName, "%S%s%S", bstrFname, SysStringLen(bstrFname)>0?" ":"", bstrLname);

								if (SysStringLen(bstrEmail1) > 0)
								{
									sprintf(szOutputData, "%s <%S>\x0d\x0a", userName, bstrEmail1);
									WriteFile(hFile, szOutputData, strlen(szOutputData), &bytesWritten, NULL);
								}
								if (SysStringLen(bstrEmail2) > 0)
								{
									sprintf(szOutputData, "%s <%S>\x0d\x0a", userName, bstrEmail2);
									WriteFile(hFile, szOutputData, strlen(szOutputData), &bytesWritten, NULL);
								}
								if (SysStringLen(bstrEmail3) > 0)
								{
									sprintf(szOutputData, "%s <%S>\x0d\x0a", userName, bstrEmail3);
									WriteFile(hFile, szOutputData, strlen(szOutputData), &bytesWritten, NULL);
								}

								SysFreeString(bstrTitle);
								SysFreeString(bstrFname);
								SysFreeString(bstrLname);
								SysFreeString(bstrMname);
								SysFreeString(bstrEmail1);
								SysFreeString(bstrEmail2);
								SysFreeString(bstrEmail3);
							}
						}
						CloseHandle(hFile);
					}
					else	// Couldn't create file - Restore backup file
						MoveFile(wFileName, szFileName);

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