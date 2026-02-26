10 REM Comprehensive IF/THEN/ELSE test
20 PRINT "Testing IF/THEN/ELSE..."
30 PRINT ""
40 
50 REM Test 1: Simple condition TRUE
60 A = 10
70 IF A = 10 THEN PRINT "Test 1: PASS" ELSE PRINT "Test 1: FAIL"
80 
90 REM Test 2: Simple condition FALSE
100 B = 20
110 IF B = 30 THEN PRINT "Test 2: FAIL" ELSE PRINT "Test 2: PASS"
120 
130 REM Test 3: Greater than (TRUE)
140 C = 50
150 IF C > 40 THEN PRINT "Test 3: PASS" ELSE PRINT "Test 3: FAIL"
160 
170 REM Test 4: Less than (FALSE)
180 D = 10
190 IF D < 5 THEN PRINT "Test 4: FAIL" ELSE PRINT "Test 4: PASS"
200 
210 REM Test 5: With string comparison
220 S$ = "HELLO"
230 IF S$ = "HELLO" THEN PRINT "Test 5: PASS" ELSE PRINT "Test 5: FAIL"
240 
250 REM Test 6: Nested IF with ELSE
260 X = 100
270 IF X > 50 THEN IF X > 90 THEN PRINT "Test 6: PASS" ELSE PRINT "Test 6: FAIL" ELSE PRINT "Test 6: FAIL"
280 
290 REM Test 7: IF/THEN/ELSE with GOTO (TRUE case)
300 Y = 42
310 IF Y = 42 THEN GOTO 330 ELSE GOTO 350
320 PRINT "Test 7: FAIL - Should not reach here"
330 PRINT "Test 7: PASS"
340 GOTO 360
350 PRINT "Test 7: FAIL - Went to ELSE"
360 
370 REM Test 8: IF/THEN/ELSE with GOTO (FALSE case)
380 Z = 100
390 IF Z = 200 THEN GOTO 410 ELSE GOTO 430
400 PRINT "Test 8: FAIL - Should not reach here"
410 PRINT "Test 8: FAIL - Went to THEN"
420 GOTO 440
430 PRINT "Test 8: PASS"
440 
450 REM Test 9: IF GOTO without THEN (TRUE case)
460 N = 42
470 IF N = 42 GOTO 490
480 PRINT "Test 9: FAIL - Should have jumped"
490 PRINT "Test 9: PASS"
500 
510 REM Test 10: IF GOTO without THEN (FALSE case)
520 M = 100
530 IF M = 200 GOTO 560
540 PRINT "Test 10: PASS - Did not jump"
550 GOTO 570
560 PRINT "Test 10: FAIL - Should not have jumped"
570 
580 REM Test 11: IF condition GOTO with range check
590 V = 150
600 IF V > 255 GOTO 630
610 PRINT "Test 11: PASS - In range"
620 GOTO 640
630 PRINT "Test 11: FAIL - Out of range"
640 
650 REM Test 12: Logical OR (FALSE case)
660 A = 100
670 IF A = 200 OR A = 300 GOTO 700
680 PRINT "Test 12: PASS - OR evaluated correctly"
690 GOTO 710
700 PRINT "Test 12: FAIL - OR condition incorrect"
710 
720 REM Test 13: Logical OR (TRUE case)
730 B = 250
740 IF B = 250 OR B = 350 GOTO 770
760 PRINT "Test 13: FAIL - Should have jumped"
770 PRINT "Test 13: PASS - OR with GOTO worked"
780 
790 REM Test 14: Logical AND (TRUE case)
800 C = 100
810 D = 200
820 IF C = 100 AND D = 200 THEN PRINT "Test 14: PASS - AND evaluated correctly" ELSE PRINT "Test 14: FAIL"
830 
840 REM Test 15: Logical AND (FALSE case)
850 E = 100
860 F = 999
870 IF E = 100 AND F = 200 THEN PRINT "Test 15: FAIL - AND should be false" ELSE PRINT "Test 15: PASS - AND short-circuit worked"
880 
890 PRINT ""
900 PRINT "All IF/THEN/ELSE tests complete!"
910 END
