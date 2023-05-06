/**
 * @File Name: decode.c
 * @brief  解析 risc-v 的指令
 * @Author : Timer email:330070781@qq.com
 * @Version : 1.0
 * @Creat Date : 2023-05-06
 * 
 */
#include "rvemu.h"

//取 data 最低两位
#define QUADRANT(data) (((data) >> 0) & 0x3) // 0b11

void insn_decode(insn_t *insn, u32 data)
{
    //取指令最低两位
    u32 quadrant = QUADRANT(data);
    //判断指令是否为 压缩指令 rvc 
    switch (quadrant)
    {
    case 0x0: fatal("unimplemented");
    case 0x1: fatal("unimplemented");
    case 0x2: fatal("unimplemented");
    case 0x3: fatal("unimplemented");
    
    default:unreachable();
        break;
    }
}