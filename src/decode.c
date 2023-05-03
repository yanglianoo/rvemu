#include "rvemu.h"

//取 data 最低两位
#define QUADRANT(data) (((data) >> 0) & 0x3)

void insn_decode(insn_t *insn, u32 data)
{
    u32 quadrant = QUADRANT(data);
    switch (quadrant)
    {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
    
    default:
        break;
    }
}