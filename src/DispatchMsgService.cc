#include "DispatchMsgService.h"

#include <algorithm>

#include "NetworkInterface.h"
#include "bike.pb.h"
#include "events_def.h"

DispatchMsgService *DispatchMsgService::DMS_ = nullptr;
pthread_mutex_t DispatchMsgService::queue_mutex;
std::queue<iEvent *> DispatchMsgService::response_events;

DispatchMsgService::DispatchMsgService() : tp_(nullptr) {}
DispatchMsgService::~DispatchMsgService() {}

// 初始线程池
BOOL DispatchMsgService::open() {
  svr_exit_ = FALSE;

  thread_mutex_create(&queue_mutex);
  tp_ = thread_pool_init();

  return tp_ ? TRUE : FALSE;
}

// 关闭线程池
void DispatchMsgService::close() {
  svr_exit_ = TRUE;

  thread_pool_destroy(tp_);
  thread_mutex_destroy(&queue_mutex);
  subscribers_.clear();

  tp_ = nullptr;
}

void DispatchMsgService::subscribe(u32 eid, iEventHandler *handler) {
  LOG_DEBUG("DispatchMsgService::subscribe eid: %u\n", eid);
  /* eid 是否已经被订阅过了 */
  T_EventHandlersMap::iterator iter = subscribers_.find(eid);
  if (iter != subscribers_.end()) {
    // 已经存在的话不需要做动作
    T_EventHandlers::iterator hdl_iter =
        std::find(iter->second.begin(), iter->second.end(), handler);
    // 不存在就尾插
    if (hdl_iter == iter->second.end()) iter->second.push_back(handler);
  } else {
    subscribers_[eid].push_back(handler);
  }
}

void DispatchMsgService::unsubscribe(u32 eid, iEventHandler *handler) {
  T_EventHandlersMap::iterator iter = subscribers_.find(eid);
  if (iter != subscribers_.end()) {
    T_EventHandlers::iterator hdl_iter =
        std::find(iter->second.begin(), iter->second.end(), handler);
    if (hdl_iter != iter->second.end()) iter->second.erase(hdl_iter);
  }
}

i32 DispatchMsgService::enqueue(iEvent *ev) {
  if (ev == NULL) return -1;
  thread_task_t *task = thread_task_alloc(0);
  task->handler = DispatchMsgService::svc;
  task->ctx = ev;
  return thread_task_post(tp_, task);
}

// TODO:
// 调查一下C++的函数被C调用，除了static外还有什么方法，另外需要考虑权限吗？友元
void DispatchMsgService::svc(void *argv) {
  DispatchMsgService *dms = DispatchMsgService::getInstance();
  iEvent *ev = (iEvent *)argv;
  if (!dms->svr_exit_) {
    LOG_DEBUG("DispatchMsgService::svc ...\n");
    iEvent *rsp = dms->process(ev);
    if (rsp) {
      rsp->dump(std::cout);
      rsp->set_args(ev->get_args());
    } else {
      // 多线程会出问题
      // if (NWIF_) NWIF_->send_response_msg(static_cast<ConnectSession
      // *>(ev->get_args()));

      // 生成终止响应事件
      rsp = new ExitRspEv();
      rsp->set_args(ev->get_args());
    }

    thread_mutex_lock(&queue_mutex);
    response_events.push(rsp);
    thread_mutex_unlock(&queue_mutex);
  }
}

