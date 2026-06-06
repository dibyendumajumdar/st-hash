// Tests created with the help of ChatGPT
#include "st.h"

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

static void test_string_insert_lookup_overwrite(void)
{
    st_table *tab = st_init_strtable();
    assert(tab != NULL);
    assert(st_table_size(tab) == 0);

    assert(st_insert(tab, (st_data_t)"a", 10) == 0);
    assert(st_insert(tab, (st_data_t)"b", 20) == 0);
    assert(st_insert(tab, (st_data_t)"c", 30) == 0);
    assert(st_table_size(tab) == 3);

    st_data_t value = 0;
    assert(st_lookup(tab, (st_data_t)"a", &value) == 1 && value == 10);
    assert(st_lookup(tab, (st_data_t)"b", &value) == 1 && value == 20);
    assert(st_lookup(tab, (st_data_t)"c", &value) == 1 && value == 30);
    assert(st_lookup(tab, (st_data_t)"missing", &value) == 0);

    assert(st_insert(tab, (st_data_t)"b", 200) == 1);
    assert(st_table_size(tab) == 3);
    assert(st_lookup(tab, (st_data_t)"b", &value) == 1 && value == 200);

    st_free_table(tab);
}

static void test_numeric_table_many_entries(void)
{
    enum { N = 1000 };
    st_table *tab = st_init_numtable();
    assert(tab != NULL);

    for (st_data_t i = 0; i < N; i++) {
        assert(st_insert(tab, i, i * 10) == 0);
    }
    assert(st_table_size(tab) == N);

    for (st_data_t i = 0; i < N; i++) {
        st_data_t value = 0;
        assert(st_lookup(tab, i, &value) == 1);
        assert(value == i * 10);
    }

    st_free_table(tab);
}

static void test_delete_and_reinsert(void)
{
    st_table *tab = st_init_strtable();
    assert(tab != NULL);

    const char *keys[] = {"a", "b", "c", "d", "e", "f"};
    for (st_data_t i = 0; i < ARRAY_LEN(keys); i++) {
        assert(st_insert(tab, (st_data_t)keys[i], i + 1) == 0);
    }

    st_data_t key = (st_data_t)"c";
    st_data_t value = 0;
    assert(st_delete(tab, &key, &value) == 1);
    assert(strcmp((const char *)key, "c") == 0);
    assert(value == 3);
    assert(st_lookup(tab, (st_data_t)"c", &value) == 0);
    assert(st_table_size(tab) == ARRAY_LEN(keys) - 1);

    key = (st_data_t)"not-present";
    value = 12345;
    assert(st_delete(tab, &key, &value) == 0);
    assert(value == 0);

    assert(st_insert(tab, (st_data_t)"c", 300) == 0);
    assert(st_lookup(tab, (st_data_t)"c", &value) == 1 && value == 300);
    assert(st_table_size(tab) == ARRAY_LEN(keys));

    st_free_table(tab);
}

static void test_shift_preserves_insertion_order(void)
{
    st_table *tab = st_init_strtable();
    assert(tab != NULL);

    const char *keys[] = {"first", "second", "third"};
    for (st_data_t i = 0; i < ARRAY_LEN(keys); i++) {
        assert(st_insert(tab, (st_data_t)keys[i], i + 100) == 0);
    }

    for (st_data_t i = 0; i < ARRAY_LEN(keys); i++) {
        st_data_t key = 0;
        st_data_t value = 0;
        assert(st_shift(tab, &key, &value) == 1);
        assert(strcmp((const char *)key, keys[i]) == 0);
        assert(value == i + 100);
    }

    st_data_t key = 0;
    st_data_t value = 999;
    assert(st_shift(tab, &key, &value) == 0);
    assert(value == 0);
    assert(st_table_size(tab) == 0);

    st_free_table(tab);
}

static void test_keys_and_values(void)
{
    st_table *tab = st_init_strtable();
    assert(tab != NULL);

    assert(st_insert(tab, (st_data_t)"x", 1) == 0);
    assert(st_insert(tab, (st_data_t)"y", 2) == 0);
    assert(st_insert(tab, (st_data_t)"z", 3) == 0);

    st_data_t keys[3] = {0};
    st_data_t values[3] = {0};
    assert(st_keys(tab, keys, 3) == 3);
    assert(st_values(tab, values, 3) == 3);

    assert(strcmp((const char *)keys[0], "x") == 0);
    assert(strcmp((const char *)keys[1], "y") == 0);
    assert(strcmp((const char *)keys[2], "z") == 0);
    assert(values[0] == 1 && values[1] == 2 && values[2] == 3);

    st_free_table(tab);
}

