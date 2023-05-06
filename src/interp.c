/**
 * @File Name: interp.c
 * @brief  解释型模拟器
 * @Author : Timer email:330070781@qq.com
 * @Version : 1.0
 * @Creat Date : 2023-05-06
 * 
 */
#include "rvemu.h"

// 定义一个函数指针的类型
typedef void(func_t)(state_t *, insn_t *);

//定义一个函数指针数组
static func_t *funcs[] = {};

/**
 * @brief  以解释的方式执行一个block
 * @param  state: 
 */
void exec_block_interp(state_t *state)
{
    static insn_t insn = {0};
    while (true)
    {
        //获取指令
        u32 data = *(u32 *)TO_HOST(state->pc);
        //解码指令
        insn_decode(&insn, data);
        //执行指令
        funcs[insn.type](state, &insn);
        //强制设置 zero 寄存器为 0
        state->gp_regs[zero] = 0;
        // 如果不继续执行，就break
        if(insn.cont) break;
        // pc 寄存器 加加，如果是压缩指令 步进2 不是压缩指令 步进 4
        state->pc += insn.rvc ? 2 : 4;
    }
    
}