# STOP and CONT

**BASIC Set:** Level II BASIC

## STOP Syntax
```
STOP
```

## STOP Description
Halts program execution immediately. Prints `STOP` to the console and stores the current line number for potential resumption with CONT.

Used for:
- Debugging by pausing execution at strategic points
- Inspecting program state before resuming
- Interactive program flow control

## STOP Parameters
None

## CONT Syntax
```
CONT
```

## CONT Description
Resumes execution from the point where a previous STOP was encountered. Only works if a STOP has been executed.

If no STOP has occurred, prints `Can't continue` and continues execution normally.

## STOP and CONT Examples

### Basic STOP Example
```
10 PRINT "Starting"
20 PRINT "About to pause"
30 STOP
40 PRINT "Resumed"
50 END
```

Output:
```
Starting
About to pause
STOP
```

### Interactive Usage
```
> RUN
Starting
About to pause
STOP
> CONT
Resumed
```

## Notes
- STOP is similar to END, but END marks the logical end of a program, while STOP is for debugging/control flow
- In batch mode, STOP halts before executing subsequent statements
- CONT is primarily useful in interactive debugging sessions
- The line number where STOP occurred is preserved for CONT to resume

## Related Statements
- END - Marks the end of a program
- BREAK (Ctrl+C) - Interrupts program execution
