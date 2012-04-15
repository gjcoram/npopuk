#include "General.h"
#include "Memory.h"
#include "String.h"

#ifdef _WIN32_WCE

// OS Versions: Windows CE .NET 4.0 and later.
#include <Pm.h>
#include <Iphlpapi.h>

// Global variables
BOOL WifiLoop = FALSE;
HANDLE hEvent;

extern HWND MainWnd;
extern OPTION op;

// Local declarations
static BOOL WifiConnByNpop = FALSE;
static BOOL SetNICPower(TCHAR *InterfaceName, BOOL Check, BOOL Enable);

#define WIFI_EVENT					TEXT("WIFI_EVENT")


/*
 * GetNetworkStatus - check if some adapter is powered and IP address is set
 */
BOOL GetNetworkStatus(void) {
	BOOL ret = FALSE;

	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	char buf[BUF_SIZE];

	ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *) mem_alloc(sizeof (IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		if (op.SocLog > 1) {
			log_save_a("Error allocating memory needed to call GetAdaptersInfo\r\n");
		}
		return FALSE;
	}

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		mem_free(&pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) mem_alloc(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			if (op.SocLog > 1) {
				log_save_a("Error allocating memory needed to call GetAdaptersInfo\r\n");
			}
			return FALSE;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			TCHAR *name;
			BOOL pwr = FALSE;
			if (name = alloc_char_to_tchar(pAdapter->AdapterName)) {
				pwr = SetNICPower(name, TRUE, FALSE);
				if (pwr == TRUE && pAdapter->IpAddressList.IpAddress.String != "0.0.0.0") {
					ret = TRUE;
				}
				mem_free(&name);
				if (ret == TRUE && op.SocLog <= 3) {
					break;
				}
			}
			if (op.SocLog > 3) {
				sprintf(buf, "\tComboIndex: \t%d\r\n", pAdapter->ComboIndex);
				log_save_a(buf);
				sprintf(buf, "\tAdapter Name: \t%s\r\n", pAdapter->AdapterName);
				log_save_a(buf);
				sprintf(buf, "\tAdapter Power: \t%s\r\n", ((pwr)? "Yes" : "No"));
				log_save_a(buf);
				sprintf(buf, "\tAdapter Desc: \t%s\r\n", pAdapter->Description);
				log_save_a(buf);
				sprintf(buf, "\tAdapter Addr: \t%.2X-%.2X-%.2X-%.2X\r\n",
						(int) pAdapter->Address[0], (int) pAdapter->Address[1],
						(int) pAdapter->Address[2], (int) pAdapter->Address[3]);
				log_save_a(buf);
				sprintf(buf, "\tIndex: \t%d\r\n", pAdapter->Index);
				log_save_a(buf);
				log_save_a("\tType: \t");
				switch (pAdapter->Type) {
				case MIB_IF_TYPE_OTHER:
					log_save_a("Other\r\n");
					break;
				case MIB_IF_TYPE_ETHERNET:
					log_save_a("Ethernet\r\n");
					break;
				case MIB_IF_TYPE_TOKENRING:
					log_save_a("Token Ring\r\n");
					break;
				case MIB_IF_TYPE_FDDI:
					log_save_a("FDDI\r\n");
					break;
				case MIB_IF_TYPE_PPP:
					log_save_a("PPP\r\n");
					break;
				case MIB_IF_TYPE_LOOPBACK:
					log_save_a("Lookback\r\n");
					break;
				case MIB_IF_TYPE_SLIP:
					log_save_a("Slip\r\n");
					break;
				default:
					sprintf(buf, "Unknown type %ld\r\n", pAdapter->Type);
					log_save_a(buf);
					break;
				}

				sprintf(buf, "\tIP Address: \t%s\r\n", pAdapter->IpAddressList.IpAddress.String);
				log_save_a(buf);
				sprintf(buf, "\tIP Mask: \t%s\r\n", pAdapter->IpAddressList.IpMask.String);
				log_save_a(buf);

				sprintf(buf, "\tGateway: \t%s\r\n", pAdapter->GatewayList.IpAddress.String);
				log_save_a(buf);
				log_save_a("\t***\r\n");

				if (pAdapter->DhcpEnabled) {
					log_save_a("\tDHCP Enabled: Yes\r\n");
					sprintf(buf, "\t  DHCP Server: \t%s\r\n",
						   pAdapter->DhcpServer.IpAddress.String);
					log_save_a(buf);

				} else {
					log_save_a("\tDHCP Enabled: No\r\n");
				}
				if (pAdapter->HaveWins) {
					log_save_a("\tHave Wins: Yes\r\n");
					sprintf(buf, "\t  Primary Wins Server:	%s\r\n",
						   pAdapter->PrimaryWinsServer.IpAddress.String);
					log_save_a(buf);
					sprintf(buf, "\t  Secondary Wins Server:  %s\r\n",
						   pAdapter->SecondaryWinsServer.IpAddress.String);
					log_save_a(buf);
				} else {
					log_save_a("\tHave Wins: No\r\n");
				}
				log_save_a("\r\n");
				pAdapter = pAdapter->Next;
			}
		}
	} else if (op.SocLog > 1) {
		sprintf(buf, "GetAdaptersInfo failed with error: %d\r\n", dwRetVal);
		log_save_a(buf);
	}
	if (pAdapterInfo)
		mem_free(&pAdapterInfo);

	return ret;
}



