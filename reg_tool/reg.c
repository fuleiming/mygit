#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>


#ifndef PAGE_SIZE
#define PAGE_SIZE (1 << 12)
#endif
int map_func(int fd, unsigned long phyAddr, void *vaddr, void* unmapAddr)
{
    int ret;
    off_t offset;
    int size;

    offset = 0x85800000;
    size = PAGE_SIZE;
    unmapAddr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    if (unmapAddr == (char *)-1) {
            fprintf(stderr, "mmap fail with errno:%d\n",errno);
            goto err;
    }
    vaddr = unmapAddr;
err:
    if (fd > 0)
            close(fd);
    return 0;
}
void pr_usage(char **argv)
{
printf("usage:\n" \
      "%s phyaddr\n" \
      "%s phyaddr 0xlen\n" \
      "%s phyaddr 32 0xdata\n" \
      "%s phyaddr 64 0xdata\n", argv[0], argv[0], argv[0], argv[0]);

}
unsigned int readValue(void *vaddr)
{
    unsigned int data;

    data = *(unsigned int *)((unsigned long)vaddr);
    
    return data;
}

void print_data(unsigned long phyAddr, void *vaddr, int len)
{
    int i;
    int N;

    if (len < 4)
        return ;
    if (len < 0x20)
    {
        printf("[%.8lx]: ", phyAddr);
        for (i = 0; i < len; i += 4)
        {
            printf("%.8x    ",*(unsigned int *)((unsigned long)vaddr + i));
        }
    }
    else
    {
        N = len / 32;
        for (i = 0; i < N; i++)
        {
            printf("[%.8lx]: %.8x    %.8x    %.8x    %.8x    %.8x    %.8x    %.8x    %.8x    ", 
                phyAddr + i * 32,
                *(unsigned int *)((unsigned long)vaddr + i * 32),
                *(unsigned int *)((unsigned long)vaddr + i * 32 + 4),
                *(unsigned int *)((unsigned long)vaddr + i * 32 + 8),
                *(unsigned int *)((unsigned long)vaddr + i * 32 + 12),
                *(unsigned int *)((unsigned long)vaddr + i * 32 + 16),
                *(unsigned int *)((unsigned long)vaddr + i * 32 + 20),
                *(unsigned int *)((unsigned long)vaddr + i * 32 + 24),
                *(unsigned int *)((unsigned long)vaddr + i * 32 + 28)
                );
        }
        len = len - N * 32;
        if (len == 0)
            return ;
        printf("[%.8lx]: ", phyAddr + N * 32);
        for (i = 0; i < len; i += 4)
            printf("%.8x    ",*(unsigned int *)((unsigned long)vaddr + N * 32 + i));
    }
}

void writeValue(void *vaddr, unsigned long bits, unsigned long data)
{
    if ((bits != 32) && (bits != 64))
    {
        printf("invalid para 'bits', should be 32 or 64\n");
        return ;
    }
    
    if (bits == 32)
        *(unsigned int *)vaddr = data;
    else
        *(unsigned long *)vaddr = data;
}

int main(int argc, char **argv)
{
    int fd;
    int val;
    void *vaddr;
    void *unmapAddr;
    unsigned long phyAddr;
    unsigned long data;
    unsigned long len;
    unsigned long bits;


    if ((argc < 2) || (argc > 4))
    {
        printf("invalid para\n");
        pr_usage(argv);
        return -1;
    }
    fd = open("/dev/mem", O_RDWR);
    if(fd == -1)
    {
	perror("open failed");
	return -1;
    }
    
    phyAddr = strtol(&argv[1][2],NULL,16);
    map_func(fd, phyAddr, vaddr, unmapAddr);
    switch (argc)
    {
    case 2:
        val = readValue(vaddr);
        printf("[%x] %x\n", phyAddr, val);
        break;
    case 3:
        len = strtol(&argv[2][2],NULL,16);
        print_data(vaddr, bits, len);
        break;

    case 4:
        bits = strtol(argv[2],NULL,10);
        data = strtol(&argv[3][2],NULL,16);
        writeValue(phyAddr, bits, data);
        break;

    default:
        return -1;
    }
    
    return 0;
}
