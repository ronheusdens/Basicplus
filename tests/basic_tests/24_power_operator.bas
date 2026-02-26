10 REM Test exponentiation operator (^)
20 PRINT "Testing power operator..."
30 PRINT ""
40 
50 REM Test 1: Basic powers
60 PRINT "2^3=";2^3
70 PRINT "3^2=";3^2
80 PRINT "5^4=";5^4
90 PRINT ""
100 
110 REM Test 2: Sixth powers
120 PRINT "2^6=";2^6
130 PRINT "3^6=";3^6
140 PRINT "4^6=";4^6
150 PRINT "5^6=";5^6
160 PRINT ""
170 
180 REM Test 3: Right associativity (2^3^2 = 2^9)
190 PRINT "2^3^2=";2^3^2
200 PRINT ""
210 
220 REM Test 4: Power in expressions
230 X = 3
240 PRINT "X^6=";X^6
250 PRINT "(X+1)^6=";(X+1)^6
260 PRINT ""
270 
280 REM Test 5: Power with decimals
290 PRINT "2.5^2=";2.5^2
300 PRINT "10^0.5=";10^0.5
310 PRINT ""
320 
330 REM Test 6: Order of operations
340 PRINT "2+3^2=";2+3^2
350 PRINT "2*3^2=";2*3^2
360 PRINT ""
370 
380 PRINT "All power tests complete!"
390 END
