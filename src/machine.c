#include "rvemu.h"

void machine_load_program(machine_t *m, char *prog)
{

    //打开 elf 文件，以只读的方式
    int fd = open(prog, O_RDONLY);
    if(fd == -1)
    {
        fatal(strerror(errno));
    }

    
    mmu_load_elf(&m->mmu,fd);
    close(fd);

    m->state.pc = (u64)m->mmu.entry;
}