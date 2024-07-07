#include "../../modules/networking/server-connection.h"

int main()
{
  tk::net::ServerConnection conn;
  conn.StartConnection();
  while (conn.Loop())
    ;
  conn.Kill();
}
