
#include <stdio.h>
#include <Windows.h>
#include <WinInet.h>

int main( int argc, const char* argv[] )
{
	// stack strings instead of string in data section
	char ua[] = {'c', 'u', 'r', 'l', ',', ' ', 'm', 'y', ' ', 'u', 's', 'e', 'r', '-', 'a', 'g', 'e', 'n', 't', 0x00};
	char host[] = {'i', 'f', 'c', 'o', 'n', 'f', 'i', 'g', '.', 'm', 'e', 0x00};

	// IntenetOpen sets the user agent HTTP header
	HINTERNET session=InternetOpen(ua, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);

	// InternetConnect handles DNS resolutions for us
	HINTERNET http=InternetConnect(session, host, 80, 0, 0, INTERNET_SERVICE_HTTP, 0, 0);
	HINTERNET hHttpRequest = HttpOpenRequest(http, "GET", "/", 0, 0, 0, INTERNET_FLAG_RELOAD, 0);

	// Headers defined here overwrite HTTP headers, even the Host header which, by default, is set by the value in InternetConnect
    //char szHeaders[] = "Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\nUser-Agent: curl, my user-agent CHANGED";
	//char szHeaders[] = "Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\nHost: Foo";
	char szHeaders[] = "Content-Type: application/x-www-form-urlencoded; charset=UTF-8";

    char szReq[1024]="";
    HttpSendRequest(hHttpRequest, szHeaders, strlen(szHeaders), szReq, strlen(szReq));
    char szBuffer[1025];
    DWORD dwRead=0;
    while(InternetReadFile(hHttpRequest, szBuffer, sizeof(szBuffer)-1, &dwRead) && dwRead) {
      szBuffer[dwRead] = 0;
      MessageBox(0,szBuffer,0,0);
	}
    InternetCloseHandle(hHttpRequest);
    InternetCloseHandle(session);
    InternetCloseHandle(http);
}
