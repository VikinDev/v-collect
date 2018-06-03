/*
  +----------------------------------------------------------------------+
  | Collection Extension                                                 |
  +----------------------------------------------------------------------+
  | Copyright (c) 2016-2018 The Viest                                    |
  +----------------------------------------------------------------------+
  | http://www.viest.me                                                  |
  +----------------------------------------------------------------------+
  | Author: viest <dev@service.viest.me>                                 |
  +----------------------------------------------------------------------+
*/

#include "include.h"
#include "common.h"

double average_no_column(zend_array *current)
{
    double sum = 0;

    ZEND_HASH_FOREACH_VAL(current, zval* value)
        if (Z_TYPE_P(value) == IS_LONG) {
            sum += Z_LVAL_P(value);
        } else if (Z_TYPE_P(value) == IS_DOUBLE) {
            sum += Z_DVAL_P(value);
        }
    ZEND_HASH_FOREACH_END();

    return sum;
}

double average_column(zend_array *current, zend_string *column)
{
    double sum = 0;

    zval args[2], ret_val;

    ZVAL_ARR(&args[0], current);
    ZVAL_STR(&args[1], column);

    call_internal_function(INTERNAL_FUN(column), 2, args, &ret_val);

    if (Z_TYPE(ret_val) == IS_ARRAY) {
        sum = average_no_column(Z_ARRVAL(ret_val));
    }

    return sum;
}

void collection_chunk(zend_array *current, zend_long length, zval *object)
{
    zval args[2], ret_val;

    ZVAL_ARR (&args[0], current);
    ZVAL_LONG(&args[1], length);

    call_internal_function(INTERNAL_FUN(chunk), 2, args, &ret_val);

    UPDATE_OBJ_COLLECTION(object, &ret_val);
}

void collection_collapse(zend_array *z_array_p, zval *ret_val)
{
    ZEND_HASH_FOREACH_VAL(z_array_p, zval *zval_p)
        if (Z_TYPE_P(zval_p) == IS_ARRAY) {
            collection_collapse(Z_ARRVAL_P(zval_p), ret_val);
        } else {
            add_next_index_zval(ret_val, zval_p);
        }
    ZEND_HASH_FOREACH_END();
}

void collection_combine(zend_array *current_collection, zval *z_arr_val, zval *ret_val)
{
    zend_long z_arr_val_length = ZVAL_ARRAY_COUNT(z_arr_val);

    ZEND_HASH_FOREACH_BUCKET(current_collection, Bucket *bucket)
        if (bucket->h < z_arr_val_length) {
            ZVAL_ARRAY_INSERT_BUCKET_KEY_ZVAL_VAL(ret_val, bucket, z_arr_val);
        }
    ZEND_HASH_FOREACH_END();
}

void collection_concat(zend_array *current_collection, zval *z_arr, zval *ret_val)
{
    GC_REFCOUNT_IS_ONE_SO_ADD(GC_REFCOUNT(current_collection), current_collection);

    Z_ARR_P(ret_val) = current_collection;

    ZEND_HASH_FOREACH_VAL(Z_ARR_P(z_arr), zval *val)
        add_next_index_zval(ret_val, val);
    ZEND_HASH_FOREACH_END();
}

void collection_in_array_val(zend_array *current_collection, zval *search_val, zval *ret_val)
{
    if (Z_TYPE_P(search_val) == IS_LONG) {
        ZEND_HASH_FOREACH_VAL(current_collection, zval *val)
            if (fast_equal_check_long(search_val, val)) {
                ZVAL_TRUE(ret_val);
                return;
            }
        ZEND_HASH_FOREACH_END();
    } else if (Z_TYPE_P(search_val) == IS_STRING) {
        ZEND_HASH_FOREACH_VAL(current_collection, zval *val)
            if (fast_equal_check_string(search_val, val)) {
                ZVAL_TRUE(ret_val);
                return;
            }
        ZEND_HASH_FOREACH_END();
    } else {
        ZEND_HASH_FOREACH_VAL(current_collection, zval *val)
            if (fast_equal_check_function(search_val, val)) {
                ZVAL_TRUE(ret_val);
                return;
            }
        ZEND_HASH_FOREACH_END();
    }

    ZVAL_FALSE(ret_val);
}

