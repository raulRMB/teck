#ifndef TECK_NET_SERVER_CONNECTION_H
#define TECK_NET_SERVER_CONNECTION_H

#include "../../common/networking/connection.h"
#include <GameNetworkingSockets/steam/isteamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>
#include <unordered_map>

namespace tk::net
{

class ServerConnection : public Connection
{
public:
  void StartConnection();

  void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
  static void OnConnectionStatusChangedStatic(SteamNetConnectionStatusChangedCallback_t* pInfo);

  bool Loop();
  void Kill();

private:
  ISteamNetworkingSockets* mNetSockets{};
  HSteamListenSocket mListenSocket;
  std::unordered_map<HSteamNetConnection, ISteamNetworkingSockets*> mConnections;
  static ServerConnection* sCallbackInstance;
};

} // namespace tk::net

#endif // TECK_NET_SERVER_CONNECTION_H
