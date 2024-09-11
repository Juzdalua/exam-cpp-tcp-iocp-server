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
bool executeQueryUpdate(ConnectionPool& pool, const string& query, const vector<string>& params) {
    try {
        auto conn = pool.getConnection();
        if (conn) {
            auto pstmt = conn->prepareStatement(query);

            for (int32 i = 0; i < params.size(); ++i) {
                pstmt->setString(i + 1, params[i]);
            }

            pstmt->executeUpdate();  // 쿼리 실행

            pool.releaseConnection(move(conn));
            return true;
        }
        else {
            cerr << "Failed to get a connection from the pool." << endl;
        }
    }
    catch (sql::SQLException& e) {
        cerr << "[SQLException: " << e.what() << "]" << endl;
        cerr << "[Error code: " << e.getErrorCode() << "]" << endl;
        cerr << "[SQL state: " << e.getSQLState() << "]" << endl;
    }
    return false;
}

unique_ptr<sql::ResultSet> executeQuery(ConnectionPool& pool, const string& query, vector<string>& params) {
    try {
        auto conn = pool.getConnection();
        if (conn) {

            // Prepare the statement
            auto pstmt = conn->prepareStatement(query);

            for (int32 i = 0; i < params.size(); ++i) {
                pstmt->setString(i + 1, params[i]);
            }

            //LogQueryParams(query, params);

            // Execute the query
            auto resultSetPtr = pstmt->executeQuery();
            unique_ptr<sql::ResultSet> res(resultSetPtr);

            // Process results
            /*while (res->next()) {
                cout << res->getString(1) << " " << res->getString(2) << endl;
            }*/

            // Release the connection
            pool.releaseConnection(move(conn));
            return res;
        }
        else {
            cerr << "Failed to get a connection from the pool." << endl;
        }
    }
    catch (sql::SQLException& e) {
        cerr << "[SQLException: " << e.what() << "]" << endl;
        cerr << "[Error code: " << e.getErrorCode() << "]" << endl;
        cerr << "[SQL state: " << e.getSQLState() << "]" << endl;
    }
}

/*

vector<string> queries = {
    "INSERT INTO first_table (column1, column2) VALUES (?, ?)",
    "INSERT INTO second_table (column1, column2) VALUES (?, ?)"
};

vector<vector<string>> params = {
    {"value1", "value2"},  // 첫 번째 쿼리에 대한 파라미터
    {"value3", "value4"}   // 두 번째 쿼리에 대한 파라미터
};

bool result = executeTransaction(pool, queries, params);

*/
bool executeTransaction(ConnectionPool& pool, const vector<string>& queries, const vector<vector<string>>& params) {
    try {
        auto conn = pool.getConnection();
        if (conn) {
            conn->setAutoCommit(false);  // 트랜잭션 시작

            bool success = true;
            try {
                for (size_t i = 0; i < queries.size(); ++i) {
                    unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(queries[i]));

                    for (size_t j = 0; j < params[i].size(); ++j) {
                        pstmt->setString(j + 1, params[i][j]);
                    }

                    /*cout << "Executing Query: " << queries[i] << endl;
                    cout << "Parameters: ";
                    for (const auto& param : params[i]) {
                        cout << param << " ";
                    }
                    cout << endl;*/

                    pstmt->executeUpdate();
                }

                conn->commit();  // 커밋
            }
            catch (sql::SQLException& e) {
                conn->rollback();  // 롤백
                cerr << "SQLException: " << e.what() << endl;
                cerr << "Error code: " << e.getErrorCode() << endl;
                cerr << "SQL state: " << e.getSQLState() << endl;
                success = false;
            }

            pool.releaseConnection(move(conn));
            return success;
        }
        else {
            cerr << "Failed to get a connection from the pool." << endl;
            return false;
        }
    }
    catch (sql::SQLException& e) {
        cerr << "SQLException: " << e.what() << endl;
        cerr << "Error code: " << e.getErrorCode() << endl;
        cerr << "SQL state: " << e.getSQLState() << endl;
        return false;
    }
}

void LogQueryParams(const string& query, vector<string>& params)
{
    cout << "Executing Query: " << query << endl;
    cout << "Parameters: ";
    for (const auto& param : params) {
        cout << param << " ";
    }
    cout << endl;
}