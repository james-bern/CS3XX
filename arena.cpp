#include <sys/mman.h>
#include <errno.h>


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
};

Arena arena_create() {
    Arena result = {};
    result._page_size = sysconf(_SC_PAGESIZE);
    result._num_reserved_pages = 1024 * 1024;
    result._reserved_memory = (char *)
        mmap(
                NULL,
                result._num_reserved_pages * result._page_size,
                PROT_NONE,
                MAP_PRIVATE | MAP_ANONYMOUS,
                -1,
                0
            );
    ASSERT(result._reserved_memory != MAP_FAILED);
    result._malloc_write_head = result._reserved_memory;

    return result;
}

char *arena_malloc(Arena *arena, uint size) {
    // NOTE: must mprotect on page boundaries

    char *_one_past_end_of_memory = arena->_reserved_memory + (arena->_num_reserved_pages * arena->_page_size);
    char *_one_past_end_of_mprotected_memory = arena->_reserved_memory + (arena->_num_committed_pages * arena->_page_size);

    if (arena->_malloc_write_head + size > _one_past_end_of_mprotected_memory) {
        // [          ][          ][          ][          ][          ][                 
        //    ^        ^                                       ^       ^                 
        //    |        |                                       |       |                 
        //    |        _one_past_end_of_mprotected_memory      |       :D                
        //    |                                                |                         
        //    _malloc_write_head                               _malloc_write_head + size 

        uint num_pages_to_mprotect = (((arena->_malloc_write_head + size) - _one_past_end_of_mprotected_memory) / arena->_page_size + 1);
        uint num_bytes_to_mprotect = num_pages_to_mprotect * arena->_page_size;
        int mprotect_result = mprotect(
                _one_past_end_of_mprotected_memory,
                num_bytes_to_mprotect,
                PROT_READ | PROT_WRITE
                );
        ASSERT(mprotect_result == 0);
        arena->_num_committed_pages += num_pages_to_mprotect;
    }

    char *result = arena->_malloc_write_head;
    arena->_malloc_write_head += size;
    ASSERT(arena->_malloc_write_head <= _one_past_end_of_memory);
    return result;
}

void arena_free(Arena *arena) {
    ASSERT(arena->_reserved_memory);
    munmap(arena->_reserved_memory, arena->_num_reserved_pages *arena->_page_size);
    *arena = {};
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
};

template <typename T> void list_push_back(ArenaList<T> *list, T element) {
    ASSERT(list->arena);
    if (list->_capacity == 0) {
        ASSERT(!list->_array);
        ASSERT(list->length == 0);
        list->_capacity = 16;
        list->_array = (T *) arena_malloc(list->arena, list->_capacity * sizeof(T));
    }
    if (list->length == list->_capacity) {
        T *new_array = (T *) arena_malloc(list->arena, 2 * list->_capacity * sizeof(T));
        memcpy(new_array, list->_array, list->_capacity * sizeof(T));
        list->_array = new_array;
        list->_capacity *= 2;
    }
    list->_array[list->length++] = element;
}


// TODO: ShortList


// TODO: let me access with the []

template <typename Key, typename Value> struct PairKeyValue {
    Key key;
    Value value;
};

template <typename Key, typename Value> struct ArenaMap {
    Arena *arena;

    PairKeyValue<Key, Value> *_array;
    uint _capacity;

    // TODO: how do we make the square brackets work for putting and getting into a map
    // T &operator [](Key key) {
    //     return map_get(key);
    // }
};


template <typename Key, typename Value> void map_put(ArenaMap<Key, Value> *map, Key key, Value value) {
    FORNOW_UNUSED(map);
    FORNOW_UNUSED(key);
    FORNOW_UNUSED(value);
}

template <typename Key, typename Value> void map_get(ArenaMap<Key, Value> *map, Key key) {
    FORNOW_UNUSED(map);
    FORNOW_UNUSED(key);
}
