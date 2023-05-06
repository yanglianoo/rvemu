#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>


#include "types.h"
#include "elfdef.h"
#include "reg.h"
#define fatalf(fmt, ...) (fprintf(stderr, "fatal: %s:%d " fmt "\n", __FILE__, __LINE__, __VA_ARGS__), exit(1))
#define fatal(msg) fatalf("%s", msg)
#define unreachable() (fatal("unreachable"), __builtin_unreachable())

#define ROUNDDOWN(x, k) ((x) & -(k))
#define ROUNDUP(x, k)   (((x) + (k)-1) & -(k))
#define MIN(x, y)       ((y) > (x) ? (x) : (y))
#define MAX(x, y)       ((y) < (x) ? (x) : (y))

#define GUEST_MEMORY_OFFSET 0x088800000000ULL

#define TO_HOST(addr)  (addr + GUEST_MEMORY_OFFSET)
#define TO_GUEST(addr) (addr - GUEST_MEMORY_OFFSET)

//指令类型
enum insn_type_t
{
    insn_addi, // addi
    num_insns, // 指令数量
};
//risc-v 指令结构体
typedef struct 
{
    i8 rd;   //目标寄存器
    i8 rs1;  //源寄存器1
    i8 rs2;  //源寄存器2
    i32 imm; // 32位立即数
    enum insn_type_t type; //指令的类型
    bool rvc; //表示指令是否是压缩指令
    bool cont; //表示是否需要继续执行下一条指令，或者停止执行并返回一个原因
} insn_t;

/**
 * @brief  mmu.c
 */
typedef struct 
{
    u64 entry;
    u64 host_alloc;
    u64 alloc;
    u64 base;
} mmu_t;

void mmu_load_elf(mmu_t *, int );


/**
 * @brief  state.c
 */
//定义一些可能导致模拟执行停止的原因
enum exit_reason_t
{
    none,           //没用停止原因
    direct_branch,  //直接跳转指定， 如 jal beq 
    indirect_branch,//间接跳转指令， 如 jalr br
    ecall,          //系统调用命令，
};

typedef struct 
{
    enum exit_reason_t exit_reason; //退出原因枚举
    u64 gp_regs[32]; //risc-v 的32个通用寄存器
    u64 pc;          //risc-v 的 pc 寄存器
} state_t;


/**
 * @brief  machine.c
 */
typedef struct 
{
    state_t state;
    mmu_t mmu;
} machine_t;

void machine_load_program(machine_t *m, char *prog);
enum exit_reason_t machine_step(machine_t *m);

/**
 * @brief  interp.c
 */

void exec_block_interp(state_t *state);

/**
 * @brief  decode.c
 */
void insn_decode(insn_t *insn, u32 data);


