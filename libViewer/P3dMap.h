#ifndef P3DMAP_H
#define P3DMAP_H

#include "P3dVector.h"

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
        size_t hash = key.hash();
        Bucket& buck = m_buckets[hash % m_bucketCount];
        return findInBucket(buck, key);
    }

    Item* insertItem(const K& key, const T& val)
    {
        size_t hash = key.hash();
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
            if(itm.key == key) return &itm;
        }
        return 0;
    }

    Bucket* m_buckets;
    size_t m_size;
    size_t m_bucketCount;

};


#endif // P3DMAP_H
