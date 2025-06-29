#pragma once
#include"const.h"
#include "RedisConPool.h"
#include"ConfigMgr.h"

class RedisMgr:public Singleton<RedisMgr>
{
	friend class Singleton<RedisMgr>;
public:
	~RedisMgr();
    //获取key对应的value
    bool Get(const std::string& key, std::string& value);
    //设置key和value
    bool Set(const std::string& key, const std::string& value);
    //双端队列，再左侧或者右侧插入删除
    bool LPush(const std::string& key, const std::string& value);
    bool LPop(const std::string& key, std::string& value);
    bool RPush(const std::string& key, const std::string& value);
    bool RPop(const std::string& key, std::string& value);
    //一个key中可能存储很多二级key，HSet设置二级key的value，重载可以处理二进制数据
    bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    //hget获取二级key的value
    std::string HGet(const std::string& key, const std::string& hkey);
    //删除指定的key
    bool Del(const std::string& key);
    //判断键值是否存在
    bool ExistsKey(const std::string& key);
    //关闭
    void Close();
private:
	RedisMgr();
    /*redis连接上下文
    redisContext* _connect;
    redisReply* _reply;*/

    //使用连接池
    std::unique_ptr<RedisConPool> _con_pool;
};

