#ifndef BIKE_USER_HANDLER_H_
#define BIKE_USER_HANDLER_H_

#include <memory>
#include <string>
#include <unordered_map>

#include "events_def.h"
#include "globals.h"
#include "iEventHandler.h"

class UserEventHandler : public iEventHandler {
 public:
  UserEventHandler();
  virtual ~UserEventHandler();
  virtual iEvent *handle(const iEvent *ev);

 private:
  // 在handle内调用
  MobileCodeRspEv *handle_mobile_code_req(MobileCodeReqEv *ev);
  i32 code_gen(); /* 生成验证码，实际中这个功能是由运营商提供的接口来实现的 */

 private:
  std::string mobile_; // phone number
  std::unordered_map<std::string, i32> m2c_;  // <mobile, icode>
  pthread_mutex_t pm_;
};
#endif  // BIKE_USER_HANDLER_H_