int zval_equal(zval *z1, zval *z2)
{
    if (Z_TYPE_P(z1) == IS_LONG && Z_TYPE_P(z2) == IS_LONG) {
        if (fast_equal_check_long(z1, z2)) {
            return 1;
        }
    } else if (Z_TYPE_P(z1) == IS_STRING && Z_TYPE_P(z2) == IS_STRING) {
        if (fast_equal_check_string(z1, z2)) {
            return 1;
        }
    } else {
        if (fast_equal_check_function(z1, z2)) {
            return 1;
        }
    }

    return 0;
}

void collection_in_array_kv(zend_array *current_collection, zval *search_key, zval *search_val, zval *ret_val)
{
    zval *find_res = NULL;

    ZEND_HASH_FOREACH_VAL(current_collection, zval *val)
        if (Z_TYPE_P(val) == IS_ARRAY) {
            if (Z_TYPE_P(search_key) == IS_LONG) {
                find_res = zend_hash_index_find(Z_ARR_P(val), Z_LVAL_P(search_key));
            } else {
                find_res = zend_hash_find(Z_ARR_P(val), Z_STR_P(search_key));
            }

            if (find_res != NULL && zval_equal(find_res, search_val) == 1) {
                ZVAL_TRUE(ret_val);
                return;

            }
        }
    ZEND_HASH_FOREACH_END();

    ZVAL_FALSE(ret_val);
}

void collection_diff(zend_array *current_collection, zval *search_val, zval *ret_val)
{
    zval args[2];

    ZVAL_ARR (&args[0], current_collection);
    ZVAL_COPY(&args[1], search_val);

    call_internal_function(INTERNAL_FUN(diff), 2, args, ret_val);
}

void collection_diff_assoc(zend_array *current_collection, zval *search_val, zval *ret_val)
{
    zval args[2];

    ZVAL_ARR (&args[0], current_collection);
    ZVAL_COPY(&args[1], search_val);

    call_internal_function(INTERNAL_FUN(diff_assoc), 2, args, ret_val);
}

void collection_except(zend_array *current_collection, zval *excluded_keys, zval *ret_val)
{
    array_init_size(ret_val, current_collection->nNumOfElements);

    // TODO 此处应该检查 current_collection 的引用计数
    ret_val->value.arr = current_collection;

    ZEND_HASH_FOREACH_VAL(Z_ARR_P(excluded_keys), zval *val)
        if (Z_TYPE_P(val) == IS_LONG) {
            if (zend_hash_index_exists(current_collection, Z_LVAL_P(val))) {
                zend_hash_index_del(Z_ARR_P(ret_val), Z_LVAL_P(val));
            }
        } else {
            if (zend_hash_exists(current_collection, Z_STR_P(val))) {
                zend_hash_del(Z_ARR_P(ret_val), Z_STR_P(val));
            }
        }
    ZEND_HASH_FOREACH_END();
}

