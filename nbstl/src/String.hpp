#ifndef NBSTL_SRC_STRING_HPP
#define NBSTL_SRC_STRING_HPP

#include <NbCore.hpp>
#include <cstring>
#include <iostream>
#include <utility>

#include "Utils.hpp"

template<typename T>
class BasicString
{
private:
    //for stl compatability
    using iterator       = T*;
    using const_iterator = const T*;

    iterator begin();
    iterator end();

    const_iterator cbegin();
    const_iterator cend();

    static constexpr size_t ADDICTIONAL_CAPACITY_AFTER_FIRST_HEAP_USAGE = 16;
    static constexpr size_t ADDICTIONAL_CAPACITY_WITH_NULL_TERMINATOR =  ADDICTIONAL_CAPACITY_AFTER_FIRST_HEAP_USAGE / sizeof(T);

    union StringData
    {
        struct HeapData
        {
            size_t flag : 1;
            size_t size : sizeof(size_t) * 8 - 1;
            size_t capacity;
            T* buffer;
        } heap;

        struct StackData
        {
            uint8_t flag : 1;
            uint8_t size : 7;
            T buffer[sizeof(HeapData) / sizeof(T) - 1];
        } stack;
    };

    StringData stringData;
    static constexpr size_t SSO_CAPACITY = sizeof(StringData::stack.buffer) / sizeof(T);

    bool isHeap() const noexcept { return stringData.heap.flag != 0; }
    bool isSSO() const noexcept { return stringData.stack.flag == 0; }

    void setHeapUsage() noexcept { stringData.heap.flag = 1; }
    void setSSOUsage() noexcept { stringData.stack.flag = 0; }

    void clearSSO() noexcept
    {
        stringData.stack.buffer[0] = T(0);
        stringData.stack.size = 0;
        setSSOUsage();
    }

    void clearHeap() noexcept
    {
        if (stringData.heap.buffer)
        {
            delete[] stringData.heap.buffer;
        }
        clearSSO();
    }

    void setSSO(const T* str, size_t len) noexcept
    {
        std::memcpy(stringData.stack.buffer, str, len * sizeof(T));
        stringData.stack.buffer[len] = T(0);
        stringData.stack.size = static_cast<uint8_t>(len);
        setSSOUsage();
    }

    void setHeap(const T* str, size_t len) noexcept
    {
        stringData.heap.buffer = new T[len + ADDICTIONAL_CAPACITY_WITH_NULL_TERMINATOR];
        std::memset(stringData.heap.buffer, 0, len + ADDICTIONAL_CAPACITY_WITH_NULL_TERMINATOR);
        std::memcpy(stringData.heap.buffer, str, len * sizeof(T));
        stringData.heap.buffer[len] = T(0);
        stringData.heap.size = len;
        stringData.heap.capacity = len + 1;
        setHeapUsage();
    }

    void allocateHeap() noexcept
    {
        T* tempBuffer = new T[stringData.stack.size + ADDICTIONAL_CAPACITY_AFTER_FIRST_HEAP_USAGE];
        std::memset(tempBuffer, 0, stringData.stack.size + ADDICTIONAL_CAPACITY_AFTER_FIRST_HEAP_USAGE);

        std::memcpy(tempBuffer, stringData.stack.buffer, stringData.stack.size * sizeof(T));

        stringData.heap.buffer = tempBuffer;
        stringData.heap.size = stringData.stack.size;
        stringData.heap.capacity = stringData.stack.size + ADDICTIONAL_CAPACITY_AFTER_FIRST_HEAP_USAGE;
        setHeapUsage();
    }

    void allocate() noexcept
    {
        T* tempBuffer = new T[stringData.heap.capacity * 2];
        std::memset(tempBuffer, 0, stringData.heap.capacity * 2);
        std::memcpy(tempBuffer, stringData.heap.buffer, stringData.heap.size * sizeof(T));

        if(stringData.heap.buffer)
        {
            delete[] stringData.heap.buffer;
        }

        stringData.heap.buffer = tempBuffer;
        stringData.heap.capacity = stringData.heap.capacity * 2;
    }

    T* dataPtr() noexcept {
        return isSSO() ? stringData.stack.buffer : stringData.heap.buffer;
    }

