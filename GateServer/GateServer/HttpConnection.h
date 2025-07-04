#pragma once
#include"const.h"

class HttpConnection:public std::enable_shared_from_this<HttpConnection>
{
	friend class LogicSystem;
public:
	HttpConnection(boost::asio::io_context & ioc);
	void Start();
	tcp::socket& GetSocket() {
		return _socket;
	}
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
	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
	//解析url
	void PreParseGetParam();
};

