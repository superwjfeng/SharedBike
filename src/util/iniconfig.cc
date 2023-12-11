#include "iniconfig.h"

#include <iniparser/iniparser.h>

#include "common.h"

Iniconfig::Iniconfig() : _is_loaded(false) {}

Iniconfig::~Iniconfig() {}

bool Iniconfig::loadfile(const std::string &path) {
  dictionary *ini = nullptr;
  // if (!_isloaded) {
  ini = iniparser_load(path.c_str());
  if (ini == nullptr) {
    fprintf(stderr, "cannot parse file: %s\n", path.c_str());
    return false;
  }

  const char *ip = iniparser_getstring(ini, "database:ip", "127.0.0.1");
  int port = iniparser_getint(ini, "databse:port", 3306);
  const char *user = iniparser_getstring(ini, "database:user", "root");
  const char *pwd = iniparser_getstring(ini, "database:pwd", "123456");
  const char *db = iniparser_getstring(ini, "databse:db", "dongnaobike");
  int sport = iniparser_getint(ini, "server:port", 9090);

  _config = _st_env_config(std::string(ip), port, std::string(user),
                           std::string(pwd), std::string(db), sport);

  iniparser_freedict(ini);
  _is_loaded = true;
  //}
  return _is_loaded;
}

const _st_env_config &Iniconfig::getconfig() { return _config; }