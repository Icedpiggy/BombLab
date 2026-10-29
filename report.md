# DataLab 实验报告

林诗航 24300240228

### 运行结果截图


### 思路

#### Phase 1

由 `phase_01.S` 可知 `defuser` 仅输出 1 个 u32，然后需要阅读 `p1_compute.S`，有 3 个子函数，每个函数输入 2 个参数做简单计算，对汇编代码进行分析并注释：（`seed` 实际为 `challenge`，此处以及下文为了方便使用 `seed`）

```S
0000000000402050 <p1_compute>:
  402050:	endbr64
  402054:	push   %rbp
  402055:	mov    %rdi,%rax # seed
  402058:	mov    %rdi,%rdx # seed
  40205b:	mov    %edi,%r8d # u32(seed)
  40205e:	shr    $0x20,%rax # seed >> 32
  402062:	shr    $0x10,%rdx # seed >> 16
  402066:	mov    %eax,%r9d # seed >> 32
  402069:	mov    %eax,%esi # seed >> 32
  40206b:	mov    %rsp,%rbp
  40206e:	call   402020 <p1_step0>
  # input (seed, seed >> 32)
  # output res0 = u32((seed + (seed >> 32) * 8) ^ 0x6ea7)
  402073:	mov    %rdx,%rsi # seed >> 16
  402076:	mov    %r9d,%edi # seed >> 32
  402079:	mov    %eax,%ecx # res0
  40207b:	call   402030 <p1_step1>
  # input (seed >> 32, seed >> 16)
  # output res1 = u32(((seed >> 16) + 0x3ea6) ^ (seed >> 32))
  402080:	mov    %r8d,%esi # u32(seed)
  402083:	mov    %eax,%edi # res1
  402085:	call   402040 <p1_step2>
  # input (u32(seed), res1)
  # output res2 = u32((res1 * 3 + u32(seed)) ^ 0xcae1)

  40208a:	pop    %rbp
  40208b:	xor    %ecx,%eax
  # res = res2 ^ res0
  40208d:	ret
```

在 `defuser` 还原该逻辑即可。

#### Phase 2

由 `phase_02.S` 可知 `defuser` 输出 6 个 u32。对汇编代码进行分析并注释（隐去输入 `out` 的部分）：

```S
  # ebx = 1
  40215a:	mov    %r13,%rax # seed
  40215d:	mov    -0x40(%rbp),%edx # out[0]
  402160:	shr    $0xd,%rax
  402164:	and    $0x3f,%eax
  402167:	xor    $0x1d,%eax # res[0] = ((seed >> 13) & 0x3f) ^ 0x1d
  40216a:	cmp    %eax,%edx # check res[0] == out[0]
  40216c:	jne    402115 <phase_02+0x25> # return 0
  40216e:	mov    $0x1,%ecx # 1

  # for (int i = 1; i != 6; i ++) %rcx 为 i
  402173:	mov    %r13,%rax # seed
  402176:	lea    (%rdx,%rdx,2),%edx
  402179:	shr    %cl,%rax
  40217c:	sub    %ecx,%edx # edx = res[i-1] * 3 - i (此时 out[i-1] == res[i-1] 显然成立)
  40217e:	and    $0x1f,%eax
  402181:	add    $0x6,%eax # eax = ((seed >> i) & 0x1f) + 6
  402184:	xor    %edx,%eax # res[i] = edx ^ eax = (res[i-1] * 3 - i) ^ (((seed >> i) & 0x1f) + 6)
  402186:	mov    -0x40(%rbp,%rcx,4),%edx # out[i]
  40218a:	cmp    %eax,%edx # check eax == out[i]
  40218c:	jne    402115 <phase_02+0x25> # return 0

  40218e:	add    $0x1,%rcx
  402192:	cmp    $0x6,%rcx
  402196:	jne    402173 <phase_02+0x83>

  402198:	jmp    402117 <phase_02+0x27> # return ebx
```

得到递推：
```
res[0] = ((seed >> 13) & 0x3f) ^ 0x1d
res[i] = (res[i-1] * 3 - i) ^ (((seed >> i) & 0x1f) + 6)
```
在 `defuser` 还原该逻辑即可。

#### Phase 3

由 `phase_03.S` 可知 `defuser` 仅输出 2 个 u32，检验 `out[0] == (seed & 7) && out[1] == p3_dispatch(seed & 7, u32((seed >> 3) ^ (seed >> 32)))`

