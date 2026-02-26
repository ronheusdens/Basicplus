10 REM RANDOM ACCESS FILE FUNCTIONS
20 PRINT "RANDOM ACCESS TEST"
30 REM Create a file and write some bytes
40 OPEN "random.bin" FOR OUTPUT AS #1
50 PUT #1, 65
60 PUT #1, 66
70 PUT #1, 67
80 PUT #1, 68
90 PUT #1, 69
100 PRINT "File position after write:" LOC(1)
110 PRINT "File size:" LOF(1)
120 CLOSE #1
130 REM Read back and verify
140 OPEN "random.bin" FOR INPUT AS #1
150 GET #1, B1
160 PRINT "First byte:" B1
170 GET #1, B2
180 PRINT "Second byte:" B2
190 PRINT "Position after 2 GETs:" LOC(1)
200 CLOSE #1
210 PRINT "OK"
220 END
