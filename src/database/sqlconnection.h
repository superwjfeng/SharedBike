#ifndef DATASTORE_MYSQL_CONNECTION_H_
#define DATASTORE_MYSQL_CONNECTION_H_

#include <mysql/errmsg.h>
#include <mysql/mysql.h>

#include <cassert>
#include <string>
#include <cstring>

#include "globals.h"
#include "logger.h"

// 封装管理查询
class SqlRecordSet {
 public:
  SqlRecordSet() : m_pRes_(NULL) {}
  explicit SqlRecordSet(MYSQL_RES* pRes) { m_pRes_ = pRes; }
  ~SqlRecordSet() {
    if (m_pRes_) {
      mysql_free_result(m_pRes_);
    }
  }

  inline void SetResult(MYSQL_RES* pRes) {
    assert(m_pRes_ == NULL);
    if (m_pRes_) {
      LOG_WARN(
          "the MYSQL_RES has already stored result, it may cause memory leak.");
    }
    m_pRes_ = pRes;
  }

  inline MYSQL_RES* GetResult() { return m_pRes_; }

  void FetchRow(MYSQL_ROW& row) { row = mysql_fetch_row(m_pRes_); }

  inline i32 GetRowCount() { return m_pRes_->row_count; }

 private:
  MYSQL_RES* m_pRes_;
};

// 封装管理连接
class MySqlConnection {
 public:
  MySqlConnection();
  ~MySqlConnection();
  MYSQL* Mysql() { return mysql_; }

  bool Init(const char* szHost, int nPort, const char* szUser,
            const char* szPasswd, const char* szDb);

  // 没有返回结果集的查询
  bool Execute(const char* szSql);
  // 有返回结果集的查询
  bool Execute(const char* szSql, SqlRecordSet& recordSet);  // MYSQL_RES *

  // 对query中的特殊字符进行转义，转义后的pDest要比pSrc多预留一些，保守起见2倍够了
  int EscapeString(const char* pSrc, int nSrcLen, char* pDest);

  void Close();

  const char* GetErrInfo();

  void Reconnect();

 private:
  MYSQL* mysql_;
};

#endif  // DATASTORE_MYSQL_CONNECTION_H_