template <typename T> struct List {
    uint length;
    uint _capacity;
    T *array;
    // T &operator [](int index) { return data[index]; }
};

template <typename T> void list_push_back(List<T> *list, T element) {
    if (list->_capacity == 0) {
        ASSERT(!list->array);
        ASSERT(list->length == 0);
        list->_capacity = 16;
        list->array = (T *) malloc(list->_capacity * sizeof(T));
    }
    if (list->length == list->_capacity) {
        list->_capacity *= 2;
        list->array = (T *) realloc(list->array, list->_capacity * sizeof(T));
    }
    list->array[list->length++] = element;
}

template <typename T> void list_free_AND_zero(List<T> *list) {
    if (list->array) free(list->array);
    *list = {};
}

template <typename T> void list_calloc_NOT_reserve(List<T> *list, unsigned int num_slots, unsigned int num_bytes_per_slot) {
    ASSERT(!list->array);
    list_free_AND_zero(list);
    list->_capacity = num_slots;
    list->length = num_slots;
    list->array = (T *) calloc(num_slots, num_bytes_per_slot);
}

template <typename T> void list_memset(List<T> *list, char byte_to_write, unsigned int num_bytes_to_write) {
    ASSERT(byte_to_write == 0); // NOTE: jim only ever calls memset(..., 0, ...); this check is just to catch his silly mistakes
    memset(list->array, byte_to_write, num_bytes_to_write);
}

template <typename T> void list_clone(List<T> *destination, List<T> *source) {
    list_free_AND_zero(destination);
    destination->length = source->length;
    destination->_capacity = source->_capacity;
    if (destination->_capacity != 0) {
        int num_bytes = destination->_capacity * sizeof(T);
        destination->array = (T *) malloc(num_bytes);
        memcpy(destination->array, source->array, num_bytes);
    }
}


template <typename T> void list_insert(List<T> *list, uint i, T element) {
    ASSERT(i <= list->length);
    list_push_back(list, {});
    memmove(&list->array[i + 1], &list->array[i], (list->length - i - 1) * sizeof(T));
    list->array[i] = element;
}

template <typename T> T list_delete_at(List<T> *list, uint i) {
    ASSERT(i >= 0);
    ASSERT(i < list->length);
    T result = list->array[i];
    memmove(&list->array[i], &list->array[i + 1], (list->length - i - 1) * sizeof(T));
    --list->length;
    return result;
}

template <typename T> void list_push_front(List<T> *list, T element) {
    list_insert(list, 0, element);
}


template <typename T> T list_pop_back(List<T> *list) {
    ASSERT(list->length != 0);
    return list_delete_at(list, list->length - 1);
}

template <typename T> T list_pop_front(List<T> *list) {
    ASSERT(list->length != 0);
    return list_delete_at(list, 0);
}

#define Queue List
#define queue_enqueue list_push_back
#define queue_dequeue list_pop_front
#define queue_free_AND_zero list_free_AND_zero
#define Stack List
#define stack_push list_push_back
#define stack_pop list_pop_back
#define stack_free_AND_zero list_free_AND_zero



template <typename Key, typename Value> struct Pair {
    union {
        struct {
            Key key;
            Value value;
        };
        struct {
            Key first;
            Key second;
        };
    };
};

template <typename Key, typename Value> struct Map {
    uint num_buckets;
    List<Pair<Key, Value>> *buckets;
    // T &operator [](int index) { return data[index]; }
};

// http://www.azillionmonkeys.com/qed/hash.html
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
    || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif
#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
        +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif
uint32_t paul_hsieh_SuperFastHash(void *_data, int len) {
    char *data = (char *) _data;

    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
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

template <typename Key, typename Value> void map_put(Map<Key, Value> *map, Key key, Value value) {
    if (!map->buckets) {
        map->num_buckets = 10001; //100003;
        map->buckets = (List<Pair<Key, Value>> *) calloc(map->num_buckets, sizeof(List<Pair<Key, Value>>));
    }
    { // TODO resizing; load factor; ...
    }
    List<Pair<Key, Value>> *bucket = &map->buckets[paul_hsieh_SuperFastHash(&key, sizeof(Key)) % map->num_buckets];
    for (Pair<Key, Value> *pair = bucket->array; pair < &bucket->array[bucket->length]; ++pair) {
        if (memcmp(&pair->key, &key, sizeof(Key)) == 0) {
            pair->value = value;
            return;
        }
    }
    list_push_back(bucket, { key, value });
}

template <typename Key, typename Value> Value *_map_get_pointer(Map<Key, Value> *map, Key key) {
    if (map->num_buckets == 0) return NULL;
    ASSERT(map->buckets);
    List<Pair<Key, Value>> *bucket = &map->buckets[paul_hsieh_SuperFastHash(&key, sizeof(Key)) % map->num_buckets];
    for (Pair<Key, Value> *pair = bucket->array; pair < &bucket->array[bucket->length]; ++pair) {
        if (memcmp(&pair->key, &key, sizeof(Key)) == 0) {
            return &pair->value;
        }
    }
    return NULL;
}

template <typename Key, typename Value> Value map_get(Map<Key, Value> *map, Key key, Value default_value = {}) {
    if (map->num_buckets == 0) return default_value;
    ASSERT(map->buckets);
    List<Pair<Key, Value>> *bucket = &map->buckets[paul_hsieh_SuperFastHash(&key, sizeof(Key)) % map->num_buckets];
    for (Pair<Key, Value> *pair = bucket->array; pair < &bucket->array[bucket->length]; ++pair) {
        if (memcmp(&pair->key, &key, sizeof(Key)) == 0) {
            return pair->value;
        }
    }
    return default_value;
}


template <typename Key, typename Value> void map_free_and_zero(Map<Key, Value> *map) {
    for (List<Pair<Key, Value>> *bucket = map->buckets; bucket < &map->buckets[map->num_buckets]; ++bucket) list_free_AND_zero(bucket);
    if (map->num_buckets) free(map->buckets);
    *map = {};
}

