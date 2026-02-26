# TRS-80 BASIC Error Messages

This chapter lists all error messages that may be thrown by the TRS-80 BASIC interpreter. Use this guide to analyze and fix issues in your BASIC programs. Each error is shown with its code, message, and a brief explanation.

| Code | Message         | Meaning / How to Fix                |
|------|----------------|-------------------------------------|
| 0    | NEXT WITHOUT FOR | A NEXT statement was found without a matching FOR loop. Check your loop structure. |
| 1    | SYNTAX ERROR   | The statement is not valid BASIC syntax. Check for typos or missing keywords. |
| 2    | RETURN WITHOUT GOSUB | A RETURN was encountered without a matching GOSUB. Ensure each RETURN has a GOSUB. |
| 3    | OUT OF DATA    | A READ statement was used but there is no more DATA. Add more DATA or check your READs. |
| 4    | ILLEGAL QUANTITY | A value is out of range or invalid (e.g., negative array size). Check your parameters. |
| 5    | OVERFLOW       | A calculation produced a value too large for the variable type. Check your math. |
| 6    | OUT OF MEMORY  | The program ran out of memory. Simplify your program or reduce array sizes. |
| 7    | UNDEFINED LINE | A GOTO, GOSUB, or similar referenced a line number that does not exist. Check your line numbers. |
| 8    | BAD SUBSCRIPT  | An array index is out of bounds. Check your DIM and array accesses. |
| 9    | REDIM'D ARRAY  | Attempted to re-dimension an array. Arrays can only be DIM'd once. |
| 10   | DIVISION BY ZERO | Attempted to divide by zero. Check your denominators. |
| 11   | TYPE MISMATCH  | Used a string where a number was expected, or vice versa. Check your variable types. |
| 12   | FILE NOT FOUND | Tried to LOAD or OPEN a file that does not exist. Check the filename and path. |
| 13   | FILE ALREADY EXISTS | Tried to SAVE to a file that already exists. Use a different name or delete the file first. |
| 14   | FILE I/O ERROR | General file input/output error. Check disk space, permissions, or file format. |
| 15   | DEVICE ERROR   | An error occurred with an external device (e.g., printer, disk). Check connections and device status. |
| 16   | ILLEGAL FUNCTION CALL | Used a function incorrectly (e.g., wrong number of arguments). Check the function usage. |
| 17   | STRING TOO LONG | A string exceeded the maximum allowed length. Shorten your strings. |
| 18   | CAN'T CONTINUE | Tried to CONT after a program stop or error. Use RUN instead. |
| 19   | BREAK (Ctrl-C) | Program was interrupted by the user. |
| 20   | STOP STATEMENT | Program stopped by STOP statement. |
| 21   | END OF PROGRAM | Program reached END statement. |
| 22   | ILLEGAL DIRECT | Statement not allowed in direct mode. Use it in a program. |
| 23   | BAD FILE NUMBER | File number is invalid or not open. Check your file numbers. |
| 24   | BAD FILE MODE  | File was opened in the wrong mode for this operation. |
| 25   | INPUT PAST END | Tried to INPUT past the end of a file. |
| 26   | BAD CHARACTER  | Invalid character in input. Check your data or input statements. |
| 27   | FEATURE NOT IMPLEMENTED | Tried to use a feature not supported by this interpreter. |

## How to Use This Table
- Find the error code or message shown by the interpreter.
- Read the explanation and suggested fix.
- Check your program for the described issue.

If you encounter an error not listed here, consult the documentation or report it as a possible bug.
