#pragma once
#include"const.h"

class HttpConnection:public std::enable_shared_from_this<HttpConnection>
{
	friend class LogicSystem;
public:
	HttpConnection(tcp::socket socket);
	void Start();
private:
	//��ʱ���
	void CheckDeadline();
	//�յ����ݺ�Ӧ��
	void WriteResponse();
	//��������,������������
	void HandleRequest();
	tcp::socket _socket;
	//���նԶ˷���������
	beast::flat_buffer _buffer{8192};
	//���նԶ�����
	http::request<http::dynamic_body> _request;
	//�ظ��Զ�
	http::response<http::dynamic_body> _response;
	//��ʱ����ⳬʱ
	net::steady_timer deadline_{
		_socket.get_executor(), std::chrono::seconds(60)
	};
};

