#pragma once

#include "io/TransactionManager.h"

#include <string>
#include <map>
#include <mutex>

namespace hyrise {
namespace net {

#define SEND_RECEIVE_BUF_SIZE 16384

struct node {
  int64_t lastHeartbeat;
  tx::transaction_cid_t lastCID;
};

enum MessageType {
  HELLO,
  HEARTBEAT_REQUEST,
  HEARTBEAT_REPLY,
  LOG_ATTACHED,
  LOG_RECEIVED,
  FIRST = HELLO,
  LAST = LOG_RECEIVED
};

constexpr size_t CHECK_LOG_FORCE_SEND_INTERVAL = 500;
constexpr size_t LOG_FORCE_SEND_THRESHOLD = 1000;
constexpr size_t MESSAGE_BUFFER_RESERVED_BYTES = 2;
constexpr size_t SEND_LOG_EVERY_N = 8;

void send(int64_t socket, MessageType type, const char* payload, size_t payloadSize);

class ClusterInterface {
  public:
    static ClusterInterface& getInstance();
    void send(MessageType type, const char* payload, size_t payloadSize);
    void internalSend(MessageType type);

    void start();

    void addNode(size_t nodeId);
    void updateNode(size_t nodeId, std::string);

    void resetConnection();

    bool getMasternode();

    size_t getNodeId();
    void setNodeId(size_t nodeId);

    void setMasterUrl(std::string master_url);

    std::map<size_t, node>& getNodes();
    std::mutex& getNodesLock();

    size_t getCurrentMasternodeId();
    void setCurrentMasternodeId(size_t currentMasternodeId);

    int64_t getLastMasterHeartbeat();
    void updateLastMasterHeartbeat();

    size_t getNextMasterNodeId();
    void setNextMasterNodeId(size_t nextMasterNodeId);
  private:
    ClusterInterface();

    void initControlInterface();
    void initDataInterface();

    void flushSendLog();

    ClusterInterface(ClusterInterface const&);
    void operator=(ClusterInterface const&);

    std::map<size_t, node> _nodes;
    std::mutex _nodesLock;

    size_t _heartbeats = 0;
    char _messageBuffer[SEND_RECEIVE_BUF_SIZE];

    size_t _currentMasternodeId = 0;
    size_t _nextMasterNodeId = 1;
    bool _masternode = false;
    size_t _nodeId = std::numeric_limits<size_t>::max();
    std::string _master_url;

    std::shared_ptr<std::thread> _controlInterfaceThread;
    std::shared_ptr<std::thread> _dataInterfaceThread;
    std::shared_ptr<std::thread> _sendLogFlushThread;

    int64_t _dataSocket;

    int64_t _lastMasterHeartbeat = 0;
    std::mutex _lastMasterHeartbeatLock;

    size_t _logMessageCount = 0;
    size_t _currentBufferSize = MESSAGE_BUFFER_RESERVED_BYTES;

    size_t _lastLogMessage = 0;

    std::mutex _sendMessageLock;
};

}
}
