#include "CServer.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port):_ioc(ioc),
	_acceptor(ioc,tcp::endpoint(tcp::v4(),port)){

}

void CServer::Start(){
	auto self = shared_from_this();
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
	//asio接收器_accpetor的异步接收
	_acceptor.async_accept(new_con->GetSocket(), [self,new_con](beast::error_code ec) {
		try {
			//出错就放弃这个链接，继续监听其它连接
			if (ec) {
				self->Start();
				return;
			}
			//处理新链接，创建HttpConnection类管理新连接
			new_con->Start();
			//继续监听
			self->Start();
		}
		catch (const std::exception& ec) {
			std::cout << "exception is " << ec.what() << std::endl;
			self->Start();
			}
		});
}
