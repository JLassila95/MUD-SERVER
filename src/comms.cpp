#if defined(__unix__) || defined(__unix)
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include <vector>
#include <cstdio>
#include <cstdlib>

#include "constants.h"

using namespace std;

int ctrlSock;
//Accepted connections
vector<int> conns;

void initComms()
{

  #if defined(_WIN32)
  WSADATA wsaData;
  int result;
  //Initialize Winsock
  result = WSAStartup(MAKEWORD(2,2), &wsaData);
  if(result != 0)
  {
    perror("WSAStartup failed");
    exit(EXIT_FAILURE);
  }
  #endif

  #if defined(__unix__) || defined(__unix)
  sockaddr_in addr_in;

  //Control socket creation
  ctrlSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(ctrlSock < 0)
  {
    perror("Control socket creation failed");
    exit(EXIT_FAILURE);
  }

  //Make the socket nonblocking
  if(fcntl(ctrlSock, F_SETFD, O_NONBLOCK, 1) < 0)
  {
    perror("Error manipulating control socket");
    exit(EXIT_FAILURE);
  }

  //Make control socket linger
  linger args = { 1, LINGER_TIMEOUT };
  if(setsockopt(ctrlSock, SOL_SOCKET, SO_LINGER, &args, sizeof(args)) < 0)
  {
    perror("Error setting control socket options");
    exit(EXIT_FAILURE);
  }

  //Bind control socket
  addr_in.sin_family = AF_INET;
  addr_in.sin_addr.s_addr = INADDR_ANY;
  addr_in.sin_port = htons( PORT );
  if( bind(ctrlSock, (sockaddr*)&addr_in, sizeof(addr_in)) < 0 )
  {
    perror("Binding control socket failed");
    exit(EXIT_FAILURE);
  }

  //Start listening for incoming connections
  if( listen(ctrlSock, BACKLOG) < 0 )
  {
    perror("Listening on control socket failed");
    exit(EXIT_FAILURE);
  }
  #endif

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
