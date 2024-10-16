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