/*
 * WifiConnect
 */
BOOL WifiConnect(HWND hWnd, int Dummy) {
	BOOL ret;

	// check if active already
	ret = SetNICPower(op.WifiDeviceName, TRUE, TRUE);
	if (ret == TRUE) {
		SetStatusTextT(MainWnd, STR_STATUS_WIFI_CONNECT, 1);
		return ret;
	}

	SetStatusTextT(MainWnd, STR_STATUS_WIFI_START, 1);
	if (op.SocLog > 1) {
		log_save_a("Activating wi-fi\r\n");
	}
	SetTimer(hWnd, ID_WIFIWAIT_TIMER, op.WifiWaitSec * 1000, NULL);

	ret = SetNICPower(op.WifiDeviceName, FALSE, TRUE);
	WifiConnByNpop = TRUE;

	// wait for connection to be established
	hEvent = CreateEvent(NULL, TRUE, FALSE, WIFI_EVENT);
	if (hEvent == NULL) {
		if (op.SocIgnoreError != 1) {
			ErrorMessage(hWnd, STR_ERR_SOCK_EVENT);
		}
		return FALSE;
	}
	WifiLoop = TRUE;
	ResetEvent(hEvent);
log_save_a("entering while loop\r\n");
	while (WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT) {
		MSG msg;
		if (GetMessage(&msg, NULL, 0, 0) == FALSE) {
			break;
		}
		MessageFunc(hWnd, &msg);
		if (WifiLoop == FALSE) {
log_save_a("breaking while loop\r\n");
			break;
		}
	}
log_save_a("done while loop\r\n");
	CloseHandle(hEvent);
	hEvent = NULL;
	if (WifiLoop == FALSE) {
log_save_a("wifi loop false\r\n");
		return FALSE;
	}
	WifiLoop = FALSE;

	if (ret) {
		SetStatusTextT(MainWnd, STR_STATUS_WIFI_CONNECT, 1);
	}
	return ret;
}

/*
 * WifiDisconnect - power off wifi, if nPOPuk turned it on or Force is true
 */
void WifiDisconnect(BOOL Force)
{
	if (Force || WifiConnByNpop) {
		if (op.SocLog > 1) {
			log_save_a("De-activating wifi\r\n");
		}
		SetNICPower(op.WifiDeviceName, FALSE, FALSE);
		WifiConnByNpop = FALSE;
		SetStatusTextT(MainWnd, STR_STATUS_WIFI_DISCONNECT, 1);
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
	}
	if (op.SocLog > 1) {
		TCHAR msg[MSG_SIZE];
		wsprintf(msg, TEXT("Queried wifi device %s, query %s, powered %s\r\n"),
			InterfaceName,
			(ret == ERROR_SUCCESS) ? TEXT("SUCCESS") : TEXT("FAILED"),
			(bDevPowered == TRUE) ? TEXT("ON") : TEXT("OFF") );
		log_save(msg);
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
