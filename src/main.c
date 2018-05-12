#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "tcp.h"
#include "crypto.h"

#ifdef _WIN32
#include "winsock2.h"
#include "windows.h"
#define close(s) closesocket(s)
#endif

int main(int argc, char **argv)
{
#define print_usage() printf("usage: -h port | -c xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxxx::port\n       -s file.txt | -r file.txt\n       -x key(max 16 bit)\n");


    if(argc == 7)
    {
        printf("key: %s\n",argv[6]);
        set_key(argv[6], strlen(argv[6]));
    }

    if(argc < 4)
    {
        print_usage();
        return 0;
    }

#ifdef WIN32
    WSADATA wsa_data;

    int error = 0;
    error = WSAStartup(MAKEWORD(2,2), &wsa_data);
    if(error < 0)
    {
        printf("WSAStartup failed with %i\n", error);
        return 0;
    }
#endif

    int m_socket = 0;
    int c_socket = 0;

    //host or connect to server
    if(argv[1][1] == 'h')
    {
        if(!host_connection(&m_socket, &c_socket, argv[2]))
            goto cleanup;
    } 
    else if(argv[1][1] == 'c')
    {
        argv[2][39] = 0;
        argv[2][40] = 0;
        if(!connect_to_host(&c_socket, argv[2], argv[2] + 41))
            goto cleanup;
    }
    else
        goto cleanup;

    //send or receive file
    if(argv[3][1] == 's')
    {
        send_file(c_socket, argv[4]);
    }
    else if(argv[3][1] == 'r')
    {
        recv_file(c_socket, argv[4]);
    }
    else 
        goto cleanup;
   
cleanup:

    if(m_socket)
        close(m_socket);
    if(c_socket)
        close(c_socket);

    return 0;
}
