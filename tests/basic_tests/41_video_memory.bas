10 REM Test 41: Video Memory (PEEK/POKE)
20 REM Demonstrates video memory at 0x3C00-0x437F (80x24 chars)
30 REM
40 CLS
50 PRINT "VIDEO MEMORY TEST"
60 PRINT "=================="
70 PRINT ""
80 REM
90 REM Test 1: Write directly to video memory using POKE
100 REM Address = 0x3C00 + (row * 80) + col
110 REM Top-left corner (row 0, col 0) = 0x3C00
120 PRINT "TEST 1: Direct POKE to video memory"
130 POKE 15360, 65
140 PRINT "POKE 15360, 65  (writes 'A' at top-left)"
150 PRINT ""
160 REM 
170 REM Test 2: Check the POKE worked with PEEK
180 PRINT "TEST 2: PEEK to read back"
190 LET V = PEEK(15360)
200 PRINT "PEEK(15360) = "; V; " (should be 65 for 'A')"
210 PRINT ""
220 REM
230 REM Test 3: Write a pattern across a row
240 PRINT "TEST 3: Pattern write"
250 REM Write "HELLO" at row 5, starting at col 0
260 REM Row 5 starts at: 0x3C00 + 5*80 = 0x3C00 + 400 = 15760 (0x3D90)
270 FOR I = 0 TO 4
280   LET ADDR = 15760 + I
290   LET CHARS = 65 + I
300   POKE ADDR, CHARS
310 NEXT I
320 PRINT "Wrote ABCDE at row 5 (may not show visually)"
330 PRINT ""
340 REM
350 REM Test 4: Write ASCII values to create text
360 PRINT "TEST 4: Using POKE for display"
370 PRINT ""
380 REM Row 8, start at col 10
390 LET BASE = 15360 + 8 * 80 + 10
400 PRINT "Writing 'HI' at row 8, col 10:"
410 POKE BASE, 72
420 POKE BASE + 1, 73
430 PRINT ""
440 REM
450 REM Test 5: Extended characters (>127)
460 PRINT "TEST 5: Extended graphics (values > 127)"
470 POKE 15360 + 12 * 80, 240
480 PRINT "POKE row 12, col 0 with value 240"
490 PRINT ""
500 REM
510 REM Test 6: Boundary check (should error)
520 PRINT "TEST 6: Out of bounds test"
530 POKE 20000, 65
540 IF ERR = 200 THEN PRINT "Error 200: Illegal memory access (expected)"
550 PRINT ""
560 END
