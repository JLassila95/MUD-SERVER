#include <iostream>

#include "comms.h"

using namespace std;

int main(int argc, char* argv[])
{
  initComms();
  processNewConnections();
  closeComms();
  return 0;
}
