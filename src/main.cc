#include <unistd.h>

#include <iostream>

#include "DispatchMsgService.h"
#include "NetworkInterface.h"
#include "bike.pb.h"
#include "event.h"
#include "events_def.h"
#include "user_event_handler.h"

// TODO: 重新研究下头文件的组织

int main() {
  iEvent *ie = new iEvent(EEVENTID_GET_MOBILE_CODE_REQ, 2);
  MobileCodeReqEv me("18266666666");
  me.dump(std::cout);

  MobileCodeRspEv mcre(200, 666666);
  mcre.dump(std::cout);

  MobileCodeRspEv mcre2(ERRC_BIKE_IS_TAKEN, 666666);
  mcre2.dump(std::cout);

  MobileCodeRspEv mcre3(500, 666666);
  mcre3.dump(std::cout);

  UserEventHandler uehl;
  uehl.handle(&me);

  DispatchMsgService *DMS = DispatchMsgService::getInstance();
  DMS->open();

  // MobileCodeReqEv *pmcre = new MobileCodeReqEv("18266666666");
  // DMS->enqueue(pmcre);
  // sleep(5);

  // DMS->close();
  // sleep(5);

  NetworkInterface *NTIF = new NetworkInterface();
  NTIF->start(8888);
  while (1 == 1) {
    NTIF->network_event_dispatch();
    sleep(1);
    // LOG_DEBUG("network_event_dispatch ...\n");
  }
  return 0;
}