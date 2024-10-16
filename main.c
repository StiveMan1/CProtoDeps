#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "src/cpd.h"

const char *str =  "\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4\1\2\3\4";

int32_t marshal_test1(void *a, cpd_ctx_marshal *ctx) {
    int32_t res = CPD_FLAG_SUCCESS;

    if ((res = cpd_marshal_str(ctx, str, strlen(str)))) return res;
    if ((res = cpd_marshal_str(ctx, str, strlen(str) / 2))) return res;
    if ((res = cpd_marshal_uint32(ctx, 123))) return res;
    if ((res = cpd_marshal_uint32(ctx, 123213))) return res;
    // if ((res = cpd_marshal(a, marshal_test1, ctx))) return res;

    return res;
}
int32_t unmarshal_test1(void *_, cpd_ctx_unmarshal *ctx) {
    char buf[strlen(str)];
    uint64_t res_size;
    int32_t res = CPD_FLAG_SUCCESS;

    if ((res = cpd_unmarshal_str(ctx, buf, strlen(str), &res_size))) return res;
    if ((res = cpd_unmarshal_str(ctx, buf, strlen(str)/2, &res_size))) return res;
    if ((res = cpd_unmarshal_uint64(ctx, &res_size))) return res;
    if ((res = cpd_unmarshal_uint64(ctx, &res_size))) return res;
    // if (cpd_unmarshal(NULL, NULL, unmarshal_test1, NULL, ctx)) return res;

    return res;
}



int main(void) {
    cpd_ctx_marshal *mctx = cpd_marshal_new();
    cpd_ctx_unmarshal *uctx = cpd_unmarshal_new();
    printf("%d", cpd_marshal(NULL, marshal_test1, mctx));

    cpd_ctx_marshal_free(mctx);
    cpd_ctx_unmarshal_free(uctx);

    // printf("%zu\n", mctx.size);
    // cpd_ctx_unmarshal *uctx = {(cpd_obj_u *)mctx.first, NULL, &tree_u};
    // uctx.first->_content += 2;
    // uctx.first->_size -= 2;
    // for (uint64_t i = 0; i < mctx.last->_size; ++i) printf("%.2x ", mctx.last->_content[i]);
    // printf("\n");
    // printf("\n");
    // printf("%d %d", cpd_unmarshal(NULL, NULL, unmarshal_test1, NULL, &uctx), SRPC_FLAG_SUCCESS);
}