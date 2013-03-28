// Copyright (c) 2012 Hasso-Plattner-Institut fuer Softwaresystemtechnik GmbH. All rights reserved.
#include "net/AsyncConnection.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctime>
#include <memory>

#include "net/Router.h"
#include "taskscheduler/SharedScheduler.h"
#include "access/RequestParseTask.h"

namespace hyrise {
namespace net {

ebb_connection *new_connection(ebb_server *server, struct sockaddr_in *addr) {
  ebb_connection *connection = (ebb_connection *)malloc(sizeof(ebb_connection));
  if (connection == nullptr) {
    return nullptr;
  }

  AsyncConnection *connection_data = new AsyncConnection();
  connection_data->addr = *addr;

  // Initializes the connection
  ebb_connection_init(connection);
  connection->data = connection_data;
  connection->new_request = new_request;
  connection->on_close = on_close;
  connection->on_timeout = on_timeout;

  connection_data->ev_loop = server->loop;
  connection_data->ev_write.data = connection_data;
  ev_async_init(&connection_data->ev_write, write_cb);
  ev_async_start(server->loop, &connection_data->ev_write);

  return connection;
}

int on_timeout(ebb_connection *connection) {
  return EBB_AGAIN;
}

ebb_request *new_request(ebb_connection *connection) {
  ebb_request *request = (ebb_request *)malloc(sizeof(ebb_request));
  ebb_request_init(request);
  request->data = connection;
  request->on_complete = request_complete;
  request->on_path = request_path;
  request->on_body = request_body;
  return request;
}

void request_complete(ebb_request *request) {
  ebb_connection *connection = (ebb_connection *)request->data;
  AsyncConnection *connection_data = (AsyncConnection *)connection->data;
  connection_data->connection = connection;
  connection_data->request = request;
  gettimeofday(&connection_data->starttime, nullptr);

  // Try to route to appropriate handler based on path
  const AbstractRequestHandlerFactory *handler_factory;
  try {
    handler_factory = Router::route(connection_data->path);
  } catch (const RouterException &exc) {
    std::string exception_message(exc.what());
    connection_data->respond("Could not route request, std::exception was: \n"
                             + exception_message);
    return;
  }
  std::shared_ptr<Task> task = handler_factory->create(connection_data);
  // Always map the first task to the first core
  task->setPreferredCore(0);
  // give RequestParseTask high priority
  task->setPriority(1);
  if (auto task2 = std::dynamic_pointer_cast<hyrise::access::RequestParseTask>(task)){
    task2->setQueryStart();
  }
  SharedScheduler::getInstance().getScheduler()->schedule(task);
}

void request_path(ebb_request *request, const char *at, size_t length) {
  ebb_connection *connection = (ebb_connection *)request->data;
  AsyncConnection *connection_data = (AsyncConnection *)connection->data;

  connection_data->path = (char *)malloc(length + 1);
  strncpy(connection_data->path, at, length);
  connection_data->path[length] = '\0';
}

void request_body(ebb_request *request, const char *at, size_t length) {
  ebb_connection *connection = (ebb_connection *)request->data;
  AsyncConnection *connection_data = (AsyncConnection *)connection->data;

  if (!connection_data->body) {
    connection_data->body = (char *)malloc(length);
    connection_data->body_len = 0;
  } else {
    connection_data->body = (char *)realloc(connection_data->body, connection_data->body_len + length);
  }
  memcpy(connection_data->body + connection_data->body_len, at, length);
  connection_data->body_len += length;
}

void write_cb(struct ev_loop *loop, struct ev_async *w, int revents) {
  AsyncConnection *conn = (AsyncConnection *) w->data;

  char *method = (char *) "";
  switch (conn->request->method) {
    case EBB_GET:
      method = (char *)"GET";
      break;
    case EBB_POST:
      method = (char *)"POST";
      break;
    default:
      break;
  }
  struct timeval endtime;
  gettimeofday(&endtime, nullptr);
  float duration = endtime.tv_sec + endtime.tv_usec / 1000000.0 - conn->starttime.tv_sec - conn->starttime.tv_usec / 1000000.0;

  time_t rawtime;
  struct tm *timeinfo;
  char timestr[80];
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S %z", timeinfo);

  if (conn->connection != nullptr) {
    conn->write_buffer = (char *)malloc(max_header_length + conn->response_length);
    conn->write_buffer_len = 0;

    // Copy the http status code
    conn->code = conn->code == 0 ? 200 : conn->code;
    conn->contentType = conn->contentType.size() == 0 ? "application/json" : conn->contentType;
    conn->write_buffer_len += snprintf((char *)conn->write_buffer, max_header_length, 
      "HTTP/1.1 %lu OK\r\nContent-Type: %s\r\nContent-Length: %lu\r\nConnection: close\r\n\r\n", 
      conn->code, 
      conn->contentType.c_str(),
      conn->response_length);

    // Append the response
    memcpy(conn->write_buffer + conn->write_buffer_len, conn->response, conn->response_length);
    conn->write_buffer_len += conn->response_length;
    ebb_connection_write(conn->connection, conn->write_buffer, conn->write_buffer_len, continue_responding);
    // We need to wait for `continue_responding` to fire to be sure the client has been sent all the data
    printf("%s [%s] %s %s (%f s)\n", inet_ntoa(conn->addr.sin_addr), timestr, method, conn->path, duration);
  } else {
    printf("%s [%s] %s %s (%f s) not sent\n", inet_ntoa(conn->addr.sin_addr), timestr, method, conn->path, duration);
  }
  ev_async_stop(conn->ev_loop, &conn->ev_write);
  // When connection is nullptr, `continue_responding` won't fire since we never sent data to the client,
  // thus, we'll need to clean up manually here, while connection has already been cleaned up in on `on_response`
  if (conn->connection == nullptr) delete conn;
}

void continue_responding(ebb_connection *connection) {
  delete(AsyncConnection *) connection->data;
  connection->data = nullptr;
  ebb_connection_schedule_close(connection);
}

void on_close(ebb_connection *connection) {
  AsyncConnection *connection_data = (AsyncConnection *)connection->data;
  if (connection_data != nullptr)
    connection_data->connection = nullptr;
  free(connection);
}

AsyncConnection::AsyncConnection() :
    request(nullptr), path(nullptr), body(nullptr), body_len(0), response(nullptr), write_buffer(nullptr), closed(false), code(0) {
}

AsyncConnection::~AsyncConnection() {
  free(path);
  free(write_buffer);
  free(request);
  free(body);
  free(response);
}

void AsyncConnection::respond(const std::string &message) {
  // Noop for dead connection
  if (connection == nullptr) return;
  response = (char *) malloc(message.size());
  response_length = message.size();
  memcpy(response, message.c_str(), message.size());
  send_response();
}
/*
void AsyncConnection::response_append(const char *message, size_t len) {
  // If we lost the connection, let's not even save the response
  if (connection == nullptr) return;

  if (!response) {
    response = (char *) malloc(len);
    response_length = 0;
  } else {
    response = (char *) realloc(response, response_length + len);
  }
  memcpy(response + response_length, message, len);
  response_length += len;
}
*/

void AsyncConnection::send_response() {
  ev_async_send(ev_loop, &ev_write);
}

bool AsyncConnection::hasBody() const{
  return body_len > 0;
}

std::string AsyncConnection::getBody() const{
  return std::string(body, body_len);
}


}
}
