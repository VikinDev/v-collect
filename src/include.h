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

#ifndef PHP_EXT_COLLECTION_INCLUDE_H
#define PHP_EXT_COLLECTION_INCLUDE_H

#include <php.h>

#include "ext/standard/php_var.h"

#include "zend_types.h"
#include "zend_exceptions.h"
#include "zend.h"
#include "zend_API.h"
#include "php.h"

#include "../php_collection.h"

// Class
extern zend_class_entry *vtiful_collection_ce;
extern zend_class_entry *vtiful_collection_exception_ce;

// Startup Module
COLLECTION_STARTUP_FUNCTION(exception);
COLLECTION_STARTUP_FUNCTION(kernel);

// Kernel Class Method
PHP_METHOD(vtiful_collection, __construct);
PHP_METHOD(vtiful_collection, __clone);
PHP_METHOD(vtiful_collection, init);
PHP_METHOD(vtiful_collection, map);
PHP_METHOD(vtiful_collection, all);
PHP_METHOD(vtiful_collection, avg);
PHP_METHOD(vtiful_collection, chunk);
PHP_METHOD(vtiful_collection, collapse);
PHP_METHOD(vtiful_collection, count);
PHP_METHOD(vtiful_collection, toArray);
PHP_METHOD(vtiful_collection, combine);
PHP_METHOD(vtiful_collection, concat);
PHP_METHOD(vtiful_collection, contains);
PHP_METHOD(vtiful_collection, diff);
PHP_METHOD(vtiful_collection, diffAssoc);
PHP_METHOD(vtiful_collection, every);
PHP_METHOD(vtiful_collection, except);
PHP_METHOD(vtiful_collection, filter);
PHP_METHOD(vtiful_collection, first);
PHP_METHOD(vtiful_collection, firstWhere);

// PHP Compatible
#ifndef GC_ADDREF
#define GC_ADDREF(p) ++(&(p->gc))->refcount
#endif

// GC
#define GC_REFCOUNT_IS_ONE_SO_ADD(refcount, val_p) \
    do {                                           \
        if (refcount == 1) {                       \
            GC_ADDREF(val_p);                      \
        }                                          \
    } while (0)


#define GC_ZVAL_ADDREF(zval_p)                                                            \
    do {                                                                                  \
        switch (Z_TYPE_P(zval_p)) {                                                       \
            case IS_STRING:                                                               \
                GC_REFCOUNT_IS_ONE_SO_ADD(GC_REFCOUNT(Z_STR_P(zval_p)), Z_STR_P(zval_p)); \
                break;                                                                    \
            case IS_ARRAY:                                                                \
                GC_REFCOUNT_IS_ONE_SO_ADD(GC_REFCOUNT(Z_ARR_P(zval_p)), Z_ARR_P(zval_p)); \
                break;                                                                    \
            case IS_OBJECT:                                                               \
                GC_REFCOUNT_IS_ONE_SO_ADD(GC_REFCOUNT(Z_OBJ_P(zval_p)), Z_OBJ_P(zval_p)); \
                break;                                                                    \
        }                                                                                 \
    } while (0)

// Define
#define CURRENT_COLLECTION \
        Z_OBJ_P(getThis())->properties

#define CURRENT_COLLECTION_COUNT \
        CURRENT_COLLECTION->nNumOfElements

#define ZVAL_ARRAY_COUNT(zval_p) \
        Z_ARR_P(zval_p)->nNumOfElements

#define COLLECTION_INIT(zval_p) \
        do {                    \
            array_init(zval_p); \
        } while (0)


#define NEW_ZVAL_OBJ(zval_p)                              \
        do {                                              \
            object_init_ex(zval_p, vtiful_collection_ce); \
        } while (0)

#define UPDATE_OBJ_COLLECTION(zo_p, z_collection_p)            \
        do {                                                   \
            if (Z_TYPE_P(z_collection_p) == IS_ARRAY) {        \
                zend_object *obj = Z_OBJ_P(zo_p);              \
                obj->properties  = Z_ARRVAL_P(z_collection_p); \
            }                                                  \
        } while (0)

#define NEW_COLLECTION_OBJ(ret_val, collection)         \
        do {                                            \
            NEW_ZVAL_OBJ(ret_val);                      \
            UPDATE_OBJ_COLLECTION(ret_val, collection); \
        } while(0)


#define INIT_FCALL(arg_num, ret_val_p) \
        zval args[arg_num];            \
        fci.param_count = arg_num;     \
        fci.retval      = ret_val_p;   \
        fci.params      = args;

#define FCALL_TWO_ARGS(bucket)                   \
        ZVAL_COPY_VALUE(&args[0], &bucket->val); \
        if (bucket->key) {                       \
            ZVAL_STR(&args[1], bucket->key);     \
        } else {                                 \
            ZVAL_LONG(&args[1], bucket->h);      \
        }                                        \
        zend_call_function(&fci, &fci_cache);

#define ZVAL_ARRAY_FIND(zval_p, index, zval_result_p)          \
    do {                                                       \
        zval_result_p = &(Z_ARR_P(zval_p)->arData[index].val); \
    } while (0)


#define ZVAL_ARRAY_INSERT_BUCKET_KEY_ZVAL_VAL(zval_p, bucket, zval_val_p)                                   \
    do {                                                                                                    \
        zval *value = NULL;                                                                                 \
        if (Z_TYPE(bucket->val) == IS_STRING) {                                                             \
            ZVAL_ARRAY_FIND(z_arr_val, bucket->h, value);                                                   \
            zend_hash_str_update(Z_ARRVAL_P(ret_val), Z_STRVAL(bucket->val), Z_STRLEN(bucket->val), value); \
        }                                                                                                   \
        if (Z_TYPE(bucket->val) == IS_LONG) {                                                               \
            ZVAL_ARRAY_FIND(z_arr_val, bucket->h, value);                                                   \
            zend_hash_index_update(Z_ARRVAL_P(ret_val), Z_LVAL(bucket->val), value);                        \
        }                                                                                                   \
    } while(0)

#endif
