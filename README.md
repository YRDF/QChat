#  仿微信聊天项目客户端服务端整合
总结开发中遇到的难点重点
## 1：使用单例模板类：
网络请求类要做成一个单例类，这样方便在任何需要发送http，tcp请求的时候调用。
#### 在网络请求中采用单例类主要有以下几方面原因：

**1. 资源共享与高效利用**  
单例类确保网络请求相关的组件（如连接池、缓存等）在整个应用中只有一份实例，所有网络请求都可以共享这些资源。例如，连接池的创建和维护成本较高，通过单例模式可以避免频繁创建和销毁连接池，提高系统的性能和资源利用率。

**2. 统一配置与管理**  
单例类可以集中管理网络请求的配置信息，如超时时间、请求头信息、代理设置等。当需要修改这些配置时，只需在单例类中进行修改，所有使用该单例的网络请求都会自动应用新的，配置便于维护和管理。

**3. 方便全局访问**  
单例类提供了一个全局唯一的访问点，使得各个模块可以方便地使用网络请求功能。无需在每个需要网络请求的地方都创建和配置请求对象，只需通过单例类就可以发起请求，简化了代码的编写和调用。

**4. 保持会话一致性**  
在某些场景下，需要保持网络请求的会话信息，如 cookies、认证信息等。单例类可以确保这些会话信息在多个请求之间共享和传递，保证会话的一致性，方便实现如用户登录状态保持等功能。

**5. 线程安全与控制**  
单例类可以在实现时加入线程安全机制，确保在多线程环境下对网络请求资源的正确访问和操作。通过合理的线程安全控制，可以避免并发访问时出现的资源竞争、数据不一致等问题，保证网络请求的稳定性和可靠性。
## 2：http管理类继承单例模板类：
#### 把自己作为模板参数T，使用奇异递归模板(CRTP)
在实现HttpMgr类时，要使用模板的**GetInstance**，就可以继承单例模板。
#### 继承enable_shared_from_this<>，这也是一种奇异递归模板
如果单例类被删除，但是网络还在请求，就会出现错误。引入**enable_shared_from_this<>** 可以使用`auto self = shared_from_this();`获取自己的智能指针，构造伪闭包并增加智能指针引用计数。这样即使单例类被删除，还在调用的网络请求也可以继续使用。
## 3：http管理类发送Post请求流程：
`QNetworkAccessManager _manager`创建`http`请求。把要传入的数据转为`QByteArray`类型。Qt中使用`QNetworkRequest request(url);`绑定url。`QNetworkReply * reply = _manager.post(request, data);`发送post请求并获取回应。完成时，`QNetworkReply`会发送`finished`信号，根据信号传递的内容处理服务端发送过来的数据。各个类通过信号槽互相传递信息不谈。根据发送的请求类型`ReqId`和请求模式`Modules`判断获取是哪种请求。把所有请求id和id对应的函数进行绑定，存储在Map中。回调函数调用Map，根据id不同回调不同函数，完成不同操作。  
#### 存储容器定义内容如下：  
```C++
QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
```
#### 向容器中放入注册消息处理如下
```C++
    //注册获取验证码的回调函数ID_GET_VARIFY_CODE
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE,[this](const QJsonObject& jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误!"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已经发送到邮箱!"),true);
        qDebug()<<"email is: "<<email;
    });
```
## 4：使用beast实现Http服务器
创建CServer类，监听端口，创建acceptor接收所有新来到的连接Socket。Http底层也是Tcp实现的，只是服务器给客户端回包之后，不再监听客户端的事件，自动断开。  
服务器监听，一定需要一个**上下文**（会在底层轮询，监听）。  
CServer类监听连接，连接到来交给HpptConnection类进行处理。CServer通过智能指针对HpptConnection进行管理，由于socket不允许复制，所以使用`std::move`进行处理。
```C++
	//处理新链接，创建HpptConnection类管理新连接
	std::make_shared<HpptConnection>(std::move(self->_socket))->Start();
```
在HpptConnection类的Start函数对连接数据进行读取，使用boost::asio的`http::async_read`函数进行异步读取。读取完之后，HandleRequest对读取的数据进行处理，使用单例类`LogicSystem`处理各种http请求（http有各种请求：get，post等）。  
和之前HttpMgr发送post请求一样。我们也在LogicSystem类中使用map存储回调函数处理post和get请求，注册回调函数到map中。**当HpptConnection类中请求处理http的get请求时就调用`HandleGet`函数，单例LogicSystem类就在get的map中寻找匹配参数的回调函数进行回调：`_get_handlers[path](con);`**。当然，这些操作的前提是要在LogicSystem单例类中把对应的url和函数在map中进行注册：
```C++
	RegGet("/get_test", [](std::shared_ptr<HpptConnection> connection) {
		beast::ostream(connection->_response.body()) << "receive get_test req";
		});
```
### 5:实现http的get和post请求时注意事项  
如上文所示，get和post请求都要先注册到map中，使用时才能回调。函数
```C++
auto body_str = boost::beast::buffers_to_string(connection->_request.body().data());  
```
可以将 HTTP 请求的 body 数据从缓冲区转换为字符串，随后对其进行JSON解析等操作。
### 6.config.ini配置文件
config.ini 文件是一种常见的配置文件，通常用于存储软件、应用程序、脚本或系统的设置和参数。它的核心目的是将程序的配置信息与程序代码本身分离开来。  
**它的主要作用和好处包括**  
**1.分离代码与配置：**  
- 将需要经常调整的设置（如数据库连接信息、文件路径、API密钥、功能开关、日志级别、超时时间等）从程序代码中提取出来。  
- 开发者或用户修改程序行为时，只需编辑这个文本文件，无需重新编译或修改源代码。

