#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>

#define MAX_BUF 4096

//Each Socket should have a buffer and buffer length
struct ConnectionState
{
    char buffer[MAX_BUF];
    size_t buffer_len;
};

int main()
{
   ConnectionState connection[FD_SETSIZE];
    memset(connection, 0, sizeof(connection));

   struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
   hints.ai_family = AF_INET;
   hints.ai_socktype = SOCK_STREAM;
   hints.ai_flags = AI_PASSIVE;


   struct addrinfo *bind_address;
   getaddrinfo(0, "8080", &hints, &bind_address);


   printf("Creating socket...\n");
   int socket_listen;
   socket_listen = socket(bind_address->ai_family,
           bind_address->ai_socktype, bind_address->ai_protocol);
   if (!(socket_listen)) {
      fprintf(stderr, "socket() failed. (%d)\n", strerror(errno));
      return 1;
   }
    printf("Binding socket to local address...\n");
    if (bind(socket_listen,
                bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", strerror(errno));
        return 1;
                }
    freeaddrinfo(bind_address);

    // listen () returns 0 on success
    printf("Listening...\n");
    if (listen(socket_listen, 5) < 0)
    {
        fprintf(stderr, "listen: %s\n", strerror(errno));
    };

    // select functionality below
    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    int max_socket = socket_listen;
    printf("Waiting for connections...\n");


    while(1) {
        fd_set reads;
        reads = master;
        if (select(max_socket+1, &reads, 0, 0, 0) < 0) {
            fprintf(stderr, "select() failed. (%d)\n", strerror(errno));
            return 1;
        }

        int i;
        for(i = 1; i <= max_socket; ++i) {
            if (FD_ISSET(i, &reads)) {

                if (i == socket_listen) {
                    struct sockaddr_storage client_address;
                    socklen_t client_len = sizeof(client_address);
                    //accept
                    int socket_client = accept(socket_listen,
                            (struct sockaddr*) &client_address,
                            &client_len);

                    if (socket_client < 0) {
                        fprintf(stderr, "accept() failed. (%d)\n",
                                strerror(errno));
                        return 1;
                    }
                    connection[socket_client].buffer_len = 0;

                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket)
                        max_socket = socket_client;

                    char address_buffer[100];
                    getnameinfo((struct sockaddr*)&client_address,
                            client_len,
                            address_buffer, sizeof(address_buffer), 0, 0,
                            NI_NUMERICHOST);
                    printf("New connection from %s\n", address_buffer);

                } else
                {
                    ConnectionState *c = connection;
                    int n = recv(i,c->buffer +c->buffer_len,MAX_BUF - c->buffer_len, 0);
                    if (n <= 0)
                    {
                        FD_CLR(i, &master);
                        close(i);
                        c->buffer_len = 0;
                        continue;
                    }
                    c->buffer_len += n;

                    while(true)  // parsing block
                    {
                       if( n < 4)
                           break;
                        uint32_t msg_length;
                        memcpy(&msg_length, c->buffer, n);
                        msg_length = ntohl(msg_length);

                        //check if message length is between 1 ot MAX_BUF
                        if (msg_length < 1 || msg_length > MAX_BUF)
                        {
                            FD_CLR(i, &master);
                            close(i);
                            c->buffer_len = 0;
                            break;
                        }

                        size_t frameSize = 4 + msg_length;
                        // Full Frame not yet received
                        if(c->buffer_len < frameSize)
                            break;

                        // Full Frame Received
                        uint8_t  msg_type= c->buffer[4];

                        if(msg_type == 0x01) //Ping Required
                        {
                            printf("Received PIN_REQ from fd %d\n", i);
                            //Response goes here
                        }
                        // Remove frame from buffer
                        memmove(c->buffer, c->buffer +frameSize, c->buffer_len - frameSize);
                        c->buffer_len -= frameSize;


                    }// parsing block while true
                }
                        }//if FD_ISSET
        }//for i to max_socket
    }//while(1)
}
