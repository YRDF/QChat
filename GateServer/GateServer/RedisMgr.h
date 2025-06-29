#pragma once
#include"const.h"
#include "RedisConPool.h"
#include"ConfigMgr.h"

class RedisMgr:public Singleton<RedisMgr>
{
	friend class Singleton<RedisMgr>;
public:
	~RedisMgr();
    //��ȡkey��Ӧ��value
    bool Get(const std::string& key, std::string& value);
    //����key��value
    bool Set(const std::string& key, const std::string& value);
    //˫�˶��У����������Ҳ����ɾ��
    bool LPush(const std::string& key, const std::string& value);
    bool LPop(const std::string& key, std::string& value);
    bool RPush(const std::string& key, const std::string& value);
    bool RPop(const std::string& key, std::string& value);
    //һ��key�п��ܴ洢�ܶ����key��HSet���ö���key��value�����ؿ��Դ������������
    bool HSet(const std::string& key, const std::string& hkey, const std::string& value);
    bool HSet(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    //hget��ȡ����key��value
    std::string HGet(const std::string& key, const std::string& hkey);
    //ɾ��ָ����key
    bool Del(const std::string& key);
    //�жϼ�ֵ�Ƿ����
    bool ExistsKey(const std::string& key);
    //�ر�
    void Close();
private:
	RedisMgr();
    /*redis����������
    redisContext* _connect;
    redisReply* _reply;*/

    //ʹ�����ӳ�
    std::unique_ptr<RedisConPool> _con_pool;
};