阅读 `p3_dispatch.S`，有 8 个子函数，每个函数输入 1 个参数 `val` 做简单计算，对汇编代码进行分析并注释（隐去 nop）：

```S
0000000000402250 <p3_dispatch>:
  402250:	endbr64
  402254:	mov    %edi,%eax # i
  402256:	mov    %esi,%edi # val
  402258:	cmp    $0x7,%eax # check i <= 7
  40225b:	ja     4022a5 <p3_dispatch+0x55> # return 0
  40225d:	notrack jmp *0x405820(,%rax,8)
  402268:	jmp    4021d0 <p3_case_2>
  402270:	jmp    402220 <p3_case_6>
  402278:	jmp    402200 <p3_case_4>
  402280:	jmp    402210 <p3_case_5>
  402288:	jmp    4021b0 <p3_case_1>
  402290:	jmp    4021e0 <p3_case_3>
  402298:	jmp    402230 <p3_case_7>
  4022a0:	jmp    4021a0 <p3_case_0>
  4022a5:	xor    %eax,%eax
  4022a7:	ret

<p3_case_0> val * 5 + 0x25a8
<p3_case_1> (val * 4) ^ 0xbeea
<p3_case_2> val * 5 + 0x2dc6
<p3_case_3> (val ^ 0x1816) + (val >> 2)
<p3_case_4> val * 3 + 0x3250
<p3_case_5> (val ^ 0xda8a) + (val >> 1)
<p3_case_6> val * 5 + 0x2f7ab
<p3_case_7> (val * 4) ^ 0xfaba
```

注意到 `notrack jmp *0x405820(,%rax,8)`，需使用 `objdump -s --start-address=0x405820 --stop-address=0x405860 bomb > temp/p3_extra.out` 指令得到跳转地址

```
Contents of section .rodata:
 405820 78224000 00000000 80224000 00000000
 405830 88224000 00000000 90224000 00000000
 405840 98224000 00000000 a0224000 00000000
 405850 68224000 00000000 70224000 00000000
```

经比对，发现 `i = 0 ... 7` 对应 `4 5 1 3 7 0 2 6`，运行 `p3_case_i` 后直接返回到 `phase_03.S` 的 `0x40231e` 继续校验 `out[1]`，不经过 `0x4022a5`

#### Phase 4

由 `phase_04.S` 可知 `defuser` 仅输出 1 个 u32，与 `p4_walk(seed & 0x7f, 0, 0x7f, u32((seed >> 32) ^ (seed >> 7)))` 的结果做比较

阅读 `p4_walk.S`，记 4 个 u32 参数为 `w0 ... w3`，对汇编代码进行分析并注释（隐去 nop）：

```S
0000000000402330 <p4_walk>:
  402330:	endbr64
  402334:	mov    %edi,%eax # w0
  402336:	mov    %edx,%edi
  402338:	sub    %esi,%edi
  40233a:	shr    $1,%edi
  40233c:	add    %esi,%edi # val = ((w2 - w1) >> 1) + w1
  40233e:	cmp    %eax,%edi # check w0 == val
  402340:	je     402390 <p4_walk+0x60>

  402342:	push   %rbp
  402343:	mov    %rsp,%rbp
  402346:	push   %rbx
  402347:	lea    (%rdi,%rcx,1),%ebx # val + w3
  40234a:	sub    $0x8,%rsp

  40234e:	cmp    %edi,%eax # check w0 < val
  402350:	jb     402370 <p4_walk+0x40>

  # w0 > val
  402352:	lea    0x1(%rdi),%esi # val + 1
  402355:	mov    %eax,%edi # w0
  402357:	call   402330 <p4_walk> # input (w0, val + 1, w2, w3)
  40235c:	lea    (%rax,%rax,4),%eax
  40235f:	xor    %ebx,%eax
  402361:	mov    -0x8(%rbp),%rbx
  402365:	leave
  402366:	xor    $0x1e,%eax # res = ((res * 5) ^ (val + w3)) ^ 0x1e
  402369:	ret

  # w0 < val
  402370:	lea    -0x1(%rdi),%edx # val - 1
  402373:	mov    %eax,%edi # w0
  402375:	call   402330 <p4_walk> # input (w0, w1, val - 1, w3)
  40237a:	lea    (%rax,%rax,4),%eax
  40237d:	xor    %ebx,%eax
  40237f:	mov    -0x8(%rbp),%rbx
  402383:	leave
  402384:	xor    $0x7,%eax # res = ((res * 5) ^ (val + w3)) ^ 0x7
  402387:	ret

  # w0 == val
  402390:	lea    0xc4(%rdi),%eax
  402396:	xor    %ecx,%eax # res = (w0 + 0xc4) ^ w3
  402398:	ret
```

