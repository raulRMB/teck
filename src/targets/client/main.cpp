/*#include "core/engine.h"*/
#include "modules/networking/client-connection.h"
#include "networking/client-connection.h"

int main(int argc, char *argv[])
{
  tk::net::ClientConnection conn = tk::net::ClientConnection();
  conn.ConnectToServer("127.0.0.1:27020");
  while (true)
  {
    conn.Loop();
  }
  conn.Kill();
}
