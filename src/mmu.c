#include "rvemu.h"

/**
 * @brief  读取 elf phdr
 * @param  phdr: program header
 * @param  ehdr: elf header
 * @param  i: program header 的条目
 * @param  file: elf 文件指针
 */
static void load_phdr(elf64_phdr_t *phdr,elf64_ehdr_t *ehdr,i64 i,FILE *file)
{
    
    if(fseek(file,ehdr->e_phoff + ehdr->e_phentsize * i, SEEK_SET) != 0)
    {
        fatal("seek file failed!");
    }

    if(fread((void *)phdr , 1 , sizeof(elf64_phdr_t), file) != sizeof(elf64_phdr_t))
    {
        fatal("file too smail...");
    }
}

static int flags_to_mmap_prot(u32 flags)
{
    return (flags & PF_R ? PROT_READ : 0) | 
           (flags & PF_W ? PROT_WRITE : 0) |
           (flags & PF_X ? PROT_EXEC : 0);
}

static void mmu_load_segment(mmu_t *mmu ,elf64_phdr_t *phdr, int fd)
{
    //获取分配的一页内存大小
    int page_size = getpagesize(); // 4K = 4096 bytes 0x1000
    // 获取本段在 elf 文件中的偏移地址
    u64 offset = phdr->p_offset;
    // 获取本段内容的虚拟地址
    u64 vaddr = TO_HOST(phdr->p_paddr);
    // 对齐地址到一页内存的边界，向下对齐
    u64 aligned_vaddr = ROUNDDOWN(vaddr, page_size);
    
    // 获取映射的内存大小
    u64 filesz = phdr->p_filesz ;

    u64 memsz = phdr->p_memsz ;

    // 转成 mmap 的可读，可写，可执行的 flags
    int prot = flags_to_mmap_prot(phdr->p_flags);

    //内存映射，将 elf 的 segment 映射到虚拟地址空间中,以页对齐的方式映射
    u64 addr = (u64)mmap((void *)aligned_vaddr, filesz, prot, MAP_PRIVATE | MAP_FIXED,
                        fd, ROUNDDOWN(offset, page_size));

    // 判断内存映射是否成功
    assert(addr == aligned_vaddr);

    // 获取 bss 段的大小,如果 bss 段很小，这里都向上页对齐的话 remaining_bss 的值会一直是零
    u64 remaining_bss = ROUNDUP(memsz, page_size) - ROUNDUP(filesz , page_size);
    //u64 remaining_bss = memsz - filesz;

    if(remaining_bss > 0)
    {
        //匿名映射，感觉没有对 bss 段做处理
        u64 addr = (u64)mmap((void *)(aligned_vaddr + ROUNDUP(filesz, page_size)),
                            remaining_bss, prot, MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED, -1, 0);
        assert(addr == aligned_vaddr + ROUNDUP(filesz, page_size));
    }

    // 更新 mmu 结构体的值
    mmu->host_alloc = MAX(mmu->host_alloc, (aligned_vaddr + ROUNDUP(memsz, page_size)));

    mmu->base = mmu->alloc = TO_GUEST(mmu->host_alloc);

}

void mmu_load_elf(mmu_t *mmu, int fd)
{

    u8 buf[sizeof(elf64_ehdr_t)];

    //读取 elf 文件 ，以二进制只读模式打开
    FILE *file = fdopen(fd, "rb");
    
    //判断读取的elf文件的大小是否小于 elf header 的长度
    if(fread(buf, 1, sizeof(elf64_ehdr_t), file) != sizeof(elf64_ehdr_t))
    {
        fatal("file too smail...");
    }

    elf64_ehdr_t *ehdr = (elf64_ehdr_t *)buf;

    //elf 文件前四个字节为 0x7f 0x45 0x4c 0x46 1
    //printf("ELFMAG: %02x%02x%02x%02x\n", ELFMAG[0], ELFMAG[1], ELFMAG[2], ELFMAG[3]);
    //判断传入文件是否为 elf 格式的文件
    if (*(u32 *)ehdr != *(u32 *)ELFMAG)
    {
        fatal("bad elf file");
    }


    //判断传入文件是否为 riscv64 的
    if (ehdr->e_machine != EM_RISCV || ehdr->e_ident[EI_CLASS] != ELFCLASS64)
    {
        fatal("only riscv64 elf file is supported");
    }

    //读取 entry
    mmu->entry = (u64)ehdr->e_entry;

    // Program Header 解析
    elf64_phdr_t phdr;

    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        load_phdr(&phdr,ehdr,i,file);
        if(phdr.p_type == PT_LOAD)
        {
            mmu_load_segment(mmu , &phdr, fd);
        }
    }
    
    
}