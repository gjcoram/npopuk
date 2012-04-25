#include "General.h"
#include "Memory.h"
#include "String.h"

#ifdef _WIN32_WCE
// OS Versions: Windows CE .NET 4.0 and later.

#include <Pm.h>
#include <winioctl.h>
#include <Ntddndis.h>
#include <Nuiouser.h>
#ifdef DEBUG
#include <Iphlpapi.h>
#endif

// Global variables
BOOL WiFiLoop = FALSE; // in the process of connecting
HANDLE hEvent = NULL;

extern HWND MainWnd;
extern OPTION op;

// Local declarations
HANDLE hNDUIO = NULL;
HANDLE hQueue = NULL;
HANDLE hThread = NULL;
HANDLE hStopEvent = NULL;
static BOOL WiFiConnByNpop = FALSE;
static BOOL SetNICPower(TCHAR *InterfaceName, BOOL Check, BOOL Enable);
DWORD WINAPI WiFiStatusProc(LPVOID pParam);

#define WIFI_QUEUE					TEXT("WIFI_QUEUE")
#define THREAD_STOP					TEXT("THREAD_STOP")
#define WIFI_EVENT					TEXT("WIFI_EVENT")

/*
 * GetNetworkStatus - check if some adapter is powered and connected
 */
BOOL GetNetworkStatus()
{
	UCHAR QueryBuffer[ 1024 ] = { 0 };
	BOOL IsActive = FALSE;
	DWORD i;

	if (hNDUIO == NULL) {
		hNDUIO = CreateFile(NDISUIO_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
							FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
							INVALID_HANDLE_VALUE);

		if (hNDUIO == INVALID_HANDLE_VALUE) {
			if (WiFiLoop == FALSE && op.SocLog > 1) {
				log_save_a("Unable to open NDIS handle\r\n");
			}
			return FALSE;
		}
	}

	if (hQueue == NULL) {
		MSGQUEUEOPTIONS mqOpt;
		mqOpt.dwSize = sizeof(MSGQUEUEOPTIONS);
		mqOpt.dwFlags = MSGQUEUE_NOPRECOMMIT;
		mqOpt.bReadAccess = TRUE;
		// Queue holds 8 messages of max BUFSIZE bytes
		mqOpt.dwMaxMessages = 8;
		mqOpt.cbMaxMessage = BUF_SIZE;
		hQueue = CreateMsgQueue(WIFI_QUEUE, &mqOpt);

		if (hQueue == NULL) {
			if (WiFiLoop == FALSE && op.SocLog > 1) {
				log_save_a("Unable to create queue to monitor WiFi events\r\n");
			}
		} else {
			// Request notifications of network events
			DWORD dwBytesRet = 0;
			NDISUIO_REQUEST_NOTIFICATION ndRN = { 0 };
			ndRN.hMsgQueue = hQueue;
			ndRN.dwNotificationTypes = NDISUIO_NOTIFICATION_MEDIA_CONNECT
										| NDISUIO_NOTIFICATION_MEDIA_DISCONNECT
										| NDISUIO_NOTIFICATION_DEVICE_POWER_UP
										| NDISUIO_NOTIFICATION_DEVICE_POWER_DOWN
										;

			if (DeviceIoControl(hNDUIO, IOCTL_NDISUIO_REQUEST_NOTIFICATION,
								(VOID*) &ndRN, sizeof(NDISUIO_REQUEST_NOTIFICATION),
								NULL, 0, NULL, NULL)) {
				DWORD dwThreadID;
				hThread = CreateThread(NULL, 0, WiFiStatusProc, MainWnd, 0, &dwThreadID);

				hStopEvent = CreateEvent(NULL, TRUE, FALSE, THREAD_STOP);

			} else {
				if (WiFiLoop == FALSE && op.SocLog > 1) {
TCHAR msg[BUF_SIZE];
wsprintf(msg, TEXT("Unable to monitor WiFi events errno=%d\r\n"), GetLastError());
log_save(msg);
//					log_save_a("Unable to monitor WiFi events\r\n");
				}
			}
		}
	}

	for (i = 0; ; i++) {
		DWORD dwBytesRet = 0;
		BOOL has_pwr = FALSE, has_wep = FALSE, has_conn = FALSE;
		NDISUIO_QUERY_BINDING* pQueryBinding = (NDISUIO_QUERY_BINDING*) QueryBuffer;
		pQueryBinding->BindingIndex = i;

		if ( DeviceIoControl(hNDUIO, IOCTL_NDISUIO_QUERY_BINDING,
							(VOID*) QueryBuffer, sizeof(QueryBuffer),
							(VOID*) QueryBuffer, sizeof(QueryBuffer),
							&dwBytesRet, NULL) ) {

			// Get the device name in the list of bindings
			WCHAR* devName = (WCHAR*) mem_calloc(pQueryBinding->DeviceNameLength * sizeof(WCHAR) );
			memcpy( devName, (UCHAR*) pQueryBinding + pQueryBinding->DeviceNameOffset,
					pQueryBinding->DeviceNameLength );
			memset(QueryBuffer, 0, sizeof(QueryBuffer));

			has_pwr = SetNICPower(devName, TRUE, FALSE);

			if (has_pwr) {
				NDISUIO_QUERY_OID nqo;

				nqo.ptcDeviceName = devName;
				nqo.Oid = OID_802_11_WEP_STATUS;
				dwBytesRet = 0;
				if (DeviceIoControl(hNDUIO, IOCTL_NDISUIO_QUERY_OID_VALUE,
									&nqo, sizeof(NDISUIO_QUERY_OID),
									&nqo, sizeof(NDISUIO_QUERY_OID),
									&dwBytesRet, NULL)) {
					// whether wep is active or not, getting a valid response
					// from this query indicates this is probably a wifi adapter
					has_wep = TRUE;
					if (op.WiFiDeviceName == NULL || *op.WiFiDeviceName == TEXT('\0')) {
						mem_free(&op.WiFiDeviceName);
						op.WiFiDeviceName = alloc_copy_t(devName);
					}
				} else {
					has_wep = FALSE;
				}
			}

			if (has_pwr) {
				NIC_STATISTICS nicStat;

				nicStat.ptcDeviceName = devName;
				dwBytesRet = 0;
				if( DeviceIoControl(hNDUIO, IOCTL_NDISUIO_NIC_STATISTICS, NULL, 0,
									&nicStat, sizeof(NIC_STATISTICS), &dwBytesRet, NULL) ) {
					has_conn = (nicStat.MediaState == MEDIA_STATE_CONNECTED) ? TRUE : FALSE;
				}
			}
			IsActive = has_pwr && has_conn;

			if (WiFiLoop == FALSE && op.SocLog > 1) {
				TCHAR msg[MSG_SIZE];
				wsprintf(msg, TEXT("Adapter '%s'  power:%s  type:%s  conn:%s\r\n"),
									devName, (has_pwr) ? TEXT("ON") : TEXT("OFF"),
									(has_wep) ? TEXT("WIFI") : TEXT("LAN"),
									(has_wep) ? TEXT("YES") : TEXT("NO"));
				log_save(msg);
			}
			mem_free(&devName);

		} else {
			break;
		}
	}

	return IsActive;
}

