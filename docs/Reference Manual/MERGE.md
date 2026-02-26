# MERGE Command Documentation (TRS-80 BASIC Interpreter)

## Overview

The `MERGE` command in the TRS-80 Level II BASIC interpreter allows you to dynamically load and combine program lines from an external BASIC source file into the currently loaded program. This enables modular programming, code reuse, and dynamic subroutine loading, closely emulating the original TRS-80 BASIC's MERGE behavior.

- **Command Type:** Program statement (can be used in programs or interactively)
- **Syntax:**
  
  ```basic
  MERGE "filename"
  ```
  - `filename`: String literal or variable containing the path to a BASIC source file (must be in text format, one statement per line, each line starting with a line number).

## Purpose and Use Cases

- **Dynamic Module Loading:** Load subroutines or code modules at runtime.
- **Code Reuse:** Share common routines across multiple programs.
- **Interactive Development:** Add code to a running program without restarting.
- **Testing:** Inject test routines or patches into an existing program.

## How It Works

1. **File Reading:**
   - The specified file is opened and read line by line.
   - Each line must begin with a valid BASIC line number.
2. **Line Merging:**
   - If a line number in the file does **not** exist in the current program, it is added.
   - If a line number **already exists**, the new line **overwrites** the existing one.
   - A warning is printed for each overwritten line ("MERGE: line XXXX overwritten").
3. **Variable and State Handling:**
   - All variables and arrays are **cleared** after a successful MERGE (as if `NEW` was executed), to avoid conflicts with new code.
   - All open files are closed.
4. **Error Handling:**
   - If the file cannot be opened or read, a `DISK BASIC ERROR` is raised (BASIC_ERR_DISK_BASIC).
   - If the file contains invalid lines (missing line numbers), a syntax error is raised.

## Example

Suppose you have a file `merge_sub.txt`:

```basic
2000 PRINT "Line within subroutine"
2010 RETURN
```

And your main program:

```basic
10 PRINT "Before merge"
20 MERGE "merge_sub.txt"
30 GOSUB 2000
40 PRINT "After subroutine"
50 END
```

**Output:**
```
Before merge
Line within subroutine
After subroutine
```

If `merge_sub.txt` also contained a line `10 PRINT "Overwritten"`, you would see:
```
MERGE: line 10 overwritten
Overwritten
Line within subroutine
After subroutine
```

## Implementation Details

- **Lexer/Parser:**
  - `MERGE` is recognized as a statement keyword.
  - The parser expects a string argument (filename).
- **AST/Executor:**
  - The executor opens the file, parses each line, and merges it into the program's line table.
  - Overwrites are detected and reported.
  - After merging, all variables and arrays are cleared, and all files are closed.
- **Testing:**
  - Automated tests exist in `tests/basic_tests/74_merge_simple.bas` and `tests/basic_tests/74_merge_gosub.bas`.

## Limitations & Notes

- The merged file must be a valid BASIC source file with line numbers.
- No dynamic resizing of arrays; all variables are reset after merge.
- Only one file can be merged per MERGE statement.
- File path can be absolute or relative; errors are reported if the file is missing or unreadable.
- MERGE can be used multiple times in a program.

## Error Messages

- `MERGE: line XXXX overwritten` — Indicates a line in the current program was replaced by a line from the merged file.
- `DISK BASIC ERROR` — File could not be opened or read.
- `SYNTAX ERROR` — File contains lines without valid line numbers.

## Related Commands

- `NEW` — Clears all program lines and variables.
- `RUN` — Executes the current program.
- `GOSUB`/`RETURN` — Used to call subroutines, often after merging new code.

## References

- See [tests/basic_tests/65_merge_test.bas](tests/basic_tests/65_merge_test.bas) for a working example.
- For implementation, refer to `src/ast-modules/lexer.c`, `parser.c`, `executor.c`.

---
_Last updated: February 2026_
