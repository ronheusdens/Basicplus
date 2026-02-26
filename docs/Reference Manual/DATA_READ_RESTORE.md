## DATA / READ / RESTORE

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
DATA value[, value...]
READ variable[, variable...]
RESTORE
```

**Description:**  
DATA stores values in the program. READ reads them into variables. RESTORE resets the read pointer.

**Example:**
```basic
10 DATA 5, 10, 15
20 READ A, B, C
30 PRINT A, B, C
40 RESTORE
50 READ X, Y, Z
60 PRINT X, Y, Z
```

**Notes:**
- Use RESTORE to reread DATA from the start.
