#ifdef OPERATING_SYSTEM_APPLE
#include <sys/mman.h>
#include <errno.h>
#else
#include <windows.h>
#endif


// // arena.cpp //////////////////////////////////////////////

// TODO: implement a map with linear probing

// TODO
// #define VIRTUAL_MEMORY_PAGE_SIZE()
// #define VIRTUAL_MEMORY_RESERVE()
// #define VIRTUAL_MEMORY_COMMIT()
// #define VIRTUAL_MEMORY_FREE()

struct Arena {
    char *_reserved_memory;
    u64 _page_size;
    u64 _num_reserved_pages;
    u64 _num_committed_pages;

    char *_malloc_write_head;

    void *malloc(uint);
    void *calloc(uint, uint);
    void free();
};

#define ARENA_MAX_NUM_ARENAS 128
Arena _arena_pool[ARENA_MAX_NUM_ARENAS];

Arena *ARENA_ACQUIRE() {
    Arena *result; {
        Arena *one_past_end_of_arena_pool = _arena_pool + ARENA_MAX_NUM_ARENAS;
        for (
                result = _arena_pool;
                result < one_past_end_of_arena_pool;
                ++result) {
            if (!result->_reserved_memory) break;
        }
        ASSERT(result != one_past_end_of_arena_pool);
    }

    *result = {};
    result->_num_reserved_pages = 1024 * 1024;

    #ifdef OPERATING_SYSTEM_APPLE
    result->_page_size = sysconf(_SC_PAGESIZE);
    #else
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        result->_page_size = si.dwPageSize;
    }
    #endif

    #ifdef OPERATING_SYSTEM_APPLE
    result->_reserved_memory = (char *)
        mmap(
                NULL,
                result->_num_reserved_pages * result->_page_size,
                PROT_NONE,
                MAP_PRIVATE | MAP_ANONYMOUS,
                -1,
                0
            );
    ASSERT(result->_reserved_memory != MAP_FAILED);
    #else
    result->_reserved_memory = (char *)
        VirtualAlloc(
                NULL,
                result->_num_reserved_pages * result->_page_size,
                MEM_RESERVE,
                PAGE_NOACCESS
                );
    ASSERT(result->_reserved_memory);
    #endif
    result->_malloc_write_head = result->_reserved_memory;

    return result;
}

void ARENA_RELEASE(Arena *arena) {
    ASSERT(arena);
    ASSERT(arena->_reserved_memory);
    #ifdef OPERATING_SYSTEM_APPLE
    munmap(
            arena->_reserved_memory,
            arena->_num_reserved_pages * arena->_page_size
          );
    #else
    VirtualFree(
            arena->_reserved_memory,
            arena->_num_reserved_pages *arena->_page_size,
            MEM_RELEASE
            );
    #endif
    *arena = {};
}

// TODO: arena_arena

void *_arena_malloc(Arena *arena, uint size) {
    // NOTE: must mprotect on page boundaries

    char *_one_past_end_of_memory = arena->_reserved_memory + (arena->_num_reserved_pages * arena->_page_size);
    char *_one_past_end_of_committed_memory = arena->_reserved_memory + (arena->_num_committed_pages * arena->_page_size);

    if (arena->_malloc_write_head + size > _one_past_end_of_committed_memory) {
        // [          ][          ][          ][          ][          ][                 
        //    ^        ^                                       ^       ^                 
        //    |        |                                       |       |                 
        //    |        _one_past_end_of_committed_memory       |       :D                
        //    |                                                |                         
        //    _malloc_write_head                               _malloc_write_head + size 

        uint num_pages_to_commit = (((arena->_malloc_write_head + size) - _one_past_end_of_committed_memory) / arena->_page_size + 1);

        uint num_bytes_to_commit = num_pages_to_commit * arena->_page_size;
        #ifdef OPERATING_SYSTEM_APPLE
        int mprotect_result = mprotect(
                _one_past_end_of_committed_memory,
                num_bytes_to_commit,
                PROT_READ | PROT_WRITE
                );
        ASSERT(mprotect_result == 0);
        #else
        LPVOID VirtualAlloc_result = VirtualAlloc(
                _one_past_end_of_committed_memory,
                num_bytes_to_commit,
                MEM_COMMIT,
                PAGE_READWRITE
                );
        ASSERT(VirtualAlloc_result);
        #endif
        arena->_num_committed_pages += num_pages_to_commit;
    }

    void *result = (void *) arena->_malloc_write_head;
    arena->_malloc_write_head += size;
    ASSERT(arena->_malloc_write_head <= _one_past_end_of_memory);
    return result;
}

void *_arena_calloc(Arena *arena, uint count, uint size_per_element) {
    uint size = count * size_per_element;
    void *result = _arena_malloc(arena, size);
    memset(result, 0, size);
    return result;
}



// // containers2.cpp ////////////////////////////////////////


template <typename T> struct ArenaList {
    Arena *arena;

    uint length;
    uint _capacity;
    T *_array;

    T &operator [](uint index) {
        ASSERT(index < length);
        return _array[index];
    }

    void push_back(T);
};

