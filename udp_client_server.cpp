//
//  udp_client_server.cpp
//  CPP_UDP_Client_Server
//
//  Created by Dariusz Adamczyk on 08/03/2021.
//  Copyright © 2021 Dariusz Adamczyk. All rights reserved.
//


#include "udp_client_server.h"
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>

# ifndef O_CLOEXEC
#  define O_CLOEXEC 02000000
# endif

# ifndef SOCK_CLOEXEC
#  define SOCK_CLOEXEC O_CLOEXEC
# endif

namespace udp_client_server
{


// ========================= CLIENT =========================

/** \brief Initialize a UDP client object.
 *
 * \param[in] addr  The address to convert to a numeric IP.
 * \param[in] port  The port number.
 */
udp_client::udp_client(const std::string& addr, int port)
    : f_port(port)
    , f_addr(addr)
{
    memset(&f_servaddr, 0, sizeof(f_servaddr));
    
    // Creating socket file descriptor
    if ( (f_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Filling server information
    f_servaddr.sin_family = AF_INET;
    f_servaddr.sin_port = htons(f_port);
    f_servaddr.sin_addr.s_addr = inet_addr(f_addr.c_str());
    
    struct timeval read_timeout;
    read_timeout.tv_sec = 0;
    read_timeout.tv_usec = 10000;
    setsockopt(f_socket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
    
    printf("UDP client has been created\n");
}

/** \brief Clean up the UDP client object.
 *
 * This function frees the address information structure and close the socket
 * before returning.
 */
udp_client::~udp_client()
{
    close(f_socket);
}

/** \brief Retrieve a copy of the socket identifier.
 *
 * This function return the socket identifier as returned by the socket()
 * function. This can be used to change some flags.
 *
 * \return The socket used by this UDP client.
 */
int udp_client::get_socket() const
{
    return f_socket;
}

/** \brief Retrieve the port used by this UDP client.
 *
 * This function returns the port used by this UDP client. The port is
 * defined as an integer, host side.
 *
 * \return The port as expected in a host integer.
 */
int udp_client::get_port() const
{
    return f_port;
}

/** \brief Retrieve a copy of the address.
 *
 * This function returns a copy of the address as it was specified in the
 * constructor. This does not return a canonalized version of the address.
 *
 * The address cannot be modified. If you need to send data on a different
 * address, create a new UDP client.
 *
 * \return A string with a copy of the constructor input address.
 */
std::string udp_client::get_addr() const
{
    return f_addr;
}

/** \brief Send a message through this UDP client.
 *
 * This function sends \p msg through the UDP client socket. The function
 * cannot be used to change the destination as it was defined when creating
 * the udp_client object.
 *
 * The size must be small enough for the message to fit. In most cases we
 * use these in Snap! to send very small signals (i.e. 4 bytes commands.)
 * Any data we would want to share remains in the Cassandra database so
 * that way we can avoid losing it because of a UDP message.
 *
 * \param[in] msg  The message to send.
 * \param[in] size  The number of bytes representing this message.
 *
 * \return -1 if an error occurs, otherwise the number of bytes sent. errno
 * is set accordingly on error.
 */
int udp_client::send(const char *msg, size_t size)
{
    return sendto(f_socket, msg, size, 0, (const struct sockaddr *) &f_servaddr, sizeof(f_servaddr));
}

/** \brief Wait on a message.
 *
 * This function waits until a message is received on this UDP server.
 * There are no means to return from this function except by receiving
 * a message. Remember that UDP does not have a connect state so whether
 * another process quits does not change the status of this UDP server
 * and thus it continues to wait forever.
 *
 * Note that you may change the type of socket by making it non-blocking
 * (use the get_socket() to retrieve the socket identifier) in which
 * case this function will not block if no message is available. Instead
 * it returns immediately.
 *
 * \param[in] msg  The buffer where the message is saved.
 * \param[in] max_size  The maximum size the message (i.e. size of the \p msg buffer.)
 *
 * \return The number of bytes read or -1 if an error occurs.
 */
int udp_client::recv(char *msg, size_t max_size)
{
   return recvfrom(f_socket, msg, max_size, MSG_WAITALL, ( struct sockaddr *) &f_cliaddr, &len);
   
}



// ========================= SEVER =========================

/** \brief Initialize a UDP server object.
 *
 *
 * \param[in] addr  The address we receive on.
 * \param[in] port  The port we receive from.
 */
udp_server::udp_server(const std::string& addr, int port)
    : f_port(port)
    , f_addr(addr)
{
    // Create socket file descriptor
    if ( (f_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Fill server information
    f_servaddr.sin_family = AF_INET;
    f_servaddr.sin_port = htons(f_port);
    f_servaddr.sin_addr.s_addr = inet_addr(f_addr.c_str());
      
    // Bind the socket with the server address
    if ( bind(f_socket, (const struct sockaddr *)&f_servaddr, sizeof(f_servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    len = sizeof(f_cliaddr);
    
    printf("UDP client has been created on: %s : %d\n", f_addr.c_str(), f_port);
}

/** \brief Clean up the UDP server.
 *
 * This function frees the address info structures and close the socket.
 */
udp_server::~udp_server()
{
    close(f_socket);
}

/** \brief The socket used by this UDP server.
 *
 * This function returns the socket identifier. It can be useful if you are
 * doing a select() on many sockets.
 *
 * \return The socket of this UDP server.
 */
int udp_server::get_socket() const
{
    return f_socket;
}

/** \brief The port used by this UDP server.
 *
 * This function returns the port attached to the UDP server. It is a copy
 * of the port specified in the constructor.
 *
 * \return The port of the UDP server.
 */
int udp_server::get_port() const
{
    return f_port;
}

/** \brief Return the address of this UDP server.
 *
 * This function returns a verbatim copy of the address as passed to the
 * constructor of the UDP server (i.e. it does not return the canonalized
 * version of the address.)
 *
 * \return The address as passed to the constructor.
 */
std::string udp_server::get_addr() const
{
    return f_addr;
}

/** \brief Return the address of lasr UDP client.
 *
 * \return The client address
 */
char * udp_server::get_client_addr() const
{
    return inet_ntoa(f_cliaddr.sin_addr);
}

/** \brief Wait on a message.
 *
 * This function waits until a message is received on this UDP server.
 * There are no means to return from this function except by receiving
 * a message. Remember that UDP does not have a connect state so whether
 * another process quits does not change the status of this UDP server
 * and thus it continues to wait forever.
 *
 * Note that you may change the type of socket by making it non-blocking
 * (use the get_socket() to retrieve the socket identifier) in which
 * case this function will not block if no message is available. Instead
 * it returns immediately.
 *
 * \param[in] msg  The buffer where the message is saved.
 * \param[in] max_size  The maximum size the message (i.e. size of the \p msg buffer.)
 *
 * \return The number of bytes read or -1 if an error occurs.
 */
int udp_server::recv(char *msg, size_t max_size)
{
    return recvfrom(f_socket, msg, max_size, MSG_WAITALL, ( struct sockaddr *) &f_cliaddr, &len);
}

/** \brief Send a message through this UDP server.
 *
 * This function sends \p msg through the UDP server socket. The function
 * cannot be used to change the destination as it was defined when creating
 * the udp_client object.
 *
 * The size must be small enough for the message to fit. In most cases we
 * use these in Snap! to send very small signals (i.e. 4 bytes commands.)
 * Any data we would want to share remains in the Cassandra database so
 * that way we can avoid losing it because of a UDP message.
 *
 * \param[in] msg  The message to send.
 * \param[in] size  The number of bytes representing this message.
 *
 * \return -1 if an error occurs, otherwise the number of bytes sent. errno
 * is set accordingly on error.
 */
int udp_server::reply(const char *msg, size_t size)
{
    return sendto(f_socket, msg, size, 0, (const struct sockaddr *) &f_cliaddr, sizeof(f_cliaddr));
}

/** \brief Wait for data to come in. [TODO]
 *
 * This function waits for a given amount of time for data to come in. If
 * no data comes in after max_wait_ms, the function returns with -1 and
 * errno set to EAGAIN.
 *
 * The socket is expected to be a blocking socket (the default,) although
 * it is possible to setup the socket as non-blocking if necessary for
 * some other reason.
 *
 * This function blocks for a maximum amount of time as defined by
 * max_wait_ms. It may return sooner with an error or a message.
 *
 * \param[in] msg  The buffer where the message will be saved.
 * \param[in] max_size  The size of the \p msg buffer in bytes.
 * \param[in] max_wait_ms  The maximum number of milliseconds to wait for a message.
 *
 * \return -1 if an error occurs or the function timed out, the number of bytes received otherwise.
 */
int udp_server::timed_recv(char *msg, size_t max_size, int max_wait_ms)
{
    return 0;
}

/** \brief Get machine local IP address
*  \return pointer to the char array with machine ip address
*/
const char * udp_helpers::get_local_ip_address(void)
{
    static char w[128];
    
    int c,i=0;
    
    FILE *f=popen("ip a | grep 'scope global' | grep -v ':' | awk '{print $2}' | cut -d '/' -f1","r");
    
    while((c=getc(f))!=EOF)i+=sprintf(w+i,"%c",c);
    
    pclose(f);
    
    return w;
}

} // namespace udp_client_server

