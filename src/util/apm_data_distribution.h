#ifndef WEBRTC_RTC_BASE_APM_DATA_DISTRIBUTION_H_
#define WEBRTC_RTC_BASE_APM_DATA_DISTRIBUTION_H_

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <vector>
#include <type_traits>
#include <condition_variable>
#include <chrono>

#include "webrtc/audio/utility/common_tool.h"
#include "webrtc/common_audio/template_util.h"
#include "webrtc/system_wrappers/include/rw_lock_wrapper.h"

namespace webrtc {
//Tests if this type is a built-in hash type
struct IsBuildInTypeHashable__ {
  template<class T, typename = typename std::enable_if<!std::is_class<T>::value>::type, typename = typename std::enable_if<!std::is_enum<T>::value>::type>
    static std::true_type Test(int);
  template<class T, typename = typename std::enable_if<std::is_same<typename std::decay<T>::type, std::string>::value>::type>
    static std::true_type Test(double);
  template<class T>
    static std::false_type Test(...);
};
template<class T>
struct IsBuildInTypeHashable : decltype(IsBuildInTypeHashable__::Test<T>(0)) {};

//Defines a structure that can be tested for <ForHash> member functions, with a return value of IsBuildInTypeHashable__<ex: int, string, float, char ...>
DefineHasMemberFunctionWithReturnTypeHandle(ForHash, ForHash, IsBuildInTypeHashable)


//default
template<bool IsBuildInType, bool ForHash, class T>
struct MyHash {
  size_t operator()(const T& val) const noexcept {
    static_assert(std::is_enum<T>::value, "your type can not hashable ");
    return static_cast<int>(val);
  }
};

//a built-in hash type
template<bool ForHash, class T>
struct MyHash<true, ForHash, T> {
  size_t operator()(const T& val) const noexcept {
    return std::hash<T>()(val);
  }
};

//Contains a <ForHash> member function that returnType is Hashable
template<class T>
struct MyHash<false, true, T> {
  size_t operator()(const T& val) const noexcept {
    auto key = val.ForHash();
    return std::hash<decltype(key)>()(key);
  }
};

namespace DataDistributionType {

enum class SubscribeMode {
  kSyncNotify = 0,
  kAsyncNotify
};

enum class ChannelStatus {
  kFree = 0,
  kBooked,
  kInUsed
};

enum class DataDistributionStatus {
  kNoData = 0,
  kHasData,
  kEnd
};

template<class DataDescription>
struct Frame {
  Frame() = default;
  Frame(size_t size, const DataDescription& oth) : data_size(size), data_format(oth) {}
  size_t data_size = 0;
  DataDescription data_format;
};

template<class DataDescription>
struct SubscribeIndex {
  bool is_valid = false;
  std::function<int(const void*, size_t, DataDescription)> callback = nullptr;
  int channel_index;
  SubscribeMode mode;

  void ReInit(SubscribeMode mode_, std::function<int(const void*, size_t, DataDescription)> callback_, int channel_index_) {
    is_valid = true;
    mode = mode_;
    callback = callback_;
    channel_index = channel_index_;
  }

  void Clear() {
    is_valid = false;
    callback = nullptr;
  }
};

template<class DataDescription>
struct Node {
  ChannelStatus status = ChannelStatus::kFree;
  std::queue<Frame<DataDescription>> q;
  std::unique_ptr<RingBufferWrapper> cache_buffer;
  std::mutex mutex_;
  size_t ring_buffer_size = 0;

  void operator=(Node& oth) {
    this->swap(oth);
  }

  void swap(Node& oth) {
    this->status = oth.status;
    std::swap(this->q, oth.q);
    this->cache_buffer = std::move(oth.cache_buffer);
    this->ring_buffer_size = oth.ring_buffer_size;
  }

  void ReInit(size_t max_cache_size) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (max_cache_size == 0) {
      return;
    }
    ring_buffer_size = max_cache_size;
    status = ChannelStatus::kInUsed;
  }

  void Clear() {
    std::lock_guard<std::mutex> lk(mutex_);
    status = ChannelStatus::kFree;
    std::queue<Frame<DataDescription>> empty;
    std::swap(q, empty);
    if (nullptr != cache_buffer) {
      cache_buffer->Clear();
    }
  }

  bool Empty() {
    return q.empty();
  }

  int WriteData(const void* data, size_t size, const DataDescription& data_format) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (ChannelStatus::kInUsed != status) {
      return 0;
    }
    if (nullptr == cache_buffer  || cache_buffer->BufferCapacity() < ring_buffer_size) {
      cache_buffer.reset(new RingBufferWrapper(ring_buffer_size));
    }
    while (cache_buffer->BufferCapacity() - cache_buffer->BufferCurrentSize() < size && q.size() > 0) {
      Frame<DataDescription>& frame = q.front();
      cache_buffer->BufferSeek(static_cast<int>(frame.data_size));
      q.pop();
    }
    size_t write_size = cache_buffer->BufferWrite(data, size);
    q.push(Frame<DataDescription>(write_size, data_format));
    return static_cast<int>(write_size);
  }