DWORD WINAPI WiFiStatusProc(LPVOID pParam)
{
	HANDLE wait_objects[] = {hQueue, hStopEvent};
	size_t object_count = sizeof(wait_objects)/sizeof(HANDLE);
log_save_a("Started WiFiStatusProc\r\n");
	if (hQueue != NULL) {
		while (WaitForMultipleObjects(object_count, wait_objects, FALSE, INFINITE) == WAIT_OBJECT_0) {
			NDISUIO_DEVICE_NOTIFICATION notification = { 0 };
			DWORD dwBytesRead = 0;
			DWORD notification_flags = 0;
log_save_a("calling ReadMsgQueue\r\n");
			if (ReadMsgQueue(hQueue, &notification, sizeof(NDISUIO_DEVICE_NOTIFICATION),
								&dwBytesRead, 0, &notification_flags)) {
				if (dwBytesRead > 0) {
					if (notification.dwNotificationType == NDISUIO_NOTIFICATION_MEDIA_CONNECT) {
						if (hEvent) {
							SetEvent(hEvent);
						}
dwBytesRead = 0;
log_save_a("  notification: connect\r\n");
					}
					if (notification.dwNotificationType == NDISUIO_NOTIFICATION_MEDIA_DISCONNECT) {
dwBytesRead = 0;
log_save_a("  notification: disconnect\r\n");
					}
					if (notification.dwNotificationType == NDISUIO_NOTIFICATION_DEVICE_POWER_UP) {
dwBytesRead = 0;
log_save_a("  notification: power up\r\n");
					}
					if (notification.dwNotificationType == NDISUIO_NOTIFICATION_DEVICE_POWER_DOWN) {
dwBytesRead = 0;
log_save_a("  notification: power down\r\n");
					}
					if (dwBytesRead != 0) {
log_save_a("  notification type unknown\r\n");
					}
				} else {
log_save_a("ReadMsgQueue got 0 bytes?\r\n");
				}
			} else {
log_save_a("ReadMsgQueue failed\r\n");
			}
		}
	}
log_save_a("Exiting WiFiStatusProc\r\n");
	ExitThread(WM_QUIT);
	return 0;
}


/*
 * WiFiConnect
 */
