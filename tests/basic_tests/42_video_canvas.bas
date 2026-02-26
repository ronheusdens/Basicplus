10 REM Test 42: Video Memory Canvas Drawing
20 REM Draw shapes and patterns using POKE
30 REM
40 CLS
50 PRINT "VIDEO MEMORY CANVAS"
60 PRINT ""
70 REM
80 REM Draw a horizontal line at row 3 using POKE
90 REM Row 3, cols 10-40: write '-' (ASCII 45)
100 FOR C = 10 TO 40
110   LET ADDR = 15360 + 3 * 80 + C
120   POKE ADDR, 45
130 NEXT C
140 PRINT "Drew horizontal line at row 3"
150 REM
160 REM Draw a vertical line at row 5-15, col 5 using '|' (ASCII 124)
170 FOR R = 5 TO 15
180   LET ADDR = 15360 + R * 80 + 5
190   POKE ADDR, 124
200 NEXT R
210 PRINT "Drew vertical line at col 5"
220 PRINT ""
230 REM
240 REM Draw a box outline at row 7-12, cols 15-30
250 REM Top and bottom edges (ASCII 45 for -)
260 FOR C = 15 TO 30
270   REM Top edge
280   POKE 15360 + 7 * 80 + C, 45
290   REM Bottom edge
300   POKE 15360 + 12 * 80 + C, 45
310 NEXT C
320 REM
330 REM Left and right edges (ASCII 124 for |)
340 FOR R = 7 TO 12
350   REM Left edge
360   POKE 15360 + R * 80 + 15, 124
370   REM Right edge
380   POKE 15360 + R * 80 + 30, 124
390 NEXT R
400 PRINT "Drew box outline at rows 7-12, cols 15-30"
410 REM
420 REM Fill the box interior with asterisks
430 FOR C = 16 TO 29
440   FOR R = 8 TO 11
450     POKE 15360 + R * 80 + C, 42
460   NEXT R
470 NEXT C
480 PRINT "Filled box with asterisks"
490 PRINT ""
500 REM
510 REM Create a checkerboard pattern in bottom-right
520 PRINT "Creating checkerboard in bottom-right..."
530 FOR R = 18 TO 23
540   FOR C = 50 TO 79
550     IF (R + C) MOD 2 = 0 THEN POKE 15360 + R * 80 + C, 35 ELSE POKE 15360 + R * 80 + C, 46
560   NEXT C
561 NEXT R
570 PRINT ""
580 PRINT "Canvas complete!"
590 END