#ifndef LIST_H
#define LIST_H

#include "Basic.h"

template<typename T, size_t Capacity>
class List {
public:
    void Push(const T& item) {
        ASSERT(m_size < Capacity);
        m_data[m_size++] = item;
    }

    void QuickRemove(size_t index) {
        ASSERT(index >= 0 && index < m_size);
        m_data[index] = m_data[m_size - 1];
        m_size -= 1;
    }

    size_t Size() const {
        return m_size;
    }

    bool Full() const {
        return m_size == Capacity;
    }

    T& operator[](size_t index) {
        ASSERT(index >= 0 && index < m_size);
        return m_data[index];
    }

    const T& operator[](size_t index) const {
        ASSERT(index >= 0 && index < m_size);
        return m_data[index];
    }
private:
    size_t m_size;
    T m_data[Capacity];
};

#endif