  size_t ReadData(void** data, size_t* max_size, DataDescription* p_data_format) {
    std::lock_guard<std::mutex> lk(mutex_);
    if (ChannelStatus::kInUsed != status || q.empty()) {
      return 0;
    }
    Frame<DataDescription>& frame = q.front();
    if (frame.data_size > *max_size) {
      free(*data);
      *max_size = frame.data_size;
      *data = calloc(*max_size, 1);
    }
    std::swap(frame.data_format, *p_data_format);
    q.pop();
    return cache_buffer->BufferRead(*data, frame.data_size);
  }

};

//When the data type is pure binary, a default type of None is defined
struct NoneClass {};
static NoneClass node_class_for_bin_data_;

} // DataDistributionType


/*
 * @ChannelDescription：A description of a data source can uniquely describe a data channel, and it must be hashed
 * example:
 *
 *  1. Implementation of <ForHash> function, returnType is int
 *   struct ExampleChannel {
 *      int a, b;
 *      bool operator== (const ExampleChannel& oth) {
 *        return a==oth.a && b == oth.b;
 *      }
 *      int ForHash() {
 *        return a * 1111 + b;
 *      }
 *   }
 *  2. Implementation of <ForHash> function, returnType is std::string
 *   struct ExampleChannel {
 *      char* name;
 *      int age;
 *      bool operator==(const ExampleChannel& oth) {
 *        return name == oth.name && age == oth.age;
 *      }
 *      std::string ForHash() {
 *        retrun std::string(name) + char(age - 'a');
 *      }
 *   }
 *@ DataDescription: Data description information for data deserialization
 *@ HashFunction: The hash function used to hash ChannelDescription
 */
template<class ChannelDescription, \
         class DataDescription = DataDistributionType::NoneClass,
         class HashFunction = MyHash<IsBuildInTypeHashable<ChannelDescription>::value, \
                                     HasFunctionForHashResult<ChannelDescription>::value,\
                                     ChannelDescription>>
class DataDistribution final {
  public:
  ~DataDistribution();
  static DataDistribution& GetInstance();

  /*
   *This function allows the user to Registered a channel of his/her own.
   *@param channel_name: The name of the channel, which is a flag used by subscribers to distinguish between channels
   *return value: if < 0: registered faild
   *               else: return channel_id
   * */

  int RegisteredDataChannel(ChannelDescription data_channel, size_t data_cache_max_size = kDefaultCacheSize);

  int RemoveDataChannel(ChannelDescription data_channel);

  /*
   *@param channel: Which channel to put the data into, and each channel is related to the data properties.
   *@param DataFormat: The data forma
   *return value:
   *       if < 0 : faild
   *       else : return the size had been written
   * */
  int PushData(int channel_id, const void* data, size_t size, DataDescription data_format);

  int PushData(const void* data, size_t size, DataDescription data_format, ChannelDescription id);

  /*
   *@param channel: Which channel to get the data, and each channel is related to the data properties.
   *@param mode: This parameter means whether your data will be called back in real time or asynchronously.
   *             We recommend that if you have a time-consuming operation in your callback function, set it to asynchronous callback mode(kAsyncNotify).
   *             If the real-time requirement is very high, set to the synchronous mode(kSyncNotify)
   *@param subscriber: Data received callback function
   *return value: if < 0 means faild
   *              else return a subscribe_id to unsubscribe
   * */
  int Subscribe(ChannelDescription channel_id, DataDistributionType::SubscribeMode mode,
      std::function<int(const void*, size_t, DataDescription)> subscriber);

  int UnSubscribe(int subscribe_id);


  private:
  DataDistribution();
  size_t FindFreeNode();
  size_t FindFreeSubscriber();
  bool ThreadFunction();

  private:
  static DataDistribution instance_;

  const static size_t kDefaultCacheSize = (size_t)(200 * 1024); // 200k

  std::unique_ptr<ThreadHandle> thread_;
  std::unique_ptr<RWLockWrapper> rw_lock_;
  void* tmp_buffer_ = nullptr;
  size_t tmp_buffer_size_ = 0;
  size_t sleep_time_ms_ = 0;
  DataDistributionType::DataDistributionStatus status_ = DataDistributionType::DataDistributionStatus::kNoData;

  std::mutex mutex_;
  std::condition_variable cond_;
  std::unordered_map<ChannelDescription, int> channel_mapping_;
  std::vector<DataDistributionType::Node<DataDescription>> node_v_;
  std::vector<DataDistributionType::SubscribeIndex<DataDescription>> subscriber_v_;
  std::unordered_map<int, std::unordered_set<int>> sync_subscriber_;
  std::unordered_map<int, std::unordered_set<int>> async_subscriber_;
};