    const T* dataPtr() const noexcept {
        return isSSO() ? stringData.stack.buffer : stringData.heap.buffer;
    }

    size_t dataSize() noexcept {
        return isSSO() ? stringData.stack.size : stringData.heap.size;
    }

    void setSize(size_t newSize) noexcept
    {
        if (isSSO())
        {
            stringData.stack.size = newSize;
        }
        else
        {
            stringData.heap.size = newSize;
        }
    }

    size_t dataCapacity() noexcept {
        return isSSO() ? SSO_CAPACITY : stringData.heap.capacity;
    }

    void ensureCapacity(size_t requiredSize) noexcept
    {
        if (isSSO())
        {
            if (requiredSize < SSO_CAPACITY)
            {
                return;
            }

            allocateHeap();
        }
        else
        {
            if (requiredSize < dataCapacity())
            {
                return;
            }

            allocate();
        }
    }


public:

    BasicString()
    {
        clearSSO();
    }

    BasicString(const T* str)
    {
        size_t len = std::char_traits<T>::length(str);
        if (len <= SSO_CAPACITY)
        {
            setSSO(str, len);
        }
        else
        {
            setHeap(str, len);
        }
    }

    BasicString(const BasicString& other)
    {
        if (other.isSSO())
        {
            setSSO(other.stringData.stack.buffer, other.stringData.stack.size);
        }
        else
        {
            setHeap(other.stringData.heap.buffer, other.stringData.heap.size);
        }
    }

    BasicString(BasicString&& other) noexcept
    {
        if (other.isSSO())
        {
            setSSO(other.stringData.stack.buffer, other.stringData.stack.size);
        }
        else
        {
            stringData.heap = other.stringData.heap;
        }

        other.clearSSO();
    }


    ~BasicString()
    {
        if (isHeap())
        {
            clearHeap();
        }
    }

    BasicString& operator=(const BasicString& other)
    {
        if (this == &other)
            return *this;

        if (isHeap())
            clearHeap();

        if (other.isSSO())
            setSSO(other.stringData.stack.buffer, other.stringData.stack.size);
        else
            setHeap(other.stringData.heap.buffer, other.stringData.heap.size);

        return *this;
    }

    BasicString& operator=(BasicString&& other) noexcept
    {
        if (this == &other)
            return *this;

        if (isHeap())
            clearHeap();

        if (other.isSSO())
            setSSO(other.stringData.stack.buffer, other.stringData.stack.size);
        else
            stringData.heap = other.stringData.heap;

        other.clearSSO();

        return *this;
    }


    const T* c_str() const
    {
        if (empty())
            return "";

        return isSSO() ? stringData.stack.buffer : stringData.heap.buffer;
    }

    size_t size() const { return isSSO() ? stringData.stack.size : stringData.heap.size; }
    size_t capacity() const { return isSSO() ? sizeof(StringData::HeapData) / sizeof(T) - 1 : stringData.heap.capacity; }
    bool empty() const { return size() == 0; }
    void clear() { isSSO() ? clearSSO() : clearHeap(); }

    T& operator[](size_t index)
    {
        return isSSO() ? stringData.stack.buffer[index] : stringData.heap.buffer[index];
    }

    const T& operator[](size_t index) const
    {
        return isSSO() ? stringData.stack.buffer[index] : stringData.heap.buffer[index];
    }

    void pushBack(const T& ch) noexcept
    {
        size_t newSize = size() + 1;
        ensureCapacity(newSize + 1);

        if (isSSO())
        {
            stringData.stack.buffer[stringData.stack.size++] = ch;
            stringData.stack.buffer[stringData.stack.size] = T(0);
        }
        else
        {
            stringData.heap.buffer[stringData.heap.size++] = ch;
            stringData.heap.buffer[stringData.heap.size] = T(0);
        }
    }

    void insert(size_t pos, const T& ch) noexcept
    {
        NB_ASSERT(pos <= size(), "insert() position out of range");

        size_t oldSize = size();
        size_t newSize = oldSize + 1;

        ensureCapacity(newSize + 1);

        T* ptr = dataPtr();

        std::memmove(ptr + pos + 1, ptr + pos, (oldSize - pos) * sizeof(T));
        ptr[pos] = ch;
        setSize(newSize);
        ptr[newSize] = T(0);
    }

