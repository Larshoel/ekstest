#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <string.h>

#define BACKLOG_SIZE 10

int tcp_connect( char* hostname, int port )
{
    int sock;
    int err;
    struct sockaddr_in serveraddr;

    /* Create socket */
    sock = socket( AF_INET, SOCK_STREAM, 0 );
    if( sock < 0 )
    {
        fprintf( stderr, "%s:%d: Failed to create a TCP socket: %s\n", __FILE__, __LINE__, strerror(errno) );
        return -1;
    }

    /* Initialize server address struct to zero */
    memset( &serveraddr, 0, sizeof(struct sockaddr_in) );

     /* Set domain to Internet */
    serveraddr.sin_family = AF_INET;

    /* Set address to localhost */
    err = inet_pton( AF_INET, hostname, &serveraddr.sin_addr );
    if( err != 1 )
    {
        char* reason = ( err == -1 ) ? strerror(errno) : "not parseable";
        fprintf( stderr, "%s:%d: Failed to convert %s to an IPv4 address: %s\n", __FILE__, __LINE__, hostname, reason );
        close( sock );
        return -1;
    }

    /* Set port number. Note that this value in particular needs to be in
     * network order. This is not the case for a lot of other functions.
     * read(2) and write(2), for example, are general system calls and take
     * a length value in regular host byte order. */
    serveraddr.sin_port = htons( port );

    /* Connect to the TCP server */
    err = connect( sock, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in) );
    if( err < 0 )
    {
        fprintf( stderr, "%s:%d: Failed to connect to %s:%d: %s\n", __FILE__, __LINE__, hostname, port, strerror(errno) );
        close( sock );
        return -1;
    }

    return sock;
}

int tcp_read( int sock, char* buffer, int n )
{
    int err;
    err = read( sock, buffer, n );
    if( err < 0 )
    {
        fprintf( stderr, "%s:%d Failed to read bytes from a TCP socket: %s\n", __FILE__, __LINE__, strerror(errno) );
    }
    return err;
}

int tcp_write( int sock, char* buffer, int bytes )
{
    int err;
    err = write( sock, buffer, bytes );
    if( err < 0 )
    {
        fprintf( stderr, "%s:%d Failed to write buffer to TCP socket: %s\n", __FILE__, __LINE__, strerror(errno) );
    }
    return err;
}

int tcp_write_loop( int sock, char* buffer, int bytes )
{
    int offset = 0;
    int err;
    while( ( offset < bytes ) && ( ( err = tcp_write( sock, &buffer[offset], bytes-offset ) ) > 0 ) )
    {
        offset = offset + err;
    }

    if( offset == bytes ) return bytes;

    return err;
}

void tcp_close( int sock )
{
    close( sock );
}

int tcp_create_and_listen( int port )
{
    int sock;
    int err;
    int yes;
    struct sockaddr_in serveraddr;

    /* Create socket */
    sock = socket( AF_INET, SOCK_STREAM, 0 );
    if( sock < 0 )
    {
        fprintf( stderr, "%s:%d: Failed to create a TCP socket: %s\n", __FILE__, __LINE__, strerror(errno) );
        return -1;
    }

    /* Initialize server address struct to zero */
    memset( &serveraddr, 0, sizeof(struct sockaddr_in) );

    serveraddr.sin_family      = AF_INET;       /* Set domain to Internet */
    serveraddr.sin_addr.s_addr = INADDR_ANY;    /* Permit connections from any address */
    serveraddr.sin_port        = htons( port ); /* Define the port */

    /* Attempt to bind the socket to this port */
    err = bind( sock, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in) );
    if( err < 0 )
    {
        fprintf( stderr, "%s:%d: Failed to bind TCP socket to localhost:%d: %s\n", __FILE__, __LINE__, port, strerror(errno) );
        close( sock );
        return -1;
    }

    /* Makes it so that you can reuse port immediately after previous user.
     * This is not required and not expected.
     */
    yes = 1;
	setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int) );

    /* Defining the queue length for TCP connect requests that arrive while this server thread
     * is busy.
     */
    err = listen( sock, BACKLOG_SIZE );
	if( err == -1 )
    {
		perror("Failed to listen to server socket");
		return -1;
	}

    return sock;
}

int tcp_accept( int server_sock )
{
    struct sockaddr_in addr;
    socklen_t addrlen;

    addrlen = sizeof(struct sockaddr_in);

    int new_sock = accept( server_sock,
                           (struct sockaddr*)&addr,
                           &addrlen );
    if (new_sock == -1)
    {
        fprintf( stderr, "%s:%d: Failed to accept TCP connections from another node: %s\n", __FILE__, __LINE__, strerror(errno) );
    }
    return new_sock;
}

int tcp_wait( fd_set* waiting_set, int wait_end )
{
    int err;

    err = select( wait_end, waiting_set, NULL, NULL, NULL );

    return err;
}

int tcp_wait_timeout( fd_set* waiting_set, int wait_end, int seconds )
{
    int err;
    
    struct timeval tv;
    tv.tv_sec  = seconds;
    tv.tv_usec = 0;

    err = select( wait_end, waiting_set, NULL, NULL, &tv );
    fprintf( stderr, "%s:%d select returned with %d\n", __FILE__, __LINE__, err );

    return err;
}

