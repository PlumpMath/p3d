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


//! \brief Replacement for std::unordered_map which makes code size too big (emscripten)
//! Very limited compared to std::unordered_map
template<typename K, typename T>
class P3dMap
{
public:
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
            return map->m_buckets[bucketIndex][itemIndex].key;
        }

        T& value()
        {
            return map->m_buckets[bucketIndex][itemIndex].val;
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

        bool hasNext()
        {
            return bucketIndex < map->m_bucketCount;
        }

    private:
        P3dMap* map;
        size_t bucketIndex;
        size_t itemIndex;
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
        Item* itm = find(key);
        if(!itm)
        {
            itm = insertItem(key, T());
        }
        return itm->val;
    }

    size_t size() { return m_size; }

    void insert(const K& key, const T& val)
    {
        insertItem(key, val);
    }

    iterator begin() { return iterator(this); }

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

    struct Item
    {
        Item(const K& k, const T& v)
        {
            key = k;
            val = v;
        }

        K key;
        T val;
    };

    typedef P3dVector<Item> Bucket;

    Item* find(const K& key)
    {
        size_t hash = P3dHasher<K>::hash(key);
        Bucket& buck = m_buckets[hash % m_bucketCount];
        return findInBucket(buck, key);
    }

    Item* insertItem(const K& key, const T& val)
    {
        size_t hash = P3dHasher<K>::hash(key);
        Bucket& buck = m_buckets[hash % m_bucketCount];
        Item* itm = findInBucket(buck, key);
        if(itm)
        {
            itm->val = val;
        }
        else
        {
            buck.push_back(Item(key, val));
            itm = &buck[buck.size() - 1];
            ++m_size;
        }
        return itm;
    }

    Item* findInBucket(Bucket& buck, const K& key)
    {
        for(size_t i = 0, il = buck.size(); i < il; ++i)
        {
            Item& itm = buck[i];
            if(P3dComperator<K>::equals(itm.key, key)) return &itm;
        }
        return 0;
    }

    Bucket* m_buckets;
    size_t m_size;
    size_t m_bucketCount;

};


#endif // P3DMAP_H