template<class ChannelDescription, class DataDescription, class HashFunction>
DataDistribution<ChannelDescription, DataDescription, HashFunction> DataDistribution<ChannelDescription, DataDescription, HashFunction>::instance_;

template<class ChannelDescription, class DataDescription, class HashFunction>
DataDistribution<ChannelDescription, DataDescription, HashFunction>::DataDistribution() : rw_lock_(RWLockWrapper::CreateRWLock()) {}

template<class ChannelDescription, class DataDescription, class HashFunction>
DataDistribution<ChannelDescription, DataDescription, HashFunction>::~DataDistribution() {
  if (nullptr != thread_) {
    {
      std::lock_guard<std::mutex> lk(mutex_);
      status_ = DataDistributionType::DataDistributionStatus::kEnd;
      cond_.notify_all();
    }
    thread_->Stop();
  }
  if (nullptr != tmp_buffer_) {
    free(tmp_buffer_);
  }
}

template<class ChannelDescription, class DataDescription, class HashFunction>
DataDistribution<ChannelDescription, DataDescription, HashFunction>& DataDistribution<ChannelDescription, DataDescription, HashFunction>::GetInstance() {
  return instance_;
}

template<class ChannelDescription, class DataDescription, class HashFunction>
size_t DataDistribution<ChannelDescription, DataDescription, HashFunction>::FindFreeNode() {
  for(size_t i = 0; i < node_v_.size(); ++i) {
     if (DataDistributionType::ChannelStatus::kFree == node_v_[i].status) {
       return i;
     }
  }
  size_t size = node_v_.empty()? 1 : node_v_.size();
  std::vector<DataDistributionType::Node<DataDescription>> v(size << 1);
  for (size_t i = 0; i < node_v_.size(); ++i) {
    v[i].swap(node_v_[i]);
  }
  std::swap(node_v_, v);
  return size;
}

template<class ChannelDescription, class DataDescription, class HashFunction>
size_t DataDistribution<ChannelDescription, DataDescription, HashFunction>::FindFreeSubscriber() {
  for (size_t i = 0; i < subscriber_v_.size(); ++i) {
    if (false == subscriber_v_[i].is_valid) {
      return i;
    }
  }
  subscriber_v_.push_back(DataDistributionType::SubscribeIndex<DataDescription>());
  return subscriber_v_.size() - 1;
}

template<class ChannelDescription, class DataDescription, class HashFunction>
int DataDistribution<ChannelDescription, DataDescription, HashFunction>::RegisteredDataChannel(ChannelDescription data_channel, size_t data_cache_max_size) {
  WriteLockScoped rw_lk(*rw_lock_);
  int index = -1;
  if (channel_mapping_.find(data_channel) != channel_mapping_.end()) {
    index = channel_mapping_[data_channel];
    if (DataDistributionType::ChannelStatus::kInUsed == node_v_[index].status) {
      return -1;
    }
  }
  if (-1 == index) {
    index = static_cast<int>(FindFreeNode());
  }
  node_v_[index].ReInit(data_cache_max_size);
  channel_mapping_[data_channel] = index;
  return index;
}

template<class ChannelDescription, class DataDescription, class HashFunction>
int DataDistribution<ChannelDescription, DataDescription, HashFunction>::RemoveDataChannel(ChannelDescription data_channel) {
  WriteLockScoped rw_lk(*rw_lock_);
  if (channel_mapping_.find(data_channel) == channel_mapping_.end()) {
    return -1;
  }
  int index = channel_mapping_[data_channel];
  if (DataDistributionType::ChannelStatus::kInUsed != node_v_[index].status) {
    return -1;
  }
  channel_mapping_.erase(data_channel);
  node_v_[index].Clear();
  return index;
}

template<class ChannelDescription, class DataDescription, class HashFunction>
int DataDistribution<ChannelDescription, DataDescription, HashFunction>::PushData(const void* data, size_t size, DataDescription data_format, ChannelDescription id) {
  int channel_id = -1;
  {
    ReadLockScoped rw_lk(*rw_lock_);
    if (channel_mapping_.find(id) == channel_mapping_.end()) {
      return -1;
    }
    channel_id = channel_mapping_[id];
  }
  return PushData(channel_id, data, size, data_format);
}

