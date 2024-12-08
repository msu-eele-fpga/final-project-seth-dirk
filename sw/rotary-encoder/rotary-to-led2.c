#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h> // for mmap
#include <fcntl.h> // for file open flags
#include <unistd.h> // for getting the page size
#include <ctype.h> // for argument parsing
#include <signal.h> // for terminating program with CTRL-C
#include <string.h> // for string copying
#include <stdarg.h> // Allow optional inputs on a function

// devmem function to write to registers
int devmem(uint32_t address, ...)
{
    // Variable argument list
    va_list args;
    va_start(args, address);

    // Check if a value argument was given for a write operation
    uint32_t value = 0;
    bool is_write = false;
    if(va_arg(args, void*) != NULL)
    {
        // Value argument was given
        value = va_arg(args, uint32_t);
        is_write = true;
    }
    va_end(args);

    //devmem components
    const size_t PAGE_SIZE = sysconf(_SC_PAGE_SIZE);
    const uint32_t ADDRESS = address;
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    uint32_t page_aligned_addr = ADDRESS & ~(PAGE_SIZE - 1);
    uint32_t *page_virtual_addr = (uint32_t *)mmap(NULL, PAGE_SIZE,
    PROT_READ | PROT_WRITE, MAP_SHARED, fd, page_aligned_addr);
    uint32_t offset_in_page = ADDRESS & (PAGE_SIZE - 1);
    volatile uint32_t *target_virtual_addr = page_virtual_addr + offset_in_page/sizeof(uint32_t*);

    // Write the value given to the address given for write operation
    // Return value read from address given for read operation
    if (is_write)
    {
        *target_virtual_addr = value;
        return 0;
    }
    else
    {
        return *target_virtual_addr;
    }
}

// Check for CTRL-C command from user
static volatile int keepRunning = 1;
void programRunner(int dummy)
{
        keepRunning= 0;
}

int main(int argc, char **argv)
{

    signal(SIGINT, programRunner);

    uint32_t encoder;
    uint32_t pattern;
    int c;

    while (keepRunning)
    {
        // Read rotary encoder register
        encoder = devmem(0xFF230000);
        // Set LED pattern based off of encoder value
        switch(encoder)
        {
            case 0x0:
                pattern = 0x80;
                break;
            case 0x1:
                pattern = 0xC0;
                break;
            case 0x2:
                pattern = 0xE0;
                break;
            case 0x3:
                pattern = 0xF0;
                break;
            case 0x4:
                pattern = 0xF8;
                break;
            case 0x5:
                pattern = 0xFC;
                break;
            case 0x6:
                pattern = 0xFE;
                break;
            case 0x7:
                pattern = 0xFF;
                break;
            default:
                pattern = 0x00;
                break;
        }
        //Write LED pattern based off of encoder register
            devmem(0xFF220000, pattern);
    }
    exit(0);
    return 0;
}