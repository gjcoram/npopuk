/* nPOPuk 
 * ppcpoom.cpp
 * Code originally from Matthew R. Pattman (?)
 * Updates copyright (C) 2006-2007 by Geoffrey Coram. All rights reserved.
 * Info at http://www.npopsupport.org.uk
 */

#include "pimstore.h"
#include "ppcpoom.h"


#ifndef _WIN32_WCE_PPC
// This function copied from POOM readme.txt,
// presumably copyright (C) Microsoft
HRESULT RegisterPOOM()
{
    HINSTANCE hPimstore;
    FARPROC pProc;
    HRESULT hr;

    // Register POOM
    hPimstore = LoadLibrary(TEXT("pimstore.dll"));
    if (!hPimstore)
    {
        return E_FAIL;
    }

    pProc = GetProcAddress(hPimstore, TEXT("DllRegisterServer"));
    if (!pProc)
    {
        FreeLibrary(hPimstore);
        return E_FAIL;
    }

    hr = pProc();
    FreeLibrary(hPimstore);
    return hr;
}
#endif

/*
 * UpdateAddressBook - reads PocketOutlook Contacts, dumps into Address Book
 */
int UpdateAddressBook(unsigned short * szFileName, int NameOrder, int NameIsComment)
{
	IPOutlookApp	*pOutlook;
	HRESULT hr;
	int retval = 0;

	// Start by initializing COM
	CoInitializeEx(NULL, 0);

	__try
	{
		hr = CoCreateInstance(__uuidof(Application), NULL, CLSCTX_INPROC_SERVER, __uuidof(IPOutlookApp), (LPVOID *) &pOutlook);
#ifndef _WIN32_WCE_PPC
		if (hr != S_OK)
		{
			RegisterPOOM();
			hr = CoCreateInstance(__uuidof(Application), NULL, CLSCTX_INPROC_SERVER, __uuidof(IPOutlookApp), (LPVOID *) &pOutlook);
		}
#endif
		if (hr != S_OK)
		{
			retval = -1;
		}
		else
		{
			if (pOutlook->Logon(NULL) != S_OK)	// Logon to Pocket Outlook
			{
				retval = -2;
			}
			else
			{
				IFolder			*pFolder;
				IPOutlookItemCollection *pContacts;
				IContact		*pContact;
				BSTR			bstrTitle, bstrFname, bstrLname, bstrMname;
				BSTR			bstrEmail1, bstrEmail2, bstrEmail3;
				BSTR			bstrCategories;
				WCHAR			wFileName[MAX_PATH];
				WCHAR			userName[100], categories[MAX_PATH];
				char			szOutputData[MAX_PATH * 4];

				pOutlook->GetDefaultFolder(olFolderContacts, &pFolder);
				if (pFolder->get_Items(&pContacts) == S_OK && pContacts)
				{
					int numItems,i;
					if (NameOrder == 1) {
						pContacts->Sort(L"[FirstName]", FALSE);
					} else {
						pContacts->Sort(L"[LastName]", FALSE);
					}
					pContacts->get_Count(&numItems);

					HANDLE hFile = INVALID_HANDLE_VALUE;
					if (numItems >= 1) {
						// Backup last file
						wsprintf(wFileName, L"%s.bak", szFileName);
						MoveFile((LPCWSTR)szFileName, wFileName);

						hFile = CreateFile((LPCWSTR)szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
					}
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
									wsprintf(userName, L"%s%s%s", bstrFname, SysStringLen(bstrFname)>0?L" ":L"", bstrLname);
								} else if (NameIsComment == 1) {
									wsprintf(userName, L"%s%s%s", bstrLname, SysStringLen(bstrLname)>0?L", ":L"", bstrFname);
								} else {
									wsprintf(userName, L"\"%s%s%s\"", bstrLname, SysStringLen(bstrLname)>0?L", ":L"", bstrFname);
								}
								if (SysStringLen(bstrCategories)>0) {
									if (NameIsComment == 1) {
										wsprintf(categories, L"\x09%s", bstrCategories);
									} else {
										wsprintf(categories, L"\x09\x09%s", bstrCategories);
									}
								} else {
									categories[0] = '\0';
								}

								if (SysStringLen(bstrEmail1) > 0) {
									FormatOutputString(szOutputData, userName, bstrEmail1, categories, NameIsComment);
									WriteFile(hFile, szOutputData, strlen(szOutputData), &bytesWritten, NULL);
								}
								if (SysStringLen(bstrEmail2) > 0) {
									FormatOutputString(szOutputData, userName, bstrEmail2, categories, NameIsComment);
									WriteFile(hFile, szOutputData, strlen(szOutputData), &bytesWritten, NULL);
								}
								if (SysStringLen(bstrEmail3) > 0) {
									FormatOutputString(szOutputData, userName, bstrEmail3, categories, NameIsComment);
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
						retval = i;
						CloseHandle(hFile);
					} else {
						// Couldn't create file (or no items) - Restore backup file
						MoveFile(wFileName, (LPCWSTR)szFileName);
						retval = -4;
					}

					RELEASE_OBJ(pContacts);
				} else {
					retval = -3;
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
		retval = -5;
	}
	CoUninitialize();
	return retval;
}


void FormatOutputString(char *ret, WCHAR *name, WCHAR *email, WCHAR *ctgy, int fmt)
{
#ifdef _WCE_OLD
	WCHAR chret[MAX_PATH * 4];
#endif
	if (fmt == 1) {
#ifdef _WCE_OLD
		wsprintf(chret, L"%s\x09%s%s\x0d\x0a", email, name, ctgy);
		WideCharToMultiByte(CP_ACP, 0, chret, -1, ret, MAX_PATH*4, NULL, NULL);
#else
		sprintf(ret, "%S\x09%S%S\x0d\x0a", email, name, ctgy);
#endif
	} else {
#ifdef _WCE_OLD
		wsprintf(chret, L"%s <%s>%s\x0d\x0a", name, email, ctgy);
		WideCharToMultiByte(CP_ACP, 0, chret, -1, ret, MAX_PATH*4, NULL, NULL);
#else
		sprintf(ret, "%S <%S>%S\x0d\x0a", name, email, ctgy);
#endif
	}
}


int AddPOOMContact(unsigned short *email, unsigned short *fname, unsigned short *lname, unsigned short *categ)
{
	IPOutlookApp	*pOutlook;
	HRESULT hr;
	int retval = 1;

	if (email == NULL) {
		return 0;
	}

	// Start by initializing COM
	CoInitializeEx(NULL, 0);

	__try
	{
		hr = CoCreateInstance(__uuidof(Application), NULL, CLSCTX_INPROC_SERVER, __uuidof(IPOutlookApp), (LPVOID *) &pOutlook);
#ifndef _WIN32_WCE_PPC
		if (hr != S_OK) // register and try again
		{
			RegisterPOOM();
			hr = CoCreateInstance(__uuidof(Application), NULL, CLSCTX_INPROC_SERVER, __uuidof(IPOutlookApp), (LPVOID *) &pOutlook);
		}
#endif
		if (hr != S_OK)
		{
			retval = -1;
		}
		else
		{
			if (pOutlook->Logon(NULL) != S_OK)	// Logon to Pocket Outlook
			{
				retval = -2;
			}
			else
			{
				IFolder			*pFolder;
				IPOutlookItemCollection *pItems;
				IContact		*pContact;

				pOutlook->GetDefaultFolder(olFolderContacts, &pFolder);
				pFolder->get_Items(&pItems);
				pItems->Add((IDispatch **)&pContact);
				if (pContact == NULL) {
					retval = -3;
				} else {
					if (fname != NULL)
						pContact->put_FirstName(fname);
					if (lname != NULL)
						pContact->put_LastName(lname);
					if (email != NULL)
						pContact->put_Email1Address(email);
					if (categ != NULL)
						pContact->put_Categories(categ);

					// Save the new contact
					pContact->Save();
					pContact->Release();
				}
				pItems->Release();
				pFolder->Release();
			}
		}
		pOutlook->Logoff();
		RELEASE_OBJ(pOutlook);
	}
	__except(1)
	{
		; // Do nothing for now!
		retval = -5;
	}
	CoUninitialize();
	return retval;
}