还原为 C 代码：

```c
uint32_t p4_walk(uint32_t w0, uint32_t w1, uint32_t w2, uint32_t w3)
{
    uint32_t val = ((w2 - w1) >> 1) + w1;
    if (w0 == val) return (w0 + 0xc4) ^ w3;
    if (w0 > val) return ((p4_walk(w0, val + 1, w2, w3) * 5) ^ (val + w3)) ^ 0x1e;
    return ((p4_walk(w0, w1, val - 1, w3) * 5) ^ (val + w3)) ^ 0x7;
}
```

事实上它就是一个在 `[0,0x7f]` 边做二分查找边乘权值的函数，二分次数极少。

#### Phase 5

由 `phase_05.S` 可知 `defuser` 仅输出 2 个 u32，在运行 `ret = p5_traverse(seed, &res)` 后，校验 `out[0] == ret && out[1] == res`

阅读 `p5_traverse.S`，对汇编代码进行分析并注释：

```S
0000000000402420 <p5_traverse>:
  402420:	endbr64
  402424:	mov    %rdi,%rax
  402427:	mov    %rdi,%rcx
  40242a:	mov    %rsi,%r8
  40242d:	and    $0x7,%eax
  402430:	shr    $0x3,%rcx
  402434:	mov    0x405b60(,%rax,8),%rdx
  # p = (seed & 7) * 8 + 0x405b60 使用 p5_array 后改为 p = p5_array + (seed & 7) * 8
  # 注意是 mov 而不是 lea，因此需多跳一步
  40243c:	mov    %rdi,%rax
  40243f:	shr    $0x20,%rdi
  402443:	shr    $0x6,%rax
  402447:	xor    %edi,%eax
  402449:	mov    0x8(%rdx),%esi
  40244c:	mov    %ecx,%edi
  40244e:	xor    $0x60a0,%eax # ret = u32((seed >> 6) ^ (seed >> 32) ^ 0x60a0)
  402453:	add    0xc(%rdx),%eax
  402456:	lea    (%rax,%rax,4),%eax
  402459:	mov    %esi,(%r8) # res = *(p + 0x8)
  40245c:	xor    %esi,%eax # ret = ((ret + *(p + 0xc)) * 5) ^ *(p + 0x8)
  40245e:	and    $0x3,%edi # check ((seed >> 3) & 3) == 0
  402461:	je     402480 <p5_traverse+0x60>
  402463:	mov    $0x1,%ecx # i = 1

  402468:	mov    (%rdx),%rdx # p = *p 使用 p5_array 后改为 p = p5_array + *p - 0x405b60
  40246b:	add    $0x1,%ecx # i ++
  40246e:	mov    0x8(%rdx),%esi
  402471:	add    0xc(%rdx),%eax
  402474:	lea    (%rax,%rax,4),%eax
  402477:	mov    %esi,(%r8) # res = *(p + 0x8)
  40247a:	xor    %esi,%eax # ret = ((ret + *(p + 0xc)) * 5) ^ *(p + 0x8)
  40247c:	cmp    %ecx,%edi # check i <= ((seed >> 3) & 3)
  40247e:	jae    402468 <p5_traverse+0x48>

  402480:	ret
```

即迭代几次，由于存在 `p = *p` 这种语句，手动即可（当然也可以写代码）探索 p 可能到达的位置，需要将 `0x405b60` 后面的一系列数据导出：

最终需要：`objdump -s --start-address=0x405b60 --stop-address=0x405c60 bomb > temp/p5_extra.out`

