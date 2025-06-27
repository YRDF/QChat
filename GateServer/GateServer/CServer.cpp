#include "CServer.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port):_ioc(ioc),
	_acceptor(ioc,tcp::endpoint(tcp::v4(),port)),_socket(ioc){

}

void CServer::Start(){
	auto self = shared_from_this();
	//asio������_accpetor���첽����
	_acceptor.async_accept(_socket, [self](beast::error_code ec) {
		try {
			//����ͷ���������ӣ�����������������
			if (ec) {
				self->Start();
				return;
			}
			//���������ӣ�����HttpConnection�����������
			std::make_shared<HttpConnection>(std::move(self->_socket))->Start();
			//��������
			self->Start();
		}
		catch (const std::exception& ec) {
			std::cout << "exception is " << ec.what() << std::endl;
			self->Start();
			}
		});
}
