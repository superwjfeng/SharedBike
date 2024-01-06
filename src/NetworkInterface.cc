#include "NetworkInterface.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <cstring>

#include "DispatchMsgService.h"

// 注意：ConnectSession 必须是纯C类型的成员变量，否则memset会出问题
// 设置为 static 是因为只在这个文件中用到
static ConnectSession *session_init(int fd, struct bufferevent *bev) {
  ConnectSession *cs = new ConnectSession();
  if (!cs) {
    LOG_ERROR("session_init: malloc failed, reason: %m\n", errno);
    return nullptr;
  }
  // 只有在ConnectSession为纯C定义时候可以用memset初始化，否则会打乱C++结构
  memset(cs, '\0', sizeof(ConnectSession));
  cs->bev = bev;
  cs->fd = fd;
  // TODO: 这里不返回也可以正确初始化？为什么？推测是编译器的优化？
  return cs;
}

void session_reset(ConnectSession *cs) {
  if (cs) {
    if (cs->read_buf) {
      delete[] cs->read_buf;
      cs->read_buf = nullptr;
    }
    if (cs->write_buf) {
      delete[] cs->write_buf;
      cs->write_buf = nullptr;
    }
    cs->session_stat = SESSION_STATUS::SS_REQUEST;
    cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;
    cs->msg_len = 0;
    cs->read_msg_len = 0;
    cs->read_header_len = 0;
  }
}

void session_free(ConnectSession *cs) {
  if (cs) {
    delete[] cs->read_buf;
    cs->read_buf = nullptr;
  }
  if (cs->write_buf) {
    delete[] cs->write_buf;
    cs->write_buf = nullptr;
  }
  delete cs;
}

NetworkInterface::NetworkInterface() : base_(nullptr), listener_(nullptr) {}

NetworkInterface::~NetworkInterface() { close(); }

bool NetworkInterface::start(int port) {
  struct sockaddr_in sin;
  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);

  base_ = event_base_new();
  if ((listener_ = evconnlistener_new_bind(
           base_, NetworkInterface::listener_cb, base_,
           LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, 512,
           (struct sockaddr *)&sin, sizeof(struct sockaddr_in))) != NULL) {
    return true;
  } else {
    return false;
  }
}

void NetworkInterface::close() {
  if (base_) {
    event_base_free(base_);
    base_ = nullptr;
  }

  if (listener_) {
    evconnlistener_free(listener_);
    listener_ = nullptr;
  }
}

// TODO: 复习一下为什么定义的时候不用static
void NetworkInterface::listener_cb(struct evconnlistener *listener,
                                   evutil_socket_t fd, struct sockaddr *sock,
                                   int socklen, void *arg) {
  // TODO: 这里的staic_cast强转
  struct event_base *base = static_cast<struct event_base *>(arg);
  LOG_DEBUG("accpet a client %d\n", fd);

  // 为这个客户端分配一个bufferevent
  struct bufferevent *bev =
      bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  ConnectSession *cs = session_init(fd, bev);
  cs->session_stat = SESSION_STATUS::SS_REQUEST;
  cs->req_stat = MESSAGE_STATUS::MS_READ_HEADER;

  strcpy(cs->remote_ip, inet_ntoa(((sockaddr_in *)sock)->sin_addr));
  LOG_DEBUG("remote ip: %s\n", cs->remote_ip);
  bufferevent_setcb(bev, handle_request, handle_response, handle_error, cs);
  bufferevent_enable(bev, EV_READ | EV_PERSIST);
  // TODO: timer 参数设置到配置文件，具体设置多少因项目而异
  bufferevent_settimeout(bev, 10, 10);
}

