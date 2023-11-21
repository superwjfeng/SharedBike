#ifndef SHBK_LOGGER_
#define SHBK_LOGGER_

#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/PatternLayout.hh>

#include "common.h"

class Logger {
 public:
  bool init(const std::string &log_conf_file);
  static Logger *instance() { return &instance_; }
  log4cpp::Category *GetHandle() { return category_; };

 protected:
  static Logger instance_;
  log4cpp::Category *category_;
};
#endif  // SHBK_LOGGER_