template <typename T> void _list_push_back(ArenaList<T> *list, T element) {
    ASSERT(list->arena);
    if (list->_capacity == 0) {
        ASSERT(!list->_array);
        ASSERT(list->length == 0);
        list->_capacity = 16;
        list->_array = (T *) _arena_malloc(list->arena, list->_capacity * sizeof(T));
    }
    if (list->length == list->_capacity) {
        T *new_array = (T *) _arena_malloc(list->arena, 2 * list->_capacity * sizeof(T));
        memcpy(new_array, list->_array, list->_capacity * sizeof(T));
        list->_array = new_array;
        list->_capacity *= 2;
    }
    list->_array[list->length++] = element;
}


void *Arena::malloc(uint a) { return _arena_malloc(this, a); }
void *Arena::calloc(uint a, uint b) { return _arena_calloc(this, a, b); }
template <typename T> void ArenaList<T>::push_back(T a) { _list_push_back(this, a); }



// TODO: ShortList


// TODO: let me access with the []

uint32_t _paul_hsieh_SuperFastHash(void *_data, size_t size);

template <typename Key, typename Value> struct PairKeyValue {
    bool in_use;
    Key key;
    Value value;
};

template <typename Key, typename Value> struct ArenaMap {
    Arena *arena;

    uint num_entries;
    uint _capacity;
    PairKeyValue<Key, Value> *_array;

    // TODO: how do we make the square brackets work for putting and getting into a map
    // T &operator [](Key key) {
    //     return map_get(key);
    // }
};

template <typename Key, typename Value> void map_reserve_for_expected_num_entries(
        ArenaMap<Key, Value> *map,
        uint expected_num_entries,
        real load_factor = 2.0
        ) {
    ASSERT(map->arena);
    ASSERT(!map->_array); // FORNOW

    map->num_entries = 0;
    // TODO: i think this is supposed to be a prime number
    map->_capacity = (1 + (uint) (load_factor * expected_num_entries));
    map->_array = (PairKeyValue<Key, Value> *) map->arena->calloc(map->_capacity, sizeof(PairKeyValue<Key, Value>));
}

template <typename Key, typename Value> void map_put(ArenaMap<Key, Value> *map, Key key, Value value) {
    ASSERT(map->arena);
    ASSERT(map->_array);
    ASSERT(map->_capacity);

    uint slot_index; {
        bool found = false;
        slot_index = (_paul_hsieh_SuperFastHash(&key, sizeof(Key)) % map->_capacity);
        for_(_, map->_capacity) {
            bool slot_in_use = (map->_array[slot_index].in_use);
            if (!slot_in_use) {
                found = true;
                break;
            }
            slot_index = (slot_index + 1) % map->_capacity;
        }
        ASSERT(found); // ArenaMap (put): Array is completely full.
    }

    ++map->num_entries;

    PairKeyValue<Key, Value> *slot = &map->_array[slot_index];
    slot->in_use = true;
    slot->key = key;
    slot->value = value;
}

template <typename Key, typename Value> Value *_map_get_pointer(ArenaMap<Key, Value> *map, Key key) {
    ASSERT(map->arena);
    ASSERT(map->_array);
    ASSERT(map->_capacity);

    uint slot_index; {
        bool found = false;
        slot_index = (_paul_hsieh_SuperFastHash(&key, sizeof(Key)) % map->_capacity);
        for_(_, map->_capacity) {
            bool slot_in_use = (map->_array[slot_index].in_use);
            bool slot_key_match = (memcmp(&map->_array[slot_index].key, &key, sizeof(Key)) == 0);
            if (slot_in_use && slot_key_match) {
                found = true;
                break;
            }
            slot_index = (slot_index + 1) % map->_capacity;
        }
        ASSERT(found); // ArenaMap (get): Key not found.
    }
    PairKeyValue<Key, Value> *slot = &map->_array[slot_index];
    return &slot->value;
}

template <typename Key, typename Value> Value map_get(ArenaMap<Key, Value> *map, Key key) {
    return *_map_get_pointer(map, key);
}

template <typename Key, typename Value> bool map_contains_key(ArenaMap<Key, Value> *map, Key key) {
    ASSERT(map->arena);
    ASSERT(map->_array);
    ASSERT(map->_capacity);

    uint slot_index = (_paul_hsieh_SuperFastHash(&key, sizeof(Key)) % map->_capacity);
    for_(_, map->_capacity) {
        bool slot_in_use = (map->_array[slot_index].in_use);
        bool slot_key_match = (memcmp(&map->_array[slot_index].key, &key, sizeof(Key)) == 0);
        if (!slot_in_use) return false;
        if (slot_in_use && slot_key_match) return true;
        slot_index = (slot_index + 1) % map->_capacity;
    }
    return false;
}

// // hash.cpp ///////////////////////////////////////////////

// http://www.azillionmonkeys.com/qed/hash.html
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif
#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8) +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif
uint32_t _paul_hsieh_SuperFastHash(void *_data, size_t size) {
    char *data = (char *) _data;

    uint32_t hash = size, tmp;
    int rem;

    if (size <= 0 || data == NULL) return 0;

    rem = size & 3;
    size >>= 2;

    /* Main loop */
    for (;size > 0; size--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}
