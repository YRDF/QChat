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

/*超时检测 检测mysql上次增删改查的时间。有一个线程去查，
上次操作的时间和这次操作的时间是否超过了规定的阈值
*/
class SqlConnection {
public:
	SqlConnection(sql::Connection* con, int64_t lasttime) :_con(con), _last_oper_time(lasttime) {}
	std::unique_ptr<sql::Connection> _con;
	int64_t _last_oper_time;
};

/*
	_pool直接操作mysql，dao调用pool
*/
class MySqlPool {
public:
	MySqlPool(const std::string& url, const std::string& user, const std::string& pass, const std::string& schema, int poolSize);
    //检测是否超时
    void checkConnection();
    //池中取出连接
    std::unique_ptr<SqlConnection> getConnection();
    //连接返回池
    void returnConnection(std::unique_ptr<SqlConnection> con);
    //关闭连接
    void Close();
    ~MySqlPool();
private:
    std::string url_;
    std::string user_;
    std::string pass_;
    //要使用哪个数据库
    std::string schema_;
    int poolSize_;
    //每个连接都放到队列
    std::queue<std::unique_ptr<SqlConnection>> pool_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> b_stop_;
    //检查是否超时线程
    std::thread _check_thread;
};

//基本用户信息
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
    //一系列增删改查操作，借助pool完成
    //注册用户
    int RegUser(const std::string& name, const std::string& email, const std::string& pwd);
private:
    std::unique_ptr<MySqlPool> pool_;
};

