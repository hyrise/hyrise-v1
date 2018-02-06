// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.

#include "net/ClusterManager.h"
#include "net/ClusterInterface.h"
#include "io/GroupCommitter.h"
#include "io/logging.h"
#include "helper/Settings.h"

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <thread>
#include <chrono>

namespace hyrise {
namespace net {

ClusterManager& ClusterManager::getInstance() {
  static ClusterManager instance;

  return instance;
}

ClusterManager::ClusterManager() {
  _thread = std::thread(&ClusterManager::run, this);
}

void ClusterManager::run() {
  while (_shouldRun) {
    std::chrono::microseconds dura( INTERVAL_ms_SECONDS );
    std::this_thread::sleep_for( dura );
    checkForHeartbeat();
  }
}

void ClusterManager::checkForHeartbeat() {
  auto lastMasterHeartbeat = ClusterInterface::getInstance().getLastMasterHeartbeat();

  auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

  if ((now - lastMasterHeartbeat > THRESHOLD_MILLISECONDS) && (lastMasterHeartbeat > 0)) {

    std::cout << "Lost connection to master..." << std::endl;
    _shouldRun = false;
    ClusterInterface::getInstance().resetConnection();

    bool isNewMaster = ClusterInterface::getInstance().getNodeId() == ClusterInterface::getInstance().getNextMasterNodeId();

    if (isNewMaster) {

      std::cout << "Trying to become new master..." << std::endl;
      std::string oldMasterLogFile = Settings::getInstance()->getPersistencyDir();
      oldMasterLogFile += std::to_string(ClusterInterface::getInstance().getCurrentMasternodeId()) + "/logs/00001.bin";

      std::cout << "Replicating..." << std::endl;
      io::Logger::getInstance().replicate(oldMasterLogFile.c_str(), 0, true);

      std::cout << "Notifying dispatcher..." << std::endl;
      notifyDispatcher();

      std::cout << "Starting group committer..." << std::endl;
      io::GroupCommitter::getInstance();

      std::cout << "I am the new master now!" << std::endl;
    } else {
      ClusterInterface::getInstance().updateLastMasterHeartbeat();
      _shouldRun = true;
    }

    ClusterInterface::getInstance().setCurrentMasternodeId(ClusterInterface::getInstance().getNextMasterNodeId());
    ClusterInterface::getInstance().setNextMasterNodeId(ClusterInterface::getInstance().getCurrentMasternodeId() + 1);

    ClusterInterface::getInstance().start();

  }
}

void ClusterManager::notifyDispatcher() {

  int sockfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;

  // TODO
  const char* DISPATCHER_ADDRESS = Settings::getInstance()->dispatcher_url.c_str();


  char buffer[256];

  const int DISPATCHER_PORT = Settings::getInstance()->dispatcher_port;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    std::cout << "Error while opening connection to dispatcher (error opening socket)" << std::endl;
    std::cout << "Dispatcher address: " << DISPATCHER_ADDRESS << " - port: " << DISPATCHER_PORT << std::endl;
    return;
  }

  server = gethostbyname(DISPATCHER_ADDRESS);
  if (server == NULL) {
    std::cout << "Error while opening connection to dispatcher (no such host)" << std::endl;
    std::cout << "Dispatcher address: " << DISPATCHER_ADDRESS << " - port: " << DISPATCHER_PORT << std::endl;
    return;
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(DISPATCHER_PORT);

  if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) {
    std::cout << "Error while opening connection to dispatcher (error connecting)" << std::endl;
    std::cout << "Dispatcher address: " << DISPATCHER_ADDRESS << " - port: " << DISPATCHER_PORT << std::endl;
    return;
  }

  // TODO
  std::string payload = "newMaster";
  sprintf(buffer, "POST /new_master HTTP/1.1\r\nHost: DISPATCHER\r\nContent-Length: %lu\r\n\r\n%s", payload.size(), payload.c_str());

  std::cout << "sending msg..." << std::endl;
  n = write(sockfd, buffer, strlen(buffer));
  if (n < 0) {
    std::cout << "Error while opening connection to dispatcher (error writing)" << std::endl;
    return;
  }

  // TODO
  bzero(buffer,256);
  n = read(sockfd,buffer,255);
  std::cout << "receiving msg..." << std::endl;
  if (n < 0) {
    std::cout << "Error while opening connection to dispatcher (error reading)" << std::endl;
    return;
  }

  std::cout << "Dispatcher successfully notified" << std::endl;
}

}
}
