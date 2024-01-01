#include <arpa/inet.h>
#include <event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/util.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../globals.h"
#include "bike.pb.h"

using namespace bike;

int tcp_connect_server(const char *server_ip, int port);
void cmd_msg_cb(int fd, short events, void *arg);
void server_msg_cb(struct bufferevent *bev, void *arg);
void event_cb(struct bufferevent *bev, short event, void *arg);

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("please intput 2 parameter\n");
    return -1;
  }

  int sockfd = tcp_connect_server(argv[1], atoi(argv[2]));
  if (sockfd == -1) {
    perror("tcp_connect error");
    return -1;
  }
  printf("connect to server successful\n");

  struct event_base *base = event_base_new();
  struct bufferevent *bev =
      bufferevent_socket_new(base, sockfd, BEV_OPT_CLOSE_ON_FREE);

  // 监听终端输入事件
  struct event *ev_cmd = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST,
                                   cmd_msg_cb, (void *)bev);
  event_add(ev_cmd, NULL);

  /*
  void bufferevent_setcb(struct bufferevent *bufev,
    bufferevent_data_cb readcb, bufferevent_data_cb writecb,
    bufferevent_event_cb eventcb, void *cbarg);
  */

  // 处理与server之间的通信
  bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, (void *)ev_cmd);
  bufferevent_enable(bev, EV_READ | EV_PERSIST);

  event_base_dispatch(base);
  printf("finished \n");

  return 0;
}

void cmd_msg_cb(int fd, short events, void *arg) {
  char msg[1024];
  // std::string proto_msg;

  // 目前只是为了触发cmd_msg_cb，小于13位的会被覆盖
  int ret = read(fd, msg, sizeof(msg));
  if (ret < 0) {
    perror("read fail ");
    exit(1);
  }

  // TODO: 复习一下glibc的字符串函数，比如strcmp 以及此时需要放 '\0'
  // 有一个很坑的点：stdin输入后会有换行符

  struct bufferevent *bev = (struct bufferevent *)arg;
  mobile_request mr;
  mr.set_mobile("18266666666");
  int len = mr.ByteSize();
  // int len = strlen(msg);
  memcpy(msg, "FBEB", 4);  // EEVENTID_GET_MOBILE_CODE_REQ
  *(u16 *)(msg + 4) = 1;
  *(i32 *)(msg + 6) = len;
  mr.SerializeToArray(msg + 10, len);

  // 把终端的消息发送给服务器端
  bufferevent_write(bev, msg, len + 10);
}

// readcb
void server_msg_cb(struct bufferevent *bev, void *arg) {
  char msg[1024];
  size_t len = bufferevent_read(bev, msg, sizeof(msg));
  if (len == 0) {
    printf("connection close. exit~\n");
    exit(1);
  } else if (len < 0) {
    perror("read fail ");
    return;
  }

  msg[len] = '\0';

  printf("recv from server<<<<< [%s] \n", msg);

  if (strncmp(msg, "FBEB", 4) == 0) {
    mobile_response mr;
    // u16 event_id = *(u16 *)(msg + 4);
    i32 len = *(i32 *)(msg + 6);
    mr.ParseFromArray(msg + 10, len);
    printf("mobile_response: code: %d, icode: %d, data: %s\n", mr.code(),
           mr.icode(), mr.data().c_str());
  }
}

typedef struct sockaddr SA;
int tcp_connect_server(const char *server_ip, int port) {
  int sockfd, status, save_errno;
  struct sockaddr_in server_addr;

  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  status = inet_aton(server_ip, &server_addr.sin_addr);

  if (status == 0)  // the server_ip is not valid value
  {
    errno = EINVAL;
    return -1;
  }

  sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) return sockfd;

  status = connect(sockfd, (SA *)&server_addr, sizeof(server_addr));

  if (status == -1) {
    save_errno = errno;
    close(sockfd);
    errno = save_errno;  // the close may be error
    return -1;
  }

  // evutil_make_socket_nonblocking(sockfd);

  return sockfd;
}

// client关闭后清理 & 异常 callback
void event_cb(struct bufferevent *bev, short event, void *arg) {
  struct event *ev = (struct event *)arg;

  if (event & BEV_EVENT_EOF)
    printf("connection closed\n");
  else if (event & BEV_EVENT_ERROR)
    printf("some other error\n");

  event_free(ev);
  bufferevent_free(bev);
}