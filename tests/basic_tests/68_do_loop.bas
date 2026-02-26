10 REM Test 1: DO WHILE
20 PRINT "Test 1: DO WHILE x < 3"
30 x = 0
40 DO WHILE x < 3
50   PRINT "x ="; x
60   x = x + 1
70 LOOP
80 PRINT "After DO WHILE"
90 PRINT ""

100 REM Test 2: DO LOOP UNTIL
110 PRINT "Test 2: DO LOOP UNTIL n >= 3"
120 n = 0
130 DO
140   PRINT "n ="; n
150   n = n + 1
160 LOOP UNTIL n >= 3
170 PRINT "After DO LOOP UNTIL"
180 PRINT ""

190 REM Test 3: DO LOOP with EXIT
200 PRINT "Test 3: DO LOOP with EXIT"
210 i = 0
220 DO
230   PRINT "i ="; i
240   i = i + 1
250   IF i = 2 THEN EXIT
260 LOOP
270 PRINT "After EXIT"
280 PRINT ""

290 REM Test 4: DO LOOP WHILE
300 PRINT "Test 4: DO LOOP WHILE j < 3"
310 j = 0
320 DO
330   PRINT "j ="; j
340   j = j + 1
350 LOOP WHILE j < 3
360 PRINT "After DO LOOP WHILE"
370 PRINT ""

380 REM Test 5: Nested DO..LOOP
390 PRINT "Test 5: Nested loops"
400 x = 0
410 DO WHILE x < 2
420   y = 0
430   DO WHILE y < 2
440     PRINT "x="; x; " y="; y
450     y = y + 1
460   LOOP
470   x = x + 1
480 LOOP
490 PRINT "After nested loops"
500 PRINT ""

510 PRINT "All tests completed!"
520 END
