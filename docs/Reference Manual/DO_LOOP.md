# DO..LOOP Control Structure

## Overview

The `DO..LOOP` structure provides flexible loop control with four distinct variants, supporting both pre-test and post-test conditions. This is an extension to standard TRS-80 Level II BASIC, complementing the existing `WHILE..WEND` loops.

## Four Variants

### 1. DO WHILE (Pre-Test Loop)

**Syntax:**
```basic
DO WHILE condition
  [statements]
LOOP
```

**Behavior:**
- Evaluates the condition **before** entering the loop
- If condition is true, executes the loop body and repeats
- If condition is false, skips the entire loop
- This is a pre-test loop (condition checked at entry)

**Example:**
```basic
10 REM Count from 0 to 2 using DO WHILE
20 x = 0
30 DO WHILE x < 3
40   PRINT "x ="; x
50   x = x + 1
60 LOOP
70 PRINT "Done"
80 END
```

**Output:**
```
x = 0
x = 1
x = 2
Done
```

### 2. DO..LOOP UNTIL (Post-Test Loop)

**Syntax:**
```basic
DO
  [statements]
LOOP UNTIL condition
```

**Behavior:**
- Executes the loop body **first**, then checks the condition
- Continues looping while condition is **false**
- Exits when condition becomes **true**
- This is a post-test loop (condition checked at exit)
- Loop always executes at least once

**Example:**
```basic
10 REM Count from 0 to 2 using DO..LOOP UNTIL
20 n = 0
30 DO
40   PRINT "n ="; n
50   n = n + 1
60 LOOP UNTIL n >= 3
70 PRINT "Done"
80 END
```

**Output:**
```
n = 0
n = 1
n = 2
Done
```

### 3. DO..LOOP WHILE (Post-Test Loop)

**Syntax:**
```basic
DO
  [statements]
LOOP WHILE condition
```

**Behavior:**
- Executes the loop body **first**, then checks the condition
- Continues looping while condition is **true**
- Exits when condition becomes **false**
- This is a post-test loop (condition checked at exit)
- Loop always executes at least once

**Example:**
```basic
10 REM Count from 0 to 2 using DO..LOOP WHILE
20 j = 0
30 DO
40   PRINT "j ="; j
50   j = j + 1
60 LOOP WHILE j < 3
70 PRINT "Done"
80 END
```

**Output:**
```
j = 0
j = 1
j = 2
Done
```

### 4. DO..LOOP (Infinite Loop)

**Syntax:**
```basic
DO
  [statements]
LOOP
```

**Behavior:**
- Creates an infinite loop
- Must use `EXIT` statement to break out
- Useful when exit condition is complex or occurs in middle of loop

**Example:**
```basic
10 REM Infinite loop with EXIT
20 i = 0
30 DO
40   PRINT "i ="; i
50   i = i + 1
60   IF i = 3 THEN EXIT
70 LOOP
80 PRINT "Done"
90 END
```

**Output:**
```
i = 0
i = 1
i = 2
Done
```

## EXIT Statement

The `EXIT` statement provides a way to exit a `DO..LOOP` prematurely.

**Syntax:**
```basic
EXIT
```

**Behavior in DO..LOOP context:**
- Immediately exits the innermost `DO..LOOP` structure
- Continues execution at the statement **after** the matching `LOOP`
- Can be used in any of the four `DO..LOOP` variants
- Respects nested loop structure (exits only innermost loop)

**Example with IF..THEN EXIT:**
```basic
10 REM Search for target value with EXIT
20 i = 0
30 DO WHILE i < 10
40   IF i = 5 THEN EXIT
50   PRINT "i ="; i
60   i = i + 1
70 LOOP
80 PRINT "Found 5, exiting loop"
90 END
```

**Output:**
```
i = 0
i = 1
i = 2
i = 3
i = 4
Found 5, exiting loop
```

**Example with nested condition:**
```basic
10 REM Multiple conditions with EXIT
20 sum = 0
30 x = 0
40 DO
50   sum = sum + x
60   IF sum > 10 THEN EXIT
70   PRINT "sum ="; sum
80   x = x + 1
90 LOOP
100 PRINT "Sum exceeded 10"
110 END
```

## Nested Loops

`DO..LOOP` structures can be nested within each other. Each `EXIT` exits only the innermost loop.

**Example:**
```basic
10 REM Nested DO loops
20 x = 0
30 DO WHILE x < 2
40   y = 0
50   DO WHILE y < 2
60     PRINT "x="; x; " y="; y
70     y = y + 1
80   LOOP
90   x = x + 1
100 LOOP
110 PRINT "Nested loops complete"
120 END
```

**Output:**
```
x= 0  y= 0
x= 0  y= 1
x= 1  y= 0
x= 1  y= 1
Nested loops complete
```

**Example with nested EXIT:**
```basic
10 REM EXIT in nested loop
20 found = 0
30 x = 0
40 DO WHILE x < 3
50   y = 0
60   DO WHILE y < 3
70     IF x = 1 AND y = 1 THEN found = 1 : EXIT
80     PRINT "x="; x; " y="; y
90     y = y + 1
100   LOOP
110   IF found = 1 THEN EXIT
120   x = x + 1
130 LOOP
140 PRINT "Found target at x=1, y=1"
150 END
```

**Output:**
```
x= 0  y= 0
x= 1  y= 0
Found target at x=1, y=1
```

## Comparison with WHILE..WEND

