// Copyright: 2020, Diez B. Roggisch, Berlin, all rights reserved
#include "nanomsg-helper.hpp"

#include <iostream>
#include <cassert>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>


std::tuple<int, int> open_nanomsg_pair_server(const std::string& uri)
{
  int socket = nn_socket(AF_SP, NN_PAIR);
  assert(socket >= 0);
  int endpoint = nn_bind (socket, uri.c_str());
  if(endpoint == -1)
  {
    std::cerr << "nn_bind error: " << errno << "\n";
    switch(errno)
    {
    case EBADF:
      std::cerr << "errno: EBADF\n";
      break;
    case EMFILE:
      std::cerr << "errno: EMFILE\n";
      break;
    case EINVAL:
      std::cerr << "errno: EINVAL\n";
      break;
    case ENAMETOOLONG:
      std::cerr << "errno: ENAMETOOLONG\n";
      break;
    case EPROTONOSUPPORT:
      std::cerr << "errno: EPROTONOSUPPORT\n";
      break;
    case EADDRNOTAVAIL:
      std::cerr << "errno: EADDRNOTAVAIL\n";
      break;
    case ENODEV:
      std::cerr << "errno: ENODEV\n";
      break;
    case EADDRINUSE:
      std::cerr << "errno: EADDRINUSE\n";
      break;
    case ETERM:
      std::cerr << "errno: ETERM\n";
      break;
    }
    assert(false);
  }
  return { socket, endpoint };
}
