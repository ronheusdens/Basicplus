10 REM Moving asterisk with visual feedback
15 CLS
20 PRINT "Asterisk bouncer - watch row 10"
25 P = 1
30 D = 1
40 N = 0
50 R = 10
55 OLDP = 1
60 WHILE N < 40
70   PRINT @ (R * 64 + 1), "P=", P, "  "
75   PRINT @ (R * 64 + OLDP), " "
80   PRINT @ (R * 64 + P), "*"
85   OLDP = P
90   P = P + D
100  IF P >= 15 THEN D = -1
110  IF P <= 1 THEN D = 1
120  N = N + 1
125  SLEEP 0.2
130 WEND
140 PRINT @ ((R+2) * 64 + 1), "Done!"
150 END
