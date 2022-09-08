//
// Created by slovak on 8/27/20.
//

#ifndef FISHI_SRC_SYSTEM_TSQUEUE_H_
#define FISHI_SRC_SYSTEM_TSQUEUE_H_

#include <utility>
#include <atomic>
#include <queue>
#include <condition_variable>
#include <optional>
#include <cassert>

template <typename T>
struct TsQueue {
  inline TsQueue(size_t max_size = -1UL) : m_maxsize(max_size), m_end(false) {};

  /* Push T to the queue. Many threads can push at the same time.
   * If the queue is full, calling thread will be suspended until
   * some other thread pop() data. */
  void push(const T&);
  void push(T&&);

  /* Close the queue.
   * Be sure all consumer threads done their writes before call this.
   * Push data to closed queue is forbidden. */
  void close();

  /* Pop and return T from the queue. Many threads can pop at the same time.
   * If the queue is empty, calling thread will be suspended.
   * If the queue is empty and closed, nullopt returned. */
  std::optional<T> pop();
  std::size_t size() const;
  bool empty();
  bool closed() const { return m_end;};
 private:
  std::queue<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable m_cv_empty, m_cv_full;
  const size_t m_maxsize;
  std::atomic<bool> m_end;
};

template<typename T>
void TsQueue<T>::push(T&& t)
{
  std::unique_lock<std::mutex> lck(m_mutex);
  while (m_queue.size() == m_maxsize && !m_end)
    m_cv_full.wait(lck);
  assert(!m_end);
  m_queue.push(std::move(t));
  m_cv_empty.notify_one();
}

template<typename T>
void TsQueue<T>::push(const T& t)
{
  std::unique_lock<std::mutex> lck(m_mutex);
  while (m_queue.size() == m_maxsize && !m_end)
    m_cv_full.wait(lck);
  assert(!m_end);
  m_queue.push(std::move(t));
  m_cv_empty.notify_one();
}

template<typename T>
std::optional<T> TsQueue<T>::pop()
{
  std::unique_lock<std::mutex> lck(m_mutex);
  while (m_queue.empty() && !m_end)
    m_cv_empty.wait(lck);
  if (m_queue.empty()) return {};
  T t = std::move(m_queue.front());
  m_queue.pop();
  m_cv_full.notify_one();
  return std::move(t);
}

template<typename T>
void TsQueue<T>::close()
{
  m_end = true;
  std::lock_guard<std::mutex> lck(m_mutex);
  m_cv_empty.notify_one();
  m_cv_full.notify_one();
}

template<typename T>
std::size_t TsQueue<T>::size() const
{
  return m_queue.size();
}

template<typename T>
bool TsQueue<T>::empty()
{
  return m_queue.empty();
}
#endif //FISHI_SRC_SYSTEM_TSQUEUE_H_
