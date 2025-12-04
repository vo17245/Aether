#pragma once
#include <utility> // for std::swap
namespace Aether
{


template <typename T>
class LocalRef{
private:
    T* ptr;              // 被管理的对象
    int* ref_count;      // 引用计数

    void Release() {
        if (ref_count) {
            --(*ref_count);
            if (*ref_count == 0) {
                delete ptr;
                delete ref_count;
            }
        }
    }

public:
    // 构造函数
    explicit LocalRef(T* p = nullptr)
        : ptr(p), ref_count(p ? new int(1) : nullptr) {}

    // 拷贝构造
    LocalRef(const LocalRef& other)
        : ptr(other.ptr), ref_count(other.ref_count) {
        if (ref_count) {
            ++(*ref_count);
        }
    }

    // 移动构造
    LocalRef(LocalRef&& other) noexcept
        : ptr(other.ptr), ref_count(other.ref_count) {
        other.ptr = nullptr;
        other.ref_count = nullptr;
    }

    // 拷贝赋值
    LocalRef& operator=(const LocalRef& other) {
        if (this != &other) {
            Release(); // 释放当前持有的资源
            ptr = other.ptr;
            ref_count = other.ref_count;
            if (ref_count) {
                ++(*ref_count);
            }
        }
        return *this;
    }

    // 移动赋值
    LocalRef& operator=(LocalRef&& other) noexcept {
        if (this != &other) {
            Release(); // 释放当前持有的资源
            ptr = other.ptr;
            ref_count = other.ref_count;
            other.ptr = nullptr;
            other.ref_count = nullptr;
        }
        return *this;
    }

    // 解引用操作符
    T& operator*() const {
        return *ptr;
    }

    // 成员访问操作符
    T* operator->() const {
        return ptr;
    }

    // 获取原始指针
    T* Get() const {
        return ptr;
    }

    // 检查是否为空
    bool IsNull() const {
        return ptr == nullptr;
    }

    // 获取引用计数
    int UseCount() const {
        return ref_count ? *ref_count : 0;
    }

    // 析构函数
    ~LocalRef() {
        Release();
    }
};

template<typename T,typename... Ts>
LocalRef<T> CreateLocalRef(Ts&&... args){
    return LocalRef<T>(new T(std::forward<Ts>(args)...));
}



}