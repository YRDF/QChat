#pragma once
#include "const.h"
#include <thread>
#include "ConfigMgr.h"
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/exception.h>

/*��ʱ��� ���mysql�ϴ���ɾ�Ĳ��ʱ�䡣��һ���߳�ȥ�飬
�ϴβ�����ʱ�����β�����ʱ���Ƿ񳬹��˹涨����ֵ
*/
class SqlConnection {
public:
	SqlConnection(sql::Connection* con, int64_t lasttime) :_con(con), _last_oper_time(lasttime) {}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};

/*
	_poolֱ�Ӳ���mysql��dao����pool
*/
class MySqlPool {
public:
	MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize);
    //����Ƿ�ʱ
    void checkConnection();
    //����ȡ������
    std::unique_ptr<SqlConnection> getConnection();
    //���ӷ��س�
    void returnConnection(std::unique_ptr<SqlConnection> con);
    //�ر�����
    void Close();
    ~MySqlPool();
private:
    std::string url_;
    std::string user_;
    std::string pass_;
    //Ҫʹ���ĸ����ݿ�
    std::string schema_;
    int poolSize_;
    //ÿ�����Ӷ��ŵ�����
    std::queue<std::unique_ptr<SqlConnection>> pool_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> b_stop_;
    //����Ƿ�ʱ�߳�
    std::thread _check_thread;
};

//�����û���Ϣ
struct UserInfo {
    std::string name;
    std::string pwd;
    int uid;
    std::string email;
};


class MySqlDao
{
public:
    MySqlDao();
    ~MySqlDao();
    //һϵ����ɾ�Ĳ����������pool���
    //ע���û�
    int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
private:
    std::unique_ptr<MySqlPool> pool_;
};

