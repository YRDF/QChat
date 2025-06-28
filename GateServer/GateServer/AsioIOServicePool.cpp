#include "AsioIOServicePool.h"
#include <iostream>

AsioIOServicePool::AsioIOServicePool(std::size_t size) :_ioServices(size),
_workGuards(size), _nextIOService(0) {
    for (std::size_t i = 0; i < size; ++i) {
        _workGuards[i] = std::make_unique<WorkGuard>(boost::asio::make_work_guard(_ioServices[i]));
    }

    //�������ioservice����������̣߳�ÿ���߳��ڲ�����ioservice
    for (std::size_t i = 0; i < _ioServices.size(); ++i) {
        _threads.emplace_back([this, i]() {
            _ioServices[i].run();
            });
    }
}

AsioIOServicePool::~AsioIOServicePool() {
    Stop();
    std::cout << "AsioIOServicePool destruct" << std::endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService() {
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size()) {
        _nextIOService = 0;
    }
    return service;
}

void AsioIOServicePool::Stop() {
    // ��ȡ������ work_guard���� io_context ���Զ��˳� run()
    for (auto& guard : _workGuards) {
        guard->reset(); // ��д�����ͷ� keep-alive
    }
    // ȷ�� io_context �Լ�Ҳ stop ������ֹ��������������
    for (auto& io : _ioServices) {
        io.stop();
    }
    for (auto& t : _threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}