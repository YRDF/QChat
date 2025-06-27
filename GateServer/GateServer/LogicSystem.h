#pragma once
#include"const.h"

class HttpConnection;
typedef std::function<void(std::shared_ptr<HttpConnection>)> HttpHandler;
class LogicSystem:public Singleton<LogicSystem>
{
	friend class Singleton<LogicSystem>;
public:
    ~LogicSystem();
    //ע��ص������͵��ûص�����
    void RegGet(std::string, HttpHandler handler);
    bool HandleGet(std::string, std::shared_ptr<HttpConnection>);
private:
    LogicSystem();
    //�洢http��post�����get����Ļص�����
    std::map<std::string, HttpHandler> _post_handlers;
    std::map<std::string, HttpHandler> _get_handlers;
};

