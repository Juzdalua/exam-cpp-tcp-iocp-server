#include "pch.h"
#include "ConnectionPool.h"

// ConnectionPool ������
ConnectionPool::ConnectionPool(size_t size) {
    ENV::loadEnvFile("../CoreLib/.env");
    driver = get_driver_instance();

    try {
        for (size_t i = 0; i < size; ++i) {
            std::unique_ptr<sql::Connection> conn(driver->connect(
                ENV::getEnvVariable("DATABASE_HOST"),
                ENV::getEnvVariable("DATABASE_USER"),
                ENV::getEnvVariable("DATABASE_PASSWORD")
            ));
            conn->setSchema(ENV::getEnvVariable("DATABASE_SCHEMA"));
            pool.push_back(std::move(conn));
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQL state: " << e.getSQLState() << std::endl;
    }
}

// ConnectionPool �Ҹ���
ConnectionPool::~ConnectionPool() {
    // unique_ptr�� �Ҹ��ڿ��� delete�� ȣ��Ǿ� ��� Ŀ�ؼ��� �ڵ����� �����մϴ�.
}

// Ŀ�ؼ��� �������� �޼���
std::unique_ptr<sql::Connection> ConnectionPool::getConnection() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!pool.empty()) {
        auto conn = std::move(pool.back());
        pool.pop_back();
        return conn;
    }
    return nullptr;
}

// Ŀ�ؼ��� ��ȯ�ϴ� �޼���
void ConnectionPool::releaseConnection(std::unique_ptr<sql::Connection> conn) {
    std::lock_guard<std::mutex> lock(mtx);
    pool.push_back(std::move(conn));
}

// �������� �޾Ƽ� �����ϴ� �Լ�
void executeQuery(ConnectionPool& pool, const std::string& query) {
    try {
        auto conn = pool.getConnection();
        if (conn) {
            // Prepare the statement
            std::unique_ptr<sql::PreparedStatement> pstmt(conn->prepareStatement(query));

            // Execute the query
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

            // Process results
            while (res->next()) {
                std::cout << res->getString(1) << " " << res->getString(2) << std::endl;
            }

            // Release the connection
            pool.releaseConnection(std::move(conn));
        }
        else {
            std::cerr << "Failed to get a connection from the pool." << std::endl;
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "SQLException: " << e.what() << std::endl;
        std::cerr << "Error code: " << e.getErrorCode() << std::endl;
        std::cerr << "SQL state: " << e.getSQLState() << std::endl;
    }
}
