100 REM Multi-statement line tests
110 REM Test 1: Simple assignments with colons
120 X=5: Y=10: Z=15
130 IF X=5 THEN PRINT "Test 1: PASS" ELSE PRINT "Test 1: FAIL"

200 REM Test 2: Assignment and PRINT on same line
210 A=100: PRINT A
220 PRINT ""

300 REM Test 3: FOR loop with PRINT and NEXT on same line
310 FOR I=1 TO 5: PRINT I;: NEXT I
320 PRINT ""

400 REM Test 4: Multiple FOR loops nested style
410 FOR J=1 TO 3: PRINT "J";: NEXT J
420 PRINT ""

500 REM Test 5: IF and assignment on same line
510 N=10: IF N>5 THEN M=1 ELSE M=0
520 IF M=1 THEN PRINT "Test 5: PASS" ELSE PRINT "Test 5: FAIL"

600 REM Test 6: String assignment and PRINT
610 S$="HELLO": T$="WORLD": PRINT S$; " "; T$

700 REM Test 7: FOR with statement after NEXT
710 FOR K=1 TO 2: PRINT K;: NEXT K: PRINT " (DONE)"

800 REM Test 8: Multiple assignments with PRINT
810 P=1: Q=2: R=3: PRINT P+Q+R

900 REM All multi-statement tests completed
910 END
RUN
