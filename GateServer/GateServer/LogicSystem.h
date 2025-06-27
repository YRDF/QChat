#pragma once
#include"const.h"

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
    ~LogicSystem();
    //注册回调函数和调用回调函数
    void RegGet(std::string, HttpHandler handler);
    bool HandleGet(std::string, std::shared_ptr<HttpConnection>);
private:
    LogicSystem();
    //存储http的post请求和get请求的回调函数
    std::map<std::string, HttpHandler> _post_handlers;
    std::map<std::string, HttpHandler> _get_handlers;
};

