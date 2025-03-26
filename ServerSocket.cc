/*
 * Copyright ©2025 Hal Perkins.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Winter Quarter 2025 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <stdio.h>       // for snprintf()
#include <unistd.h>      // for close(), fcntl()
#include <sys/types.h>   // for socket(), getaddrinfo(), etc.
#include <sys/socket.h>  // for socket(), getaddrinfo(), etc.
#include <arpa/inet.h>   // for inet_ntop()
#include <netdb.h>       // for getaddrinfo()
#include <errno.h>       // for errno, used by strerror()
#include <string.h>      // for memset, strerror()
#include <iostream>      // for std::cerr, etc.

#include "./ServerSocket.h"

extern "C" {
  #include "libhw1/CSE333.h"
}

using std::string;
using std::to_string;
namespace hw4 {

ServerSocket::ServerSocket(uint16_t port) {
  port_ = port;
  listen_sock_fd_ = -1;
}

ServerSocket::~ServerSocket() {
  // Close the listening socket if it's not zero.  The rest of this
  // class will make sure to zero out the socket if it is closed
  // elsewhere.
  if (listen_sock_fd_ != -1)
    close(listen_sock_fd_);
  listen_sock_fd_ = -1;
}

bool ServerSocket::BindAndListen(int ai_family, int *const listen_fd) {
  // Use "getaddrinfo," "socket," "bind," and "listen" to
  // create a listening socket on port port_.  Return the
  // listening socket through the output parameter "listen_fd"
  // and set the ServerSocket data member "listen_sock_fd_"

  // Use the logic of Listen code in lecture
  // STEP 1:
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = ai_family;
  hints.ai_socktype = SOCK_STREAM;  // stream
  hints.ai_flags = AI_PASSIVE;      // use wildcard "in6addr_any" address
  hints.ai_flags |= AI_V4MAPPED;    // use v4-mapped v6 if no v6 found
  hints.ai_protocol = IPPROTO_TCP;  // tcp protocol
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  struct addrinfo *result;
  if (getaddrinfo(nullptr, to_string(port_).c_str(), &hints, &result) !=
      0) {
    return false;
  }
  int socket_fd = -1;
  for (struct addrinfo *temp = result; temp != nullptr; temp = temp->ai_next) {
    // Create a socket and bind it
    // if invalid socket then move onto the next address in the loop
    if ((socket_fd = socket(temp->ai_family, temp->ai_socktype,
      temp->ai_protocol)) == -1) {
      continue;
    }
    int optval = -1;
    Verify333(setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &optval,
                         sizeof(optval)) == 0);
    // if bind works then set approp return values and break loop
    if (bind(socket_fd, temp->ai_addr, temp->ai_addrlen) == 0) {
      sock_family_ = temp->ai_family;
      break;
    }

    // bind fails, close socket, and move onto next one
    close(socket_fd);
    socket_fd = -1;
  }
  freeaddrinfo(result);

  // now want to listen to the socket
  // note edge case here is if last addr checked
  // socket_fd is -1, so we need to check for this
  // before we listrn to socket
  if (socket_fd == -1) {
    return false;
  }
  // fail listen needs to close socket and return false
  if (listen(socket_fd, SOMAXCONN) == -1) {
    close(socket_fd);
    return false;
  }
  *listen_fd = socket_fd;
  listen_sock_fd_ = socket_fd;
  return true;
}

bool ServerSocket::Accept(int *const accepted_fd,
                          string *const client_addr,
                          uint16_t *const client_port,
                          string *const client_dns_name,
                          string *const server_addr,
                          string *const server_dns_name) const {
  // STEP 2:
  // First, I am going to loop until I can accept a connection
  // from a client
  // has to handle both IPv4 and IPv6
  // Code is adapted from lecture
  struct sockaddr_storage caddr;
  int client_fd;
  socklen_t caddr_len = sizeof(caddr);
  while (1) {
    client_fd =
        accept(listen_sock_fd_, reinterpret_cast<struct sockaddr *>(&caddr),
               &caddr_len);
    if (client_fd < 0) {
      if ((errno == EINTR) || (errno == EAGAIN) || errno == (EWOULDBLOCK)) {
        continue;
      }
      // At this point cannot establish, so need to return false
      return false;
    }
    break;
  }

  // At this point connection handled, so have to
  // update the accepted_fd
  *accepted_fd = client_fd;

  // Now need to see if IP address is
  // IPv4 or IPv6 and then set the client_addr
  // and client_port accordingly
  if (caddr.ss_family == AF_INET) {  // IPv4
    char ipstring[INET_ADDRSTRLEN];
    struct sockaddr_in *v4addr = reinterpret_cast<struct sockaddr_in*>(&caddr);
    inet_ntop(AF_INET, &(v4addr->sin_addr), ipstring, INET_ADDRSTRLEN);
    *client_addr = string(ipstring);
    *client_port = ntohs(v4addr->sin_port);
  } else {  // IPv6
    char ipstring[INET6_ADDRSTRLEN];
    struct sockaddr_in6 *v6addr =
      reinterpret_cast<struct sockaddr_in6*>(&caddr);
    inet_ntop(AF_INET6, &(v6addr->sin6_addr), ipstring, INET6_ADDRSTRLEN);
    *client_addr = string(ipstring);
    *client_port = ntohs(v6addr->sin6_port);
  }

  // Get client DNS name
  char dns[1024];
  Verify333(getnameinfo(reinterpret_cast<struct sockaddr*>(&caddr),
    caddr_len, dns, sizeof(dns), NULL, 0, 0) == 0);
  *client_dns_name = string(dns);

  // Get client server information
  char hname[1024];
  hname[0] = '\0';
  // server using IPv4 addr
  if (sock_family_ == AF_INET) {
    struct sockaddr_in srvr;
    socklen_t srvr_len = sizeof(srvr);
    char addrbuf[INET_ADDRSTRLEN];
    getsockname(client_fd, (struct sockaddr *) &srvr, &srvr_len);
    inet_ntop(AF_INET, &srvr.sin_addr, addrbuf, INET_ADDRSTRLEN);
    getnameinfo((const struct sockaddr *) &srvr,
                srvr_len, hname, 1024, nullptr, 0, 0);
    *server_addr = string(addrbuf);
    *server_dns_name = string(hname);
  } else {
      // The server is using an IPv6 address.
      struct sockaddr_in6 srvr;
      socklen_t srvrlen = sizeof(srvr);
      char addrbuf[INET6_ADDRSTRLEN];
      getsockname(client_fd, (struct sockaddr *)&srvr, &srvrlen);
      inet_ntop(AF_INET6, &srvr.sin6_addr, addrbuf, INET6_ADDRSTRLEN);
      getnameinfo((const struct sockaddr *)&srvr, srvrlen, hname, 1024, nullptr,
                  0, 0);
      *server_addr = string(addrbuf);
      *server_dns_name = string(hname);
  }
  return true;
}

}  // namespace hw4
