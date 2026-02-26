# STEP TRACE Debugger Guide

The STEP TRACE system is a powerful interactive debugger for TRS-80 BASIC programs that allows you to control execution, set breakpoints, inspect variables, and step through code line by line.

## Overview

STEP TRACE lets you:
- **Enable/Disable** tracing of program execution
- **Set/Remove** breakpoints at specific line numbers
- **Step** through statements one at a time
- **Continue** execution without tracing
- **Inspect** all variables and their current values
- **List** active breakpoints
- **Quit** execution safely

## Commands

All STEP TRACE commands are issued at the interpreter prompt (interactive mode) or during program execution when a trace break occurs.

### TRON - Enable Tracing
```
TRON
```
Enables instruction tracing. The program pauses before executing each statement, displaying:
- Current line number and statement
- All variables and their values
- Interactive prompt for user action

**Example:**
```
Ready
> TRON
Trace ON
> RUN
(program execution pauses at each statement)
```

### TROFF - Disable Tracing
```
TROFF
```
Disables instruction tracing and continues program execution without pausing.

**Example:**
```
[TRACE] > TROFF
Trace OFF
```

### STEP - Single-Step Mode
```
STEP
```
Enables single-step mode. Similar to TRON, but automatically steps through each statement by default, returning control to you after each statement.

**Example:**
```
Ready
> STEP
Step mode enabled
> RUN
(program pauses after each statement)
```

### CONT - Continue Execution
```
CONT
```
Disables tracing/stepping and allows the program to continue to completion or until it hits a breakpoint.

**Example:**
```
[TRACE] > CONT
Continuing...
```

### BREAK - Set Breakpoint
```
BREAK line_number
```
Sets a breakpoint at the specified line number. When the program reaches this line, execution pauses and displays trace information, regardless of whether tracing is enabled.

**Syntax:**
```
BREAK 100
BREAK 250
```

**Example:**
```
Ready
> BREAK 100
Breakpoint set at line 100
> RUN
(program runs until line 100, then pauses)
```

**Limits:**
- Maximum 256 breakpoints per program
- Cannot set breakpoint on non-existent line number
- Setting same breakpoint twice returns an error

### NOBREAK - Remove Breakpoint
```
NOBREAK line_number
NOBREAK ALL
```
Removes a specific breakpoint or clears all breakpoints.

**Example:**
```
[TRACE] > NOBREAK 100
Breakpoint removed at line 100

[TRACE] > NOBREAK ALL
All breakpoints cleared
```

## Interactive Trace Commands

When a trace break occurs (either via TRON, STEP, or breakpoint), the interpreter displays:
```
[TRACE] (S=step, C=continue, V=vars, L=list breaks, Q=quit):
```

At this prompt, enter one character to control execution:

### S - Step to Next Statement
```
[TRACE] > S
```
Executes the current statement and pauses at the next one. Use this to carefully navigate through your code.

### C - Continue to Next Break or End
```
[TRACE] > C
```
Disables tracing and continues full-speed execution until the program ends or hits a breakpoint.

### V - Display All Variables
```
[TRACE] > V
```
Shows all currently defined variables with their values:
- Numeric variables show their value
- String variables show their contents in quotes
- Arrays show `[array]` indicator
- Limits display to 30 variables; if more exist, shows count of remaining

**Output Example:**
```
Variables:
X = 10
Y = 5
NAME$ = "JOHN"
VALUES() = [array]
(showing 4 of 4 variables)
```

### L - List Active Breakpoints
```
[TRACE] > L
```
Shows all currently set breakpoints with their line numbers.

**Output Example:**
```
Breakpoints:
100
250
350
(3 breakpoints set)
```

### Q - Quit Execution
```
[TRACE] > Q
```
Terminates program execution immediately. Variables retain their last values.

## Practical Workflows

### Debugging a Loop

Find where a loop exits unexpectedly:

```basic
10 FOR I = 1 TO 100
20 IF I > 50 THEN GOTO 100
30 PRINT I
40 NEXT I
100 PRINT "LOOP FINISHED"
```

**Workflow:**
```
Ready
> BREAK 30
Breakpoint set at line 30
> RUN
[TRACE at line 30]
> V
Variables:
I = 1
> S
[TRACE at line 40]
> C
(continues until next breakpoint at line 30)
LOOP FINISHED
```

### Tracing Variable Changes

Monitor how variables change during execution:

```basic
10 LET X = 0
20 FOR I = 1 TO 10
30 LET X = X + I
40 PRINT X
50 NEXT I
```

