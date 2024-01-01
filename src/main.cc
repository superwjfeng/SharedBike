#include <unistd.h>

#include <iostream>

#include "DispatchMsgService.h"
#include "NetworkInterface.h"
#include "bike.pb.h"
#include "event.h"
#include "events_def.h"
#include "user_event_handler.h"
#include "iniconfig.h"
#include "logger.h"

// TODO: 重新研究下头文件的组织

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cout << "Please input shbk <config file path> <log file config>!"
              << std::endl;
    return -1;
  }

  if (!Logger::getInstance()->init(std::string(argv[2]))) {
    fprintf(stderr, "init log module failed.\n");
    return -2;
  }

  Iniconfig config;
  if (!config.loadfile(std::string(argv[1]))) {
    // std::cout << "load" + std::string(argv[1]) + "is failed" << std::endl;
    LOG_ERROR("load %s failed.", argv[1]);
    // 等价于 LOG_ERROR Logger::instance()->GetLogger()->error()
    return -3;
  }

  // iEvent *ie = new iEvent(EEVENTID_GET_MOBILE_CODE_REQ, 2);
  // MobileCodeReqEv me("18266666666");
  // me.dump(std::cout);

  // MobileCodeRspEv mcre(200, 666666);
  // mcre.dump(std::cout);

  // MobileCodeRspEv mcre2(ERRC_BIKE_IS_TAKEN, 666666);
  // mcre2.dump(std::cout);

  // MobileCodeRspEv mcre3(500, 666666);
  // mcre3.dump(std::cout);

  // TODO: 做一个接口来隐藏UserEventHandler的初始化
  UserEventHandler uehl;
  // uehl.handle(&me);

  DispatchMsgService *DMS = DispatchMsgService::getInstance();
  DMS->open();

  // BOOL test = DMS->open();
  // std::cout << test << std::endl;

  // MobileCodeReqEv *pmcre = new MobileCodeReqEv("18266666666");
  // DMS->enqueue(pmcre);
  // sleep(5);

  NetworkInterface *NTIF = new NetworkInterface();
  NTIF->start(8888);
  while (1 == 1) {
    NTIF->network_event_dispatch();
    sleep(1);
    LOG_DEBUG("network_event_dispatch ...\n");
  }

  DMS->close();
  // sleep(5);
  return 0;
}