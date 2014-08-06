#ifndef P3DMAP_H
#define P3DMAP_H

#define USE_STD_MAP 0

#include <cstring>

template<typename K>
struct P3dHash
{
    size_t operator() (const K& k) const {return k.hash();}
};

template<>
struct P3dHash<uint32_t>
{
    size_t operator() (const uint32_t& k) const {return k;}
};

template<>
struct P3dHash<const char*>
{
    size_t operator() (const char* const& k) const
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

template<typename K>
class P3dComperator
{
public:
    bool operator() (const K& k1, const K& k2) const {return k1 == k2;}
};

template<>
class P3dComperator<const char*>
{
public:
    bool operator() (const char* const& k1, const char* const& k2) const
    {
        return strcmp(k1, k2) == 0;
    }
};

#if USE_STD_MAP

#include <unordered_map>
template<typename K, typename T>
class P3dMap: public std::unordered_map<K, T, P3dHash<K>, P3dComperator<K> >
{
public:
    P3dMap() : std::unordered_map<K, T, P3dHash<K>, P3dComperator<K> >() {}
    explicit P3dMap(size_t bucketCount) : std::unordered_map<K, T, P3dHash<K>, P3dComperator<K> >(bucketCount) {}
    void insert(const K& key, const T& val)
    {
        std::unordered_map<K, T, P3dHash<K>, P3dComperator<K> >::insert(std::pair<K, T>(key, val));
    }

    // not implemented
    void dumpBucketLoad() {}
};

#else // USE_STD_MAP

#include "P3dVector.h"
#include "PlatformAdapter.h"

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

template<class K, class T>
inline P3dPair<K,T>
P3dMakePair(K first, T second)
{
    return P3dPair<K,T>(first, second);
}

//! \brief Replacement for std::unordered_map which makes code size too big (emscripten)
//! Very limited compared to std::unordered_map
template<typename K, typename T>
class P3dMap
{
private:
    P3dLogger logger{"core.P3dMap", P3dLogger::LOG_VERBOSE};

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
            itm = insertPair(value_type(key, T()));
        }
        return itm->second;
    }

    size_t size() { return m_size; }

    void insert(const K& key, const T& val)
    {
        insertPair(value_type(key, val));
    }

    void insert(const value_type& newItem)
    {
        insertPair(newItem);
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
        logger.verbose("Bucket sizes min/max/avg: %d/%d/%d", minSize, maxSize, avgSize);
    }

private:
    // disable copy ctor
    P3dMap(const P3dMap&) {;}
    // disable assignment
    P3dMap& operator=(const P3dMap&) {;}

    typedef P3dVector<value_type> Bucket;

    value_type* find(const K& key)
    {
        size_t hash = m_Hash(key);
        Bucket& buck = m_buckets[hash % m_bucketCount];
        return findInBucket(buck, key);
    }

    value_type* insertPair(const value_type& pair)
    {
        size_t hash = m_Hash(pair.first);
        Bucket& buck = m_buckets[hash % m_bucketCount];
        value_type* itm = findInBucket(buck, pair.first);
        if(itm)
        {
            itm->second = pair.second;
        }
        else
        {
            buck.push_back(value_type(pair.first, pair.second));
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
            if(m_Comperator(itm.first, key)) return &itm;
        }
        return 0;
    }

    P3dHash<K> m_Hash;
    P3dComperator<K> m_Comperator;
    Bucket* m_buckets;
    size_t m_size;
    size_t m_bucketCount;

};

#endif // USE_STD_MAP

#endif // P3DMAP_H
