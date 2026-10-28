# DataLab 实验报告

林诗航 24300240228

### 运行结果截图


### 思路

#### Phase 1

由 `phase_01.S` 可知 `defuser` 仅输出 1 个 u32，然后需要阅读 `p1_compute.S`，有 3 个子函数，每个函数输入两个参数做简单计算，对汇编代码进行分析并注释：（`seed` 实际为 `challenge`，此处以及下文为了方便使用 `seed`）

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

### 使用的工具

Deepseek-V4.1-Flash-1017: 仅提问 x86 汇编语法，与 arm64 的对比（本人先前学过 arm64），没有将本 Lab 中的具体代码拿去提问模型。