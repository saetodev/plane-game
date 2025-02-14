#ifndef LIST_H
#define LIST_H

#include "Basic.h"

#include <string.h>

template<typename T, size_t Cap>
class List {
public:
    void Push(const T& item) {
        ASSERT(m_size < Cap);
        m_data[m_size] = item;
        m_size += 1;
    }

    void Remove(size_t index) {
        ASSERT(index >= 0 && index < m_size);

        if (index == m_size - 1) {
            m_size -= 1;
            return;
        }

        void* dest = &m_data[index];
        void* src = &m_data[index + 1];

        memmove(dest, src, sizeof(T) * (m_size - index));

        m_size -= 1;
    }

    void QuickRemove(size_t index) {
        ASSERT(index >= 0 && index < m_size);
        m_data[index] = m_data[m_size - 1];
        m_size -= 1;
    }

    void Clear() {
        m_size = 0;
    }

    size_t Size() const {
        return m_size;
    }

    size_t Capacity() const {
        return Cap;
    }

    bool Full() const {
        return m_size == Cap;
    }

    bool Empty() const {
        return m_size == 0;
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
    size_t m_size      = 0;
    T m_data[Cap] = {};
};

#endif
