#include <winsock2.h>
#include <windows.h>
#include <iphlpapi.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib,"ws2_32.lib")

using namespace std;

#define MAX_NAME 32
#define MAX_IP 39
#define DEFAULT_SERVER "www.0th.me"
#define DEFAULT_PATH "/iplogger"
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

int get(char *server, char *path, char *name, char *ip){
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "WSAStartup failed.\n";
		system("pause");
		return 1;
	}

	int retVal = 0;
	char * url = (char *)malloc(MAX_IP + MAX_NAME + strlen(server));
	char * req = (char *)malloc(strlen(url) + 100);
	sprintf(url, "%s/?name=%s&ip=%s", path, name, ip);
	sprintf(req, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", url, server);

	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct hostent *host;
	host = gethostbyname(server);
	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(80);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);
	cout << "Connecting...\n";
	if (connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0){
		cout << "Could not connect";
		// system("pause");
		retVal = 1;
	}
	else {
		cout << "Connected.\n";
		send(Socket, req, strlen(req), 0);
		printf("%s", req);
		closesocket(Socket);
	}

	free(url);
	free(req);
	WSACleanup();
	return retVal;
}

int main(int argc, char *argv[])
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	UINT i;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		printf("Error allocating memory needed to call GetAdaptersinfo\n");
		return 1;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			printf("Error allocating memory needed to call GetAdaptersinfo\n");
			return 1;
		}
	}

	char szPath[128] = "", name[MAX_NAME];
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	gethostname(szPath, sizeof(szPath));
	WSACleanup();

	char server[128] = DEFAULT_SERVER;
	char path[128] = DEFAULT_PATH;

	if (argc > 1) 
		sscanf(argv[1], "http://%[^/]%s", server, path);

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		while (pAdapter) {
			sprintf(name, "%s_%d", szPath, pAdapter->Index);
			get(server, path, name, pAdapter->IpAddressList.IpAddress.String);
			pAdapter = pAdapter->Next;
		}
	}
	else {
		printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);
	}
	if (pAdapterInfo)
		FREE(pAdapterInfo);

	system("pause");

	return 0;
}
