#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t res0 = (challenge + (challenge >> 32) * 8) ^ 0x6ea7;
    uint32_t res1 = ((challenge >> 16) + 0x3ea6) ^ (challenge >> 32);
    uint32_t res2 = (res1 * 3 + challenge) ^ 0xcae1;
    values[0] = res2 ^ res0;

    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;

    values[0] = ((challenge >> 13) & 0x3f) ^ 0x1d;
    for (int i = 1; i < 6; i ++)
        values[i] = (values[i - 1] * 3 - i) ^ (((challenge >> i) & 0x1f) + 6);

    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    values[0] = challenge & 7;
    uint32_t val = (challenge >> 3) ^ (challenge >> 32);
    switch (values[0])
    {
        case 0: values[1] = val * 3 + 0x3250;            break;
        case 1: values[1] = (val ^ 0xda8a) + (val >> 1); break;
        case 2: values[1] = (val * 4) ^ 0xbeea;          break;
        case 3: values[1] = (val ^ 0x1816) + (val >> 2); break;
        case 4: values[1] = (val * 4) ^ 0xfaba;          break;
        case 5: values[1] = val * 5 + 0x25a8;            break;
        case 6: values[1] = val * 5 + 0x2dc6;            break;
        case 7: values[1] = val * 5 + 0x2f7ab;           break;
    }

    return write_answer(out, cap, values, count);
}

uint32_t p4_walk(uint32_t w0, uint32_t w1, uint32_t w2, uint32_t w3)
{
    uint32_t val = ((w2 - w1) >> 1) + w1;
    if (w0 == val) return (w0 + 0xc4) ^ w3;
    if (w0 > val) return ((p4_walk(w0, val + 1, w2, w3) * 5) ^ (val + w3)) ^ 0x1e;
    return ((p4_walk(w0, w1, val - 1, w3) * 5) ^ (val + w3)) ^ 0x7;
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    values[0] = p4_walk(challenge & 0x7f, 0, 0x7f, (challenge >> 32) ^ (challenge >> 7));
    return write_answer(out, cap, values, count);
}

uint8_t p5_array[] = {
    0x40,0x5c,0x40,0x00,0x00,0x00,0x00,0x00,0xb0,0x5b,0x40,0x00,0x00,0x00,0x00,0x00,
    0x20,0x5c,0x40,0x00,0x00,0x00,0x00,0x00,0xc0,0x5b,0x40,0x00,0x00,0x00,0x00,0x00,
    0xf0,0x5b,0x40,0x00,0x00,0x00,0x00,0x00,0x50,0x5c,0x40,0x00,0x00,0x00,0x00,0x00,
    0x00,0x5c,0x40,0x00,0x00,0x00,0x00,0x00,0xe0,0x5b,0x40,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7c,0x06,0x00,0x00,0x29,0x8b,0x00,0x00,
    0x20,0x5c,0x40,0x00,0x00,0x00,0x00,0x00,0x59,0x02,0x00,0x00,0x16,0xa6,0x00,0x00,
    0xf0,0x5b,0x40,0x00,0x00,0x00,0x00,0x00,0x44,0x09,0x00,0x00,0x35,0x1f,0x00,0x00,
    0x10,0x5c,0x40,0x00,0x00,0x00,0x00,0x00,0x49,0x08,0x00,0x00,0x0c,0x52,0x00,0x00,
    0x30,0x5c,0x40,0x00,0x00,0x00,0x00,0x00,0x34,0x07,0x00,0x00,0x57,0x13,0x00,0x00,
    0x50,0x5c,0x40,0x00,0x00,0x00,0x00,0x00,0x42,0x00,0x00,0x00,0x89,0xa1,0x00,0x00,
    0xe0,0x5b,0x40,0x00,0x00,0x00,0x00,0x00,0x5b,0x0a,0x00,0x00,0xec,0x51,0x00,0x00,
    0xa0,0x5b,0x40,0x00,0x00,0x00,0x00,0x00,0x5c,0x05,0x00,0x00,0x4b,0xb4,0x00,0x00,
    0xc0,0x5b,0x40,0x00,0x00,0x00,0x00,0x00,0x28,0x03,0x00,0x00,0x66,0x42,0x00,0x00,
    0xd0,0x5b,0x40,0x00,0x00,0x00,0x00,0x00,0x1a,0x0b,0x00,0x00,0x30,0xdd,0x00,0x00,
    0xb0,0x5b,0x40,0x00,0x00,0x00,0x00,0x00,0x4b,0x04,0x00,0x00,0x9b,0x80,0x00,0x00,
    0x00,0x5c,0x40,0x00,0x00,0x00,0x00,0x00,0x3f,0x01,0x00,0x00,0x24,0x09,0x00,0x00
};

int solve_phase_5(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;

    uint32_t res = 0;
    uint32_t ret = (challenge >> 6) ^ (challenge >> 32) ^ 0x60a0;

    uint8_t* p = p5_array + (challenge & 7) * 8;
    for (int cnt = ((challenge >> 3) & 3) + 1; cnt; cnt --)
    {
        p = p5_array + *(uint64_t*)p - 0x405b60;
        res = *(uint32_t*)(p + 8);
        ret = ((ret + *(uint32_t*)(p + 12)) * 5) ^ res;
    }

    values[0] = ret, values[1] = res;
    return write_answer(out, cap, values, count);
}

uint32_t p6_op(uint32_t x, uint32_t op)
{
    switch (op)
    {
        case 0: return x >> 5 | (x & 0x1f) << 27;
        case 1: return (x << 1 | x >> 31) ^ 0x3046;
        case 2: return (x + 0x5f12) ^ (x >> 5);
        case 3: return (x ^ 0x3b2d) + x * 4;
        case 4: return (x + 0x5f12) ^ (x >> 5);
        case 5: return ((x >> 2) ^ x) + 0xc053;
        case 6: return (x ^ 0x3b2d) + x * 4;
        case 7: return ((x >> 5) + 0xd6b3) ^ (x * 3);
    }
    return 0;
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;

    uint32_t x = (challenge >> 6) ^ (challenge >> 32);
    x = p6_op(x, challenge & 7) ^ 0xb379;
    x = p6_op(x, (challenge >> 3) & 7) + 0xd9dc;

    values[0] = x;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
    (void)challenge;
    (void)context;
    (void)out;
    (void)cap;
    // TODO（选做）: 根据 secret_gate、secret_copy 和 secret_phase 的汇编构造完整的第七行，写入 out 后返回 0。
    // 第七行不一定只有整数，可以直接用 snprintf 写入；还没完成时保持返回 1，测试会显示 not attempted。
    return 1;
}
