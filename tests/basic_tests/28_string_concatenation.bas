100 REM Comprehensive String Concatenation Tests
110 REM Test 1: Basic string concatenation
120 A$="HELLO"
130 B$=" WORLD"
140 C$=A$+B$
150 IF C$="HELLO WORLD" THEN PRINT "Test 1: PASS" ELSE PRINT "Test 1: FAIL"

200 REM Test 2: Multiple concatenations
210 X$="A"
220 Y$="B"
230 Z$="C"
240 R$=X$+Y$+Z$
250 IF R$="ABC" THEN PRINT "Test 2: PASS" ELSE PRINT "Test 2: FAIL"

300 REM Test 3: Concatenating with literals
310 NAME$="JOHN"
320 GREETING$="HELLO "+NAME$
330 IF GREETING$="HELLO JOHN" THEN PRINT "Test 3: PASS" ELSE PRINT "Test 3: FAIL"

400 REM Test 4: Empty string concatenation
410 E$=""
420 F$="TEXT"
430 G$=E$+F$
440 IF G$="TEXT" THEN PRINT "Test 4: PASS" ELSE PRINT "Test 4: FAIL"

500 REM Test 5: Concatenation with integers using STR$
510 NUM=42
520 MSG$="THE ANSWER IS "+STR$(NUM)
530 IF MSG$="THE ANSWER IS 42" THEN PRINT "Test 5: PASS" ELSE PRINT "Test 5: FAIL"

600 REM Test 6: Multiple concatenations in one expression
610 T1$="A"
620 T2$="B"
630 T3$="C"
640 MULTI$=T1$+T2$+T3$
650 IF MULTI$="ABC" THEN PRINT "Test 6: PASS" ELSE PRINT "Test 6: FAIL"

700 REM Test 7: Long string concatenation
710 S1$="THE "
720 S2$="QUICK "
730 S3$="BROWN "
740 S4$="FOX"
750 LONG$=S1$+S2$+S3$+S4$
760 IF LONG$="THE QUICK BROWN FOX" THEN PRINT "Test 7: PASS" ELSE PRINT "Test 7: FAIL"

800 REM Test 8: Concatenation in PRINT statement
810 FIRST$="RADIO"
820 SECOND$="SHACK"
830 COMBINED$=FIRST$+" "+SECOND$
840 IF COMBINED$="RADIO SHACK" THEN PRINT "Test 8: PASS" ELSE PRINT "Test 8: FAIL"

900 REM Test 9: Type mismatch error
910 ON ERROR GOTO 950
920 A$="X"+5
930 PRINT "Test 9: FAIL - Should have raised type mismatch"
940 GOTO 970
950 IF ERR=13 THEN PRINT "Test 9: PASS" ELSE PRINT "Test 9: FAIL - Wrong error code (got" ERR ")"
960 GOTO 970
970 REM Continue

1000 REM All tests completed
1010 END
RUN