static void test_case_insensitive_string_table(void)
{
    st_table *tab = st_init_strcasetable();
    assert(tab != NULL);

    assert(st_insert(tab, (st_data_t)"Hello", 42) == 0);

    st_data_t value = 0;
    assert(st_lookup(tab, (st_data_t)"hello", &value) == 1 && value == 42);
    assert(st_lookup(tab, (st_data_t)"HELLO", &value) == 1 && value == 42);

    assert(st_insert(tab, (st_data_t)"hElLo", 99) == 1);
    assert(st_table_size(tab) == 1);
    assert(st_lookup(tab, (st_data_t)"HELLO", &value) == 1 && value == 99);

    st_free_table(tab);
}

static int delete_even_callback(st_data_t key, st_data_t value, st_data_t arg)
{
    (void)value;
    st_data_t *sum = (st_data_t *)arg;
    *sum += key;
    return (key % 2 == 0) ? ST_DELETE : ST_CONTINUE;
}

static void test_foreach_delete(void)
{
    st_table *tab = st_init_numtable();
    assert(tab != NULL);

    for (st_data_t i = 0; i < 10; i++) {
        assert(st_insert(tab, i, i + 1000) == 0);
    }

    st_data_t sum = 0;
    assert(st_foreach(tab, delete_even_callback, (st_data_t)&sum) == 0);
    assert(sum == 45);
    assert(st_table_size(tab) == 5);

    for (st_data_t i = 0; i < 10; i++) {
        st_data_t value = 0;
        assert(st_lookup(tab, i, &value) == (i % 2 != 0));
    }

    st_free_table(tab);
}

static int update_insert_or_increment(st_data_t *key, st_data_t *value,
                                      st_data_t arg, int existing)
{
    (void)key;
    if (existing) {
        *value += arg;
    }
    else {
        *value = arg;
    }
    return ST_CONTINUE;
}

static void test_update_insert_and_modify(void)
{
    st_table *tab = st_init_numtable();
    assert(tab != NULL);

    assert(st_update(tab, 7, update_insert_or_increment, 10) == 0);
    st_data_t value = 0;
    assert(st_lookup(tab, 7, &value) == 1 && value == 10);

    assert(st_update(tab, 7, update_insert_or_increment, 5) == 1);
    assert(st_lookup(tab, 7, &value) == 1 && value == 15);

    st_free_table(tab);
}

static void test_copy_is_independent(void)
{
    st_table *orig = st_init_strtable();
    assert(orig != NULL);
    assert(st_insert(orig, (st_data_t)"a", 1) == 0);
    assert(st_insert(orig, (st_data_t)"b", 2) == 0);

    st_table *copy = st_copy(orig);
    assert(copy != NULL);

    assert(st_insert(orig, (st_data_t)"a", 100) == 1);
    assert(st_insert(copy, (st_data_t)"b", 200) == 1);

    st_data_t value = 0;
    assert(st_lookup(orig, (st_data_t)"a", &value) == 1 && value == 100);
    assert(st_lookup(copy, (st_data_t)"a", &value) == 1 && value == 1);
    assert(st_lookup(orig, (st_data_t)"b", &value) == 1 && value == 2);
    assert(st_lookup(copy, (st_data_t)"b", &value) == 1 && value == 200);

    st_free_table(copy);
    st_free_table(orig);
}

static void test_clear(void)
{
    st_table *tab = st_init_numtable();
    assert(tab != NULL);

    for (st_data_t i = 0; i < 20; i++) {
        assert(st_insert(tab, i, i * 2) == 0);
    }
    assert(st_table_size(tab) == 20);

    st_clear(tab);
    assert(st_table_size(tab) == 0);

    st_data_t value = 0;
    assert(st_lookup(tab, 10, &value) == 0);
    assert(st_insert(tab, 10, 123) == 0);
    assert(st_lookup(tab, 10, &value) == 1 && value == 123);

    st_free_table(tab);
}

int main(void)
{
    test_string_insert_lookup_overwrite();
    test_numeric_table_many_entries();
    test_delete_and_reinsert();
    test_shift_preserves_insertion_order();
    test_keys_and_values();
    test_case_insensitive_string_table();
    test_foreach_delete();
    test_update_insert_and_modify();
    test_copy_is_independent();
    test_clear();

    puts("all st tests passed");
    return 0;
}