**2.提高灵活性和可维护性：**  
- 更改配置变得非常容易和快速。
- 同一份代码可以轻松地在不同环境（开发、测试、生产）运行，只需加载不同的 config.ini 文件（或文件中的不同部分）。
- 系统管理员或最终用户（如果允许）可以方便地根据需求调整设置，而不需要懂编程。

**3.提高安全性（一定程度）：**
- 可以将敏感信息（如密码、API密钥）从代码中移除，存储在配置文件中（注意：配置文件本身也需要保护，防止未授权访问！）。
- 代码库（如Git）中可以不包含包含敏感信息的实际 config.ini，而是提供一个模板（如 config.ini.example），实际配置在部署时单独处理。  
**4.可读性强：** 
.ini 格式是一种简单、直观的纯文本格式，易于人类阅读和理解（相比XML或JSON有时更简洁）。  
#### .ini 文件格式的特点（通常） 
- **节（Sections）**：使用方括号 [] 将配置项分组。例如 [Database], [Logging], [API]。  
- **键值对（Key-Value Pairs）**：每个配置项由 键 = 值 的形式定义。例如 host = localhost, port = 5432, debug = True。  
- **注释（Comments）**：使用分号 ; 或井号 # 开头的行表示注释，用于解释说明，程序会忽略这些行。  
- **类型**：值通常是字符串，但程序读取后会根据上下文解析成需要的类型（整数、布尔值、浮点数等）。  
### 一个典型的 config.ini 示例  
```ini
; 数据库连接配置
[Database]
host = localhost
port = 3306
username = db_user
password = secure_password_123 ; 注意：真实密码应妥善保管！
database_name = my_app_db

; 应用程序设置
[AppSettings]
debug_mode = False
log_level = INFO ; 可以是 DEBUG, INFO, WARNING, ERROR
max_connections = 100
data_dir = /var/data/my_app

; 外部API配置
[ExternalAPI]
api_url = https://api.example.com/v1
api_key = your_api_key_here
timeout_seconds = 30
```  
### 7.GateServer和VerifyServer之间通过GRPC通信
**gRPC 是一个让不同进程（通常是不同服务器或客户端与服务器）像调用本地函数一样调用远程函数的框架，而 .proto 文件定义了这些“函数”的签名（方法名、参数、返回值）以及参数/返回值的数据结构。**  
**.proto 文件的作用:**
- 定义数据结构 (message): 描述你要传输的数据的字段（名称、类型、顺序）。  
- 定义服务接口 (service): 描述你的 gRPC 服务提供哪些方法（函数），以及这些方法的输入 (request) 和输出 (response) 消息类型。  
- 作为“合同” (Contract): 这是客户端和服务端共享的、约定好的接口规范，是代码生成的基础。   

**gRPC和传统CS架构相同，但是其可以跨语言。完美解决了服务端和客户端使用不同编程语言进行开发和交互的难题，成为构建分布式系统（尤其是微服务）中服务间通信的优选方案。**  
比如这个gRPC的proto文件：  
```.proto
syntax = "proto3";

package message;

service VarifyService {
  rpc GetVarifyCode (GetVarifyReq) returns (GetVarifyRsp) {}
}

message GetVarifyReq {
  string email = 1;
}

message GetVarifyRsp {
  int32 error = 1;
  string email = 2;
  string code = 3;
}
```
- 服务端将提供名为 VarifyService 的服务  
- 该服务包含一个 RPC 方法 GetVarifyCode  
- 客户端调用时需要传递 GetVarifyReq 参数  
- 服务端处理完成后返回 GetVarifyRsp 结果  
**表示要进行的服务是：客户端调用GetVarifyReq传送参数email给服务器，服务器收到参数后调用GetVarifyCode函数。返回GetVarifyRsp参数给客户端。**  

