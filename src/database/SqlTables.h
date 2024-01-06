#ifndef BRKS_COMMON_DATASERVER_SQLTABLES_H_
#define BRKS_COMMON_DATASERVER_SQLTABLES_H_

#include <memory>

#include "globals.h"
#include "sqlconnection.h"

class SqlTables {
 public:
  SqlTables(std::shared_ptr<MySqlConnection> sqlConn) : sqlConn_(sqlConn) {}

  bool CreateUserInfoTable() {
    const char* pUserInfoTable =
        "\
    CREATE TABLE IF NOT EXISTS userinfo ( \
    id         int(16)      NOT NULL PRIMARY KEY AUTO_INCREMENT, \
    mobile     varchar(16)  NOT NULL default '13000000000', \
    username   varchar(128) NOT NULL default '', \
    verify     int(4)       NOT NULL default '0', \
    registertm timestamp    NOT NULL default CURRENT_TIMESTAMP, \
    money      int(4)       NOT NULL default 0, \
    INDEX      mobile_index(mobile) \
    )";

    if (!sqlConn_->Execute(pUserInfoTable)) {
      LOG_ERROR("create userinfo table failed, error msg: %s",
                sqlConn_->GetErrInfo());
      return false;
    }
    return true;
  }

  bool CreateBikeInfoTable() {
    const char* pBikeInfoTable =
        "\
    CREATE TABLE IF NOT EXISTS bikeinfo( \
    id        int          NOT NULL PRIMARY KEY AUTO_INCREMENT, \
    devno     int          NOT NULL, \
    status    tinyint(1)   NOT NULL default 0, \
    trouble   int          NOT NULL default 0, \
    tmsg      varchar(256) NOT NULL default '', \
    latitude  double(10,6) NOT NULL default 0, \
    longitude double(10,6) NOT NULL default 0, \
    unique(devno) \
    )";
    if (!sqlConn_->Execute(pBikeInfoTable)) {
      LOG_ERROR("create bikeinfo table failed, error msg: %s",
                sqlConn_->GetErrInfo());
      return false;
    }
    return true;
  }

 private:
  std::shared_ptr<MySqlConnection> sqlConn_;
};

#endif  // BRKS_COMMON_DATASERVER_SQLTABLES_H_