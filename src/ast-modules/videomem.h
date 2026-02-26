#ifndef VIDEOMEM_H
#define VIDEOMEM_H

#include <stdint.h>

/*
 * Virtual Video Memory System
 * Emulates TRS-80 Level II video memory (80x24 character display)
 * Base address: 0x3C00 (15360 decimal)
 * Range: 0x3C00 - 0x437F (15360 - 17279)
 * Layout: Row N starts at VIDEO_BASE + (N * VIDEO_COLS)
 */

#define VIDEO_BASE  0x3C00   /* 15360 - authentic TRS-80 Level II */
#define VIDEO_COLS  80       /* Columns per row */
#define VIDEO_ROWS  24       /* Total rows */
#define VIDEO_SIZE  1920     /* VIDEO_COLS * VIDEO_ROWS */
#define VIDEO_END   (VIDEO_BASE + VIDEO_SIZE - 1)  /* 0x437F = 17279 */

/* Video memory control operations */

/**
 * Initialize video memory system
 * Fills the 1920-byte buffer with spaces (0x20)
 */
void videomem_init(void);

/**
 * Read a byte from video memory
 * @param address: 0x3C00 - 0x437F (15360 - 17279)
 * @return: Byte value (0-255) on success, or -1 on illegal access
 */
int videomem_peek(uint32_t address);

/**
 * Write a byte to video memory
 * @param address: 0x3C00 - 0x437F (15360 - 17279)
 * @param value: 0-255 (ASCII or extended graphics)
 * @return: 0 on success, -1 on illegal access
 */
int videomem_poke(uint32_t address, int value);

/**
 * Write a character at screen position
 * Updates video memory at the cursor position
 * @param row: 0-23
 * @param col: 0-79
 * @param ch: Character to write
 */
void videomem_write_char(int row, int col, unsigned char ch);

/**
 * Get pointer to video memory buffer
 * @return: Pointer to 1920-byte video RAM
 */
char* videomem_get_buffer(void);

/**
 * Get character at position
 * @param row: 0-23
 * @param col: 0-79
 * @return: Character at position
 */
unsigned char videomem_get_char(int row, int col);

/**
 * Scroll video memory up one line
 * Shifts all bytes up by VIDEO_COLS, fills bottom line with spaces
 * TRS-80 authentic behavior
 */
void videomem_scroll_up(void);

/**
 * Clear video memory
 * Fills entire buffer with spaces (0x20), no visual updates
 * Note: CLS command clears display visually only; this fills RAM
 */
void videomem_clear(void);

/**
 * Print video memory contents for debugging
 */
void videomem_print_debug(void);

#endif /* VIDEOMEM_H */
