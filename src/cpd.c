#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpd.h"

#define BLACK   0
#define RED     1

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

typedef struct tree_node_st {
    uint64_t obj;
    uint64_t pos;
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

struct cpd_ctx_unmarshal_st {
    cpd_obj_u *first;
    cpd_obj_u *last;

    tree_t *tree;
};
typedef struct cpd_ctx_marshal_st {
    cpd_obj_m *first;
    cpd_obj_m *last;

    uint64_t size;
    tree_t *tree;
} cpd_ctx_marshal;



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


uint16_t tree_sides[256];
tree_node_t *tree_parents[256];

uint64_t tree_find_pos(const tree_t *tree, const uint64_t obj) {
    const tree_node_t *node = tree->root;

    while (node) {
        if (obj == node->obj) return node->pos;
        node = node->childs[node->obj < obj];
    }
    return 0;
}
uint64_t tree_find_obj(const tree_t *tree, const uint64_t pos) {
    const tree_node_t *node = tree->root;

    while (node) {
        if (pos == node->pos) return node->pos;
        node = node->childs[node->pos < pos];
    }
    return 0;
}
void tree_optimize(tree_t *tree, int pos) {
    while (--pos >= 0) {
        int side = tree_sides[pos];
        tree_node_t *g_ = tree_parents[pos]; // Grand Parent
        tree_node_t *y_ = g_->childs[1 - side]; // Unlce
        tree_node_t *x_ = tree_parents[pos + 1]; // Parent

        if (x_->color == BLACK) break;
        if (y_ && y_->color == RED) {
            x_->color = BLACK;
            y_->color = BLACK;
            g_->color = RED;

            --pos;
            continue;
        }

        if (side == 1 - tree_sides[pos + 1]) {
            y_ = x_->childs[1 - side]; // y_ is child
            x_->childs[1 - side] = y_->childs[side];
            y_->childs[side] = x_;
            x_ = g_->childs[side] = y_;
        }
        g_->color = RED;
        x_->color = BLACK;
        g_->childs[side] = x_->childs[1 - side];
        x_->childs[1 - side] = g_;

        if (pos == 0) tree->root = x_;
        else tree_parents[pos - 1]->childs[tree_sides[pos - 1]] = x_;
        break;
    }

    tree->root->color = BLACK;
}
void tree_insert_by_pos(tree_t *tree, const uint64_t _obj, const uint64_t _pos) {
    tree_node_t *node = tree->root;
    int side = 0, pos = -1;

    while (node) {
        if (_pos == node->pos) return;
        tree_parents[++pos] = node;
        side = tree_sides[pos] = node->pos < _pos;
        node = node->childs[side];
    }

    tree_node_t *new_node = calloc(1, sizeof(tree_node_t));
    new_node->pos = _pos;
    new_node->obj = _obj;
    new_node->color = RED;
    if (pos == -1) tree->root = new_node;
    else tree_parents[pos]->childs[side] = new_node;

    tree_optimize(tree, pos);
}
void tree_insert_by_obj(tree_t *tree, const uint64_t _obj, const uint64_t _pos) {
    tree_node_t *node = tree->root;
    int side = 0, pos = -1;

    while (node) {
        if (_pos == node->pos) return;
        tree_parents[++pos] = node;
        side = tree_sides[pos] = node->pos < _pos;
        node = node->childs[side];
    }

    tree_node_t *new_node = calloc(1, sizeof(tree_node_t));
    new_node->pos = _pos;
    new_node->obj = _obj;
    new_node->color = RED;
    if (pos == -1) tree->root = new_node;
    else tree_parents[pos]->childs[side] = new_node;

    tree_optimize(tree, pos);
}
void neurons_tree_free(tree_t *tree) {
    tree_node_t *next = tree->root;
    int pos = -1;

    while (1) {
        while (next) {
            tree_parents[++pos] = next;
            next = next->childs[0];
        }
        if (pos == -1) break;
        tree_node_t *node = tree_parents[pos--];
        next = node->childs[1];

        free(node);
    }

    tree->root = NULL;
}

#define CDP_CONTEXT_NEW(type, ctx, obj) \
type *obj = calloc(1, sizeof(type));    \
if (obj != NULL) ctx->last = obj;       \
else

#define CDP_CONTEXT_APPEND(ctx, obj)        \
if (ctx->first == NULL) ctx->first = obj;   \
else ctx->last->next = obj;

#define CDP_MARSHAL_HEADER                                      \
if (ctx == NULL) return CPD_FLAG_ERR_NULLPTR;                   \
CDP_CONTEXT_NEW(cpd_obj_m, ctx, obj) return CPD_FLAG_ERR_ALLOC; \
CDP_CONTEXT_APPEND(ctx, obj)                                    \
uint8_t _str[16] = {0};                                         \
uint8_t _type; \
uint64_t _size;

#define CDP_UNMARSHAL_HEADER                            \
if (ctx == NULL) return CPD_FLAG_ERR_NULLPTR;           \
if (ctx->first == NULL) return CPD_FLAG_ERR_NULLPTR;    \
const cpd_obj_u *obj = ctx->first;                      \
ctx->first = ctx->first->next;                          \

#define CDP_MARSHAL_OBJECT_INIT(type, size)     \
*obj = (cpd_obj_m){type, malloc(size), size};   \
if (obj->_content == NULL)


int32_t cpd_basic_data_marshal(cpd_ctx_marshal *ctx, const uint64_t val) {
    CDP_MARSHAL_HEADER

    _size = cpd_basic_marshal(val, _str, &_type);
    CDP_MARSHAL_OBJECT_INIT(_type | cpd_type_int, _size) return CPD_FLAG_ERR_ALLOC;
    ctx->size += _size + 1;

    memcpy(obj->_content, _str, _size);
    return CPD_FLAG_SUCCESS;
}
int32_t cpd_marshal_str(cpd_ctx_marshal *ctx, const char *str, const uint64_t size) {
    CDP_MARSHAL_HEADER

    _size = cpd_basic_marshal(size, _str, &_type);
    CDP_MARSHAL_OBJECT_INIT(_type | cpd_type_string, size + _size) return CPD_FLAG_ERR_ALLOC;
    ctx->size += size + _size + 1;

    memcpy(obj->_content, _str, _size);
    memcpy(obj->_content + _size, str, size);
    return CPD_FLAG_SUCCESS;
}
int32_t cpd_marshal(void *_obj, const cpd_marshal_func func, cpd_ctx_marshal *ctx) {
    CDP_MARSHAL_HEADER

    const uint64_t src_pos = tree_find_pos(ctx->tree, (uint64_t) _obj);
    if (src_pos) {
        _size = cpd_basic_marshal(src_pos, _str, &_type);
        CDP_MARSHAL_OBJECT_INIT(_type | cpd_type_link, _size) return CPD_FLAG_ERR_ALLOC;
        ctx->size += _size + 1;

        memcpy(obj->_content, _str, _size);
        return CPD_FLAG_SUCCESS;
    }
    if (_obj != NULL) tree_insert_by_obj(ctx->tree, (uint64_t) _obj, ++ctx->tree->count);

    cpd_ctx_marshal *_ctx = calloc(1, sizeof(cpd_ctx_marshal));
    if (_ctx == NULL) return CPD_FLAG_ERR_ALLOC;
    _ctx->tree = ctx->tree;

    int32_t res = func(_obj, _ctx);
    if (res != 0) goto end;

    _size = cpd_basic_marshal(_ctx->size, _str, &_type);
    CDP_MARSHAL_OBJECT_INIT(_type | cpd_type_compose, _ctx->size + _size) {
        res = CPD_FLAG_ERR_ALLOC;
        goto end;
    }
    ctx->size += _ctx->size + _size;

    memcpy(obj->_content, _str, _size);
    for (const cpd_obj_m *elm = _ctx->first; elm != NULL; elm = elm->next) {
        obj->_content[_size++] = elm->_type;
        memcpy(obj->_content + _size, elm->_content, elm->_size);
        _size += elm->_size;
    }

    end:
    for (cpd_obj_m *elm = _ctx->first, *next; elm != NULL; elm = next) {
        next = elm->next;
        if (elm->_content != NULL) free(elm->_content);
        free(elm);
    }
    free(_ctx);
    return res;
}


int32_t cpd_basic_data_unmarshal(cpd_ctx_unmarshal *ctx, uint64_t *val) {
    CDP_UNMARSHAL_HEADER
    if ((obj->_type & 0x0c) != cpd_type_int) return CPD_FLAG_ERR_TYPE;

    *val = obj->_size;

    return CPD_FLAG_SUCCESS;
}
int32_t cpd_unmarshal_str(cpd_ctx_unmarshal *ctx, char *str, const uint64_t size, uint64_t *res_size) {
    CDP_UNMARSHAL_HEADER

    if ((obj->_type & 0x0c) != cpd_type_string) return CPD_FLAG_ERR_TYPE;
    if (obj->_content == NULL) return CPD_FLAG_ERR_NULLPTR;

    *res_size = size < obj->_size? size: obj->_size;
    memcpy(str, obj->_content, *res_size);
    return CPD_FLAG_SUCCESS;
}
int32_t cpd_unmarshal(void **_obj, void *_data, const cpd_unmarshal_func func, cpd_obj_new func_new, cpd_ctx_unmarshal *ctx) {
    CDP_UNMARSHAL_HEADER

    if ((obj->_type & 0x0c) != cpd_type_compose) {
        if ((obj->_type & 0x0c) != cpd_type_link) return CPD_FLAG_ERR_TYPE;
        if (_obj != NULL) *_obj = (void *) tree_find_obj(ctx->tree, obj->_size);
        return CPD_FLAG_SUCCESS;
    }
    if (obj->_content == NULL) return CPD_FLAG_ERR_NULLPTR;
    if (_obj != NULL) {
        if (func_new != NULL) *_obj = func_new(_data);
        else *_obj = NULL;
        tree_insert_by_pos(ctx->tree, (uint64_t) *_obj, ++ctx->tree->count);
    }

    cpd_ctx_unmarshal *_ctx = calloc(1, sizeof(cpd_ctx_unmarshal));
    if (_ctx == NULL) return CPD_FLAG_ERR_ALLOC;
    _ctx->tree = ctx->tree;

    int32_t res = CPD_FLAG_SUCCESS;
    for (uint64_t pos = 0; pos < obj->_size;) {
        CDP_CONTEXT_NEW(cpd_obj_u, _ctx, ctx_obj) {
            res = CPD_FLAG_ERR_ALLOC;
            goto end;
        }
        CDP_CONTEXT_APPEND(_ctx, ctx_obj)

        ctx_obj->_type = obj->_content[pos++];
        pos += cpd_basic_unmarshal(obj->_content + pos, ctx_obj->_type, &ctx_obj->_size);
        if ((ctx_obj->_type & cpd_type_val) == cpd_type_len) {
            ctx_obj->_content = obj->_content + pos;
            pos += ctx_obj->_size;
        }
        if (pos > obj->_size) {
            res = CPD_FLAG_ERR_EOF;
            goto end;
        }
    }

    res = func(_obj, _ctx);
    end:
    for (cpd_obj_u *elm = _ctx->first, *next; elm != NULL; elm = next) {
        next = elm->next;
        free(elm);
    }
    free(_ctx);
    return res;
}

#define CPD_CONSTRUCT(a, b) a##b
#define CPD_MARSHAL_CONSTRUCT(type, name)                                           \
int32_t CPD_CONSTRUCT(cpd_marshal, name)(cpd_ctx_marshal *ctx, const type name) {   \
    return cpd_basic_data_marshal(ctx, (uint64_t) name);                            \
}
#define CPD_UNMARSHAL_CONSTRUCT(type, name)                                         \
int32_t CPD_CONSTRUCT(cpd_unmarshal, name)(cpd_ctx_unmarshal *ctx, type *name) {    \
    return cpd_basic_data_unmarshal(ctx, (uint64_t *) name);                        \
}

CPD_MARSHAL_CONSTRUCT(double, _double)
CPD_MARSHAL_CONSTRUCT(float , _float )

CPD_MARSHAL_CONSTRUCT(uint64_t, _uint64)
CPD_MARSHAL_CONSTRUCT(uint32_t, _uint32)
CPD_MARSHAL_CONSTRUCT(uint16_t, _uint16)
CPD_MARSHAL_CONSTRUCT(uint8_t , _uint8 )

CPD_MARSHAL_CONSTRUCT(int64_t, _int64)
CPD_MARSHAL_CONSTRUCT(int32_t, _int32)
CPD_MARSHAL_CONSTRUCT(int16_t, _int16)
CPD_MARSHAL_CONSTRUCT(int8_t , _int8 )

CPD_UNMARSHAL_CONSTRUCT(double, _double)
CPD_UNMARSHAL_CONSTRUCT(float , _float )

CPD_UNMARSHAL_CONSTRUCT(uint64_t, _uint64)
CPD_UNMARSHAL_CONSTRUCT(uint32_t, _uint32)
CPD_UNMARSHAL_CONSTRUCT(uint16_t, _uint16)
CPD_UNMARSHAL_CONSTRUCT(uint8_t , _uint8 )

CPD_UNMARSHAL_CONSTRUCT(int64_t, _int64)
CPD_UNMARSHAL_CONSTRUCT(int32_t, _int32)
CPD_UNMARSHAL_CONSTRUCT(int16_t, _int16)
CPD_UNMARSHAL_CONSTRUCT(int8_t , _int8 )