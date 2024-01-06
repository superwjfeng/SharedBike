#include "sqlconnection.h"

// MySQL API 是纯C的，所以这里用了malloc，而不是new
MySqlConnection::MySqlConnection() : mysql_(NULL) {
  mysql_ = (MYSQL*)malloc(sizeof(MYSQL));
}

MySqlConnection::~MySqlConnection() {
  if (mysql_ != NULL) {
    mysql_close(mysql_);
    free(mysql_);
    mysql_ = NULL;
  }
  // return;
}

bool MySqlConnection::Init(const char* szHost, int nPort, const char* szUser,
                           const char* szPasswd, const char* szDb) {
  LOG_INFO("MySqlConnection::Init: enter Init \n");
  if ((mysql_ = (mysql_init(mysql_))) == NULL) {
    LOG_ERROR("MySqlConnection::Init: init failed %s, %d", this->GetErrInfo(),
              errno);
    return false;
  }

  // 设置自动重连
  char cAuto = 1;
  // 成功返回0
  if (mysql_options(mysql_, MYSQL_OPT_RECONNECT, &cAuto) != 0) {
    LOG_ERROR("MySqlConnection::Init: reconnect failed %s, %d",
              this->GetErrInfo(), errno);
  }

  if ((mysql_ = mysql_real_connect(mysql_, szHost, szUser, szPasswd, szDb,
                                   nPort, NULL, 0)) == NULL) {
    LOG_ERROR("MySqlConnection::Init: connect failed %s, %d",
              this->GetErrInfo(), errno);
    return false;
  }

  LOG_INFO("MySqlConnection::Init: Init success\n");
  return true;
}

bool MySqlConnection::Execute(const char* szSql) {
  if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0) {
    if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR) {
      Reconnect();
    }
    return false;
  }
  return true;
}

bool MySqlConnection::Execute(const char* szSql, SqlRecordSet& recordSet) {
  if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0) {
    if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR) {
      Reconnect();
    }
    return false;
  }

  MYSQL_RES* pRes = mysql_store_result(mysql_);
  if (!pRes) return NULL;
  recordSet.SetResult(pRes);
  return true;
}

int MySqlConnection::EscapeString(const char* pSrc, int nSrcLen, char* pDest) {
  if (!mysql_) return 0;
  // TODO: 研究一下这个转义
  return mysql_real_escape_string(mysql_, pDest, pSrc, nSrcLen);
}

void MySqlConnection::Close() { mysql_close(mysql_); }

const char* MySqlConnection::GetErrInfo() { return mysql_error(mysql_); }

void MySqlConnection::Reconnect() { mysql_ping(mysql_); }