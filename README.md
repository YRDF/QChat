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
