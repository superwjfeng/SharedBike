#ifndef SHBK_COMMON_CONFIGDEF_H_
#define SHBK_COMMON_CONFIGDEF_H_

#include "common.h"

typedef struct st_env_config {
 public:
  // database
  std::string _db_ip;
  unsigned int _db_port;
  std::string _db_user;
  std::string _db_pwd;
  std::string _db_name;
  // server config
  unsigned short _svr_port;

 public:
  st_env_config(){};
  st_env_config(const std::string &db_ip, unsigned int db_port,
                const std::string &db_user, const std::string &db_pwd,
                const std::string &db_name, unsigned short svr_port)
      : _db_ip(db_ip),
        _db_port(db_port),
        _db_user(db_user),
        _db_pwd(db_pwd),
        _db_name(db_name),
        _svr_port(svr_port) {}

  st_env_config &operator=(const st_env_config &config) {
    if (this != &config) {
      _db_ip = config._db_ip;
      _db_port = config._db_port;
      _db_user = config._db_user;
      _db_pwd = config._db_pwd;
      _db_name = config._db_name;
      _svr_port = config._svr_port;
    }
    return *this;
  }

} _st_env_config;

#endif  // SHBK_COMMON_CONFIGDEF_H_