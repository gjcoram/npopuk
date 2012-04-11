#include "General.h"
#include "Memory.h"

#ifdef _WIN32_WCE

// OS Versions: Windows CE .NET 4.0 and later.
#include <Pm.h>
#include <wininet.h>

static BOOL WifiConnByNpop = FALSE;
static BOOL SetNICPower(TCHAR *InterfaceName, BOOL Check, BOOL Enable);

BOOL WifiLoop = FALSE;

extern OPTION op;

/*
 * GetWifiStatus
 */
BOOL GetWifiStatus(void) {
	return SetNICPower(op.WifiDeviceName, TRUE, FALSE);
}

/*
 * WifiConnect
 */
BOOL WifiConnect(HWND hWnd, int Dummy) {
	BOOL ret;
	DWORD dwFlags = 0;
	TCHAR msg[BUF_SIZE];

	ret = InternetGetConnectedState(&dwFlags, 0);
	if(ret) {
		wsprintf(msg, TEXT("GetConnState returns CONF=%d LAN=%d MODEM=%d OFFLINE=%d RAS=%d WIFI=%d\n"),
			(dwFlags & INTERNET_CONNECTION_CONFIGURED) ? 1 :0,
			(dwFlags & INTERNET_CONNECTION_LAN) ? 1 :0,
			(dwFlags & INTERNET_CONNECTION_MODEM) ? 1 :0,
			(dwFlags & INTERNET_CONNECTION_OFFLINE) ? 1 :0,
			(dwFlags & INTERNET_RAS_INSTALLED) ? 1 :0,
			(dwFlags & 0x12) ? 1 :0 );
		MessageBox(hWnd, msg, WINDOW_TITLE, MB_OK);
	}  else {
		MessageBox(hWnd, TEXT("failed to get conn state"), WINDOW_TITLE, MB_OK);
	}

	if (dwFlags & INTERNET_CONNECTION_LAN) {
		MessageBox(hWnd, TEXT("LAN connection deteccted"), WINDOW_TITLE, MB_OK);
	} else {

		ret = SetNICPower(op.WifiDeviceName, TRUE, TRUE);
		if (ret == FALSE) {
			ret = SetNICPower(op.WifiDeviceName, FALSE, TRUE);
			WifiConnByNpop = TRUE;
			MessageBox(hWnd, TEXT("activating wifi"), WINDOW_TITLE, MB_OK);
		}
	}
	return ret;
}

/*
 * WifiDisconnect - power off wifi, if nPOPuk turned it on or Force is true
 */
void WifiDisconnect(BOOL Force)
{
	if (Force || WifiConnByNpop) {
		SetNICPower(op.WifiDeviceName, FALSE, FALSE);
		WifiConnByNpop = FALSE;
	}
}

/*
 * SetNICPower - Enables or Disables NIC power
 */
static BOOL SetNICPower(TCHAR *InterfaceName, BOOL Check, BOOL Enable)
{
	TCHAR szName[MAX_PATH];
	CEDEVICE_POWER_STATE Dx = PwrDeviceUnspecified;    
	BOOL bDevPowered = TRUE;
	DWORD ret;

	wsprintf(szName, TEXT("%s\\%s"), PMCLASS_NDIS_MINIPORT, InterfaceName);
	szName[MAX_PATH-1]=TEXT('\0');

	ret = GetDevicePower(szName, POWER_NAME, &Dx);

	if (ret != ERROR_SUCCESS || D4 == Dx) {
		bDevPowered = FALSE;
MessageBox(NULL, TEXT("failure or not powered"), WINDOW_TITLE, MB_OK);
	} else {
MessageBox(NULL, TEXT("powered up!"), WINDOW_TITLE, MB_OK);
	}

	if (Check == TRUE) {
		ret = bDevPowered; 
	} else if (Enable != bDevPowered) {
		if (Enable == TRUE && bDevPowered == FALSE) {
			Dx = D0;
		} else if (Enable == FALSE && bDevPowered == TRUE) {
			Dx = D4;
		}
		ret = SetDevicePower(szName, POWER_NAME, Dx);
	}

	return bDevPowered;
}


#else
/* 
 * DisableEnableConnections - from CodeGuru
 */
HRESULT DisableEnableConnections(BOOL bEnable)
{
	INetConnectionManager *pNetConnectionManager = NULL;
	HRESULT hr = E_FAIL;

	CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_ConnectionManager, NULL,
							CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
							IID_INetConnectionManager,
							(void**)(&pNetConnectionManager) );
	if (SUCCEEDED(hr)) {
		// Get an enumerator for the set of connections on the system
		IEnumNetConnection* pEnumNetConnection;
		ULONG ulCount = 0;
		BOOL fFound = FALSE;
		HRESULT hrT = S_OK;

		pNetConnectionManager->EnumConnections(NCME_DEFAULT, &pEnumNetConnection);
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
       
		/*
			Enumerate through the list of adapters on the system and look for the one we want
			NOTE: To include per-user RAS connections in the list, you need to set the COM
			Proxy Blanket on all the interfaces. This is not needed for All-user RAS
			connections or LAN connections.
		*/
        do {
			NETCON_PROPERTIES* pProps = NULL;
			INetConnection *   pConn;

			// Find the next (or first connection)
            hrT = pEnumNetConnection->Next(1, &pConn, &ulCount);

			if (SUCCEEDED(hrT) && 1 == ulCount) {
				// Get the connection properties
				hrT = pConn->GetProperties(&pProps);

				if (S_OK == hrT) {
					if (bEnable) {
						hr = pConn->Connect();
					} else {
						hr = pConn->Disconnect();
					}
					CoTaskMemFree (pProps->pszwName);
					CoTaskMemFree (pProps->pszwDeviceName);
					CoTaskMemFree (pProps);
				}
				pConn->Release();
				pConn = NULL;
			}

		} while (SUCCEEDED(hrT) && 1 == ulCount && !fFound);

        if (FAILED(hrT)) {
			hr = hrT;
		}
		pEnumNetConnection->Release();
	}
   
	//if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_RETRY)) {
	//	printf("Could not enable or disable connection (0x%08x)\r\n", hr);
	//}

	pNetConnectionManager->Release();
	CoUninitialize();

	return hr;
}
#endif
