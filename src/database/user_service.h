#ifndef BRKS_SVR_USER_SERVICE_H_
#define BRKS_SVR_USER_SERVICE_H_

#include <memory>

#include "events_def.h"
#include "globals.h"
#include "sqlconnection.h"

class UserService {
 public:
  UserService(std::shared_ptr<MySqlConnection> sql_conn)
      : sql_conn_(sql_conn) {}
  bool exist(std::string &mobile);
  bool insert(const std::string &mobile);

 private:
  std::shared_ptr<MySqlConnection> sql_conn_;
};

#endif  // BRKS_SVR_USER_SERVICE_H_