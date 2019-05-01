#include "0cc.h"

/* Vector functions */

Vector *new_vector() {
    Vector *vec = malloc(sizeof(Vector));

    int default_capacity = 16;

    vec->data = malloc(sizeof(void *) * default_capacity);
    vec->capacity = default_capacity;
    vec->len = 0;

    return vec;
}

void vec_push(Vector *vec, void *elem) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len] = elem;
    vec->len++;
}

/* Map functions */

Map *new_map() {
    Map *map = malloc(sizeof(Map));

    map->keys = new_vector();
    map->vals = new_vector();

    return map;
}

void map_push(Map *map, char *key, void *val) {
    vec_push(map->keys, (void *)key);
    vec_push(map->vals, val);
}

void *map_get(Map *map, char *key) {
    for (int i = map->keys->len - 1; i >= 0; i--) {
        if (strcmp((char *)map->keys->data[i], key) == 0) {
            return map->vals->data[i];
        }
    }

    return NULL;
}
