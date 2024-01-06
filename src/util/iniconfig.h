#ifndef SHBK_COMMON_INICONFIG_
#define SHBK_COMMON_INICONFIG_

#include "common.h"
#include "configdef.h"

class Iniconfig {
 protected:
  Iniconfig();

 public:
  ~Iniconfig();
  static Iniconfig* getInstance();
  bool loadfile(const std::string& path);
  const _st_env_config& getconfig();

 private:
  _st_env_config _config;
  bool _is_loaded;
  static Iniconfig* _inst;
};
#endif  // SHBK_COMMON_INICONFIG
