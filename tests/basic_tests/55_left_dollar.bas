10 REM Test LEFT$ function - extract left portion of string
20 PRINT "LEFT$ tests:"
30 LET S$ = "HELLO"
40 PRINT LEFT$(S$, 1)
50 PRINT LEFT$(S$, 3)
60 PRINT LEFT$(S$, 5)
70 PRINT LEFT$("ABCDEF", 0)
80 PRINT LEFT$("TESTING", 4)
90 END