| Feature | DO WHILE | WHILE..WEND |
|---------|----------|------------|
| Test timing | Pre-test (entry) | Pre-test (entry) |
| Loop may not execute | Yes (if condition false) | Yes (if condition false) |
| EXIT support | Yes | No |
| POST-test loops | LOOP UNTIL, LOOP WHILE | Not available |
| Infinite loops | DO..LOOP | Not available |
| Nesting | Full support | Full support |

### Examples Showing Differences

**DO WHILE (pre-test, can be skipped):**
```basic
10 x = 10
20 DO WHILE x < 5
30   PRINT "This never prints"
40   x = x + 1
50 LOOP
```

**DO..LOOP UNTIL (post-test, always executes once):**
```basic
10 x = 10
20 DO
30   PRINT "This prints once" : REM Always executes at least once
40   x = x + 1
50 LOOP UNTIL x < 5
```

## Use Cases

### 1. Processing Sequence Until Condition
```basic
10 REM Process items until sum exceeds limit
20 sum = 0
30 item = 1
40 DO
50   sum = sum + item
60   PRINT "Item"; item; " sum="; sum
70   item = item + 1
80 LOOP UNTIL sum > 20
90 END
```

### 2. Menu Loop with EXIT
```basic
10 REM Simple menu loop
20 DO
30   PRINT "1=Add  2=Delete  3=Exit"
40   INPUT "Choose: "; choice
50   IF choice = 1 THEN PRINT "Add selected"
60   IF choice = 2 THEN PRINT "Delete selected"
70   IF choice = 3 THEN EXIT
80 LOOP
90 PRINT "Program ended"
100 END
```

### 3. Retry Logic
```basic
10 REM Retry until success
20 tries = 0
30 DO
40   tries = tries + 1
50   PRINT "Try #"; tries
60   REM Simulate success on try 3
70   IF tries = 3 THEN EXIT
80   PRINT "Failed, retrying..."
90 LOOP
100 PRINT "Success!"
110 END
```

### 4. Data Processing with Early Exit
```basic
10 REM Process array until special value
20 DIM values(10)
30 FOR i = 1 TO 10
40   READ values(i)
50 NEXT i
60 x = 1
70 DO WHILE x <= 10
80   PRINT "Value:"; values(x)
90   IF values(x) = 99 THEN EXIT
100  x = x + 1
110 LOOP
120 PRINT "Processing stopped"
130 DATA 1,2,3,99,5,6,7,8,9,10
140 END
```

## Error Conditions

### EXIT Outside DO..LOOP
When `EXIT` is used outside a `DO..LOOP`, the entire program terminates (equivalent to `END`).

```basic
10 PRINT "Start"
20 EXIT        REM Terminates program here
30 PRINT "Never reached"
40 END
```

### Unmatched LOOP
A `LOOP` statement without a matching `DO` will generate a syntax error.

```basic
10 LOOP        REM ERROR: LOOP without DO
20 END
```

## Implementation Details

### Scope and Visibility
- Loop variables are visible throughout their scope
- Variables created or modified in loop persist after loop exit

```basic
10 x = 0
20 DO WHILE x < 3
30   y = x * 2
40   x = x + 1
50 LOOP
60 PRINT "After loop: x="; x; " y="; y
70 REM x=3, y=4
80 END
```

### Condition Re-evaluation
- Pre-test conditions (DO WHILE) re-evaluated at each loop iteration
- Post-test conditions (LOOP UNTIL, LOOP WHILE) re-evaluated after body execution

## Advanced Examples

### 1. Prime Number Checker
```basic
10 REM Find prime numbers up to 20
20 n = 2
30 DO WHILE n <= 20
40   i = 2
50   prime = 1
60   DO WHILE i * i <= n
70     IF n MOD i = 0 THEN prime = 0 : EXIT
80     i = i + 1
90   LOOP
100  IF prime = 1 THEN PRINT n; " is prime"
110  n = n + 1
120 LOOP
130 END
```

### 2. Binary Search Simulation
```basic
10 REM Binary search
20 target = 35
30 low = 1
40 high = 100
50 found = 0
60 DO
70   mid = (low + high) / 2
80   IF mid = target THEN found = 1 : EXIT
90   IF mid < target THEN low = mid + 1
100 IF mid > target THEN high = mid - 1
110 LOOP UNTIL low > high
120 IF found = 1 THEN PRINT "Found at"; mid
130 IF found = 0 THEN PRINT "Not found"
140 END
```

### 3. Fibonacci Sequence
```basic
10 REM Generate Fibonacci numbers until > 1000
20 a = 1
30 b = 1
40 DO
50   PRINT a
60   temp = a + b
70   a = b
80   b = temp
90   IF a > 1000 THEN EXIT
100 LOOP
110 PRINT "Done"
120 END
```

## Compatibility Notes

- **DO..LOOP** is an extension to standard TRS-80 Level II BASIC
- Use `--strict` mode flag to reject DO..LOOP if TRS-80 compatibility is required
- Equivalent functionality in strict TRS-80 can be achieved with `WHILE..WEND`
- The `EXIT` statement differs from typical BASIC interpreter behavior where EXIT might exit the REPL; here EXIT is exclusively for loop control

## Summary Table

| Loop Type | Condition Timing | When Exits | Loop Always Runs |
|-----------|------------------|-----------|-----------------|
| DO WHILE | Pre (entry) | When condition false | No |
| DO..LOOP UNTIL | Post (exit) | When condition true | Yes |
| DO..LOOP WHILE | Post (exit) | When condition false | Yes |
| DO..LOOP | None | Only via EXIT | Yes* |

*Only if EXIT is used; otherwise infinite
