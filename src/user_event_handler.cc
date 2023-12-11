#include "user_event_handler.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "DispatchMsgService.h"
#include "threadpool/thread.h"
// #include "logger.h"

/*
TODO:
*/

UserEventHandler::UserEventHandler() : iEventHandler("UserEventHandler") {
  // 构造函数实现订阅事件的处理
  DispatchMsgService::getInstance()->subscribe(EEVENTID_GET_MOBILE_CODE_REQ,
                                               this);
  DispatchMsgService::getInstance()->subscribe(EEVENTID_LOGIN_REQ, this);
  thread_mutex_create(&pm_);
}

UserEventHandler::~UserEventHandler() {
  // 析构函数实现退订事件的处理
  DispatchMsgService::getInstance()->unsubscribe(EEVENTID_GET_MOBILE_CODE_REQ,
                                                 this);
  DispatchMsgService::getInstance()->unsubscribe(EEVENTID_LOGIN_REQ, this);
  thread_mutex_destroy(&pm_);
}

iEvent *UserEventHandler::handle(const iEvent *ev) {
  if (ev == NULL) {
    // LOG_ERROR("input ev is nULL");
    // printf("input ev is NULL");
  }

  u32 eid = ev->get_eid();

  if (eid == EEVENTID_GET_MOBILE_CODE_REQ) {
    return handle_mobile_code_req((MobileCodeReqEv *)ev);
    /* TODO:
    return handle_mobile_code_req(static_cast<MobileCodeReqEv *>(ev));
    */
  } else if (eid == EEVENTID_LOGIN_REQ) {
    // TODO: return handle_login_req((LoginEv *)ev);
  } else if (eid == EEVENTID_RECHARGE_REQ) {
  } else if (eid == EEVENTID_ACCOUNT_BALANCE_REQ) {
  } else if (eid == EEVENTID_LIST_ACCOUNT_RECORDS_REQ) {
  }

  return NULL;
}

MobileCodeRspEv *UserEventHandler::handle_mobile_code_req(MobileCodeReqEv *ev) {
  i32 icode = 0;
  std::string mobile_ = ev->get_mobile();
  // TODO: try to use log, and can we output lot to terminal
  // TODO: what's the difference between terminal and output in vs code?
  printf("try to get mobile phone %s validate code.\n", mobile_.c_str());

  icode = code_gen();

  // TODO: 为什么要同步？
  thread_mutex_lock(&pm_);
  m2c_[mobile_] = icode;
  thread_mutex_unlock(&pm_);

  printf("mobile: %s, code: %d\n", mobile_.c_str(), icode);

  return new MobileCodeRspEv(200, icode);
}

i32 UserEventHandler::code_gen() {
  srand((unsigned int)time(NULL));
  return (unsigned int)(rand() % (999999 - 100000) + 1000000);
}