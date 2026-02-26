10 REM Test STRING$ function - repeats a string N times
20 PRINT "STRING$ tests:"
30 PRINT STRING$(5, "*")
40 PRINT STRING$(3, "-")
50 PRINT STRING$(0, "X")
60 LET S$ = STRING$(4, "A")
70 PRINT S$
80 PRINT STRING$(5, "12")
90 END
