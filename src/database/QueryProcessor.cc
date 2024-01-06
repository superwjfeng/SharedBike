#include "QueryProcessor.h"

QueryProcessor::QueryProcessor(std::shared_ptr<MySqlConnection> conn)
    : mysqlconn_(conn), ueh_(new UserEventHandler()) {}

bool QueryProcessor::init() {
  SqlTables tables(mysqlconn_);
  tables.CreateUserInfoTable();
  tables.CreateBikeInfoTable();
  return true;
}

// reset 无参，用于解除所有权并释放原有的资源
QueryProcessor::~QueryProcessor() { ueh_.reset(); }