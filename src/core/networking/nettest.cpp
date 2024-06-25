#include "nettest.h"

namespace jet
{

Server *Server::sCallbackInstance = nullptr;

void Server::StartConnection()
{
  sCallbackInstance = this;

  SteamDatagramErrMsg errMsg;
  if (!GameNetworkingSockets_Init(nullptr, errMsg))
  {
    Logger::Error("Server: {}", errMsg);
  }

  mNetSockets = SteamNetworkingSockets();

  SteamNetworkingIPAddr serverAddress;
  serverAddress.Clear();
  serverAddress.ParseString("0.0.0.0:27020");

  SteamNetworkingConfigValue_t opt;
  opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *)OnConnectionStatusChangedStatic);

  mListenSocket = mNetSockets->CreateListenSocketIP(serverAddress, 1, &opt);
  if (mListenSocket == k_HSteamListenSocket_Invalid)
  {
    Logger::Error("Failed to listen socket");
  }
  else
  {
    Logger::Info("Server: Listening on port {}", serverAddress.m_port);
  }
}

void Server::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo)
{
  Logger::Info("NetStatusChanged");

  if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_Connecting)
  {
    mNetSockets->AcceptConnection(pInfo->m_hConn);
  }
  else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_Connected)
  {
    Logger::Info("Server: Client connected!");
    mConnections[pInfo->m_hConn] = mNetSockets;
  }
  else if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ClosedByPeer ||
           pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
  {
    Logger::Error("Server: Client disconnected or problem detected: {}", pInfo->m_info.m_szEndDebug);
  }
}

void Server::Loop()
{
  mNetSockets->RunCallbacks();

  for (auto &[conn, sockets] : mConnections)
  {
    SteamNetworkingMessage_t *pIncommingMsg = nullptr;
    int numMsgs = sockets->ReceiveMessagesOnConnection(conn, &pIncommingMsg, 1);
    if (numMsgs > 0 && pIncommingMsg)
    {
      Logger::Info("Receive message {}", std::string((char *)pIncommingMsg->m_pData, pIncommingMsg->m_cbSize));
      pIncommingMsg->Release();
    }
  }
}

void Server::Kill()
{
  GameNetworkingSockets_Kill();
}

void Server::OnConnectionStatusChangedStatic(SteamNetConnectionStatusChangedCallback_t *pInfo)
{
  sCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
}

} // namespace jet
