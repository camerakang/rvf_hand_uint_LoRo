/// @brief 线程安全的环形队列

#ifndef __RING_QUEUE_HPP__
#define __RING_QUEUE_HPP__

#include <cstdint>

template <typename _DataType>
class RingQueue
{
private:
  _DataType *__buf{nullptr};
  uint32_t __size{2};
  uint32_t __tail{0};
  uint32_t __head{0};

  uint32_t __size_end_pos{1}; // 数据总长度-1，__size-1

  bool __push_lock{false}; // push锁定

  /// @brief 读取当前缓存指针所在位置
  /// @param pos 需要读取的值
  /// @return 在缓存中的位置
  inline const uint32_t __mod(uint32_t pos) const
  {
    return (pos & __size_end_pos);
  }

  inline const uint32_t __head_pos() const
  {
    return __mod(__head);
  }

  inline const uint32_t __tail_pos() const
  {
    return __mod(__tail);
  }

  /// @brief 计算为2的平方数
  /// @param num 需要判读的值
  /// @return 返回是否成功
  inline const int32_t __is_power_of_two(uint32_t num) const
  {
    if (num < 2)
    {
      return 0;
    }
    return (num & (num - 1)) == 0;
  }

  /// @brief 寻找最接近参数的2的平方数
  /// @param num 需要处理的值
  /// @return 2的平方数
  inline uint32_t __roundup_power_of_two(uint32_t num)
  {
    if (num == 0)
    {
      return 2;
    }
    int32_t i = 0;
    for (; num != 0; ++i)
    {
      num >>= 1;
    }
    return 1U << i;
  }

public:
  /// @brief 从队列可用的空间里面申请一块缓存，类型由push操作，此处由系统自行生成
  /// @param repush 是否允许重复push当前被锁定的数据
  /// @return 返回可用的缓存，如果失败返回nullptr
  _DataType *pre_push(bool repush = false)
  {
    if (__push_lock && !repush)
    {
      return nullptr;
    }
    __push_lock = true;
    return &__buf[__tail_pos()];
  }

  /// @brief 从队列可用的空间里面申请一块缓存，并将缓存的内容使用dbuf进行替换
  /// @param dbuf 指定的缓存地址
  /// @param repush 是否允许重复push当前被锁定的数据
  /// @return 返回可用的缓存，如果失败返回nullptr
  _DataType *pre_push(_DataType *dbuf, bool repush = false)
  {
    if (__push_lock && !repush)
    {
      return nullptr;
    }
    __push_lock = true;
    _DataType *result{&__buf[__tail_pos()]};
    if (!__push_lock)
    {
      *result = *dbuf;
    }
    return result;
  }

  /// @brief 确定已经申请的空间可以加载队列
  /// @param confirm 是否确认将数据写入
  void push(bool confirm = true)
  {
    if (!__push_lock)
    {
      return;
    }
    if (confirm)
    {
      ++__tail;
    }
    __push_lock = false;
  }

  /// @brief 判断当前是否正在写入数据
  /// @return true/false 正在写入，空闲状态
  const bool pushing() const
  {
    return __push_lock;
  }

  /// @brief 移除队列头部的元素
  void pop()
  {
    if (!empty())
    {
      ++__head;
    }
  }

  /// @brief 访问队列头部元素的值
  /// @return 返回头数据，空返回nullptr
  _DataType *front()
  {
    if (empty())
    {
      return nullptr;
    }
    return &__buf[__head_pos()];
  }

  /// @brief 判断队列是否为空
  /// @return 返回头数据，空返回nullptr
  _DataType *back()
  {
    if (empty())
    {
      return nullptr;
    }
    return &__buf[__tail_pos()];
  }

  /// @brief 清空数据
  void clear()
  {
    __head = __tail;
  }

  /// @brief 读取当前的缓存区域容量
  /// @return 数据最大长度，如果需要得到相关的字节数量，需要再乘以数据类型对应的字节长度
  const uint32_t capacity() const
  {
    return __size;
  }

  /// @brief 判断缓存是否为空
  /// @return 是否为空
  const bool empty() const
  {
    return __head == __tail;
  }

  /// @brief 判断缓存是否已经满了
  /// @return 是否已满
  const bool full() const
  {
    return __size == (__tail - __head);
  }

  /// @brief 查询当前的数据长度
  /// @return 数据长度
  const uint32_t len() const
  {
    return __tail - __head;
  }

  /// @brief 查询可以使用的缓存长度
  /// @return 可用缓存长度
  const uint32_t remain() const
  {
    return __size - __tail + __head;
  }

  /// @brief 手动分配内存空间
  /// @param capacity 容量
  /// @return 是否分配成功
  uint32_t allocate(const uint32_t capacity)
  {
    __size = capacity;
    if (!__is_power_of_two(__size))
    {
      __size = __roundup_power_of_two(__size);
    }
    __buf = new _DataType[__size];
    __size_end_pos = __size - 1;
    return __size;
  }

  /// @brief 释放缓存
  void delalloc()
  {
    if (__buf)
    {
      delete[] __buf;
    }
  }

  /// @brief 通过[]来访问元素
  ///        注意：此功能不进行越界处理，需要使用者自行判断
  /// @param pos 读取数组位置
  /// @return 返回指定地址的值的引用
  template <typename _IdxType, typename = std::enable_if<
                                   std::is_integral<_IdxType>::value ||
                                   std::is_unsigned<_IdxType>::value>>
  inline _DataType &operator[](const _IdxType pos)
  {
    return __buf[__mod(pos + __head)];
  }

  RingQueue(const RingQueue &) = delete;
  RingQueue &operator=(const RingQueue &) = delete;

  RingQueue(RingQueue &&other) : __buf(other.__buf), __size(other.__size), __tail(other.__tail), __head(other.__head), __size_end_pos(other.__size_end_pos)
  {
  }

  RingQueue &operator=(RingQueue &&other)
  {
    if (__buf)
    {
      delete[] __buf;
    }
    __buf = other.__buf;
    __size = other.__size;
    __tail = other.__tail;
    __head = other.__head;
    __size_end_pos = other.__size_end_pos;
    // 删除原对象的值
    other.__buf = nullptr;
    other.__size = 0;
    other.__tail = 0;
    other.__head = 0;
    other.__size_end_pos = 0;
  }

  RingQueue(const uint32_t capacity)
  {
    allocate(capacity);
  }

  virtual ~RingQueue()
  {
    delalloc();
  }
};

#endif // __RING_QUEUE_HPP__
