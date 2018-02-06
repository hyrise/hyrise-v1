#include "io/BufferedLogger.h"
#include "net/ClusterInterface.h"
#include "net/ClusterManager.h"
#include "helper/types.h"
#include "helper/Settings.h"

#include <nanomsg/nn.h>
#include <nanomsg/survey.h>
#include <iostream>
#include <unistd.h>
#include <thread>

namespace hyrise {
namespace net {

ClusterInterface& ClusterInterface::getInstance() {
  static ClusterInterface instance;

  return instance;
}

void heartbeatRespond(std::string master_url, size_t port) {
    int64_t sock = nn_socket(AF_SP, NN_RESPONDENT);
    assert(sock >= 0);

    std::string address = "tcp://" + master_url + ":" + std::to_string(port);

    nn_connect(sock, address.c_str());

    bool firstTime = true;

    char receiveBuffer[8];

    std::this_thread::yield();

    while (1) {
        int64_t sbytes = nn_recv (sock, receiveBuffer, 8, 0);
        if (sbytes >= 0) {
            if (firstTime) {
              net::ClusterInterface::getInstance().updateLastMasterHeartbeat();
              send(sock, MessageType::HELLO, nullptr, 0);
              firstTime = false;
            } else {
              net::ClusterInterface::getInstance().updateLastMasterHeartbeat();

              tx::transaction_cid_t currentCid = tx::TransactionManager::getInstance().getLastCommitId();
              auto currentCidAsString = std::to_string(currentCid);

              send(sock, MessageType::HEARTBEAT_REPLY, currentCidAsString.c_str(), currentCidAsString.length());
            }


        } else {
            printf("Some error? Or just termination?\n");
        }
    }

    nn_close(sock);
}

void heartbeatRequest(std::string master_url, size_t port) {
    int64_t sock = nn_socket(AF_SP, NN_SURVEYOR);
    assert(sock >= 0);

    std::string address = "tcp://0.0.0.0:" + std::to_string(port);

    int64_t socketId = nn_bind(sock, address.c_str());
    assert(socketId >= 0);

    const int HEARTBEAT_INTERVAL = 200;
    nn_setsockopt(sock, NN_SURVEYOR, NN_SURVEYOR_DEADLINE, &HEARTBEAT_INTERVAL, sizeof(HEARTBEAT_INTERVAL));


    char receiveBuffer[8];

    while (1) {
        send(sock, MessageType::HEARTBEAT_REQUEST, nullptr, 0);

        while (1) {
            int64_t bytes = nn_recv(sock, receiveBuffer, 8, 0);
            if (bytes >= 0) {
                std::string message = std::string(static_cast<char*>(receiveBuffer), bytes);
                auto messageType = message[0];
                size_t senderId = message[1];
                auto payload = message.substr(MESSAGE_BUFFER_RESERVED_BYTES);

                if (messageType == MessageType::HELLO) {
                  std::cout << "GOT new client!" << std::endl;
                  net::ClusterInterface::getInstance().addNode(senderId);
                } else if (messageType == MessageType::HEARTBEAT_REPLY) {
                  net::ClusterInterface::getInstance().updateNode(senderId, payload);
                }
            } else {
                if (errno == EFSM || errno == ETIMEDOUT) {
                    break;
                }
            }
        }
    }
}

void logdataHandling(size_t port) {
  int64_t sock = nn_socket(AF_SP, NN_RESPONDENT);
  assert(sock >= 0);

  std::string address = "tcp://" + Settings::getInstance()->master_url+ ":" + std::to_string(port);

  nn_connect(sock, address.c_str());

  char receiveBuffer[SEND_RECEIVE_BUF_SIZE];

  while (1) {
    int64_t bytes = nn_recv(sock, receiveBuffer, SEND_RECEIVE_BUF_SIZE, 0);

    if (bytes >= 0) {
      const char messageType = receiveBuffer[0];

      if (messageType == MessageType::LOG_ATTACHED) {
        send(sock, MessageType::LOG_RECEIVED, nullptr, 0);
        io::BufferedLogger::getInstance().replicate(&receiveBuffer[MESSAGE_BUFFER_RESERVED_BYTES], bytes - MESSAGE_BUFFER_RESERVED_BYTES);
      }
    }
  }
}

void ClusterInterface::start() {
    initControlInterface();
    initDataInterface();
}

void ClusterInterface::initControlInterface() {
  if (_masternode)
    _controlInterfaceThread = std::make_shared<std::thread>(heartbeatRequest, _master_url, 5556 + _nodeId * 2);
  else
    _controlInterfaceThread = std::make_shared<std::thread>(heartbeatRespond, _master_url, 5556 + _currentMasternodeId * 2);
}

void ClusterInterface::initDataInterface() {
  if (_masternode) {
    _dataSocket = nn_socket(AF_SP, NN_SURVEYOR);
    assert(_dataSocket >= 0);

    std::string address = "tcp://*:" + std::to_string(5555 + _nodeId * 2);

    nn_bind(_dataSocket, address.c_str());

    _sendLogFlushThread = std::make_shared<std::thread>(&ClusterInterface::flushSendLog, this);
  } else
    _dataInterfaceThread = std::make_shared<std::thread>(logdataHandling, 5555 + _currentMasternodeId * 2);
}

ClusterInterface::ClusterInterface() {}

void ClusterInterface::flushSendLog() {
  std::chrono::milliseconds flushInterval(CHECK_LOG_FORCE_SEND_INTERVAL);

  while (1) {
    std::this_thread::sleep_for(flushInterval);

    std::lock_guard<std::mutex> lock(_sendMessageLock);
    if (_currentBufferSize > MESSAGE_BUFFER_RESERVED_BYTES && std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count() - _lastLogMessage > LOG_FORCE_SEND_THRESHOLD) {
      std::cout << "Flushing now!" << std::endl;

      internalSend(MessageType::LOG_ATTACHED);
    }
  }
}

void send(int64_t socket, MessageType type, const char* payload, size_t payloadSize) {
  if (type < MessageType::FIRST || type > MessageType::LAST) {
    std::cout << "Unsupported MessageType!" << std::endl;
    return;
  }

  char messageBuffer[16];

  messageBuffer[0] = type;
  messageBuffer[1] = ClusterInterface::getInstance().getNodeId();
  memcpy(&messageBuffer[MESSAGE_BUFFER_RESERVED_BYTES], payload, payloadSize);

  const int status = nn_send(socket, messageBuffer, payloadSize + MESSAGE_BUFFER_RESERVED_BYTES, 0);

  if (status != payloadSize + MESSAGE_BUFFER_RESERVED_BYTES)
    std::cout << "Error while sending!" << std::endl;
}

void ClusterInterface::internalSend(MessageType type) {
  _messageBuffer[0] = type;
  _messageBuffer[1] = _nodeId;

  const int status = nn_send(_dataSocket, _messageBuffer, _currentBufferSize, 0);

  char receiveBuffer[8];

  for (size_t i = 0; i < _nodes.size(); ++i) {
    int bytes = nn_recv(_dataSocket, receiveBuffer, 8, 0);
    if (bytes >= 0) {
      const char messageType = receiveBuffer[0];

      if (messageType != MessageType::LOG_RECEIVED)
        std::cout << "Wrong reply for log message!" << std::endl;
    } else {
      if (errno == EFSM || errno == ETIMEDOUT) {
        break;
      }
    }
  }

  if (status != _currentBufferSize)
    std::cout << "Error while sending!" << std::endl;

  _currentBufferSize = MESSAGE_BUFFER_RESERVED_BYTES;

  _lastLogMessage = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

void ClusterInterface::send(MessageType type, const char* payload, size_t payloadSize) {
  if (type < MessageType::FIRST || type > MessageType::LAST) {
    std::cout << "Unsupported MessageType!" << std::endl;
    return;
  }

  std::lock_guard<std::mutex> lock(_sendMessageLock);

  if (_currentBufferSize + payloadSize > SEND_RECEIVE_BUF_SIZE) {
    // std::cout << "_messageBuffer size is getting too big! Forcing message sending!" << std::endl;

    internalSend(type);

    ++_logMessageCount;
  }

  memcpy(&_messageBuffer[_currentBufferSize], payload, payloadSize);
  _currentBufferSize += payloadSize;

  if (_logMessageCount % SEND_LOG_EVERY_N == (SEND_LOG_EVERY_N - 1)) {
    internalSend(type);
  }

  ++_logMessageCount;
}

void ClusterInterface::resetConnection() {
  pthread_cancel(_controlInterfaceThread->native_handle());
  _controlInterfaceThread->join();

  pthread_cancel(_dataInterfaceThread->native_handle());
  _dataInterfaceThread->join();
}

void ClusterInterface::addNode(size_t nodeId) {
  std::lock_guard<std::mutex> lock(_nodesLock);
  _nodes[nodeId] = {std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(), 0};
}

void ClusterInterface::updateNode(size_t nodeId, std::string transactionCid) {
  ++_heartbeats;

  std::lock_guard<std::mutex> lock(_nodesLock);
  _nodes[nodeId].lastHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
  _nodes[nodeId].lastCID = std::stoi(transactionCid);

  if (_heartbeats % (20 * _nodes.size()) == 0) {
    std::cout << "My Last CID: " << tx::TransactionManager::getInstance().getLastCommitId() << std::endl;

    for (auto kv : _nodes) {
      std::cout << kv.first << ": Last heartbeat: " << kv.second.lastHeartbeat << " Last CID: " << kv.second.lastCID << std::endl;
    }
    _heartbeats = 0;
  }
}

bool ClusterInterface::getMasternode() {
  return _masternode;
}

size_t ClusterInterface::getNodeId() {
  return _nodeId;
}

void ClusterInterface::setNodeId(size_t nodeId) {
  _nodeId = nodeId;

  if (_currentMasternodeId != _nodeId)
    ClusterManager::getInstance();
}

void ClusterInterface::setMasterUrl(std::string master_url) {
  _master_url = master_url;
}

std::map<size_t, node>& ClusterInterface::getNodes() {
  return _nodes;
}

std::mutex& ClusterInterface::getNodesLock() {
  return _nodesLock;
}

size_t ClusterInterface::getCurrentMasternodeId() {
  return _currentMasternodeId;
}

void ClusterInterface::setCurrentMasternodeId(size_t currentMasterNodeId) {
  _currentMasternodeId = currentMasterNodeId;

  if (_currentMasternodeId == _nodeId)
    _masternode = true;
}

int64_t ClusterInterface::getLastMasterHeartbeat() {
  std::lock_guard<std::mutex> lock(_lastMasterHeartbeatLock);
  return _lastMasterHeartbeat;
}

void ClusterInterface::updateLastMasterHeartbeat() {
  std::lock_guard<std::mutex> lock(_lastMasterHeartbeatLock);
  _lastMasterHeartbeat = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

size_t ClusterInterface::getNextMasterNodeId() {
  return _nextMasterNodeId;
}

void ClusterInterface::setNextMasterNodeId(size_t nextMasterNodeId) {
  _nextMasterNodeId = nextMasterNodeId;
}

}
}