/* 线程池驱动 */
iEvent *DispatchMsgService::process(const iEvent *ev) {
  LOG_DEBUG("DispatchMsgService::process -ev: %p\n", ev);
  if (ev == NULL) return NULL;
  u32 eid = ev->get_eid();
  LOG_DEBUG("DispatchMsgService::process -eid: %p\n", eid);

  if (eid == EEVENTID_UNKNOWN) {
    LOG_WARN("DispatchMsgService::process: unkown event id %d\n", eid);
    return NULL;
  }

  T_EventHandlersMap::iterator handlers = subscribers_.find(eid);
  if (handlers == subscribers_.end()) {
    LOG_WARN("DispatchMsgService: no event handler subscribed %d\n", eid);
    return NULL;
  }

  /* 给订阅的user_event_handler都发一遍 */
  iEvent *rsp = NULL;
  for (auto iter = handlers->second.begin(); iter != handlers->second.end();
       iter++) {
    iEventHandler *handler = *iter;
    LOG_DEBUG("DispatchMsgService::process: get handler: %s\n",
              handler->get_name().c_str());
    // printf("DispatchMsgService::process: get handler: %s\n",
    //        handler->get_name().c_str());
    // rsp = handler->handle(ev);
    // TODO: 使用vector或list来返回多个rsp
    rsp = handler->handle(ev);
  }
  return rsp;
}

DispatchMsgService *DispatchMsgService::getInstance() {
  // 懒汉,
  // TODO:改成double clock checking
  if (DMS_ == nullptr) {
    DMS_ = new DispatchMsgService();
  }
  return DMS_;
}

iEvent *DispatchMsgService::parseEvent(const char *msg, u32 len, u32 eid) {
  if (!msg) {
    LOG_ERROR("DispatchMsgService::parseEvent - message is null[eid: %d].\n",
              eid);
    return nullptr;
  }

  if (eid == EEVENTID_GET_MOBILE_CODE_REQ) {
    bike::mobile_request mr;
    if (mr.ParseFromArray(msg, len)) {
      MobileCodeReqEv *ev = new MobileCodeReqEv(mr.mobile());
      return ev;
    }
  } else if (eid == EEVENTID_LOGIN_REQ) {
    bike::login_request lr;
    if (lr.ParseFromArray(msg, len)) {
      LoginReqEv *ev = new LoginReqEv(lr.mobile(), lr.icode());
      return ev;
    }
  } else {
    return nullptr;
  }
}

void DispatchMsgService::handleAllResponseEvent(NetworkInterface *interface) {
  bool done = false;
  while (!done) {
    iEvent *ev = nullptr;
    thread_mutex_lock(&queue_mutex);
    if (!response_events.empty()) {
      ev = response_events.front();
      response_events.pop();
    } else {
      done = true;
    }
    thread_mutex_unlock(&queue_mutex);
    if (!done) {
      if (ev->get_eid() == EEVENTID_GET_MOBILE_CODE_RSP) {
        LOG_DEBUG(
            "DispatchMsgService::handleAllResponseEvent - id: "
            "EEVENTID_GET_MOBILE_CODE_RSP\n");
        assembleResponse(ev, EEVENTID_GET_MOBILE_CODE_RSP, interface);
      } else if (ev->get_eid() == EEVENTID_LOGIN_RSP) {
        LOG_DEBUG(
            "DispatchMsgService::handleAllResponseEvent - id: "
            "EEVENTID_LOGIN_RSP\n");
        assembleResponse(ev, EEVENTID_LOGIN_RSP, interface);
      } else if (ev->get_eid() == EEVENTID_EXIT_RSP) {
        ConnectSession *cs = (ConnectSession *)ev->get_args();
        cs->response = ev;
        interface->send_response_msg(cs);
      }
    }
  }
}

void DispatchMsgService::assembleResponse(iEvent *ev, EventID eid,
                                          NetworkInterface *interface) {
  ConnectSession *cs = (ConnectSession *)ev->get_args();
  cs->response = ev;

  // Serialize requested data
  cs->msg_len = ev->ByteSize();  // ev形成多态
  printf("cs->msg_len: %d\n", cs->msg_len);
  cs->write_buf = new char[MESSAGE_HEADER_LEN + cs->msg_len];

  // Assemble header
  memcpy(cs->write_buf, MESSAGE_HEADER, 4);
  *(u16 *)(cs->write_buf + 4) = eid;
  *(i32 *)(cs->write_buf + 6) = cs->msg_len;

  // Assemble serialized data
  ev->SerializeToArray(cs->write_buf + MESSAGE_HEADER_LEN, cs->msg_len);
  // TODO: 打印protobuf序列化后的内容
  interface->send_response_msg(cs);
}