#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpd.h"

typedef enum {
    cpd_basic_int = 0x00,
    cpd_basic_var = 0x02,

    cpd_basic_int64 = cpd_basic_int | 0x00,
    cpd_basic_int32 = cpd_basic_int | 0x01,

    cpd_basic_var_pos = cpd_basic_var | 0x00,
    cpd_basic_var_neg = cpd_basic_var | 0x01,

    cpd_type_val = 0x08,
    cpd_type_len = 0x00,

    cpd_type_int = cpd_type_val | 0x00,
    cpd_type_link = cpd_type_val | 0x04,

    cpd_type_string = cpd_type_len | 0x00,
    cpd_type_compose = cpd_type_len | 0x04,
} cpd_types;

typedef struct {
    uint64_t obj;
    uint64_t pos;
} tree_data_t;

typedef struct tree_node_st {
    tree_data_t data;
    struct tree_node_st *childs[2];
    char color;
} tree_node_t;

typedef struct {
    tree_node_t *root;
    uint64_t count;
} tree_t;

typedef struct cpd_obj_m_st {
    uint8_t _type;

    uint8_t *_content;
    uint64_t _size;

    struct cpd_obj_m_st *next;
} cpd_obj_m;

typedef struct cpd_obj_u_st {
    uint8_t _type;

    const uint8_t *_content;
    uint64_t _size;

    struct cpd_obj_u_st *next;
} cpd_obj_u;

typedef struct {
    cpd_obj_u *first;
    cpd_obj_u *last;

    tree_t *tree;
} cpd_ctx_unmarshal;

typedef struct {
    cpd_obj_m *first;
    cpd_obj_m *last;

    uint64_t size;
    tree_t *tree;
} cpd_ctx_marshal;

typedef struct {
    uint8_t _str[16];
    uint64_t _size;
} varint_t;


typedef void *(cpd_obj_new)(void *);
typedef int32_t(*cpd_marshal_func)(void *, cpd_ctx_marshal *);
typedef int32_t(*cpd_unmarshal_func)(void *, cpd_ctx_unmarshal *);


uint64_t cpd_varint_marshal(uint64_t _val, uint8_t *_str) {
    uint64_t _size = 0;
    for (; _val > 0x7F; _val >>= 7) _str[_size++] = _val & 0x7F | 0x80;
    _str[_size++] = _val;
    return _size;
}
uint64_t cpd_basic_marshal(const uint64_t _val, uint8_t *_str, uint8_t *_type) {
    uint64_t _size = 0;
    for (uint64_t val = _val; val; val >>= 8) _size++;

    if (_size <= 4) {
        _size = cpd_varint_marshal(_val, _str);
        if (_size < 4) goto end_p;
        _size = cpd_varint_marshal(-(uint32_t) _val, _str);
        if (_size < 4) goto end_n;
        *_type = cpd_basic_int32;
    } else {
        _size = cpd_varint_marshal(_val, _str);
        if (_size < 8) goto end_p;
        _size = cpd_varint_marshal(-_val, _str);
        if (_size < 8) goto end_n;
        *_type = cpd_basic_int64;
    }
    _size = 0;
    for (uint64_t val = _val; val; val >>= 8) _str[_size++] = val & 0xFF;
    return _size;

    end_p:
    *_type = cpd_basic_var_pos;
    return _size;

    end_n:
    *_type = cpd_basic_var_neg;
    return _size;
}
uint64_t cpd_basic_unmarshal(const uint8_t *_str, const uint64_t _type, uint64_t *val) {
    uint64_t _val = 0;
    int64_t _neg = 1;
    uint64_t _pos = 0;
    switch (_type & cpd_basic_var_neg) {
        case cpd_basic_int64:
            for (uint64_t i = 0; i < 4; ++i, ++_pos) _val |= ((uint64_t) _str[_pos]) << (_pos * 8);
        case cpd_basic_int32:
            for (uint64_t i = 0; i < 4; ++i, ++_pos) _val |= ((uint64_t) _str[_pos]) << (_pos * 8);
        break;
        case cpd_basic_var_neg:
            _neg = -1;
        case cpd_basic_var_pos:
            for (;_str[_pos] & 0x80; ++_pos) _val |= ((uint64_t) _str[_pos] & 0x7F) << (_pos * 7);
            _val |= ((uint64_t) _str[_pos] & 0x7F) << (_pos * 7);
            ++_pos;
        break;
        default:;
    }
    *val = _val * _neg;
    return _pos;
}


int32_t cpd_basic_data_marshal(cpd_ctx_marshal *ctx, const uint64_t val) {
    if (ctx == NULL) return -1;
    cpd_obj_m *obj = calloc(1, sizeof(cpd_obj_m));
    if (obj != NULL) ctx->last = obj;
    else return -1;
    if (ctx->first == NULL) ctx->first = obj;
    else ctx->last->next = obj;

    uint8_t _type;
    uint8_t _str[16] = {0};
    const uint64_t _size = cpd_basic_marshal(val, _str, &_type);
    *obj = (cpd_obj_m){_type | cpd_type_int, malloc(_size), _size};
    if (obj->_content == NULL)  return -1;
    ctx->size += _size + 1;

    memcpy(obj->_content, _str, _size);
    return 0;
}
int32_t cpd_marshal_str(cpd_ctx_marshal *ctx, const char *str, const uint64_t size) {
    if (ctx == NULL) return -1;
    cpd_obj_m *obj = calloc(1, sizeof(cpd_obj_m));
    if (obj != NULL) ctx->last = obj;
    else return -1;
    if (ctx->first == NULL) ctx->first = obj;
    else ctx->last->next = obj;

    uint8_t _str[16] = {0};
    uint8_t _type;
    const uint64_t _size = cpd_basic_marshal(size, _str, &_type);

    *obj = (cpd_obj_m){_type | cpd_type_string, malloc(size + _size), size + _size};
    if (obj->_content == NULL) return -1;
    ctx->size += size + _size + 1;

    memcpy(obj->_content, _str, _size);
    memcpy(obj->_content + _size, str, size);
    return 0;
}


int32_t cpd_basic_data_unmarshal(cpd_ctx_unmarshal *ctx, uint64_t *val) {
    if (ctx == NULL) return -1;
    if (ctx->first == NULL) return -1;
    const cpd_obj_u *obj = ctx->first;
    ctx->first = ctx->first->next;
    if ((obj->_type & 0x0c) != cpd_type_int) return -1;

    *val = obj->_size;

    return 0;
}
int32_t cpd_unmarshal_str(cpd_ctx_unmarshal *ctx, char *str, const uint64_t size, uint64_t *res_size) {
    if (ctx == NULL) return -1;
    if (ctx->first == NULL) return -1;
    const cpd_obj_u *obj = ctx->first;
    ctx->first = ctx->first->next;

    if ((obj->_type & 0x0c) != cpd_type_string) return -1;
    if (obj->_content == NULL) return -1;

    *res_size = size < obj->_size? size: obj->_size;
    memcpy(str, obj->_content, *res_size);
    return 0;
}