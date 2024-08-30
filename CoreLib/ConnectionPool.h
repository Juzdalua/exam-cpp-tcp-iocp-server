#pragma once
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include "ENV.h"

class ConnectionPool {
public:
    ConnectionPool(size_t size);
    ~ConnectionPool();

    std::unique_ptr<sql::Connection> getConnection();
    void releaseConnection(std::unique_ptr<sql::Connection> conn);

private:
    std::vector<std::unique_ptr<sql::Connection>> pool;
    sql::Driver* driver;
    std::mutex mtx;
};

// 쿼리문을 받아서 실행하는 함수
void executeQuery(ConnectionPool& pool, const std::string& query);