生成message.grpc.pb.h和message.grpc.pb.cc文件(**gRPC通信的接口**):
`G:\cppsoft\grpc\visualpro\third_party\protobuf\Debug\protoc.exe  -I="." --grpc_out="." --plugin=protoc-gen-grpc="G:\cppsoft\grpc\visualpro\Debug\grpc_cpp_plugin.exe" "message.proto"`  
生成message.pb.h和message.pb.cc文件(**接口所用的参数**):
`G:\cppsoft\grpc\visualpro\third_party\protobuf\Debug\protoc.exe --cpp_out=. "message.proto"`  
#### 创建一个gRPC通信的单例类VerifyGrpcClient进行gRPC通信，传输邮箱，获取VerifyServer上的验证码。在LogicSystem中调用。  
### 8.创建ConfigMgr类管理config.ini配置文件
ConfigMgr做成一个单例类为全局访问使用。把config.ini的所有参数读取到map中，在各个文件中显示。  
### 9.nodejs实现邮箱认证服务

### 10.iocontexpool线程池管理iocontext
让多个iocontext跑在不同的线程中  
**`AsioIOServicePool`存储多个ioc(根据cpu的核心数确定线程池的线程数进而确定ioc的数量)。**  
初始化ioc(上下文)时，必须让其绑定`WorkGuard`，不然其初始化就会被释放。使用智能指针`WorkGuardPtr`来管理`WorkGuard`。线程存储在vector中  
`std::vector<std::thread> _threads`。  
### 初始化连接池：
```C++
AsioIOServicePool::AsioIOServicePool(std::size_t size) :_ioServices(size),
_workGuards(size), _nextIOService(0) {
    for (std::size_t i = 0; i < size; ++i) {
        _workGuards[i] = std::make_unique<WorkGuard>(boost::asio::make_work_guard(_ioServices[i]));
    }

    //遍历多个ioservice，创建多个线程，每个线程内部启动ioservice
    for (std::size_t i = 0; i < _ioServices.size(); ++i) {
        _threads.emplace_back([this, i]() {
            _ioServices[i].run();
            });
    }
}
```  
### GetIOService返回要调用的（跑在不同线程的）ioc:
```C++
boost::asio::io_context& AsioIOServicePool::GetIOService() {
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size()) {
        _nextIOService = 0;
    }
    return service;
}
```  
在这里，主函数有一个ioc，连接池有x个ioc。当连接到来时，主函数ioc先接收到连接，随后在CServer函数中将其转移给连接池的ioc进行通信。这样就会有多个ioc同时管理多个连接。    
### 11.grpc连接池
因为使用了多线程ioc，所以可能会有多个连接同时调用单例类`VarifyService`，进行gRPC操作。所以我们要做一个RPC连接池，对gRPC连接进行保护。  
由于`VarifyGrpcClient`是单例，其构造函数只会调用一次，因此被`VarifyGrpcClient`创建的连接池`RPConPool`也只会被创建一次。  
在连接池中，使用一个queue队列来存储stub：
```C++
std::queue<std::unique_ptr<VarifyService::Stub>> connections_;
```  
每有连接时，就从队列中取出stub进行使用:  
```C++
std::unique_ptr<VarifyService::Stub> getConnection();
```  
使用队列时要加锁。使用完stub要将其返回队列:
```C++
void returnConnection(std::unique_ptr<VarifyService::Stub> context);
```
## 上面代码所实现的功能：
**GateServer收到Client发送的请求后，会调用grpc服务，访问VarifyServer，VarifyServer会随机生成验证码，并且调用邮箱模块发送邮件给指定邮箱。同时把发送的结果给GateServer，GateServer再将消息回传给客户端。**
### 12.redis和redis连接池
为实现验证码有效期，可以用redis管理过期的验证码自动删除，key为邮箱，value为验证码，过期时间为3min。  
因为hredis的原始操作比较麻烦，所以我们封装redis操作类`RedisMgr`。**在创建池的时候，我们可以先写一个单例连接，然后再扩展出连接池。** 所以我们先创建一个单例类`RedisMgr`来对redis进行操作，随后封装`RedisConPool`连接池类。和之前封装grpc操作和RPC连接池一样。  
使用redis连接时，先建立连接，然后进行auth认证，才能进行set，get等操作。  
连接redis需要的参数：
```C++
    //redis连接上下文
    redisContext* _connect;
    //redis的回应
    redisReply* _reply;
```  
连接池也是封装这参数,再队列中进行使用：
```C++
std::queue<redisContext*> connections_;
```