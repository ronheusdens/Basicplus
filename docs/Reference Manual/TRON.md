# TRON

**BASIC Set:** Extension

## Syntax
```
TRON
```

## Description
Enables instruction tracing mode for debugging. When enabled, the interpreter pauses before executing each statement and displays:
- The current line number and statement being executed
- All currently defined variables with their values
- An interactive prompt allowing step-by-step control

Contrast with [STEP](STEP.md) for single-step mode and [TROFF](TROFF.md) to disable tracing.

## Interactive Trace Commands

When a trace break occurs, the interpreter displays a prompt with the following options:

| Command | Action |
|---------|--------|
| **S** | Step to next statement |
| **C** | Continue to next breakpoint or end |
| **V** | Display all variables |
| **L** | List active breakpoints |
| **Q** | Quit execution |

## Parameters
None

## Example
```
Ready
> TRON
Trace ON
> RUN program.bas
```

## Related Commands
- [TROFF](TROFF.md) - Disable tracing
- [STEP](STEP.md) - Single-step mode
- [CONT](CONT.md) - Continue to next break
- [BREAK](BREAK.md) - Set breakpoint
- [NOBREAK](NOBREAK.md) - Remove breakpoint

## Notes
- Not part of Level II BASIC; provided as an extension
- Adds significant execution overhead
- Best used with small programs or when focusing on specific lines
- For more control, use [BREAK](BREAK.md) to set targeted breakpoints instead
- Interactive tracing requires terminal input; file-based input auto-continues
- See [STEP TRACE Debugger Guide](../tracer.md) for detailed debugging workflows and examples
