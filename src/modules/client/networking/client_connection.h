#ifndef TECK_NET_CLIENT_CONNECTION_H
#define TECK_NET_CLIENT_CONNECTION_H

#include "modules/common/networking/connection.h"
#include <GameNetworkingSockets/steam/isteamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>
#include <GameNetworkingSockets/steam/steamnetworkingtypes.h>
#include <string>

namespace tk::net
{

class ClientConnection : public Connection
{
  static inline void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);

public:
  void ConnectToServer(std::string address);
  void Loop();
  void Kill();

  void SendExitMessage();

private:
  ISteamNetworkingSockets* mNetSockets;
  HSteamNetConnection mConnection;
};

} // namespace tk::net

#endif // TECK_NET_CLIENT_CONNECTION_H
