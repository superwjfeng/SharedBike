#include <assert.h>
#include <errno.h>
#include <event.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bike.pb.h"

#define BUFLEN 1024

using namespace bike;

typedef struct _ConnectStat {
  // struct event*  ev;
  struct bufferevent *bev;
  char buf[BUFLEN];
} ConnectStat;

// echo 服务实现相关代码
ConnectStat *stat_init(int fd, struct bufferevent *bev);

void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
                 struct sockaddr *sock, int socklen, void *arg);

// accept_connection(int fd, short events, void* arg);
void do_echo_request(struct bufferevent *bev, void *arg);
void event_cb(struct bufferevent *bev, short event, void *arg);
// void do_echo_response(int fd, short events, void *arg);

int tcp_server_init(int port, int listen_num);

struct event_base *base;

int main(int argc, char **argv) {
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(9999);

  base = event_base_new();

  struct evconnlistener *listener = evconnlistener_new_bind(
      base, listener_cb, base, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 10,
      (struct sockaddr *)&sin, sizeof(struct sockaddr_in));

  event_base_dispatch(base);

  evconnlistener_free(listener);
  event_base_free(base);

  return 0;
}

ConnectStat *stat_init(int fd, struct bufferevent *bev) {
  ConnectStat *temp = NULL;
  temp = (ConnectStat *)malloc(sizeof(ConnectStat));

  if (!temp) {
    fprintf(stderr, "malloc failed. reason: %m\n");
    return NULL;
  }

  memset(temp, '\0', sizeof(ConnectStat));
  temp->bev = bev;
}

/*
一个新客户端连接上服务器此函数就会被调用,当此函数被调用时，libevent已经帮我们accept了这个客户端。
该客户端的文件描述符为fd
*/
void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
                 struct sockaddr *sock, int socklen, void *arg) {
  printf("accept a client %d\n", fd);

  struct event_base *base = (struct event_base *)arg;

  // 为这个客户端分配一个bufferevent
  struct bufferevent *bev =
      bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  ConnectStat *stat = stat_init(fd, bev);

  bufferevent_setcb(bev, do_echo_request, NULL, event_cb, stat);
  bufferevent_enable(bev, EV_READ | EV_PERSIST);
}

void do_echo_request(struct bufferevent *bev, void *arg) {
  std::string proto_msg;
  ConnectStat *stat = (ConnectStat *)arg;
  char *msg = stat->buf;
  printf("do echo request ...\n");

  size_t len = bufferevent_read(bev, msg, BUFLEN);

  msg[len] = '\0';
  // parse protobuf message
  
  proto_msg = msg;

  list_account_records_request larr;
  larr.ParseFromString(proto_msg);

  printf("recv from client<<<< %s\n", larr.mobile().c_str());

  bufferevent_write(bev, larr.mobile().c_str(), larr.mobile().length());
}

void event_cb(struct bufferevent *bev, short event, void *arg) {
  ConnectStat *stat = (ConnectStat *)arg;

  if (event & BEV_EVENT_EOF)
    printf("connection closed\n");
  else if (event & BEV_EVENT_ERROR)
    printf("some other error\n");

  // 这将自动close套接字和free读写缓冲区
  bufferevent_free(bev);
  free(stat);
}
