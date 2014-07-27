#ifndef P3DVECTOR_H
#define P3DVECTOR_H

#define USE_STD_VECTOR 0

#if USE_STD_VECTOR

#include <vector>
#define P3dVector std::vector

#else // USE_STD_VECTOR

#include <cstdint>
#include <cstdlib>
#include <cassert>

//! \brief Replacement for std::vector which makes code size too big (emscripten)
//! Very limited compared to std::vector
template<typename T>
class P3dVector
{
public:
    P3dVector()
    {
        m_size = 0;
        m_data = 0;
        m_capacity = 0;
    }

    explicit P3dVector(size_t capacity)
    {
        m_size = 0;
        m_data = 0;
        reserve(capacity);
    }

    ~P3dVector()
    {
        free(m_data);
    }

    size_t size() const { return m_size; }
    const T* data() { return m_data; }

    void reserve(size_t capacity)
    {
        if(capacity > m_capacity)
        {
            m_data = static_cast<T*>(realloc(m_data, sizeof(T) * capacity));
            m_capacity = capacity;
        }
    }

    void clear()
    {
        free(m_data);
        m_data = 0;
        m_size = 0;
        m_capacity = 0;
    }

    void push_back(const T& val)
    {
        if(m_size >= m_capacity)
        {
            size_t new_size = m_capacity * 5 / 4;
            // always add at least 8
            if(new_size < m_capacity + 8) new_size = m_capacity + 8;
            reserve(new_size);
        }
        m_data[m_size] = val;
        ++m_size;
    }

    T& operator[] (size_t index)
    {
        assert(index < m_size);
        return m_data[index];
    }

    const T& operator[] (size_t index) const
    {
        if(index >= m_size)
        {
            reserve(index + 1);
            m_size = index + 1;
        }
        return m_data[index];
    }

private:
    // disable copy ctor
    P3dVector(const P3dVector&) {;}
    // disable assignment
    P3dVector& operator=(const P3dVector&) {;}

    T* m_data;
    size_t m_size;
    size_t m_capacity;
};

#endif // USE_STD_VECTOR

#endif // P3DVECTOR_H
