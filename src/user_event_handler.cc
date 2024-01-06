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
#include "iniconfig.h"
#include "sqlconnection.h"
#include "threadpool/thread.h"
#include "user_service.h"
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
    LOG_ERROR("input ev is nULL");
  }

  u32 eid = ev->get_eid();

  if (eid == EEVENTID_GET_MOBILE_CODE_REQ) {
    return handle_mobile_code_req((MobileCodeReqEv *)ev);
    /* TODO:
    return handle_mobile_code_req(static_cast<MobileCodeReqEv *>(ev));
    */
  } else if (eid == EEVENTID_LOGIN_REQ) {
    return handle_login_req((LoginReqEv *)ev);
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

  return new MobileCodeRspEv(ERRC_SUCCESS, icode);
}

// 暂时用它来模拟产生验证码，之后需要用运营商的API来替换
// TODO: 运营商的业务如何实现？
i32 UserEventHandler::code_gen() {
  srand((unsigned int)time(NULL));
  return (unsigned int)(rand() % (999999 - 100000) + 100000);
}

LoginRspEv *UserEventHandler::handle_login_req(LoginReqEv *ev) {
  LoginRspEv *loginEv = nullptr;
  std::string mobile = ev->get_mobile();
  i32 icode = ev->get_icode();
  LOG_DEBUG("try to handle login ev, mobile = %s, code = %d", mobile.c_str(),
            icode);
  thread_mutex_lock(&pm_);
  auto iter = m2c_.find(mobile);
  if (((iter != m2c_.end()) && (icode != iter->second)) ||
      (iter == m2c_.end())) {
    // TODO: 这里return会死锁，考虑用LOCK_GUARD？
    // return new LoginRspEv(ERRC_INVALID_DATA);
    loginEv = new LoginRspEv(ERRC_INVALID_DATA);
  }
  thread_mutex_unlock(&pm_);
  if (loginEv) return loginEv;

  // 如果验证成功，则要判断用户在数据库是否成功，不成功则插入用户记录
  std::shared_ptr<MySqlConnection> mysqlconn(new MySqlConnection);

  st_env_config conf_args = Iniconfig::getInstance()->getconfig();
  if (!mysqlconn->Init(conf_args._db_ip.c_str(), conf_args._db_port,
                       conf_args._db_user.c_str(), conf_args._db_pwd.c_str(),
                       conf_args._db_name.c_str())) {
    LOG_ERROR(
        "UserEventHandler::handle_login_req - Database connection init "
        "failed\n");
    return new LoginRspEv(ERRC_PROCESS_FAILED);
  }

  UserService us(mysqlconn);
  bool res = false;
  if (!us.exist(mobile)) {
    res = us.insert(mobile);
    if (!res) {
      LOG_ERROR("insert user(%s) to db failed", mobile.c_str());
      return new LoginRspEv(ERRC_PROCESS_FAILED);
    }
  }

  return new LoginRspEv(ERRC_SUCCESS);
}