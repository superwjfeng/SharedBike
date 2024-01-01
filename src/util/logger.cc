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

  root_category_ = &log4cpp::Category::getRoot();
  // TODO: 让error有颜色
  // root_category_->error("\033[1;31mThis is an error message\033[0m");
  
  return true;
}
