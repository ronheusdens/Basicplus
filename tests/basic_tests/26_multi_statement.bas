100 REM Test 1: Simple assignments with colons
110 X=5: Y=10: Z=15
120 IF X=5 THEN PRINT "Test 1: PASS" ELSE PRINT "Test 1: FAIL"

200 REM Test 2: FOR loop with PRINT and NEXT on same line
210 FOR I=1 TO 5: PRINT I;: NEXT I
220 PRINT ""

300 REM Test 3: Multiple statements with assignment
310 A=100: B=200: C=A+B
320 IF C=300 THEN PRINT "Test 3: PASS" ELSE PRINT "Test 3: FAIL"

400 REM Test 4: GOTO in multi-statement line
410 X=1: GOTO 430
420 PRINT "Test 4: FAIL"
430 PRINT "Test 4: PASS"

500 REM Test 5: GOSUB in multi-statement line
510 X=0: GOSUB 540: PRINT "Test 5: PASS"
520 GOTO 600
540 X=100
550 RETURN

600 REM Test 6: DIM and array access on same line
610 DIM ARR(3): ARR(0)=99
620 IF ARR(0)=99 THEN PRINT "Test 6: PASS" ELSE PRINT "Test 6: FAIL"

700 REM Test 7: Multiple colons with math
710 P=2: Q=3: R=P*Q
720 IF R=6 THEN PRINT "Test 7: PASS" ELSE PRINT "Test 7: FAIL"

800 REM Test 8: String operations on same line
810 S$="HELLO": T$=" WORLD": U$=S$+T$
820 IF U$="HELLO WORLD" THEN PRINT "Test 8: PASS" ELSE PRINT "Test 8: FAIL"

900 REM All tests done
910 END
