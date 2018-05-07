#include <stdio.h>

#include <stdlib.h>
#include <string.h>

#include "tcp.h"

uint8_t std_key[] = {
    4, 234, 133, 23, 213, 14, 5, 200, 164, 123, 143, 25, 73, 78, 53, 211,
    8, 234, 133, 23, 213, 14, 5, 200, 164, 123, 143, 25, 73, 78, 53, 211,
    4, 234, 133, 23, 213, 14, 5, 200, 164, 123, 143, 25, 73, 78, 53, 211,
    9, 234, 133, 23, 213, 14, 5, 200, 164, 123, 143, 25, 73, 78, 53, 211,
    4, 234, 133, 23, 213, 14, 5, 200, 164, 123, 143, 25, 73, 78, 53, 211,
    9, 234, 133, 23, 213, 14, 5, 200, 164, 123, 143, 25, 73, 78, 53, 211,
    4, 234, 133, 23, 213, 14, 5, 200, 164, 123, 143, 25, 73, 78, 53, 211,
    4, 234, 133, 23, 213, 14, 5, 200, 164, 123, 143, 25, 73, 78, 53, 211,
    4, 234, 133, 23, 213, 14, 5, 200, 164, 123, 143, 25, 73, 78, 53, 211,
    4, 234, 133, 23, 213, 14, 5, 200, 164, 123, 143, 25, 73, 78, 53, 211
};

int main(int argc, char **argv)
{
#define print_usage() printf("usage: -h port | -c xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxxx::port\n       -s file.txt | -r file.txt\n       -x keyfile.txt\n");


    if(argc == 7)
    {
        printf("key: %s\n",argv[6]);
        uint8_t *key_p = std_key;
        int offset = sizeof(std_key) / strlen(argv[6]);
        for(int i = 0; i < strlen(argv[6]); i++)
        {
            *key_p = argv[6][i];
            key_p += offset;
        }
    }

    if(argc < 4)
    {
        print_usage();
        return 0;
    }

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
        printf("test ip: %s port: %s\n", argv[2], argv[2] + 41);
        if(!connect_to_host(&c_socket, argv[2], argv[2] + 41))
            goto cleanup;
    }
    else
        goto cleanup;

    //send or receive file
    if(argv[3][1] == 's')
    {
        send_file(c_socket, argv[4], std_key);
    }
    else if(argv[3][1] == 'r')
    {
        recv_file(c_socket, argv[4], std_key);
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
