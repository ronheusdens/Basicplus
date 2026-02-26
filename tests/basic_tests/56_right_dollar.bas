10 REM Test RIGHT$ function - extract right portion of string
20 PRINT "RIGHT$ tests:"
30 LET S$ = "HELLO"
40 PRINT RIGHT$(S$, 1)
50 PRINT RIGHT$(S$, 3)
60 PRINT RIGHT$(S$, 5)
70 PRINT RIGHT$("ABCDEF", 0)
80 PRINT RIGHT$("TESTING", 4)
90 END
