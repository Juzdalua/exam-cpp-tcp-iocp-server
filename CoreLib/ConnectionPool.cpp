#include "pch.h"
#include "ConnectionPool.h"

// ConnectionPool ������
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

// ConnectionPool �Ҹ���
ConnectionPool::~ConnectionPool() {
    // unique_ptr�� �Ҹ��ڿ��� delete�� ȣ��Ǿ� ��� Ŀ�ؼ��� �ڵ����� �����մϴ�.
    cout << "Disconnect DB" << endl;
    pool.clear();
}

// Ŀ�ؼ��� �������� �޼���
unique_ptr<sql::Connection> ConnectionPool::getConnection() {
    lock_guard<mutex> lock(mtx);
    if (!pool.empty()) {
        auto conn = move(pool.back());
        pool.pop_back();
        return conn;
    }
    return nullptr;
}

// Ŀ�ؼ��� ��ȯ�ϴ� �޼���
void ConnectionPool::releaseConnection(unique_ptr<sql::Connection> conn) {
    lock_guard<mutex> lock(mtx);
    pool.push_back(move(conn));
}

// �������� �޾Ƽ� �����ϴ� �Լ�
unique_ptr<sql::ResultSet> executeQuery(ConnectionPool& pool, const string& query, vector<string>& params) {
    try {
        auto conn = pool.getConnection();
        if (conn) {
            // Prepare the statement
            unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(query));

            for (int32 i = 0; i < params.size(); ++i) {
                pstmt->setString(i + 1, params[i]);
            }

            cout << "Executing Query: " << query << endl;
            cout << "Parameters: ";
            for (const auto& param : params) {
                cout << param << " ";
            }
            cout << endl;

            // Execute the query
            unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

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
        cerr << "SQLException: " << e.what() << endl;
        cerr << "Error code: " << e.getErrorCode() << endl;
        cerr << "SQL state: " << e.getSQLState() << endl;
    }
}
