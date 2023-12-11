#ifndef BIKE_EVENT_HANDLER_H_
#define BIKE_EVENT_HANDLER_H_
#include <string>

#include "ievent.h"
#include "eventtype.h"
#include "globals.h"

class iEventHandler {
 public:
  iEventHandler(const char *name) : name_(name) {}
  virtual ~iEventHandler() {}
  virtual iEvent *handle(const iEvent *ev) { return NULL; }
  // TODO: std::string &get_name() const { return name_; }
  std::string &get_name() { return name_; }

 private:
  std::string name_;
};

#endif  // BIKE_EVENT_HANDLER_H_
