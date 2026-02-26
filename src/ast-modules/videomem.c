#include "videomem.h"
#include <string.h>
#include <stdio.h>

/* Static video memory buffer */
static unsigned char video_ram[VIDEO_SIZE];

void videomem_init(void)
{
    /* Fill entire video memory with spaces (0x20) */
    memset(video_ram, 0x20, VIDEO_SIZE);
}

int videomem_peek(uint32_t address)
{
    /* Validate address is within video memory range */
    if (address < VIDEO_BASE || address > VIDEO_END)
    {
        return -1; /* Illegal memory access */
    }

    /* Calculate offset within video_ram buffer */
    uint32_t offset = address - VIDEO_BASE;
    return (int)video_ram[offset];
}

int videomem_poke(uint32_t address, int value)
{
    /* Validate address is within video memory range */
    if (address < VIDEO_BASE || address > VIDEO_END)
    {
        return -1; /* Illegal memory access */
    }

    /* Validate value is in byte range */
    if (value < 0 || value > 255)
    {
        return -1; /* Invalid value */
    }

    /* Calculate offset within video_ram buffer */
    uint32_t offset = address - VIDEO_BASE;
    video_ram[offset] = (unsigned char)value;

    return 0; /* Success */
}

void videomem_write_char(int row, int col, unsigned char ch)
{
    /* Bounds check */
    if (row < 0 || row >= VIDEO_ROWS || col < 0 || col >= VIDEO_COLS)
    {
        return; /* Out of bounds, ignore */
    }

    /* Calculate offset: row * VIDEO_COLS + col */
    int offset = row * VIDEO_COLS + col;
    video_ram[offset] = ch;
}

char *videomem_get_buffer(void)
{
    return (char *)video_ram;
}

unsigned char videomem_get_char(int row, int col)
{
    if (row < 0 || row >= VIDEO_ROWS || col < 0 || col >= VIDEO_COLS)
    {
        return 0x20; /* Out of bounds, return space */
    }

    int offset = row * VIDEO_COLS + col;
    return video_ram[offset];
}

void videomem_scroll_up(void)
{
    /*
     * Shift all bytes up by one line (VIDEO_COLS bytes)
     * Example: bytes 80-159 move to 0-79, bytes 160-239 move to 80-159, etc.
     * Bottom line (rows 23) fills with spaces
     */

    /* Move rows 1-23 to rows 0-22 */
    memmove(video_ram, video_ram + VIDEO_COLS, VIDEO_COLS * (VIDEO_ROWS - 1));

    /* Fill bottom row (row 23, offset 1840-1919) with spaces */
    memset(video_ram + VIDEO_COLS * (VIDEO_ROWS - 1), 0x20, VIDEO_COLS);
}

void videomem_clear(void)
{
    memset(video_ram, 0x20, VIDEO_SIZE);
}

void videomem_print_debug(void)
{
    printf("=== VIDEO MEMORY (0x3C00-0x437F) ===\n");
    for (int row = 0; row < VIDEO_ROWS; row++)
    {
        printf("Row %2d: ", row);
        for (int col = 0; col < VIDEO_COLS; col++)
        {
            unsigned char ch = videomem_get_char(row, col);
            if (ch >= 32 && ch < 127)
            {
                printf("%c", ch);
            }
            else if (ch < 32)
            {
                printf("."); /* Control character */
            }
            else
            {
                printf("[%02X]", ch); /* Show hex for extended graphics */
            }
        }
        printf("\n");
    }
}
