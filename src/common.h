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

#ifndef PHP_EXT_COLLECTION_COMMON_H
#define PHP_EXT_COLLECTION_COMMON_H

#define VAL_IN_COLLECTION(cc, search, ret) \
    collection_in_array_val(cc, search, ret)

#define KV_IN_COLLECTION(cc, k, v, ret) \
    collection_in_array_kv(cc, k, v, ret)

#define INTERNAL_ARR    "array_"
#define INTERNAL_FUN(m) INTERNAL_ARR#m

void call_internal_function(const char *function_name, uint32_t param_count, zval params[], zval *ret_val);

int zval_equal(zval *z1, zval *z2);
int zval_comparison_operations(zend_string *operator, zval *left, zval *right);

double average_no_column(zend_array *current);
double average_column(zend_array *current, zend_string *column);
void   collection_chunk(zend_array *current, zend_long length, zval *object);
void   collection_collapse(zend_array *z_array_p, zval *ret_val);
void   collection_combine(zend_array *current_collection, zval *z_arr_val, zval *ret_val);
void   collection_concat(zend_array *current_collection, zval *z_arr, zval *ret_val);
void   collection_in_array_val(zend_array *current_collection, zval *search_val, zval *ret_val);
void   collection_in_array_kv(zend_array *current_collection, zval *search_key, zval *search_val, zval *ret_val);
void   collection_diff(zend_array *current_collection, zval *search_val, zval *ret_val);
void   collection_diff_assoc(zend_array *current_collection, zval *search_val, zval *ret_val);
void   collection_except(zend_array *current_collection, zval *excluded_keys, zval *ret_val);

#endif
