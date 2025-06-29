#include "RedisConPool.h"

RedisConPool::RedisConPool(size_t poolSize, const char* host, int port, const char* pwd):
		poolSize_(poolSize), host_(host), port_(port), b_stop_(false){
    //��ʼ������
    for (size_t i = 0; i < poolSize_; ++i) {
        auto* context = redisConnect(host, port);
        if (context == nullptr || context->err != 0) {
            if (context != nullptr) {
                redisFree(context);
            }
            continue;
        }

        auto reply = (redisReply*)redisCommand(context, "AUTH %s", pwd);
        if (reply->type == REDIS_REPLY_ERROR) {
            std::cout << "��֤ʧ��" << std::endl;
            //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
            freeReplyObject(reply);
            redisFree(context);
            continue;
        }

        //ִ�гɹ� �ͷ�redisCommandִ�к󷵻ص�redisReply��ռ�õ��ڴ�
        freeReplyObject(reply);
        std::cout << "��֤�ɹ�" << std::endl;
        connections_.push(context);
    }
}

RedisConPool::~RedisConPool()
{
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connections_.empty()) {
        //�ƺ���ҪredisFree(context);
        auto* context = connections_.front();
        redisFree(context);
        connections_.pop();
    }
}

redisContext* RedisConPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] {
        if (b_stop_) {
            return true;
        }
        //�����false�̻߳����
        return !connections_.empty();
        });
    //���ֹͣ��ֱ�ӷ��ؿ�ָ��
    if (b_stop_) {
        return  nullptr;
    }
    auto* context = connections_.front();
    connections_.pop();
    return context;
}

void RedisConPool::returnConnection(redisContext* context)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (b_stop_) {
        return;
    }
    connections_.push(context);
    cond_.notify_one();
}

void RedisConPool::Close()
{
    b_stop_ = true;
    cond_.notify_all();
}
