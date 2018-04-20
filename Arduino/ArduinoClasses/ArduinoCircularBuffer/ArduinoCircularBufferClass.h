

template <class T>
class ArduinoCircularBuffer {
public:
    explicit ArduinoCircularBuffer(size_t size) :
    buf_(new T(size)),
    size_(size)
    {
        //empty constructor
    }
    
    ~ArduinoCircularBuffer()
    {
        delete[] buf_;
    }
    
    
    void put(T item)
    {
        //std::lock_guard<std::mutex> lock(mutex_);
        
        buf_[head_] = item;
        head_ = (head_ + 1) % size_;
        
        if(head_ == tail_)
        {
            tail_ = (tail_ + 1) % size_;
        }
    }
    
    T get(void)
    {
        //std::lock_guard<std::mutex> lock(mutex_);
        
        if(empty())
        {
            return T();
        }
        
        //Read data and advance the tail (we now have a free space)
        auto val = buf_[tail_];
        tail_ = (tail_ + 1) % size_;
        
        return val;
    }
    
    void reset(void)
    {
        //std::lock_guard<std::mutex> lock(mutex_);
        head_ = tail_;
    }
    
    bool empty(void) const
    {
        //if head and tail are equal, we are empty
        return head_ == tail_;
    }
    
    bool full(void) const
    {
        //If tail is ahead the head by 1, we are full
        return ((head_ + 1) % size_) == tail_;
    }
    
    size_t size(void) const
    {
        return size_ - 1;
    }
    
    T* getBuffer (void) {
        return buf_;
    }
    
private:
    //std::mutex mutex_;
    T* buf_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_;
    
};



