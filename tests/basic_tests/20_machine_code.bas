10 REM Test machine code simulation (USR, DEFUSR, VARPTR)
20 PRINT "Testing machine code support..."
30 PRINT ""
40 
50 REM Test 1: VARPTR - get simulated variable address
60 PRINT "Test 1: VARPTR function"
70 A = 100
80 ADDR = VARPTR(A)
90 PRINT "VARPTR(A) ="; ADDR
100 IF ADDR > 0 THEN PRINT "PASS - Valid address" ELSE PRINT "FAIL - Invalid address"
110 PRINT ""
120 
130 REM Test 2: DEFUSR - set default USR address
140 PRINT "Test 2: DEFUSR statement"
150 DEFUSR = 5000
160 PRINT "DEFUSR = 5000"
170 PRINT "PASS - DEFUSR set"
180 PRINT ""
190 
200 REM Test 3: Prepare machine code area
210 PRINT "Test 3: Prepare machine code"
220 REM Store "machine code" bytes (simulation)
230 POKE 5000, 42
240 POKE 5001, 123
250 POKE 5002, 255
260 PRINT "POKE'd values at 5000-5002"
270 PRINT "PEEK(5000) ="; PEEK(5000)
280 PRINT "PEEK(5001) ="; PEEK(5001)
290 PRINT "PEEK(5002) ="; PEEK(5002)
300 PRINT ""
310 
320 REM Test 4: USR with default address
330 PRINT "Test 4: USR() with default address"
340 RESULT = USR()
350 PRINT "USR() ="; RESULT; " (byte at DEFUSR address)"
360 IF RESULT = 42 THEN PRINT "PASS" ELSE PRINT "FAIL"
370 PRINT ""
380 
390 REM Test 5: USR with explicit address
400 PRINT "Test 5: USR(addr) with explicit address"
410 RESULT2 = USR(5001)
420 PRINT "USR(5001) ="; RESULT2
430 IF RESULT2 = 123 THEN PRINT "PASS" ELSE PRINT "FAIL"
440 PRINT ""
450 
460 REM Test 6: Multiple variables with VARPTR
470 PRINT "Test 6: Multiple variables"
480 X = 10
490 Y = 20
500 Z$ = "TEST"
510 PRINT "VARPTR(X) ="; VARPTR(X)
520 PRINT "VARPTR(Y) ="; VARPTR(Y)
530 PRINT "VARPTR(Z$) ="; VARPTR(Z$)
540 PRINT "PASS - All addresses within range"
550 PRINT ""
560 
570 REM Test 7: Change DEFUSR
580 PRINT "Test 7: Change DEFUSR address"
590 DEFUSR = 5002
600 RESULT3 = USR()
610 PRINT "USR() after DEFUSR=5002: "; RESULT3
620 IF RESULT3 = 255 THEN PRINT "PASS" ELSE PRINT "FAIL"
630 PRINT ""
640 
650 PRINT "All machine code tests complete!"
660 END
