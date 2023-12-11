#ifndef SHBK_LOGGER_
#define SHBK_LOGGER_

#include <log4cpp/Category.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/PropertyConfigurator.hh>

#include "common.h"

class Logger {
 public:
  bool init(const std::string &log_conf_file);
  static Logger *instance() { return &instance_; }
  log4cpp::Category *GetHandle() { return category_; }

 protected:
  static Logger instance_;  // TODO: 复习单例模式
  log4cpp::Category *category_;
};

#define LOG_INFO Logger::instance()->GetHandle()->info
#define LOG_DEBUG Logger::instance()->GetHandle()->debug
#define LOG_ERROR Logger::instance()->GetHandle()->error
#define LOG_WARN Logger::instance()->GetHandle()->warn

#endif  // SHBK_LOGGER_