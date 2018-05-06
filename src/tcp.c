#include "tcp.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "util.h"

int host_connection(int *m_socket, int *c_socket, char *port)
{
    int reuseaddr = 0;
    struct sockaddr_in6 addr;

    *m_socket = socket(AF_INET6, SOCK_STREAM, 0);

    if(*m_socket == -1)
    {
        printf("error opening socket\n");
        *m_socket = 0;
        return 0;
    }

    setsockopt(*m_socket, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(atoi(port));
    addr.sin6_addr = in6addr_any;

    if(bind(*m_socket, (struct sockaddr*) &addr, sizeof(addr)) == -1)
    {
        printf("error binding socket\n");
        return 0;
    }

    if(listen(*m_socket, 2) == -1)
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
    struct sockaddr_in6 addr;

    printf("trying to connect to %s on port %s\n", ip6, port);

    *c_socket = socket(AF_INET6, SOCK_STREAM, 0);

    if(*c_socket == -1)
    {
        printf("error opening socket\n");
        *c_socket = 0;
        return 0;
    }

    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(atoi(port));

    inet_pton(AF_INET6, "::1", &addr.sin6_addr);    //::1 is ipv6 loopback
    //inet_pton(AF_INET6, ip6, &addr.sin6_addr);

    if(connect(*c_socket, (struct sockaddr*) &addr, sizeof(addr)) == -1)
    {
        printf("error connection to server\n");
        return 0;
    }

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

int send_file(int s, char *url, int encryption)
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
    char *buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int total_bytes = 0;

    int read_size = 0;
    while((read_size = fread(buffer, 1, BUFFER_SIZE, file)) > 0)
    {
        //encrypt buffer
         
        //

        int sent_bytes = 0;
        int bytes = 0;
        while(sent_bytes < read_size)
        {
            bytes = write(s, buffer + sent_bytes, read_size - sent_bytes);

            if(bytes < 0)
            {
                printf("error sending bytes\n");
                return 0;
            }

            if(bytes == 0)
                break;

            sent_bytes += bytes;

            printf("\rsent: %i|%i", sent_bytes, h.size);
        }

        memset(buffer, 0, BUFFER_SIZE);
    }

    printf("\n");

    return read_size;
}

int recv_file(int s, char *url, int encryption)
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
    char *buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    int total_bytes = 0;

    int bytes = 0;
    do
    {
        bytes = recv(s, buffer, BUFFER_SIZE, 0);

        if(bytes > 0)
        {
            //decrpyt buffer

            //


            fwrite(buffer, 1, bytes, file);
        }

        memset(buffer, 0, BUFFER_SIZE);

        total_bytes += bytes;

        printf("\rreceived: %i|%i", total_bytes, h.size);

    } while(bytes > 0);

    printf("\n");

    return total_bytes;
}




