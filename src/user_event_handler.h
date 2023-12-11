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
  MobileCodeRspEv *handle_mobile_code_req(MobileCodeReqEv *ev);
  i32 code_gen(); /* 生成验证码 */

 private:
  std::string mobile_; // phone number
  std::unordered_map<std::string, i32> m2c_;  // <mobile, code>
  pthread_mutex_t pm_;
};
#endif  // BIKE_USER_HANDLER_H_