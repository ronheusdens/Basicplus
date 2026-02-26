10 REM Test WHILE with array condition
20 DIM A(3)
30 A(0) = 1
40 A(1) = 2
50 A(2) = 3
60 I = 0
70 WHILE I < 3 AND A(I) < 4
80   PRINT A(I)
90   I = I + 1
100 WEND
110 END
