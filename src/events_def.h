#ifndef BIKE_EVENT_DEF_H_
#define BIKE_EVENT_DEF_H_

#include <iostream>
#include <sstream>
#include <string>

#include "bike.pb.h"
#include "globals.h"
#include "ievent.h"

class MobileCodeReqEv : public iEvent {
 public:
  MobileCodeReqEv(const std::string &mobile)
      : iEvent(EEVENTID_GET_MOBILE_CODE_REQ, iEvent::generateSegNo()) {
    msg_.set_mobile(mobile);
  }

  const std::string &get_mobile() { return msg_.mobile(); }
  virtual std::ostream &dump(std::ostream &out) const;
  virtual i32 ByteSize() { return msg_.ByteSize(); }
  virtual bool SerializeToArray(char *buf, int len) {
    return msg_.SerializeToArray(buf, len);
  }
  
 private:
  bike::mobile_request msg_;
};

class MobileCodeRspEv : public iEvent {
 public:
  MobileCodeRspEv(i32 code, i32 icode)
      : iEvent(EEVENTID_GET_MOBILE_CODE_RSP, iEvent::generateSegNo()) {
    msg_.set_code(code);
    msg_.set_icode(icode);
    msg_.set_data(getReasonByErrorCode(code));
  }

  const i32 get_code() { return msg_.code(); }
  const i32 get_icode() { return msg_.icode(); }
  const std::string &get_data() { return msg_.data(); }

  virtual std::ostream &dump(std::ostream &out) const;
  virtual i32 ByteSize() { return msg_.ByteSize(); }
  virtual bool SerializeToArray(char *buf, int len) {
    return msg_.SerializeToArray(buf, len);
  }

 private:
  bike::mobile_response msg_;
};

class ExitRspEv : public iEvent {
 public:
  ExitRspEv() : iEvent(EEVENTID_EXIT_RSP, iEvent::generateSegNo()) {}
};

#endif  // BIKE_EVENT_DEF_H_z