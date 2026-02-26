10 REM Test PRINT@ (cursor positioning)
20 PRINT "Testing PRINT@ cursor positioning..."
30 PRINT ""
40 
50 REM Test 1: Print at position 0 (top-left)
60 PRINT@ 0, "Test 1: Position 0"
70 PRINT ""
80 PRINT ""
90 
100 REM Test 2: Print at position 64 (start of row 1)
110 PRINT@ 64, "Test 2: Position 64 (row 1)"
120 PRINT ""
130 PRINT ""
140 
150 REM Test 3: Print at position 128 (start of row 2)
160 PRINT@ 128, "Test 3: Position 128 (row 2)"
170 PRINT ""
180 PRINT ""
190 
200 REM Test 4: Print at calculated position (row 3, col 10)
210 POS = 3 * 64 + 10
220 PRINT@ POS, "Test 4: Calculated position"
230 PRINT ""
240 PRINT ""
250 
260 REM Test 5: Multiple PRINT@ on same line
270 PRINT@ 256, "A"
280 PRINT@ 260, "B"
290 PRINT@ 264, "C"
300 PRINT ""
310 PRINT ""
320 
330 REM Test 6: Print@ with expression
340 X = 5
350 Y = 10
360 PRINT@ X * 64 + Y, "Test 6: Expression (5,10)"
370 PRINT ""
380 PRINT ""
390 
400 PRINT ""
410 PRINT "All PRINT@ tests complete!"
420 END
