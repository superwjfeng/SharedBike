#include "common.h"
#include "iniconfig.h"
#include "configdef.h"

//sb.exe ./conf/shared_bike.ini
int main(int argc, char **argv) {
  if (argc !=2) {
    std::cout << "Please input shbk <config file path>!" << std::endl;
    return -1;
  }

  Iniconfig config;
  if (!config.loadfile(std::string(argv[1]))) {
    std::cout << "load" + std::string(argv[1]) + "is failed" << std::endl;
    return -2;
  }

  _st_env_config conf_args = config.getconfig();
  //std::cout << "[database] ip:" + 

  return 0;
}