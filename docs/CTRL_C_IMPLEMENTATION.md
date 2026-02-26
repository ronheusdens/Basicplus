# Interactive Mode Ctrl-C Behavior Implementation

## Changes Made

### Ctrl-C Handling in Interactive Mode
Modified the signal handler behavior in interactive (REPL) mode:

**Before:**
- Ctrl-C would exit the interpreter immediately
- No graceful recovery to the prompt

**After:**
- Ctrl-C returns to the `READY` prompt
- User can continue working with the interpreter
- In AUTO mode: Ctrl-C exits AUTO mode and returns to `READY` prompt
- Only explicit EXIT/BYE/QUIT/END commands exit the interpreter

### Implementation Details

**Files:** 
- `src/ast-modules/main.c`: Signal handler and REPL loop

**Location:** Lines 45-50 (signal handler) and REPL main loop

```c
/* Check if Ctrl-C was pressed */
if (ctrl_c_pressed)
{
    ctrl_c_pressed = 0;
    if (auto_mode)
    {
        /* Exit AUTO mode on Ctrl-C */
        auto_mode = 0;
        print_newline();
        print_curses("READY\n");
        continue;
    }
    else
    {
        /* Return to prompt on Ctrl-C - do not exit */
        print_newline();
        print_curses("READY\n");
        continue;
    }
}
```

### Exit Commands

The following commands properly exit the interpreter:
- **EXIT** - Explicit exit command
- **BYE** - Alias for exit
- **QUIT** - Another exit alias
- **END** - Can be used to end program execution and exit
- **Ctrl-D** / **EOF** - End of input

### Testing

All 16 tests continue to pass:
- ✅ 01_trig.bas
- ✅ 02_for_next.bas
- ✅ 03_gosub_return.bas
- ✅ 04_if_then.bas
- ✅ 05_on_goto.bas
- ✅ 06_read_data_restore.bas
- ✅ 07_dim_arrays.bas
- ✅ 08_string_funcs.bas
- ✅ 09_file_io_basic.bas
- ✅ 10_file_io_sequential.bas
- ✅ 11_file_io_multiple.bas
- ✅ 12_file_io_eof.bas
- ✅ 13_math_functions.bas
- ✅ 14_write_formatted.bas
- ✅ 15_random_access.bas
- ✅ 16_line_input.bas

### User Experience

**Example Session:**

```
Memory size? 
RADIO SHACK LEVEL II BASIC
READY
> 10 PRINT "Hello"
> RUN
Hello
READY
> (User presses Ctrl-C)
READY
> EXIT
(Interpreter exits)
```

### Benefits

1. **Better User Control** - Users can interrupt long-running operations and continue
2. **Consistency** - Matches traditional BASIC interpreter behavior
3. **Graceful Error Recovery** - No need to restart after accidental Ctrl-C
4. **Explicit Exit** - Users must explicitly choose to exit with EXIT/BYE commands