```
Contents of section .rodata:
 405b60 405c4000 00000000 b05b4000 00000000
 405b70 205c4000 00000000 c05b4000 00000000
 405b80 f05b4000 00000000 505c4000 00000000
 405b90 005c4000 00000000 e05b4000 00000000
 405ba0 00000000 00000000 7c060000 298b0000
 405bb0 205c4000 00000000 59020000 16a60000
 405bc0 f05b4000 00000000 44090000 351f0000
 405bd0 105c4000 00000000 49080000 0c520000
 405be0 305c4000 00000000 34070000 57130000
 405bf0 505c4000 00000000 42000000 89a10000
 405c00 e05b4000 00000000 5b0a0000 ec510000
 405c10 a05b4000 00000000 5c050000 4bb40000
 405c20 c05b4000 00000000 28030000 66420000
 405c30 d05b4000 00000000 1a0b0000 30dd0000
 405c40 b05b4000 00000000 4b040000 9b800000
 405c50 005c4000 00000000 3f010000 24090000
```

注意到除了 `405ba0`，可达的每一个地址的后继地址都以 0 结尾，且 `405b98 -> 405be0 -> 405c30 -> 405bd0 -> 405c10 -> 405ba0` 超过 4 次，无需考虑不合法的地址。

把它制为 `uint8_t` 数组 `p5_array`，以后地址应加上 `p5_array` 减去 `0x405b60`，然后使用 `uint8_t *p`，`p = p5_array + *(uint64_t*)p - 0x405b60` 以及 `res = *(uint32_t*)(p + 8)` 等，还原该逻辑即可。

#### Phase 6

由 `phase_06.S` 可知 `defuser` 仅输出 1 个 u32，与 `p6_dispatch(seed)` 的结果做比较

阅读 `p6_dispatch.S`，对汇编代码进行分析并注释：

```S
0000000000402580 <p6_dispatch>:
  402580:	endbr64
  402584:	push   %rbp
  402585:	mov    %rsp,%rbp
  402588:	push   %rbx
  402589:	mov    %rdi,%rbx # seed
  40258c:	shr    $0x6,%rdi # seed >> 6
  402590:	mov    %rbx,%rax # seed
  402593:	shr    $0x20,%rax # seed >> 32
  402597:	xor    %eax,%edi # u32((seed >> 6) ^ (seed >> 32))
  402599:	mov    %rbx,%rax # seed
  40259c:	sub    $0x8,%rsp
  4025a0:	and    $0x38,%ebx # seed & 0x38
  4025a3:	and    $0x7,%eax # seed & 7
  4025a6:	call   *0x405e00(,%rax,8)
  # call *(0x405e00 + (seed & 7) * 8)
  # input u32((seed >> 6) ^ (seed >> 32))
  # output r1
  4025ad:	xor    $0xb379,%eax
  4025b2:	mov    %eax,%edi
  4025b4:	call   *0x405e00(%rbx)
  # call *(0x405e00 + (seed & 0x38))
  # input (r1 ^ 0xb379)
  # output r2
  4025ba:	mov    -0x8(%rbp),%rbx
  4025be:	leave
  4025bf:	add    $0xd9dc,%eax # res = r2 + 0xd9dc
  4025c4:	ret
```

函数地址的数组位于 `0x405e00 - 0x405e40`，导出：

`objdump -s --start-address=0x405e00 --stop-address=0x405e40 bomb > temp/p6_extra.out`

```
Contents of section .rodata:
 405e00 50254000 00000000 60254000 00000000
 405e10 70254000 00000000 40254000 00000000
 405e20 70254000 00000000 20254000 00000000
 405e30 40254000 00000000 00254000 00000000
```

8 个地址，6 个子函数，地址从低到高为 `p6_op_0 ... p6_op_5`，8 个地址对应 `3 4 5 2 5 1 2 0`，每个函数输入 1 个参数 `val` 做简单计算，对汇编代码进行分析得到：

```
402500 <p6_op_0>: ((x >> 5) + 0xd6b3) ^ (x * 3)
402520 <p6_op_1>: ((x >> 2) ^ x) + 0xc053
402540 <p6_op_2>: (x ^ 0x3b2d) + x * 4
402550 <p6_op_3>: x >> 5 | (x & 0x1f) << 27
402560 <p6_op_4>: (x << 1 | x >> 31) ^ 0x3046
402570 <p6_op_5>: (x + 0x5f12) ^ (x >> 5)
```

在 `defuser` 还原该逻辑即可。

#### Honor

### 使用的工具

Deepseek-V4.1-Flash-1017: 仅提问 x86 汇编语法，与 arm64 的对比（本人先前学过 arm64），没有将本 Lab 中的具体代码拿去提问模型。