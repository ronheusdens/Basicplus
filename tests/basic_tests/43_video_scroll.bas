10 REM Test 43: Video Memory Scrolling
20 REM Demonstrate TRS-80 style scrolling and video memory persistence
30 REM
40 CLS
50 PRINT "VIDEO MEMORY SCROLLING TEST"
60 PRINT ""
70 REM
80 REM Write a message to video memory before scrolling
90 REM This message will be at the top of video memory
100 PRINT "Writing marker text..."
110 LET ADDR = 15360 + 1 * 80 + 20
120 FOR I = 0 TO 9
130   POKE ADDR + I, 65 + I
140 NEXT I
150 PRINT "Wrote ABCDEFGHIJ at row 1, col 20"
160 PRINT ""
170 REM
180 REM Now print enough lines to cause scrolling
190 PRINT "Printing lines to cause scrolling..."
200 FOR L = 1 TO 15
210   PRINT "Line "; L; " of the scroll test"
220 NEXT L
230 PRINT ""
240 PRINT "Screen has scrolled. Check if marker text"
250 PRINT "has moved up in video memory."
260 PRINT ""
270 REM
280 REM Attempt to PEEK original position (should be different after scroll)
290 PRINT "Checking video memory after scroll:"
290 LET V1 = PEEK(15360 + 1 * 80 + 20)
300 PRINT "PEEK at original position = "; V1
310 PRINT ""
320 PRINT "Test complete!"
330 END