**Workflow:**
```
Ready
> TRON
Trace ON
> RUN
[TRACE at line 10] 10: LET X = 0
Variables:
X = 0
> S
[TRACE at line 20] 20: FOR I = 1 TO 10
> S
(continue stepping with S to watch X change each iteration)
```

### Finding Program Errors

Isolate where a syntax error occurs:

```basic
10 LET A = 10
20 LET B = 20
30 LET C = A / B
40 LET D = 100 / 0     REM Error here!
50 PRINT C, D
```

**Workflow:**
```
Ready
> BREAK 40
Breakpoint set at line 40
> RUN
[TRACE at line 40] 40: LET D = 100 / 0
Variables:
A = 10
B = 20
C = 0.5
> V
(examine values before error)
> S
Error: Division by zero (line 40)
```

### Multi-Step Debugging Strategy

Use multiple breakpoints to narrow down issues:

```basic
10 LET X = 0
20 GOSUB 100
30 LET Y = X * 2
40 PRINT Y
50 END
100 LET X = 10
110 RETURN
```

**Workflow:**
```
Ready
> BREAK 20
Breakpoint set at line 20
> BREAK 30
Breakpoint set at line 30
> RUN
[TRACE at line 20] (checkpoint before GOSUB)
> V
X = 0
> C
[TRACE at line 30] (checkpoint after GOSUB)
> V
X = 10  (verify GOSUB worked)
> C
20
```

## Tips and Best Practices

### 1. Use Breakpoints for Faster Debugging
```
BREAK 150   (instead of stepping 50+ times)
CONT
```

### 2. Combine TRON with STEP
```
TRON        (see what's happening)
RUN
[TRACE]> C  (continue to problem area)
STEP        (then enable step mode once near it)
```

### 3. Check Variables Before Conditions
```
[TRACE at line 50] IF X > 10 THEN GOTO 100
> V  (see what X is before checking condition)
```

### 4. Use NOBREAK ALL to Reset
```
NOBREAK ALL  (clear all breakpoints)
RUN          (start fresh)
```

### 5. Strategic Breakpoint Placement
- Place at GOSUB entry points
- Place at conditional branches
- Place after data read operations
- Place before division operations

## Common Issues and Solutions

### "Trace is frozen at prompt"
**Problem:** The trace prompt isn't responding to commands.  
**Solution:** Ensure you're running in interactive mode. File-based execution doesn't support interactive tracing.

### "Too many breakpoints"
**Problem:** Error message "Breakpoint limit exceeded"  
**Solution:** Clear unused breakpoints with `NOBREAK ALL` or `NOBREAK line_number`.

### "Variable not showing at breakpoint"
**Problem:** Expected variable doesn't appear in V command.  
**Solution:** Variable may not exist yet or may have been in a different scope. Check if variable was initialized on an earlier line.

### "STEP mode doesn't pause"
**Problem:** STEP mode immediately runs to next breakpoint.  
**Solution:** You may have pressed C (continue) instead of S (step). Breakpoints still take priority.

## Performance Notes

- **Tracing adds overhead**: Programs run slower with TRON/STEP enabled
- **Breakpoints are efficient**: Breakpoints alone have minimal performance impact
- **Non-interactive mode**: If redirecting input, tracing auto-continues (doesn't block)
- **Variable display**: Showing 30+ variables may slow display

## Example Programs for Testing

### Simple Counter
```basic
10 FOR I = 1 TO 5
20 PRINT "COUNT: "; I
30 NEXT I
40 END
```

Run with: `TRON` then `RUN test_counter.bas`

### Array Processing
```basic
10 DIM NUMS(5)
20 FOR I = 1 TO 5
30 LET NUMS(I) = I * 10
40 NEXT I
50 FOR I = 1 TO 5
60 PRINT NUMS(I)
70 NEXT I
80 END
```

Run with: `BREAK 40` then `RUN test_arrays.bas`

### Conditional Logic
```basic
10 LET X = 50
20 IF X > 100 THEN PRINT "BIG"
30 IF X < 100 THEN PRINT "SMALL"
40 IF X = 100 THEN PRINT "EQUAL"
50 END
```

Run with: `TRON` then `RUN test_conditions.bas`

## Summary

The STEP TRACE system provides professional-grade debugging capabilities for BASIC programs through:

| Feature | Command | Use Case |
|---------|---------|----------|
| Full tracing | TRON | See everything |
| Step-by-step | STEP | Careful inspection |
| Fast breakpoints | BREAK/NOBREAK | Focus on problem areas |
| Variable inspection | V command | Debug data |
| Continue execution | CONT/C | Resume after pauses |
| List breakpoints | L command | Verify debugging setup |

Master these tools to efficiently debug complex BASIC programs on the TRS-80 platform.
