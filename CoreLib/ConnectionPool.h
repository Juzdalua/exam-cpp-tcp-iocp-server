#pragma once
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include "ENV.h"

class ConnectionPool {
public:
    ConnectionPool(int32 size);
    ~ConnectionPool();

    unique_ptr<sql::Connection> getConnection();
    void releaseConnection(unique_ptr<sql::Connection> conn);

private:
    vector<unique_ptr<sql::Connection>> pool;
    sql::Driver* driver;
    mutex mtx;
};

// 쿼리문을 받아서 실행하는 함수
bool executeQueryUpdate(ConnectionPool& pool, const string& query, const vector<string>& params);
unique_ptr<sql::ResultSet> executeQuery(ConnectionPool& pool, const string& query, vector<string>& params);
