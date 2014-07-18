#ifndef P3DMAP_H
#define P3DMAP_H

#include "P3dVector.h"
#include "PlatformAdapter.h"

#include <cstring>

template<typename K>
class P3dHasher
{
public:
    size_t static hash(const K& k) {return k.hash();}
};

template<typename K>
class P3dComperator
{
public:
    bool static equals(const K& k1, const K& k2) {return k1 == k2;}
};

template<>
class P3dHasher<uint32_t>
{
public:
    size_t static hash(const uint32_t& k) {return k;}
};

template<>
class P3dHasher<const char*>
{
public:
    size_t static hash(const char* const& k)
    {
        size_t i = 0;
        size_t h1 = k[i++];
        size_t h2 = h1;
        while(h2)
        {
            h2 = k[i++];
            h1 ^= h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2);
        }
        return h1;
    }
};

template<>
class P3dComperator<const char*>
{
public:
    bool static equals(const char* const& k1, const char* const& k2)
    {
        return strcmp(k1, k2) == 0;
    }
};

template<typename K, typename T>
struct P3dPair
{
    P3dPair(const K& k, const T& v)
    {
        first = k;
        second = v;
    }

    K first;
    T second;
};

//! \brief Replacement for std::unordered_map which makes code size too big (emscripten)
//! Very limited compared to std::unordered_map
template<typename K, typename T>
class P3dMap
{
public:
    typedef P3dPair<K, T> value_type;

    class iterator {
    public:
        iterator(P3dMap* _map)
        {
            map = _map;
            bucketIndex = 0;
            itemIndex = -1;
            operator++();
        }

        const K& key()
        {
            return map->m_buckets[bucketIndex][itemIndex].first;
        }

        T& value()
        {
            return map->m_buckets[bucketIndex][itemIndex].second;
        }

        iterator& operator++()
        {
            ++itemIndex;
            while(itemIndex >= map->m_buckets[bucketIndex].size())
            {
                ++bucketIndex;
                itemIndex = 0;
                if(bucketIndex >= map->m_bucketCount) return *this;
            }
            return *this;
        }

//        bool hasNext()
//        {
//            return bucketIndex < map->m_bucketCount;
//        }

        bool operator==(const iterator& other)
        {
            return map == other.map && bucketIndex == other.bucketIndex && itemIndex == other.itemIndex;
        }

        bool operator!=(const iterator& other)
        {
            return !operator==(other);
        }

        value_type operator*()
        {
            return map->m_buckets[bucketIndex][itemIndex];
        }

    private:
        P3dMap* map;
        size_t bucketIndex;
        size_t itemIndex;

        friend class P3dMap;
    };

    P3dMap()
    {
        m_size = 0;
        m_bucketCount = 1024;
        m_buckets = new Bucket[m_bucketCount];
    }

    explicit P3dMap(size_t bucketCount)
    {
        m_size = 0;
        m_bucketCount = bucketCount;
        m_buckets = new Bucket[m_bucketCount];
    }

    ~P3dMap()
    {
        delete [] m_buckets;
    }

    void clear()
    {
        m_size = 0;
        for(size_t i = 0, il = m_bucketCount; i < il; ++i)
        {
            m_buckets[i].clear();
        }
    }

    size_t count(const K& key)
    {
        return find(key) ? 1 : 0;
    }

    T& operator[] (const K& key)
    {
        value_type* itm = find(key);
        if(!itm)
        {
            itm = insertItem(key, T());
        }
        return itm->second;
    }

    size_t size() { return m_size; }

    void insert(const K& key, const T& val)
    {
        insertItem(key, val);
    }

    void insert(const value_type& newItem)
    {
        insertItem(newItem.first, newItem.second);
    }

    iterator begin() { return iterator(this); }
    iterator end() {
        iterator itr(this);
        itr.itemIndex = 0;
        itr.bucketIndex = m_bucketCount;
        return itr;
    }

    // for debugging
    void dumpBucketLoad() {
        size_t maxSize = 0;
        size_t minSize = size();
        size_t totalSize = 0;
        for(size_t i = 0; i < m_bucketCount; ++i)
        {
            totalSize += m_buckets[i].size();
            if(maxSize < m_buckets[i].size()) maxSize = m_buckets[i].size();
            if(minSize > m_buckets[i].size()) minSize = m_buckets[i].size();
        }
        size_t avgSize = 0;
        if(m_bucketCount) avgSize = totalSize / m_bucketCount;
        P3D_LOGD("Bucket sizes min/max/avg: %d/%d/%d", minSize, maxSize, avgSize);
    }

private:
    // disable copy ctor
    P3dMap(const P3dMap&) {;}
    // disable assignment
    P3dMap& operator=(const P3dMap&) {;}

    typedef P3dVector<value_type> Bucket;

    value_type* find(const K& key)
    {
        size_t hash = P3dHasher<K>::hash(key);
        Bucket& buck = m_buckets[hash % m_bucketCount];
        return findInBucket(buck, key);
    }

    value_type* insertItem(const K& key, const T& val)
    {
        size_t hash = P3dHasher<K>::hash(key);
        Bucket& buck = m_buckets[hash % m_bucketCount];
        value_type* itm = findInBucket(buck, key);
        if(itm)
        {
            itm->second = val;
        }
        else
        {
            buck.push_back(value_type(key, val));
            itm = &buck[buck.size() - 1];
            ++m_size;
        }
        return itm;
    }

    value_type* findInBucket(Bucket& buck, const K& key)
    {
        for(size_t i = 0, il = buck.size(); i < il; ++i)
        {
            value_type& itm = buck[i];
            if(P3dComperator<K>::equals(itm.first, key)) return &itm;
        }
        return 0;
    }

    Bucket* m_buckets;
    size_t m_size;
    size_t m_bucketCount;

};

template<class K, class T>
inline P3dPair<K,T>
P3dMakePair(K first, T second)
{
    return P3dPair<K,T>(first, second);
}

#endif // P3DMAP_H
