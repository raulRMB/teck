#include "client_connection.h"
#include "core/logger.h"
#include <GameNetworkingSockets/steam/steamnetworkingsockets.h>

namespace tk::net
{

void ClientConnection::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
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

void ClientConnection::ConnectToServer(std::string address)
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
  opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnSteamNetConnectionStatusChanged);

  mConnection = mNetSockets->ConnectByIPAddress(serverAddress, 1, &opt);
  if (mConnection == k_HSteamNetConnection_Invalid)
  {
    Logger::Error("Failed to connect to server");
  }
}

void ClientConnection::Loop()
{
  mNetSockets->RunCallbacks();

  SteamNetworkingMessage_t* pIncommingMsg = nullptr;
  int numMsgs = mNetSockets->ReceiveMessagesOnConnection(mConnection, &pIncommingMsg, 1);
  if (numMsgs > 0 && pIncommingMsg)
  {
    Logger::Info("Received message: {}", pIncommingMsg->m_pData);
    pIncommingMsg->Release();
  }

  SendExitMessage();
}

void ClientConnection::SendExitMessage()
{
  std::string message("exit");
  mNetSockets->SendMessageToConnection(mConnection, message.c_str(), message.size(), k_nSteamNetworkingSend_Reliable,
                                       nullptr);
}

void ClientConnection::Kill()
{
  GameNetworkingSockets_Kill();
}

} // namespace tk::net
