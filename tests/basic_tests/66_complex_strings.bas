10 REM Test complex string operations
20 PRINT "Complex STRING operations:"
30 LET PHRASE$ = "The quick brown fox"
40 LET FIRST$ = LEFT$(PHRASE$, 3)
50 LET LAST$ = RIGHT$(PHRASE$, 3)
60 LET MIDDLE$ = MID$(PHRASE$, 5, 5)
70 PRINT "First:", FIRST$
80 PRINT "Middle:", MIDDLE$
90 PRINT "Last:", LAST$
100 PRINT "Repeated first:", STRING$(3, FIRST$)
110 PRINT "Total length:", LEN(PHRASE$)
120 END
