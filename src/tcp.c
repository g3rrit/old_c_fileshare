#include "tcp.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "winsock2.h"
#include "mstcpip.h"
#include "ws2tcpip.h"
#define inet_pton(...) InetPton(__VA_ARGS__)
#else
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#endif

#include "util.h"

int host_connection(int *m_socket, int *c_socket, char *port)
{
    printf("hosting connection on port %s\n", port);

    int rc = 0;

    struct addrinfo hints;
    struct addrinfo *res = 0;
    struct addrinfo *ptr = 0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo(0, port, &hints, &res);

    if(rc != 0)
    {
        printf("error getting address info\n");
        return 0;
    }

    ptr = res;

    *m_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if(*m_socket < 0)
    {
        printf("error opening socket\n");

        return 0;
    }

    rc = bind(*m_socket, ptr->ai_addr, ptr->ai_addrlen);

    if(rc == -1)
    {
        printf("eror binding socket\n");
        return 0;
    }

    rc = listen(*m_socket, 2);

    if(rc == -1)
    {
        printf("error listening on socket\n");
        return 0;
    }

    printf("listening for connections...\n");

    *c_socket = accept(*m_socket, 0, 0);

    if(*c_socket == -1)
    {
        printf("error acception client connection\n");
        *c_socket = 0;
        return 0;
    }

    printf("client connected\n");

    return 1;
}

int connect_to_host(int *c_socket, char *ip6, char *port)
{
    printf("trying to connect to %s on port %s\n", ip6, port);

    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *ptr;

    struct sockaddr_in6 addr;

    int rc = 0;

    memset(&hints, 0 , sizeof(hints));
    memset(&addr, 0, sizeof(addr));

    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;

    rc = getaddrinfo(ip6, 0, &hints, &res);

    if(rc != 0)
    {
        printf("error getting address info\n");
        return 0;
    }

    ptr = res;

    while(res)
    {
        if(res->ai_family == AF_INET6)
        {
            memcpy(&addr, res->ai_addr, res->ai_addrlen);

            addr.sin6_port = htons(atoi(port));
            addr.sin6_family = AF_INET6;
            
            break;
        }

        res = res->ai_next;
    }


    *c_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if(*c_socket == -1)
    {
        printf("error opening socket\n");
        *c_socket = 0;
        return 0;
    }

    if(connect(*c_socket, (struct sockaddr*) &addr, sizeof(addr)) == -1)
    {
        printf("error connection to server\n");
        return 0;
    }

    freeaddrinfo(ptr);

    printf("connection established\n");

    return 1;
}

//sending receiving

struct header
{
    uint32_t size;
};

int send_header(int s, struct header *h)
{
    int sent_bytes = 0;
    int bytes = 0;
    while(sent_bytes < sizeof(struct header))
    {
        bytes = write(s, h + sent_bytes, sizeof(struct header) - sent_bytes);
        if(bytes < 0)
            return 0;
        if(bytes == 0)
            break;

        sent_bytes += bytes;
    }

    return sent_bytes;
}

int recv_header(int s, struct header *h)
{
    int recv_bytes = 0;
    int bytes = 0;
    do
    {
        bytes = recv(s, h + recv_bytes, sizeof(struct header) - recv_bytes, 0);
        recv_bytes += bytes;

    } while(recv_bytes < sizeof(struct header));

    return recv_bytes;
}

int send_file(int s, char *url)
{
    FILE *file = fopen(url, "rb");

    if(!file)
    {
        printf("error opening file %s\n", url);
        return 0;
    }

    printf("starting to send file: %s\n", url);

    struct header h = { .size = get_file_size(file) };
    send_header(s, &h);

    printf("sent: 0|%i", h.size);

#define BUFFER_SIZE 1024
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int crypt_size = ((float)h.size/1024.f) - h.size/1024 > 0 ? ((h.size/1024) + 1) * BUFFER_SIZE : (h.size/1024) * BUFFER_SIZE;

    int total_bytes = 0;

    int read_size = 0;
    while((read_size = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    {
        //encrypt buffer
        encrypt(buffer, read_size);
        //

        int sent_bytes = 0;
        int bytes = 0;
        while(sent_bytes < BUFFER_SIZE)
        {
            bytes = write(s, buffer + sent_bytes, BUFFER_SIZE - sent_bytes);

            if(bytes < 0)
            {
                printf("error sending bytes\n");
                return 0;
            }

            if(bytes == 0)
                break;

            sent_bytes += bytes;

            printf("\rsent: %i|%i", sent_bytes, crypt_size);
        }

        memset(buffer, 0, BUFFER_SIZE);
    }

    printf("\n");

    return read_size;
}

int recv_file(int s, char *url)
{
    FILE *file = fopen(url, "wb");

    if(!file)
    {
        printf("error opening file %s\n", url);
        return 0;
    }

    printf("starting to write to file: %s\n", url);

    struct header h = { .size = 0 };
    recv_header(s, &h);

    printf("received: 0|%i", h.size);

#define BUFFER_SIZE 1024
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int block_count = h.size/BUFFER_SIZE;
    int last_block_size = h.size % BUFFER_SIZE;
    block_count += last_block_size > 0 ? 1 : 0;

    int total_bytes = 0;

    int bytes = 0;
    int recv_bytes = 0;
    do
    {
        bytes = recv(s, buffer + recv_bytes, BUFFER_SIZE - recv_bytes, 0);
        recv_bytes += bytes;

        if(bytes > 0 && recv_bytes == BUFFER_SIZE)
        {
            //decrpyt buffer
            decrypt(buffer, BUFFER_SIZE);
            //

            if(block_count != 1)
            {
                total_bytes += recv_bytes;
                fwrite(buffer, 1, recv_bytes, file);
            }
            else
            {
                total_bytes += last_block_size;
                fwrite(buffer, 1, last_block_size, file);
            }

            memset(buffer, 0, BUFFER_SIZE);
            block_count--;
            recv_bytes = 0;
        }

        printf("\rreceived: %i|%i", total_bytes, h.size);

    } while(bytes > 0);

    printf("\n");

    return total_bytes;
}




