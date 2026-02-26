10 REM Test complex math with LOG, EXP, LN
20 PRINT "Complex MATH operations:"
30 LET X = 5
40 LET Y = LOG(X) + LOG(2)
50 PRINT "LOG(5) + LOG(2) =", Y
60 LET Z = EXP(Y)
70 PRINT "EXP(LOG(5) + LOG(2)) =", Z
80 REM Should be 10 (5*2)
90 LET W = LN(10) / LN(2)
100 PRINT "LN(10) / LN(2) =", W
110 REM This is log base 2 of 10
120 END
