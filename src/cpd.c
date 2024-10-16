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

