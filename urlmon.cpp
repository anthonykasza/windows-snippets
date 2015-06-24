#include <stdio.h>
#include <tchar.h>

#include <Windows.h>
#include <UrlMon.h>

#pragma comment(lib, "urlmon.lib")

int main(int argc, char * argv[])
{
TCHAR url[] = TEXT("http://ifconfig.me");

    printf("Downloading from Url: %S\n", url);
    TCHAR path[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, path);
    wsprintf(path, TEXT("%s\\index.html"), path);
    printf("Downloading to Path: %S\n", path);
    HRESULT res = URLDownloadToFile(NULL, url, path, 0, NULL);

    if(res == S_OK) {
        printf("Ok\n");
    } else if(res == E_OUTOFMEMORY) {
        printf("Buffer length invalid, or insufficient memory\n");
    } else if(res == INET_E_DOWNLOAD_FAILURE) {
        printf("URL is invalid\n");
    } else {
        printf("Other error: %d\n", res);
    }

    return 0;
}