int zval_comparison_operations(zend_string *operator, zval *left, zval *right)
{
    if (strcmp(operator->val, "==") == 0) {
        if (zval_equal(left, right)) {
            return 1;
        }

        return 0;
    }

    if (strcmp(operator->val, "!=") == 0) {
        if (zval_equal(left, right)) {
            return 0;
        }

        return 1;
    }

    if (strcmp(operator->val, ">=") == 0) {
        if (EXPECTED(Z_TYPE_P(left) == IS_LONG)) {
            if (EXPECTED(Z_TYPE_P(right) == IS_LONG)) {
                return (Z_LVAL_P(left) >= Z_LVAL_P(right));
            } else if (EXPECTED(Z_TYPE_P(right) == IS_DOUBLE)) {
                return ((double)Z_LVAL_P(left) >= Z_DVAL_P(right));
            }
        } else if (EXPECTED(Z_TYPE_P(left) == IS_DOUBLE)) {
            if (EXPECTED(Z_TYPE_P(right) == IS_DOUBLE)) {
                return (Z_DVAL_P(left) >= Z_DVAL_P(right));
            } else if (EXPECTED(Z_TYPE_P(right) == IS_LONG)) {
                return (Z_DVAL_P(left) >= ((double)Z_LVAL_P(right)));
            }
        }

        return 0;
    }

    if (strcmp(operator->val, "<=") == 0) {
        if (EXPECTED(Z_TYPE_P(left) == IS_LONG)) {
            if (EXPECTED(Z_TYPE_P(right) == IS_LONG)) {
                return (Z_LVAL_P(left) <= Z_LVAL_P(right));
            } else if (EXPECTED(Z_TYPE_P(right) == IS_DOUBLE)) {
                return ((double)Z_LVAL_P(left) <= Z_DVAL_P(right));
            }
        } else if (EXPECTED(Z_TYPE_P(left) == IS_DOUBLE)) {
            if (EXPECTED(Z_TYPE_P(right) == IS_DOUBLE)) {
                return (Z_DVAL_P(left) <= Z_DVAL_P(right));
            } else if (EXPECTED(Z_TYPE_P(right) == IS_LONG)) {
                return (Z_DVAL_P(left) <= ((double)Z_LVAL_P(right)));
            }
        }

        return 0;
    }

    if (strcmp(operator->val, ">") == 0) {
        if (EXPECTED(Z_TYPE_P(left) == IS_LONG)) {
            if (EXPECTED(Z_TYPE_P(right) == IS_LONG)) {
                return (Z_LVAL_P(left) > Z_LVAL_P(right));
            } else if (EXPECTED(Z_TYPE_P(right) == IS_DOUBLE)) {
                return ((double)Z_LVAL_P(left) > Z_DVAL_P(right));
            }
        } else if (EXPECTED(Z_TYPE_P(left) == IS_DOUBLE)) {
            if (EXPECTED(Z_TYPE_P(right) == IS_DOUBLE)) {
                return (Z_DVAL_P(left) > Z_DVAL_P(right));
            } else if (EXPECTED(Z_TYPE_P(right) == IS_LONG)) {
                return (Z_DVAL_P(left) > ((double)Z_LVAL_P(right)));
            }
        }

        return 0;
    }

    if (strcmp(operator->val, "<") == 0) {
        if (EXPECTED(Z_TYPE_P(left) == IS_LONG)) {
            if (EXPECTED(Z_TYPE_P(right) == IS_LONG)) {
                return (Z_LVAL_P(left) < Z_LVAL_P(right));
            } else if (EXPECTED(Z_TYPE_P(right) == IS_DOUBLE)) {
                return ((double)Z_LVAL_P(left) < Z_DVAL_P(right));
            }
        } else if (EXPECTED(Z_TYPE_P(left) == IS_DOUBLE)) {
            if (EXPECTED(Z_TYPE_P(right) == IS_DOUBLE)) {
                return (Z_DVAL_P(left) < Z_DVAL_P(right));
            } else if (EXPECTED(Z_TYPE_P(right) == IS_LONG)) {
                return (Z_DVAL_P(left) < ((double)Z_LVAL_P(right)));
            }
        }

        return 0;
    }

    return 0;
}

void call_internal_function(const char *function_name, uint32_t param_count, zval params[], zval *ret_val)
{
    zval z_f_name;

    ZVAL_STRINGL(&z_f_name, function_name, strlen(function_name));

    call_user_function(EG(function_table), NULL, &z_f_name, ret_val, param_count, params);

    zval_ptr_dtor(&z_f_name);
}