BOOL WiFiConnect(HWND hWnd, int Dummy)
{
	BOOL ret;

	// check if active already
	ret = SetNICPower(op.WiFiDeviceName, TRUE, TRUE);
	if (ret == TRUE) {
		SetStatusTextT(MainWnd, STR_STATUS_WIFI_CONNECT, 1);
		return ret;
	}

	SetStatusTextT(MainWnd, STR_STATUS_WIFI_START, 1);
	if (op.SocLog > 1) {
		log_save_a("Activating WiFi\r\n");
	}
	SetTimer(hWnd, ID_WIFIWAIT_TIMER, op.WiFiWaitSec * 1000, NULL);

	ret = SetNICPower(op.WiFiDeviceName, FALSE, TRUE);
	WiFiConnByNpop = TRUE;

	// wait for connection to be established
	hEvent = CreateEvent(NULL, TRUE, FALSE, WIFI_EVENT);
	if (hEvent == NULL) {
		if (op.SocIgnoreError != 1) {
			ErrorMessage(hWnd, STR_ERR_SOCK_EVENT);
		}
		return FALSE;
	}
	WiFiLoop = TRUE;

log_save_a("entering while loop\r\n");
	while (WaitForSingleObject(hEvent, 0) == WAIT_TIMEOUT) {
		MSG msg;
		if (GetMessage(&msg, NULL, 0, 0) == FALSE) {
			// WiFiTimer expired?
log_save_a("wifi loop got no message - timer expired?\r\n");
			break;
		}
		MessageFunc(hWnd, &msg);
		if (GetNetworkStatus() == TRUE) {
			// Device is powered and connected
			ret = TRUE;
log_save_a("connection established, breaking while loop\r\n");
			break;
		}
		if (WiFiLoop == FALSE) {
			// WiFiDisconnect called while waiting for connection?
log_save_a("breaking while loop\r\n");
			break;
		}
	}
log_save_a("done while loop\r\n");
	CloseHandle(hEvent);
	hEvent = NULL;
	if (WiFiLoop == FALSE) {
log_save_a("wifi loop false\r\n");
		return FALSE;
	}
	WiFiLoop = FALSE;

	if (ret) {
		SetStatusTextT(MainWnd, STR_STATUS_WIFI_CONNECT, 1);
	}
	return ret;
}

/*
 * WiFiDisconnect - power off wifi, if nPOPuk turned it on or Force is true
 */
void WiFiDisconnect(BOOL Force)
{
	if (Force || WiFiConnByNpop) {
		if (op.SocLog > 1) {
			log_save_a("De-activating WiFi\r\n");
		}
		SetNICPower(op.WiFiDeviceName, FALSE, FALSE);
		WiFiConnByNpop = FALSE;
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
	if (WiFiLoop == FALSE && op.SocLog > 1) {
		TCHAR msg[MSG_SIZE];
		wsprintf(msg, TEXT("Queried wifi device %s, query %s, powered %s (%d)\r\n"),
			InterfaceName,
			(ret == ERROR_SUCCESS) ? TEXT("SUCCESS") : TEXT("FAILED"),
			(bDevPowered == TRUE) ? TEXT("ON") : TEXT("OFF"), (int)Dx);
		log_save(msg);
	}

	if (Check == TRUE) {
		ret = bDevPowered; 
	} else if (Enable != bDevPowered) {
		if (Enable == TRUE && bDevPowered == FALSE) {
			Dx = D0; // turn on
		} else if (Enable == FALSE && bDevPowered == TRUE) {
			Dx = D4; // turn off
		}
		ret = SetDevicePower(szName, POWER_NAME, Dx);
	}

	return bDevPowered;
}

/*
 * FreeWiFiInfo - stop monitoring events
 */
void FreeWiFiInfo(void)
{
	if (hNDUIO) {
if(		DeviceIoControl(hNDUIO, IOCTL_NDISUIO_CANCEL_NOTIFICATION,
						NULL, 0, NULL, 0, NULL, NULL)
						) {
	log_save_a("cancelled notification\r\n");
} else {
	TCHAR msg[MSG_SIZE];
	wsprintf(msg, TEXT("FAILED to cancel notification errono %d\r\n"), GetLastError());
	log_save(msg);
}
		CloseHandle(hNDUIO);
		hNDUIO = NULL;
	}
else log_save_a("hNDUIO was not open to cancel notification\r\n");

	if(hStopEvent) {
		SetEvent(hStopEvent);
		hStopEvent = NULL;
	}
	if (hQueue) {
		CloseMsgQueue(hQueue);
		hQueue = NULL;
	}
	if (hThread) {
		// thread should terminate after receiving hStopEvent
		hThread = NULL;
	}

	return;
}

#ifdef DEBUG
/*
 * PrintAdapterInfo - print information retrieved by GetAdaptersInfo
 */
BOOL PrintAdapterInfo()
{
	BOOL ret = FALSE;

	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	char buf[BUF_SIZE];

	ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *) mem_alloc(sizeof (IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		log_save_a("Error allocating memory needed to call GetAdaptersInfo\r\n");
		return FALSE;
	}

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		mem_free(&pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) mem_alloc(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			log_save_a("Error allocating memory needed to call GetAdaptersInfo\r\n");
			return FALSE;
		}
	}

	// Unfortunately, if an adapter is powered off, it probably won't appear
	// in this list.
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
			}
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
	} else {
		sprintf(buf, "GetAdaptersInfo failed with error: %d\r\n", dwRetVal);
		log_save_a(buf);
	}
	if (pAdapterInfo)
		mem_free(&pAdapterInfo);

	return ret;
}
#endif

#else // _WIN32_WCE
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
