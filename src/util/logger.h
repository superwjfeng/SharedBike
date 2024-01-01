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
  static Logger *getInstance() { return &instance_; }
  log4cpp::Category *GetLogger() { return root_category_; }

 protected:
  static Logger instance_;  // TODO: 复习单例模式
  log4cpp::Category *root_category_;
};

#define LOG_INFO Logger::getInstance()->GetLogger()->info
#define LOG_DEBUG Logger::getInstance()->GetLogger()->debug
#define LOG_ERROR Logger::getInstance()->GetLogger()->error
#define LOG_WARN Logger::getInstance()->GetLogger()->warn

#endif  // SHBK_LOGGER_