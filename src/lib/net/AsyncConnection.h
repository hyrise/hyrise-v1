// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#ifndef SRC_LIB_NET_ASYNCCONNECTION_H_
#define SRC_LIB_NET_ASYNCCONNECTION_H_

#include <arpa/inet.h>
#include <sys/time.h>
#include <cstdlib>
#include <ev.h>

#include <string>

#include "net/AbstractConnection.h"

#include "ebb/ebb.h"

#define max_header_length 512

namespace hyrise {
namespace net {

class AsyncConnection : public AbstractConnection {
 public:
  ev_async ev_write;
  struct ev_loop *ev_loop;
  ebb_connection *connection;
  ebb_request *request;
  struct sockaddr_in addr;
  struct timeval starttime;
  char *path;

  char *body;
  size_t body_len;

  char *write_buffer;
  size_t write_buffer_len;

  bool keep_alive_flag;
  bool waiting_for_response = false;

  AsyncConnection();
  ~AsyncConnection();
  void reset();
  virtual std::string getBody() const;
  virtual bool hasBody() const;
  virtual std::string getPath() const;
  virtual void respond(const std::string &message, size_t status=200, const std::string& contentType="application/json");
 private:
  virtual void send_response();
};

ebb_connection *new_connection(ebb_server *server, struct sockaddr_in *addr);

ebb_request *new_request(ebb_connection *connection);

void request_complete(ebb_request *request);

void request_path(ebb_request *request, const char *at, size_t length);

void request_body(ebb_request *request, const char *at, size_t length);

void write_cb(struct ev_loop *loop, struct ev_async *w, int revents);

void continue_responding(ebb_connection *connection);

void on_close(ebb_connection *connection);

int on_timeout(ebb_connection *connection);

}
}

#endif  // SRC_LIB_NET_ASYNCCONNECTION_H_
