#ifndef BRKS_QUERY_PROCESSOR_H_
#define BRKS_QUERY_PROCESSOR_H_

#include <memory>

#include "user_event_handler.h"
#include "sqlconnection.h"
#include "SqlTables.h"

class QueryProcessor {
 public:
  QueryProcessor(std::shared_ptr<MySqlConnection> conn);
  bool init();
  virtual ~QueryProcessor();

 private:
  std::shared_ptr<MySqlConnection> mysqlconn_;
  std::shared_ptr<UserEventHandler> ueh_;  // 账户管理系统
};

#endif  // BRKS_QUERY_PROCESSOR_H_