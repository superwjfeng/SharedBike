#include "common.h"
#include "configdef.h"
#include "iniconfig.h"
#include "logger.h"

// sb.exe ./conf/shared_bike.ini
int main(int argc, char **argv) {
  if (argc < 3) {
    std::cout << "Please input shbk <config file path> <log file config>!"
              << std::endl;
    return -1;
  }

  if (!Logger::instance()->init(std::string(argv[2]))) {
    fprintf(stderr, "init log module failed.\n");
    return -2;
  }

  Iniconfig config;
  if (!config.loadfile(std::string(argv[1]))) {
    // std::cout << "load" + std::string(argv[1]) + "is failed" << std::endl;
    LOG_ERROR("load %s failed.", argv[1]);
    // 等价于 LOG_ERROR Logger::instance()->GetHandle()->error()
    return -3;
  }

  _st_env_config conf_args = config.getconfig();
  // printf("[database] ip: %s, port %d, user: %s, pwd: %s, db: %s\n",
  //        conf_args._db_ip.c_str(), conf_args._db_port,
  //        conf_args._db_user.c_str(), conf_args._db_pwd.c_str(),
  //        conf_args._db_name.c_str());
  // printf("[server] port: %d\n", conf_args._svr_port);
  LOG_INFO("[database] ip: %s, port %d, user: %s, pwd: %s, db: %s\n",
         conf_args._db_ip.c_str(), conf_args._db_port,
         conf_args._db_user.c_str(), conf_args._db_pwd.c_str(),
         conf_args._db_name.c_str());
  LOG_INFO("[server] port: %d\n", conf_args._svr_port);

  return 0;
}