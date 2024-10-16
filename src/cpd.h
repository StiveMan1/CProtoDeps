#ifndef CPD_H
#define CPD_H

#include <stdint.h>

#define CPD_VERSION "0.0.1"

#define CPD_FLAG_ERR_NULLPTR   0x01
#define CPD_FLAG_ERR_ALLOC     0x02
#define CPD_FLAG_ERR_TYPE      0x03
#define CPD_FLAG_ERR_EOF       0x04
#define CPD_FLAG_SUCCESS       0x00

typedef struct cpd_ctx_marshal_st cpd_ctx_marshal;
typedef struct cpd_ctx_unmarshal_st cpd_ctx_unmarshal;

int32_t cpd_marshal_double(cpd_ctx_marshal *ctx, double _double);
int32_t cpd_marshal_float (cpd_ctx_marshal *ctx, float  _float );

int32_t cpd_marshal_uint64(cpd_ctx_marshal *ctx, uint64_t _uint64);
int32_t cpd_marshal_uint32(cpd_ctx_marshal *ctx, uint32_t _uint32);
int32_t cpd_marshal_uint16(cpd_ctx_marshal *ctx, uint16_t _uint16);
int32_t cpd_marshal_uint8 (cpd_ctx_marshal *ctx, uint8_t  _uint8 );

int32_t cpd_marshal_int64(cpd_ctx_marshal *ctx, int64_t _int64);
int32_t cpd_marshal_int32(cpd_ctx_marshal *ctx, int32_t _int32);
int32_t cpd_marshal_int16(cpd_ctx_marshal *ctx, int16_t _int16);
int32_t cpd_marshal_int8 (cpd_ctx_marshal *ctx, int8_t  _int8 );

int32_t cpd_unmarshal_double(cpd_ctx_unmarshal *ctx, double *_double);
int32_t cpd_unmarshal_float (cpd_ctx_unmarshal *ctx, float  *_float );

int32_t cpd_unmarshal_uint64(cpd_ctx_unmarshal *ctx, uint64_t *_uint64);
int32_t cpd_unmarshal_uint32(cpd_ctx_unmarshal *ctx, uint32_t *_uint32);
int32_t cpd_unmarshal_uint16(cpd_ctx_unmarshal *ctx, uint16_t *_uint16);
int32_t cpd_unmarshal_uint8 (cpd_ctx_unmarshal *ctx, uint8_t  *_uint8 );

int32_t cpd_unmarshal_int64(cpd_ctx_unmarshal *ctx, int64_t *_int64);
int32_t cpd_unmarshal_int32(cpd_ctx_unmarshal *ctx, int32_t *_int32);
int32_t cpd_unmarshal_int16(cpd_ctx_unmarshal *ctx, int16_t *_int16);
int32_t cpd_unmarshal_int8 (cpd_ctx_unmarshal *ctx, int8_t  *_int8 );

#endif //CPD_H
