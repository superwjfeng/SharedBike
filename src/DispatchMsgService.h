/*
  负责分发消息服务模块，也就是把外部收到的信息，转换为内部事件，即
  data->msg->event
  的解码过程，然后再把事件投递至线程池的消息队列，由线程池调用其 process
  方法来对事件进行处理，最终调用每个 event 的handler方法来处理
*/

#ifndef BIKE_DISPATCH_SERVICE_H_
#define BIKE_DISPATCH_SERVICE_H_

#include <queue>
#include <unordered_map>
#include <vector>

#include "NetworkInterface.h"
#include "eventtype.h"
#include "iEventHandler.h"
#include "ievent.h"
#include "threadpool/thread_pool.h"

class DispatchMsgService {
 protected:
  // TODO: 为什么单例不能给参数？
  DispatchMsgService();

 public:
  virtual ~DispatchMsgService();

  virtual BOOL open();
  virtual void close();

  /* 订阅者模式 */
  virtual void subscribe(u32 eid, iEventHandler *handler);
  virtual void unsubscribe(u32 eid, iEventHandler *handler);
  virtual i32 enqueue(iEvent *ev);  // 把事件投递到线程池中进行处理
  virtual iEvent *process(const iEvent *ev);  // 对具体的事件进行分发处理

  // C的线程池回调 service，C++的类函数多了一个this，所以设置为static
  static void svc(void *argv);
  static DispatchMsgService *getInstance();  // 单例
  iEvent *parseEvent(const char *msg, u32 len, u32 eid);

  // 让interface把所有的响应序列化后发送出去
  void handleAllResponseEvent(NetworkInterface *interface);
  void assembleResponse(iEvent *ev, EventID eid, NetworkInterface *interface);

 protected:
  thread_pool_t *tp_;
  static DispatchMsgService *DMS_;  // 单例

  typedef std::vector<iEventHandler *> T_EventHandlers;
  typedef std::unordered_map<u32, T_EventHandlers> T_EventHandlersMap;
  T_EventHandlersMap subscribers_;

  bool svr_exit_;

  // 被static方法调用，只能定义为static
  static std::queue<iEvent *> response_events;  // 响应事件队列
  static pthread_mutex_t queue_mutex;
};

#endif  // BIKE_DISPATCH_SERVICE_H_