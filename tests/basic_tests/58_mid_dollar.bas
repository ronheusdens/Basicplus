10 REM Test MID$ function - extract substring from middle
20 PRINT "MID$ tests:"
30 LET S$ = "HELLO"
40 PRINT MID$(S$, 1, 1)
50 PRINT MID$(S$, 2, 3)
60 PRINT MID$(S$, 3, 2)
70 PRINT MID$("ABCDEF", 2, 4)
80 PRINT MID$("TESTING", 1, 7)
90 PRINT MID$("ABC", 2)
100 END
