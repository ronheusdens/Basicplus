10 REM Test WHILE with string comparison
20 S$ = "A"
30 WHILE S$ < "D"
40   PRINT S$
50   S$ = CHR$(ASC(S$) + 1)
60 WEND
70 END
