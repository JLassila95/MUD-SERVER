
#if defined(_WIN32)
#define _WIN32_WINNT 0x0501
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(__unix__) || defined(__unix)
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "constants.h"

using namespace std;

#if defined(_WIN32)
SOCKET ctrlSock = INVALID_SOCKET;
#elif defined(__unix__) || defined(__unix)
int ctrlSock;
#endif
//Accepted connections
vector<int> conns;

void initComms()
{
  int iResult;

  #if defined(_WIN32)
  WSADATA wsaData;
  //Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if(iResult != 0)
  {
    perror("WSAStartup failed");
    exit(EXIT_FAILURE);
  }
  #endif

  struct addrinfo *result = NULL, hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;
  iResult = getaddrinfo(NULL, PORT, &hints, &result);
  if(iResult != 0)
  {
    perror("getaddrinfo failed");
    #if defined(_WIN32)
    WSACleanup();
    #endif
    exit(EXIT_FAILURE);
  }

  //Control socket creation
  ctrlSock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  #if defined(_WIN32)
  if(ctrlSock == INVALID_SOCKET)
  #elif defined(__unix__) || defined(__unix)
  if(ctrlSock < 0)
  #endif
  {
    perror("Failed to create control socket");
    freeaddrinfo(result);
    #if defined(_WIN32)
    WSACleanup();
    #endif
    exit(EXIT_FAILURE);
  }

  #if defined(__unix__) || defined(__unix)
  //Make the socket nonblocking
  iResult = fcntl(ctrlSock, F_SETFD, O_NONBLOCK, 1);
  if(iResult < 0)
  {
    perror("Error manipulating control socket");
    freeaddrinfo(result);
    close(ctrlSock);
    exit(EXIT_FAILURE);
  }
  //Make control socket linger
  linger args = { 1, LINGER_TIMEOUT };
  iResult = setsockopt(ctrlSock, SOL_SOCKET, SO_LINGER, &args, sizeof(args));
  if(iResult < 0)
  {
    perror("Error setting control socket options");
    freeaddrinfo(result);
    close(ctrlSock);
    exit(EXIT_FAILURE);
  }
  #endif

  //Bind control socket
  iResult = bind(ctrlSock, result->ai_addr, static_cast<int>(result->ai_addrlen));
  #if defined(_WIN32)
  if(iResult == SOCKET_ERROR)
  #elif defined(__unix__) || defined(__unix)
  if(iResult < 0)
  #endif
  {
    perror("Binding control socket failed");
    freeaddrinfo(result);
    #if defined(_WIN32)
    closesocket(ctrlSock);
    WSACleanup();
    #elif defined(__unix__) || defined(__unix)
    close(ctrlSock);
    #endif
    exit(EXIT_FAILURE);
  }
  freeaddrinfo(result);

  //Start listening for incoming connections
  iResult = listen(ctrlSock, SOMAXCONN);
  #if defined(_WIN32)
  if(iResult == SOCKET_ERROR)
  #elif defined(__unix__) || defined(__unix)
  if(iResult < 0)
  #endif
  {
    perror("Listening on control socket failed");
    #if defined(_WIN32)
    closesocket(ctrlSock);
    WSACleanup();
    #elif defined(__unix__) || defined(__unix)
    close(ctrlSock);
    #endif
    exit(EXIT_FAILURE);
  }

}

void closeComms()
{
  #if defined(__unix__) || defined(__unix)
  for(int sck : conns)
  {
    if(close(sck) < 0)
    {
      perror("Failed to close client connection");
    }
  }
  if(close(ctrlSock) < 0)
  {
    perror("Failed to close control socket");
  }
  #endif
}

void connectNewPlayer()
{
  #if defined(__unix__) || defined(__unix)
  int sck_in = accept(ctrlSock, nullptr, nullptr);
  if(sck_in < 0)
  {
    perror("Accepting new connection failed");
    return;
  }
  conns.push_back(sck_in);
  #endif
}
