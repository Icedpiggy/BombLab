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
  402055:	mov    %rdi,%rax // seed
  402058:	mov    %rdi,%rdx // seed
  40205b:	mov    %edi,%r8d // u32(seed)
  40205e:	shr    $0x20,%rax // seed >> 32
  402062:	shr    $0x10,%rdx // seed >> 16
  402066:	mov    %eax,%r9d // seed >> 32
  402069:	mov    %eax,%esi // seed >> 32
  40206b:	mov    %rsp,%rbp
  40206e:	call   402020 <p1_step0>
  // input (seed, seed >> 32)
  // output res0 = u32((seed + (seed >> 32) * 8) ^ 0x6ea7)
  402073:	mov    %rdx,%rsi // seed >> 16
  402076:	mov    %r9d,%edi // seed >> 32
  402079:	mov    %eax,%ecx // res0
  40207b:	call   402030 <p1_step1>
  // input (seed >> 32, seed >> 16)
  // output res1 = u32(((seed >> 16) + 0x3ea6) ^ (seed >> 32))
  402080:	mov    %r8d,%esi // u32(seed)
  402083:	mov    %eax,%edi // res1
  402085:	call   402040 <p1_step2>
  // input (u32(seed), res1)
  // output res2 = u32((res1 * 3 + u32(seed)) ^ 0xcae1)

  40208a:	pop    %rbp
  40208b:	xor    %ecx,%eax
  // res = res2 ^ res0
  40208d:	ret
```

在 `defuser` 还原该逻辑即可。

#### Phase 2

由 `phase_02.S` 可知 `defuser` 输出 6 个 u32。对汇编代码进行分析并注释（隐去输入 `out` 的部分）：

```S
  // ebx = 1
  40215a:	mov    %r13,%rax // seed
  40215d:	mov    -0x40(%rbp),%edx // out[0]
  402160:	shr    $0xd,%rax
  402164:	and    $0x3f,%eax
  402167:	xor    $0x1d,%eax // res[0] = ((seed >> 13) & 0x3f) ^ 0x1d
  40216a:	cmp    %eax,%edx // check res[0] == out[0]
  40216c:	jne    402115 <phase_02+0x25> // return 0
  40216e:	mov    $0x1,%ecx // 1

  // for (int i = 1; i != 6; i ++) %rcx 为 i
  402173:	mov    %r13,%rax // seed
  402176:	lea    (%rdx,%rdx,2),%edx
  402179:	shr    %cl,%rax
  40217c:	sub    %ecx,%edx // edx = res[i-1] * 3 - i (此时 out[i-1] == res[i-1] 显然成立)
  40217e:	and    $0x1f,%eax
  402181:	add    $0x6,%eax // eax = ((seed >> i) & 0x1f) + 6
  402184:	xor    %edx,%eax // res[i] = edx ^ eax = (res[i-1] * 3 - i) ^ (((seed >> i) & 0x1f) + 6)
  402186:	mov    -0x40(%rbp,%rcx,4),%edx // out[i]
  40218a:	cmp    %eax,%edx // check eax == out[i]
  40218c:	jne    402115 <phase_02+0x25> // return 0

  40218e:	add    $0x1,%rcx
  402192:	cmp    $0x6,%rcx
  402196:	jne    402173 <phase_02+0x83>

  402198:	jmp    402117 <phase_02+0x27> // return ebx
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
  402254:	mov    %edi,%eax // i
  402256:	mov    %esi,%edi // val
  402258:	cmp    $0x7,%eax // check i <= 7
  40225b:	ja     4022a5 <p3_dispatch+0x55> // return 0
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

注意到 `notrack jmp *0x405820(,%rax,8)`，需使用 `objdump -s --start-address=0x405820 --stop-address=0x405860 bomb > p3_extra.out` 指令得到跳转地址

```
Contents of section .rodata:
 405820 78224000 00000000 80224000 00000000  x"@......"@.....
 405830 88224000 00000000 90224000 00000000  ."@......"@.....
 405840 98224000 00000000 a0224000 00000000  ."@......"@.....
 405850 68224000 00000000 70224000 00000000  h"@.....p"@.....
```

经比对，发现 `i = 0 ... 7` 对应 `4 5 1 3 7 0 2 6`，运行 `p3_case_i` 后直接返回到 `phase_03.S` 的 `0x40231e` 继续校验 `out[1]`，不经过 `0x4022a5`


### 使用的工具

Deepseek-V4.1-Flash-1017: 仅提问 x86 汇编语法，与 arm64 的对比（本人先前学过 arm64），没有将本 Lab 中的具体代码拿去提问模型。