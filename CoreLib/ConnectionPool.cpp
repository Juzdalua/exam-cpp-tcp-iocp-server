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
bool executeQueryUpdate(ConnectionPool& pool, const string& query, const vector<string>& params) {
    try {
        auto conn = pool.getConnection();
        if (conn) {
            auto pstmt = conn->prepareStatement(query);

            for (int32 i = 0; i < params.size(); ++i) {
                pstmt->setString(i + 1, params[i]);
            }

            pstmt->executeUpdate();  // ���� ����

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
    {"value1", "value2"},  // ù ��° ������ ���� �Ķ����
    {"value3", "value4"}   // �� ��° ������ ���� �Ķ����
};

bool result = executeTransaction(pool, queries, params);

*/
bool executeTransaction(ConnectionPool& pool, const vector<string>& queries, const vector<vector<string>>& params) {
    try {
        auto conn = pool.getConnection();
        if (conn) {
            conn->setAutoCommit(false);  // Ʈ����� ����

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

                conn->commit();  // Ŀ��
            }
            catch (sql::SQLException& e) {
                conn->rollback();  // �ѹ�
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