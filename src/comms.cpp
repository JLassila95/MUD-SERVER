#include <vector>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "constants.h"

using namespace std;

int ctrlSock;
//Accepted connections
vector<int> conns;

void initComms()
{
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

}

void closeComms()
{
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
}

void connectNewPlayer()
{
  int sck_in = accept(ctrlSock, nullptr, nullptr);
  if(sck_in < 0)
  {
    perror("Accepting new connection failed");
    return;
  }
  conns.push_back(sck_in);
}
