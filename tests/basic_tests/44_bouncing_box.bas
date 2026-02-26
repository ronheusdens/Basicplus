10 REM Test 44: Video Memory Visual Demo
20 REM Creates a bouncing box using video memory POKE
30 REM (Similar to the asterisk demo but with boxes)
40 REM
50 CLS
60 PRINT "BOUNCING BOX DEMO (Video Memory)"
70 PRINT ""
80 PRINT "The box will bounce within the display area."
90 PRINT "Press Ctrl-C to stop."
100 PRINT ""
110 SLEEP 1
120 REM
130 REM Initial position
140 LET X = 5
150 LET Y = 5
160 LET DX = 1
161 LET DY = 1
170 REM Box dimensions
180 LET W = 10
181 LET H = 4
182 REM
183 REM Main loop
190 FOR LOOP = 1 TO 100
200 REM Draw box at X, Y using direct position
210 REM Top and bottom horizontal
220   FOR C = X TO X + W
230     LET ADDR = 15360 + Y * 80 + C
232     POKE ADDR, 45
240     LET ADDR2 = 15360 + (Y + H) * 80 + C
242     POKE ADDR2, 45
250   NEXT C
260 REM Left and right vertical
270   FOR R = Y TO Y + H
280     LET ADDR = 15360 + R * 80 + X
282     POKE ADDR, 124
290     LET ADDR2 = 15360 + R * 80 + (X + W)
292     POKE ADDR2, 124
300   NEXT R
310 REM
320 REM Update position
330   LET X = X + DX
340   LET Y = Y + DY
350 REM
360 REM Bounce off edges
370   IF X <= 0 OR X >= 69 THEN LET DX = -DX
380   IF Y <= 0 OR Y >= 19 THEN LET DY = -DY
390 REM
400   SLEEP 0.1
410 NEXT LOOP
420 REM
430 CLS
440 PRINT "Animation complete!"
450 END
