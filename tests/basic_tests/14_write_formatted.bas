10 REM WRITE# FORMATTED OUTPUT TEST
20 PRINT "WRITE TEST"
30 REM Create a CSV file with formatted output
40 OPEN "output.csv" FOR OUTPUT AS #1
50 WRITE #1, "Name", "Age", "Score"
60 WRITE #1, "Alice", 25, 95.5
70 WRITE #1, "Bob", 30, 88.0
80 WRITE #1, "Charlie", 28, 92.3
90 CLOSE #1
100 REM Read and verify
110 OPEN "output.csv" FOR INPUT AS #1
120 INPUT #1, N1$, N2$, N3$
130 PRINT N1$ " " N2$ " " N3$
140 INPUT #1, V1$, V2, V3
150 PRINT V1$ " " V2 " " V3
160 CLOSE #1
170 PRINT "OK"
180 END
