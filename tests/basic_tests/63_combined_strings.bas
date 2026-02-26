10 REM Test combined string functions
20 PRINT "Combined STRING$ tests:"
30 LET S$ = "HELLO WORLD"
40 PRINT LEFT$(S$, 5)
50 PRINT RIGHT$(S$, 5)
60 PRINT MID$(S$, 7, 5)
70 PRINT LEN(S$)
80 PRINT STRING$(3, LEFT$(S$, 1))
90 END
