
#if defined(_WIN32)
#define _WIN32_WINNT 0x0501
#include <winsock2.h>
#include <ws2tcpip.h>
#elif defined(__unix__) || defined(__unix)
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "constants.h"
#include "client.h"

using namespace std;

#if defined(_WIN32)
SOCKET ctrlSock = INVALID_SOCKET;
#elif defined(__unix__) || defined(__unix)
int ctrlSock;
#endif
//Accepted connections
vector<Client*> clients;

void initComms()
{
  printf("Initializing communications...");

  int iResult;

  #if defined(_WIN32)
  WSADATA wsaData;
  //Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if(iResult != 0)
  {
    printf("WSAStartup failed: %d\n", iResult);
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
    #if defined(_WIN32)
    printf("getaddrinfo failed: %d\n", WSAGetLastError());
    WSACleanup();
    #elif defined(__unix__) || defined(__unix)
    perror("getaddrinfo failed");
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
    #if defined(_WIN32)
    printf("socket failed: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    #elif defined(__unix__) || defined(__unix)
    perror("socket failed");
    freeaddrinfo(result);
    #endif
    exit(EXIT_FAILURE);
  }

  
  //Make the socket nonblocking
  #if defined(_WIN32)
  u_long iMode = 1;
  iResult = ioctlsocket(ctrlSock, FIONBIO, &iMode);
  if(iResult == SOCKET_ERROR)
  {
    printf("ioctlsocket failed: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(ctrlSock);
    WSACleanup();
    exit(EXIT_FAILURE);
  }
  #elif defined(__unix__) || defined(__unix)
  iResult = fcntl(ctrlSock, F_SETFD, O_NONBLOCK, 1);
  if(iResult < 0)
  {
    perror("fcntl failed");
    freeaddrinfo(result);
    close(ctrlSock);
    exit(EXIT_FAILURE);
  }
  #endif

  //Make control socket linger
  linger args = { 1, LINGER_TIMEOUT };
  iResult = setsockopt(ctrlSock, SOL_SOCKET, SO_LINGER, (char*)&args, sizeof(args));
  #if defined(_WIN32)
  if(iResult == SOCKET_ERROR)
  #elif defined(__unix__) || defined(__unix)
  if(iResult < 0)
  #endif
  {
    #if defined(_WIN32)
    printf("setsockopt failed: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(ctrlSock);
    WSACleanup();
    #elif defined(__unix__) || defined(__unix)
    perror("setsockopt failed");
    freeaddrinfo(result);
    close(ctrlSock);
    #endif
    exit(EXIT_FAILURE);
  }

  //Bind control socket
  iResult = bind(ctrlSock, result->ai_addr, static_cast<int>(result->ai_addrlen));
  #if defined(_WIN32)
  if(iResult == SOCKET_ERROR)
  #elif defined(__unix__) || defined(__unix)
  if(iResult < 0)
  #endif
  {
    #if defined(_WIN32)
    printf("bind failed: %d\n", WSAGetLastError());
    freeaddrinfo(result);
    closesocket(ctrlSock);
    WSACleanup();
    #elif defined(__unix__) || defined(__unix)
    perror("bind failed");
    freeaddrinfo(result);
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
    #if defined(_WIN32)
    printf("listen failed: %d\n", WSAGetLastError());
    closesocket(ctrlSock);
    WSACleanup();
    #elif defined(__unix__) || defined(__unix)
    perror("listen failed");
    close(ctrlSock);
    #endif
    exit(EXIT_FAILURE);
  }

  printf("READY\n");
}

void closeComms()
{
  printf("Closing communications...");
  for(Client* client : clients)
  {
    int sck = client->getSocket();
    #if defined(_WIN32)
    closesocket(sck);
    #elif defined(__unix__) || defined(__unix)
    close(sck);
    #endif
    delete(client);
  }
  #if defined(_WIN32)
  closesocket(ctrlSock);
  WSACleanup();
  #elif defined(__unix__) || defined(__unix)
  close(ctrlSock);
  #endif
  printf("READY\n");
}

void processNewConnections()
{
  sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  #if defined(_WIN32)
  SOCKET clientSock;
  #elif defined(__unix__) || defined(__unix)
  int clientSock;
  #endif
  clientSock = accept(ctrlSock, (sockaddr*) &clientAddr, &clientAddrLen);
  #if defined(_WIN32)
  int err = WSAGetLastError();
  if(clientSock == INVALID_SOCKET && err != WSAEWOULDBLOCK)
  #elif defined(__unix__) || defined(__unix)
  if(clientSock < 0)
  #endif
  {
    #if defined(_WIN32)
    printf("accept failed: %d\n", err);
    #elif defined(__unix__) || defined(__unix)
    perror("accept failed");
    #endif
    return;
  }

  Client* newClient = new Client(inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port), clientSock);
  printf("New connection from address %s port %d\n", newClient->getAddress().data(), newClient->getPort());
  clients.push_back(newClient);
}