    void insert(size_t pos, const T* str) noexcept
    {
        NB_ASSERT(pos <= size(), "insert() position out of range");

        size_t oldSize = size();
        size_t strSize = std::strlen(str);
        size_t newSize = oldSize + strSize;

        ensureCapacity(newSize + 1);

        T* ptr = dataPtr();

        std::memmove(ptr + pos + strSize, ptr + pos, (oldSize - pos) * sizeof(T));

        std::memcpy(ptr + pos, str, strSize * sizeof(T));

        setSize(newSize);
        ptr[newSize] = T(0);
    }


    void insert(size_t pos, const T* str, size_t count) noexcept
    {
        NB_ASSERT(pos <= size(), "insert() position out of range");

        if (count == 0) return;

        size_t oldSize = size();
        size_t newSize = oldSize + count;

        ensureCapacity(newSize + 1);

        T* ptr = dataPtr();

        std::memmove(ptr + pos + count, ptr + pos, (oldSize - pos) * sizeof(T));
        std::memcpy(ptr + pos, str, count * sizeof(T));

        setSize(newSize);
        ptr[newSize] = T(0);
    }

    void insert(size_t pos, const BasicString& other) noexcept
    {
        NB_ASSERT(pos <= size(), "insert() position out of range");

        size_t count = other.size();
        if (count == 0) return;

        size_t oldSize = size();
        size_t newSize = oldSize + count;

        ensureCapacity(newSize + 1);

        T* ptr = dataPtr();

        std::memmove(ptr + pos + count, ptr + pos, (oldSize - pos) * sizeof(T));
        std::memcpy(ptr + pos, other.dataPtr(), count * sizeof(T));

        setSize(newSize);
        ptr[newSize] = T(0);
    }

    void erase(size_t pos) noexcept
    {
        NB_ASSERT(pos < size(), "erase() position out of range");

        T* ptr = dataPtr();
        size_t oldSize = size();

        std::memmove(ptr + pos, ptr + pos + 1, (oldSize - pos - 1) * sizeof(T));

        setSize(oldSize - 1);
        ptr[size()] = T(0); 
    }

    void erase(size_t pos, size_t count) noexcept
    {
        NB_ASSERT(pos <= size(), "erase() position out of range");

        size_t oldSize = size();
        if (pos + count > oldSize)
            count = oldSize - pos;

        T* ptr = dataPtr();

        std::memmove(ptr + pos, ptr + pos + count, (oldSize - pos - count) * sizeof(T));

        setSize(oldSize - count);
        ptr[size()] = T(0); 
    }

    void replace(const BasicString& pattern, const BasicString& replacement) noexcept
    {
        StringPosition pos = bmhFindFast(c_str(), size(), pattern.c_str(), pattern.size());
        if(pos)
        {
            if (pattern.size() == size())
            {
                std::memcpy(dataPtr(), replacement.c_str(), replacement.size() * sizeof(T));
            }
            else
            {
                erase(pos.getPosition(), pattern.size());
                insert(pos.getPosition(), replacement.c_str());
            }

        }
    }

    void reserve(size_t newCapacity) noexcept
    {
        // ���� � SSO � ������ ���� � ������ ����������
        if (isSSO())
        {
            if (newCapacity <= SSO_CAPACITY)
            {
                return;
            }

            // ������� �� SSO � heap
            size_t oldSize = stringData.stack.size;
            T* temp = new T[newCapacity + 1];
            std::memcpy(temp, stringData.stack.buffer, oldSize * sizeof(T));
            temp[oldSize] = T(0);

            stringData.heap.buffer = temp;
            stringData.heap.size = oldSize;
            stringData.heap.capacity = newCapacity + 1;
            setHeapUsage();
            return;
        }

        // ��� � heap
        if (newCapacity + 1 <= stringData.heap.capacity)
        {
            return; // ��� ���������� ������
        }

        T* newBuffer = new T[newCapacity + 1];
        std::memcpy(newBuffer, stringData.heap.buffer, stringData.heap.size * sizeof(T));
        newBuffer[stringData.heap.size] = T(0);

        delete[] stringData.heap.buffer;

        stringData.heap.buffer = newBuffer;
        stringData.heap.capacity = newCapacity + 1;
    }

};

#endif
