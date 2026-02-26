# TROFF

**BASIC Set:** Extension

## Syntax
```
TROFF
```

## Description
Disables instruction tracing mode, allowing the program to execute at full speed without pausing at each statement. Tracing was previously enabled by [TRON](TRON.md) or [STEP](STEP.md).

Note: [BREAK](BREAK.md) breakpoints remain active even after TROFF.

## Parameters
None

## Example
```
[TRACE] > TROFF
Trace OFF
```

## Related Commands
- [TRON](TRON.md) - Enable full tracing
- [STEP](STEP.md) - Single-step mode (use TROFF to exit)
- [CONT](CONT.md) - Continue to next breakpoint
- [BREAK](BREAK.md) - Set breakpoint (remains active after TROFF)
- [NOBREAK](NOBREAK.md) - Remove breakpoint

## Notes
- Not part of Level II BASIC; provided as an extension
- Turns off interactive pausing from TRON or STEP commands
- Breakpoints set with [BREAK](BREAK.md) command still function
- Used within the interactive trace prompt (during program execution) to resume execution
- See [STEP TRACE Debugger Guide](../tracer.md) for detailed debugging workflows
