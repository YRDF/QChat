#pragma once
#include"const.h"

class HttpConnection:public std::enable_shared_from_this<HttpConnection>
{
	friend class LogicSystem;
public:
	HttpConnection(tcp::socket socket);
	void Start();
private:
	//超时检测
	void CheckDeadline();
	//收到数据后应答
	void WriteResponse();
	//处理请求,解析包体内容
	void HandleRequest();
	tcp::socket _socket;
	//接收对端发来的数据
	beast::flat_buffer _buffer{8192};
	//接收对端请求
	http::request<http::dynamic_body> _request;
	//回复对端
	http::response<http::dynamic_body> _response;
	//定时器检测超时
	net::steady_timer deadline_{
		_socket.get_executor(), std::chrono::seconds(60)
	};
};