void NetworkInterface::handle_request(struct bufferevent *bev, void *arg) {
  ConnectSession *cs = static_cast<ConnectSession *>(arg);
  if (cs->session_stat != SESSION_STATUS::SS_REQUEST) {
    LOG_WARN("NetworkInterface::handle_requset - wrong session state[%d].\n",
             cs->session_stat);
    return;
  }
  if (cs->req_stat == MESSAGE_STATUS::MS_READ_HEADER) {
    i32 len = bufferevent_read(bev, cs->header + cs->read_header_len,
                               MESSAGE_HEADER_LEN - cs->read_header_len);
    cs->read_header_len += len;
    cs->header[cs->read_header_len] = '\0';
    LOG_DEBUG("recv from client <<<< %s\n", cs->header);
    if (cs->read_header_len == MESSAGE_HEADER_LEN) {
      if (strncmp(cs->header, MESSAGE_HEADER, strlen(MESSAGE_HEADER)) == 0) {
        // 取出事件类型和消息长度
        cs->eid = *((u16 *)(cs->header + 4));
        cs->msg_len = *((i32 *)(cs->header + 6));
        LOG_DEBUG(
            "NetworkInterface::handle_request - read %d bytes in header, "
            "message len: %d\n",
            cs->read_header_len, cs->msg_len);

        /* 必须对信息长度做合法性检查，否则开10亿个字节直接把程序弄崩了 */
        if (cs->msg_len < 1 || cs->msg_len > MAX_MSG_LEN) {
          LOG_ERROR("NetworkInterface::handle_request wrong message, len: %u\n",
                    cs->msg_len);
          bufferevent_free(bev);
          session_free(cs);
          return;
        }

        cs->read_buf = new char[cs->msg_len];
        cs->req_stat = MESSAGE_STATUS::MS_READ_MESSAGE;
        cs->read_msg_len = 0;
      } else {
        LOG_ERROR("NetworkInterace::handle_request - Invalid request from %s\n",
                  cs->remote_ip);
        bufferevent_free(bev);
        session_free(cs);
        return;
      }
    }
  }
  if (cs->req_stat == MESSAGE_STATUS::MS_READ_MESSAGE &&
      evbuffer_get_length(bufferevent_get_input(bev)) > 0) {
    i32 len = bufferevent_read(bev, cs->read_buf + cs->read_msg_len,
                               cs->msg_len - cs->read_msg_len);
    cs->read_msg_len += len;
    LOG_DEBUG(
        "NetworkInterface::bufferevent_read: %d bytes, message len: %d read "
        "len: %d\n",
        len, cs->msg_len, cs->read_msg_len);

    if (cs->msg_len == cs->read_msg_len) {
      cs->session_stat = SESSION_STATUS::SS_RESPONSE;
      iEvent *ev = DispatchMsgService::getInstance()->parseEvent(
          cs->read_buf, cs->read_msg_len, cs->eid);
      delete[] cs->read_buf;
      cs->read_buf = nullptr;
      cs->read_msg_len = 0;
      if (ev) {
        ev->set_args(cs);  // 让事件保存cs，以便之后response的使用
        DispatchMsgService::getInstance()->enqueue(ev);
      } else {
        LOG_ERROR(
            "NetworkInterface::handle_reqeust ev is NULL, remote ip: %s, eid: "
            "%d\n",
            cs->remote_ip, cs->eid);
        bufferevent_free(bev);
        session_free(cs);
        return;
      }
    }
  }
}

void NetworkInterface::handle_response(struct bufferevent *bev, void *arg) {
  // LOG_DEBUG("NetworkInterface::handle_response ...\n");
}

// 超时、连接关闭、读写出错等异常情况指定的回调函数
void NetworkInterface::handle_error(struct bufferevent *bev, short event,
                                    void *arg) {
  ConnectSession *cs = static_cast<ConnectSession *>(arg);
  LOG_DEBUG("NetworkInterface::handle_error ...\n");
  if (event & BEV_EVENT_EOF) {
    LOG_DEBUG("connection closed\n");
  } else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_READING)) {
    LOG_WARN("NetworkInterface::reading timeout ...\n");
  } else if ((event & BEV_EVENT_TIMEOUT) && (event & BEV_EVENT_WRITING)) {
    LOG_WARN("NetworkInterface::writing timeout ...\n");
  } else if (event & BEV_EVENT_ERROR) {
    LOG_ERROR("NetworkInterface::some other error ...\n");
  }

  bufferevent_free(bev);
  session_free(cs);
}

void NetworkInterface::network_event_dispatch() {
  // 处理完请求事件就返回，因为还要发送响应，所以不能用event_dispatch()
  event_base_loop(base_, EVLOOP_NONBLOCK);
  // 处理响应事件，回复响应消息
  DispatchMsgService::getInstance()->handleAllResponseEvent(this);
}

void NetworkInterface::send_response_msg(ConnectSession *cs) {
  // 有些事件可能不需要响应
  if (cs->response == nullptr) {
    bufferevent_free(cs->bev);
    if (cs->request) delete cs->request;
    session_free(cs);
  } else {
    // TODO: 这里和handle_response回调的关系是？
    bufferevent_write(cs->bev, cs->write_buf, cs->msg_len + MESSAGE_HEADER_LEN);
    session_reset(cs);
    LOG_INFO("send_response_msg: sent\n");
  }
}