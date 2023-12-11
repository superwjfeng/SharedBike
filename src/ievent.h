#ifndef BIKE_EVENT_H_
#define BIKE_EVENT_H_

#include <iostream>
#include <string>

#include "eventtype.h"
#include "globals.h"

class iEvent {
 public:
  iEvent(u32 eid, u32 sn);

  virtual ~iEvent();

  /* for debug */
  virtual std::ostream &dump(std::ostream &out) const { return out; }

  virtual i32 ByteSize() { return 0; }
  virtual bool SerializeToArray(char *buf, int len) { return true; }

  /* generateSegNo will be called in initializer list */
  u32 generateSegNo();
  u32 get_eid() const { return eid_; }
  u32 get_sn() const { return sn_; }
  void *get_args() { return args_; }
  void set_eid(const u32 eid) { eid_ = eid; };
  void set_args(void *args) { args_ = args; }

 private:
  u32 eid_; /* event ID */
  u32 sn_;  /* internal sequential number for each event */
  void *args_;
};

#endif  // BIKE_EVENT_H_