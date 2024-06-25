#pragma once

#include "GameNetworkingSockets/steam/isteamnetworkingutils.h"
#include "core/logger.h"
#include "core/types.h"
#include <GameNetworkingSockets/steam/isteamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/steamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/steamnetworkingtypes.h>
#include <string>
#include <unordered_map>

namespace jet
{

class Server
{
public:
  void StartConnection();

  void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo);
  static void OnConnectionStatusChangedStatic(SteamNetConnectionStatusChangedCallback_t *pInfo);

  void Loop();
  void Kill();

private:
  ISteamNetworkingSockets *mNetSockets{};
  HSteamListenSocket mListenSocket;
  std::unordered_map<HSteamNetConnection, ISteamNetworkingSockets *> mConnections;
  static Server *sCallbackInstance;
};

class Client
{
  static inline void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
  {
    if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected)
    {
      Logger::Info("Connected to server!");
    }
    else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer ||
             pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
    {
      Logger::Error("Disconnected from server or problem detected: {}", pInfo->m_info.m_szEndDebug);
    }
  }

public:
  inline void ConnectToServer(std::string address)
  {
    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg))
    {
      Logger::Error("Server: {}", errMsg);
    }

    mNetSockets = SteamNetworkingSockets();

    SteamNetworkingIPAddr serverAddress;
    serverAddress.Clear();
    serverAddress.ParseString(address.c_str());

    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *)OnSteamNetConnectionStatusChanged);

    mConnection = mNetSockets->ConnectByIPAddress(serverAddress, 1, &opt);
    if (mConnection == k_HSteamNetConnection_Invalid)
    {
      Logger::Error("Failed to connect to server");
    }
  }

  inline void Loop()
  {
    mNetSockets->RunCallbacks();

    SteamNetworkingMessage_t *pIncommingMsg = nullptr;
    int numMsgs = mNetSockets->ReceiveMessagesOnConnection(mConnection, &pIncommingMsg, 1);
    if (numMsgs > 0 && pIncommingMsg)
    {
      Logger::Info("Received message: {}", pIncommingMsg->m_pData);
      pIncommingMsg->Release();
    }

    mNetSockets->SendMessageToConnection(mConnection, "Hello\n", 6, k_nSteamNetworkingSend_Reliable, nullptr);
  }

  inline void Kill()
  {
    GameNetworkingSockets_Kill();
  }

private:
  ISteamNetworkingSockets *mNetSockets;
  HSteamNetConnection mConnection;
};

} // namespace jet
