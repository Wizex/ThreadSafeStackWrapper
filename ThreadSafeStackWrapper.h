#ifndef THREADSAFESTACKWRAPPER_H
#define THREADSAFESTACKWRAPPER_H

#include <stack>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>

template <typename T>
class thread_safe_stack_wrapper{
private:
  std::stack<T> data;
  std::condition_variable cond;
  mutable std::shared_mutex m;

public:
  thread_safe_stack_wrapper() = default;
  ~thread_safe_stack_wrapper() = default;

  void push(T&& element){
    {
      std::lock_guard<std::shared_mutex> l(m);
      data.push(std::forward<T>(element));
    }
    cond.notify_one();
  }

  template <typename... Args>
  void emplace(Args... args){
    {
      std::lock_guard<std::shared_mutex> lock(m);
      data.emplace(std::forward<T>(args)...);
    }
    cond.notify_one();
  }

  bool try_pop(){
    std::lock_guard<std::shared_mutex> lock(m);
    if(data.empty())
      return false;

    data.pop();
    return true;
  }

  void wait_and_pop(){
    std::unique_lock<std::shared_mutex> lock(m);
    cond.wait(m, [this](){
       return !data.empty();
      });
    data.pop();
  }

  bool empty() const{
    std::shared_lock lock(m);
    return data.empty();
  }

  size_t size() const{
    std::shared_lock lock(m);
    return data.size();
  }

  T& top(){
    std::lock_guard<std::shared_mutex> lock(m);
    return data.top();
  }

  void swap(thread_safe_stack_wrapper& object){
    std::lock(m, object.m);
    std::lock_guard<std::shared_mutex> first(m, std::adopt_lock);
    std::lock_guard<std::shared_mutex> second(object.m, std::adopt_lock);
    data.swap(object.data);
  }

};

#endif // THREADSAFESTACKWRAPPER_H
