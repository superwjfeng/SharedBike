#include "logger.h"

Logger Logger::instance_;  // TODO: 为什么单例这里要定义？

bool Logger::init(const std::string &log_conf_file) {
  try {
    log4cpp::PropertyConfigurator::configure(log_conf_file);
  } catch (log4cpp::ConfigureFailure &f) {
    std::cerr << "load log config file" << log_conf_file.c_str()
              << "failed with result : " << f.what() << std::endl;
    return false;
  }

  category_ = &log4cpp::Category::getRoot();
  return true;
}