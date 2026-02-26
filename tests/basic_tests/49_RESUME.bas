10 ON ERROR GOTO 100
20 PRINT "Before error"
30 X = 1 / 0   ' Intentional error: division by zero
40 PRINT "After error"
50 END
100 PRINT "Error handler activated"
110 PRINT "ERR="; ERR; " ERL="; ERL
120 RESUME 40
