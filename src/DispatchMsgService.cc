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

BOOL DispatchMsgService::open() {
  svr_exit_ = FALSE;

  thread_mutex_create(&queue_mutex);
  thread_pool_init();

  return tp_ ? TRUE : FALSE;
}

void DispatchMsgService::close() {
  svr_exit_ = TRUE;

  thread_pool_destroy(tp_);
  thread_mutex_destroy(&queue_mutex);
  subscribers_.clear();

  tp_ = nullptr;
}

void DispatchMsgService::subscribe(u32 eid, iEventHandler *handler) {
  // TODO: LOG_DEBUG("DispatchMsgService::subscribe eid: %u\n", eid);
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

/* 线程池驱动 */
iEvent *DispatchMsgService::process(const iEvent *ev) {
  // LOG_DEBUG("DispatchMsgService::process -ev: %p\n", ev);
  if (ev == NULL) return NULL;
  u32 eid = ev->get_eid();
  // LOG_DEBUG("DispatchMsgService::process -eid: %p\n", eid);

  if (eid == EEVENTID_UNKNOWN) {
    // LOG_WARN("DispatchMsgService::process: unkown event id %d", eid);
    return NULL;
  }

  // LOG_DEBUG("dispatch ev: %d\n", ev->get_eid());

  T_EventHandlersMap::iterator handlers = subscribers_.find(eid);
  if (handlers == subscribers_.end()) {
    // LOG_WARN("DispatchMsgService: no any event handler subscribed %d", eid);
    return NULL;
  }

  /* 给订阅用户都发一遍 */
  iEvent *rsp = NULL;
  for (auto iter = handlers->second.begin(); iter != handlers->second.end();
       iter++) {
    iEventHandler *handler = *iter;
    // LOG_DEBUG("DispatchMsgService::process: get handler: %s\n",
    // printf("DispatchMsgService::process: get handler: %s\n",
    //        handler->get_name().c_str());
    // rsp = handler->handle(ev);
    // TODO: 使用vector或list来返回多个rsp
    rsp = handler->handle(ev);
  }
  return rsp;
}

void DispatchMsgService::svc(void *argv) {
  DispatchMsgService *dms = DispatchMsgService::getInstance();
  iEvent *ev = (iEvent *)argv;
  if (!dms->svr_exit_) {
    // LOG_DEBUG("DispatchMsgService::svc ...\n");
    iEvent *rsp = dms->process(ev);
    if (rsp) {
      rsp->dump(std::cout);
      rsp->set_args(ev->get_args());

      thread_mutex_lock(&queue_mutex);
      response_events.push(rsp);
      thread_mutex_unlock(&queue_mutex);
    } else {
      // 多线程会出问题
      // if (NWIF_) NWIF_->send_response_msg(static_cast<ConnectSession
      // *>(ev->get_args()));

      // 生成终止响应事件
      rsp = new ExitRspEv();
      rsp->set_args(ev->get_args());
    }
  }
}

DispatchMsgService *DispatchMsgService::getInstance() {
  // 懒汉,
  // TODO：改成double clock checking
  if (DMS_ == nullptr) {
    DMS_ = new DispatchMsgService();
  }
  return DMS_;
}

iEvent *DispatchMsgService::parseEvent(const char *msg, u32 len, u32 eid) {
  if (!msg) {
    // LOG_ERROR("DispatchMsgService::parseEvent - message is null[eid: %d].\n",
    // eid);
    return nullptr;
  }
  if (eid == EEVENTID_GET_MOBILE_CODE_REQ) {
    bike::mobile_request mr;
    if (!mr.ParseFromArray(msg, len)) {
      MobileCodeReqEv *ev = new MobileCodeReqEv(mr.mobile());
      return ev;
    }
  } else if (eid == EEVENTID_LOGIN_REQ) { /**/
  }
  return nullptr;
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
        MobileCodeRspEv *mcre = static_cast<MobileCodeRspEv *>(ev);
        // LOG_DEBUG("DispatchMsgService::handleAllResponseEvent - id:
        // EEVENTID_GET_MOBILE_CODE_RSP\n");
        ConnectSession *cs = (ConnectSession *)ev->get_args();
        cs->response = ev;
        // 序列化请求数据
        cs->msg_len = mcre->ByteSize();
        cs->write_buf = new char[cs->msg_len + MESSAGE_HEADER_LEN];

        // 组装头部
        memcpy(cs->write_buf, MESSAGE_HEADER_ID, 4);
        *(u16 *)(cs->write_buf + 4) = EEVENTID_GET_MOBILE_CODE_RSP;
        *(i32 *)(cs->write_buf + 6) = cs->msg_len;

        mcre->SerializeToArray(cs->write_buf + MESSAGE_HEADER_LEN, cs->msg_len);
        interface->send_response_msg(cs);
      } else if (ev->get_eid() == EEVENTID_EXIT_RSP) {
        ConnectSession *cs = (ConnectSession *)ev->get_args();
        cs->response = ev;
        interface->send_response_msg(cs);
      }
    }
  }
}