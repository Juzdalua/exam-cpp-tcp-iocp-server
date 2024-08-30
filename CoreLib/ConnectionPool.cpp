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
