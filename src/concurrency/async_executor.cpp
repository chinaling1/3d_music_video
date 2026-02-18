#include "async_executor.h"

namespace v3d {
namespace concurrency {

AsyncExecutor& AsyncExecutor::getInstance() {
    static AsyncExecutor instance;
    return instance;
}

AsyncExecutor::AsyncExecutor()
    : threadPool_(nullptr) {
}

AsyncExecutor::~AsyncExecutor() {
}

void AsyncExecutor::setThreadPool(ThreadPool* pool) {
    threadPool_ = pool;
}

ThreadPool* AsyncExecutor::getThreadPool() {
    return threadPool_;
}

}
}