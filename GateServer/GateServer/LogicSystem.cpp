#include "LogicSystem.h"
#include "HttpConnection.h"
#include "VarifyGrpcClient.h"
#include "RedisMgr.h"
#include "MysqlMgr.h"

LogicSystem::~LogicSystem()
{
}

bool LogicSystem::HandleGet(std::string path, std::shared_ptr<HttpConnection> con)
{
	if (_get_handlers.find(path) == _get_handlers.end()) {
		return false;
	}
	int i = 0;
	for (auto& elem : con->_get_params) {
		i++;
		beast::ostream(con->_response.body()) << "param" << i << " key is " << elem.first;
		beast::ostream(con->_response.body()) << ", " << " value is " << elem.second << std::endl;
	}

	_get_handlers[path](con);
	return true;
	
}

void LogicSystem::RegPost(std::string url, HttpHandler handler)
{
	_post_handlers.insert(make_pair(url, handler));
}

bool LogicSystem::HandlePost(std::string path, std::shared_ptr<HttpConnection> con)
{
	if (_post_handlers.find(path) == _post_handlers.end()) {
		return false;
	}

	_post_handlers[path](con);
	return true;
}

void LogicSystem::RegGet(std::string url, HttpHandler handler)
{
	_get_handlers.insert(make_pair(url,handler));
}

LogicSystem::LogicSystem()
{
	RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
		beast::ostream(connection->_response.body()) << "receive get_test req";
		});

	RegPost("/get_varifycode", [](std::shared_ptr<HttpConnection> connection) {
			auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
			std::cout << "receive body is " << body_str << std::endl;
			//回给对方
			connection->_response.set(http::field::content_type, "text/json");
			Json::Value root;
			Json::Reader reader;
			Json::Value src_root;
			bool parse_success = reader.parse(body_str, src_root);
			if (!parse_success) {
				std::cout << "Failed to parse JSON data!" << std::endl;
				root["error"] = ErrorCodes::Error_Json;
				std::string jsonstr = root.toStyledString();
				beast::ostream(connection->_response.body()) << jsonstr;
				return true;
			}

			auto email = src_root["email"].asString();
			//使用grpc传送邮箱到verifyservice，获取邮箱验证码
			GetVarifyRsp rsp =  VarifyGrpcClient::GetInstance()->GetVarifyCode(email);

			std::cout << "email is " << email << std::endl;
			root["error"] = rsp.error();
			root["email"] = src_root["email"];
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		});

	//客户端注册
	RegPost("/user_register", [](std::shared_ptr<HttpConnection> connection) {
		auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());
		std::cout << "receive body is " << body_str << std::endl;
		//设置回包格式和回包内容
		connection->_response.set(http::field::content_type, "text/json");
		Json::Value root;
		Json::Reader reader;
		Json::Value src_root;
		bool parse_success = reader.parse(body_str, src_root);
		if (!parse_success) {
			std::cout << "Failed to parse JSON data!" << std::endl;
			root["error"] = ErrorCodes::Error_Json;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		auto email = src_root["email"].asString();
		auto name = src_root["user"].asString();
		auto pwd = src_root["passwd"].asString();
		auto confirm = src_root["confirm"].asString();

		if (pwd != confirm) {
			std::cout << "password err " << std::endl;
			root["error"] = ErrorCodes::PasswdErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		//先查找redis中email对应的验证码是否合理
		std::string  varify_code;
		bool b_get_varify = RedisMgr::GetInstance()->Get(CODEPREFIX+src_root["email"].asString(), varify_code);
		if (!b_get_varify) {
			std::cout << " get varify code expired" << std::endl;
			root["error"] = ErrorCodes::VarifyExpired;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		if (varify_code != src_root["varifycode"].asString()) {
			std::cout << " varify code error" << std::endl;
			root["error"] = ErrorCodes::VarifyCodeErr;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		//在mysql数据库进行注册RegUser，判断用户是否已经在数据库存在
		int uid = MysqlMgr::GetInstance()->RegUser(name, email, pwd);
		if (uid == 0 || uid == -1) {
			std::cout << " user or email exist" << std::endl;
			root["error"] = ErrorCodes::UserExist;
			std::string jsonstr = root.toStyledString();
			beast::ostream(connection->_response.body()) << jsonstr;
			return true;
		}

		//返还给客户端的数据
		root["error"] = 0;
		root["email"] = email;
		root["uid"] = uid;
		std::cout << "uid is " << uid << std::endl;
		root["user"] = name;
		root["passwd"] = pwd;
		root["confirm"] = confirm;
		root["varifycode"] = src_root["varifycode"].asString();
		std::string jsonstr = root.toStyledString();
		beast::ostream(connection->_response.body()) << jsonstr;
		return true;
		});
}