template<class ChannelDescription, class DataDescription, class HashFunction>
int DataDistribution<ChannelDescription, DataDescription, HashFunction>::PushData(int channel_id, const void* data, size_t size, DataDescription data_format) {
  ReadLockScoped rw_lk(*rw_lock_);
  if (nullptr == data || static_cast<size_t>(channel_id) >= node_v_.size() ||
      DataDistributionType::ChannelStatus::kInUsed != node_v_[channel_id].status) {
    return -1;
  }
  int write_size = static_cast<int>(size);
  if (async_subscriber_.find(channel_id) != async_subscriber_.end() &&
      !async_subscriber_[channel_id].empty()) {
    write_size = node_v_[channel_id].WriteData(data, size, data_format);
  }
  if (sync_subscriber_.find(channel_id) != sync_subscriber_.end()) {
    for(int index : sync_subscriber_[channel_id]) {
      if (nullptr != subscriber_v_[index].callback) {
        subscriber_v_[index].callback(data, size, data_format);
      }
    }
  }
  if (DataDistributionType::DataDistributionStatus::kNoData == status_) {
    std::lock_guard<std::mutex> lk(mutex_);
    status_ =  DataDistributionType::DataDistributionStatus::kHasData;
    cond_.notify_all();
  }
  return write_size;
}

template<class ChannelDescription, class DataDescription, class HashFunction>
int DataDistribution<ChannelDescription, DataDescription, HashFunction>::Subscribe(ChannelDescription channel_id, DataDistributionType::SubscribeMode mode,
    std::function<int(const void*, size_t, DataDescription)> subscriber) {
  if (nullptr == subscriber) {
    return -1;
  }
  WriteLockScoped rw_lk(*rw_lock_);
  int channel_index = 0;
  if (channel_mapping_.find(channel_id) == channel_mapping_.end()) {
     channel_index = static_cast<int>(FindFreeNode());
     node_v_[channel_index].status =  DataDistributionType::ChannelStatus::kBooked;
     channel_mapping_[channel_id] = channel_index;
  } else {
    channel_index = channel_mapping_[channel_id];
  }

  int index = static_cast<int>(FindFreeSubscriber());
  subscriber_v_[index].ReInit(mode, subscriber, channel_index);
  if (mode == DataDistributionType::SubscribeMode::kSyncNotify) {
    sync_subscriber_[channel_index].insert(index);
  } else {
    if (nullptr == thread_) {
      tmp_buffer_size_ = 48000 * 1 * 10 * sizeof(int16_t) / 1000; // 48k 1channel 10ms
      tmp_buffer_ = calloc(tmp_buffer_size_, 1);
      thread_ = ThreadHandle::CreateThread([this]() { return this->ThreadFunction(); }, "DataDistributionThread");
      thread_->Start();
    }
    async_subscriber_[channel_index].insert(index);
  }
  return index;
}

template<class ChannelDescription, class DataDescription, class HashFunction>
int DataDistribution<ChannelDescription, DataDescription, HashFunction>::UnSubscribe(int subscribe_id) {
  WriteLockScoped rw_lk(*rw_lock_);
  if (static_cast<size_t>(subscribe_id) >= subscriber_v_.size() || !subscriber_v_[subscribe_id].is_valid) {
    return -1;
  }
  DataDistributionType::SubscribeIndex<DataDescription>& subscriber = subscriber_v_[subscribe_id];
  if (DataDistributionType::SubscribeMode::kSyncNotify == subscriber_v_[subscribe_id].mode) {
    sync_subscriber_[subscriber.channel_index].erase(subscribe_id);
  } else {
    async_subscriber_[subscriber.channel_index].erase(subscribe_id);
  }
  subscriber.Clear();
  return subscribe_id;
}

template<class ChannelDescription, class DataDescription, class HashFunction>
bool DataDistribution<ChannelDescription, DataDescription, HashFunction>::ThreadFunction() {
  bool not_empty = false;
  DataDescription data_format;
  {
    ReadLockScoped rw_lk(*rw_lock_);
    for (auto& ee : async_subscriber_) {
      if (!node_v_[ee.first].Empty() && !async_subscriber_[ee.first].empty()) {
        not_empty = true;
        size_t read_size = node_v_[ee.first].ReadData(&tmp_buffer_, & tmp_buffer_size_, &data_format);
        for (int subscribe_index : ee.second) {
          subscriber_v_[subscribe_index].callback(tmp_buffer_, read_size, data_format);
        }
      }
    }
  }
  if (!not_empty) {
    sleep_time_ms_ += 20;
    std::unique_lock<std::mutex> lk(mutex_);
    if (status_ == DataDistributionType::DataDistributionStatus::kEnd) {
      return false;
    }
    status_ = DataDistributionType::DataDistributionStatus::kNoData;
    cond_.wait_for(lk, std::chrono::milliseconds(sleep_time_ms_));
  } else {
    sleep_time_ms_ = 0;
  }
  return DataDistributionType::DataDistributionStatus::kEnd != status_;
}
}  // namespace rtc

#endif  // WEBRTC_RTC_BASE_APM_DATA_DISTRIBUTION_H_
