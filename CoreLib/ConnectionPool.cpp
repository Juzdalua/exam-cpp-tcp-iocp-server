#include "pch.h"
#include "ConnectionPool.h"

// ConnectionPool 생성자
ConnectionPool::ConnectionPool(int32 size) 
{
    ENV::loadEnvFile("../CoreLib/.env");

    driver = get_driver_instance();

    try {
        for (size_t i = 0; i < size; ++i) {
            unique_ptr<sql::Connection> conn(driver->connect(
                ENV::getEnvVariable("DATABASE_HOST"),
                ENV::getEnvVariable("DATABASE_USER"),
                ENV::getEnvVariable("DATABASE_PASSWORD")
            ));
            conn->setSchema(ENV::getEnvVariable("DATABASE_SCHEMA"));
            pool.push_back(move(conn));
        }
    }
    catch (sql::SQLException& e) {
        cerr << "SQLException: " << e.what() << endl;
        cerr << "Error code: " << e.getErrorCode() << endl;
        cerr << "SQL state: " << e.getSQLState() << endl;
    }
}

// ConnectionPool 소멸자
ConnectionPool::~ConnectionPool() {
    // unique_ptr의 소멸자에서 delete가 호출되어 모든 커넥션을 자동으로 해제합니다.
    cout << "Disconnect DB" << endl;
    pool.clear();
}

// 커넥션을 가져오는 메서드
unique_ptr<sql::Connection> ConnectionPool::getConnection() {
    lock_guard<mutex> lock(mtx);
    if (!pool.empty()) {
        auto conn = move(pool.back());
        pool.pop_back();
        return conn;
    }
    return nullptr;
}

// 커넥션을 반환하는 메서드
void ConnectionPool::releaseConnection(unique_ptr<sql::Connection> conn) {
    lock_guard<mutex> lock(mtx);
    pool.push_back(move(conn));
}

// 쿼리문을 받아서 실행하는 함수
void executeQuery(ConnectionPool& pool, const string& query) {
    try {
        auto conn = pool.getConnection();
        if (conn) {
            // Prepare the statement
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(query));

            // Execute the query
            unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            // Process results
            while (res->next()) {
                cout << res->getString(1) << " " << res->getString(2) << endl;
            }

            // Release the connection
            pool.releaseConnection(move(conn));
        }
        else {
            cerr << "Failed to get a connection from the pool." << endl;
        }
    }
    catch (sql::SQLException& e) {
        cerr << "SQLException: " << e.what() << endl;
        cerr << "Error code: " << e.getErrorCode() << endl;
        cerr << "SQL state: " << e.getSQLState() << endl;
    }
}

void worker(ConnectionPool& pool) {
    string query = "SELECT * FROM user";
    executeQuery(pool, query);
}
