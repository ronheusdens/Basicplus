# DATA, DIM, READ, RESTORE Reference

**TRS-80 Level II BASIC Interpreter**  
**Version**: 1.5.0  
**Status**: ✅ FULLY SUPPORTED

---

## Overview

All 4 requested data handling features are **already implemented and working**:

| Feature | Status | Description |
|---------|--------|-------------|
| **DATA** | ✅ | Define data values in program |
| **DIM** | ✅ | Declare arrays (single/multi-dimensional) |
| **READ** | ✅ | Read values from DATA statements |
| **RESTORE** | ✅ | Reset DATA pointer to beginning |

---

## DATA Statement

Defines a list of constant values (numbers or strings) that can be read by READ statements.

### Syntax
```basic
DATA value1, value2, value3, ...
```

### Examples

**Numeric Data:**
```basic
100 DATA 10, 20, 30, 40, 50
```

**String Data:**
```basic
200 DATA "Alice", "Bob", "Carol"
```

**Mixed Data:**
```basic
300 DATA 1001, "John", 25, "Active"
```

### Notes
- DATA statements can appear anywhere in the program
- Multiple DATA statements are processed sequentially
- Values are read in order across all DATA statements
- Strings can be quoted or unquoted (quotes recommended)

---

## READ Statement

Reads values from DATA statements and assigns them to variables.

### Syntax
```basic
READ variable1, variable2, variable3, ...
```

### Examples

**Basic READ:**
```basic
10 READ A, B, C
20 PRINT A; B; C
30 DATA 10, 20, 30
```
**Output:** `10 20 30`

**String READ:**
```basic
10 READ NAME$, CITY$
20 PRINT NAME$; " from "; CITY$
30 DATA "Alice", "Boston"
```
**Output:** `Alice from Boston`

**READ in Loop:**
```basic
10 FOR I = 1 TO 5
20   READ X
30   PRINT X
40 NEXT I
50 DATA 10, 20, 30, 40, 50
```

### Notes
- READ advances the data pointer automatically
- Reading past available data causes an error
- Variables must match data types (numeric/string)

---

## RESTORE Statement

Resets the DATA pointer to the beginning, allowing data to be re-read.

### Syntax
```basic
RESTORE
```

### Example

```basic
10 READ A, B
20 PRINT "First read:"; A; B
30 RESTORE
40 READ X, Y
50 PRINT "Second read:"; X; Y
60 DATA 10, 20
```

**Output:**
```
First read: 10 20
Second read: 10 20
```

### Use Cases
- Re-reading data multiple times
- Resetting after partial read
- Implementing data loops

---

## DIM Statement

Declares arrays with specified dimensions. Required before using array subscripts.

### Syntax
```basic
DIM arrayname(size)                    ' 1D array
DIM arrayname(rows, cols)              ' 2D array
DIM arrayname(d1, d2, d3)              ' 3D array
DIM array1(size1), array2(size2), ...  ' Multiple arrays
```

### Array Indexing
- **Zero-based**: Arrays start at index 0
- `DIM A(5)` creates array with indices 0, 1, 2, 3, 4, 5 (6 elements)
- Maximum index is the dimension size

### Examples

**1D Numeric Array:**
```basic
10 DIM SCORES(10)
20 LET SCORES(0) = 95
30 LET SCORES(5) = 87
40 PRINT SCORES(0); SCORES(5)
```

**1D String Array:**
```basic
10 DIM NAMES$(5)
20 LET NAMES$(0) = "Alice"
30 LET NAMES$(1) = "Bob"
40 PRINT NAMES$(0); NAMES$(1)
```

**2D Array (Matrix):**
```basic
10 DIM MATRIX(3, 3)
20 LET MATRIX(0, 0) = 1
30 LET MATRIX(0, 1) = 2
40 LET MATRIX(1, 0) = 3
50 PRINT MATRIX(0,0); MATRIX(0,1); MATRIX(1,0)
```

**Multiple Arrays:**
```basic
10 DIM A(10), B(5), NAMES$(20)
20 LET A(0) = 100
30 LET B(3) = 50
40 LET NAMES$(0) = "Test"
```

**3D Array:**
```basic
10 DIM CUBE(5, 5, 5)
20 LET CUBE(2, 3, 4) = 999
30 PRINT CUBE(2, 3, 4)
```

---

## Complete Examples

### Example 1: Student Records

```basic
10 REM Student grade system
20 DIM STUDENTS$(5), GRADES(5)
30 REM Read student data
40 FOR I = 0 TO 4
50   READ STUDENTS$(I), GRADES(I)
60 NEXT I
70 REM Display records
80 FOR I = 0 TO 4
90   PRINT STUDENTS$(I); ": "; GRADES(I)
100 NEXT I
110 DATA "Alice", 95
120 DATA "Bob", 87
130 DATA "Carol", 92
140 DATA "Dave", 78
150 DATA "Eve", 88
160 END
```

### Example 2: Matrix Operations

```basic
10 REM 2x2 Matrix addition
20 DIM A(1,1), B(1,1), C(1,1)
30 REM Initialize matrices
40 FOR I = 0 TO 1
50   FOR J = 0 TO 1
60     READ A(I,J)
70   NEXT J
80 NEXT I
90 FOR I = 0 TO 1
100   FOR J = 0 TO 1
110     READ B(I,J)
120   NEXT J
130 NEXT I
140 REM Add matrices
150 FOR I = 0 TO 1
160   FOR J = 0 TO 1
170     LET C(I,J) = A(I,J) + B(I,J)
180   NEXT J
190 NEXT I
200 REM Display result
210 PRINT "Result:"
220 FOR I = 0 TO 1
230   FOR J = 0 TO 1
240     PRINT C(I,J);
250   NEXT J
260   PRINT
270 NEXT I
280 DATA 1, 2, 3, 4
290 DATA 5, 6, 7, 8
300 END
```

### Example 3: Re-reading Data

```basic
10 REM Calculate average twice
20 DIM NUMS(5)
30 REM First pass - read and sum
40 LET SUM = 0
50 FOR I = 0 TO 4
60   READ NUMS(I)
70   LET SUM = SUM + NUMS(I)
80 NEXT I
90 PRINT "Sum:"; SUM
100 PRINT "Average:"; SUM / 5
110 REM Second pass - re-read
120 RESTORE
130 LET SUM2 = 0
140 FOR I = 0 TO 4
150   READ X
160   LET SUM2 = SUM2 + X
170 NEXT I
180 PRINT "Verify sum:"; SUM2
190 DATA 10, 20, 30, 40, 50
200 END
```

### Example 4: Lookup Table

```basic
10 REM Month name lookup
20 DIM MONTHS$(12)
30 FOR I = 1 TO 12
40   READ MONTHS$(I)
50 NEXT I
60 INPUT "Enter month number (1-12): "; M
70 IF M < 1 OR M > 12 THEN PRINT "Invalid": GOTO 60
80 PRINT "Month: "; MONTHS$(M)
90 DATA "January", "February", "March"
100 DATA "April", "May", "June"
110 DATA "July", "August", "September"
120 DATA "October", "November", "December"
130 END
```

---

## Array Limitations

### Maximum Dimensions
- Supports up to 3 dimensions: `DIM A(x, y, z)`
- Higher dimensions not supported

### Array Size
- Limited by available memory
- Large arrays may cause out-of-memory errors

### Dynamic Sizing
- Array size must be constant (not variable)
- Cannot resize arrays after DIM

---

## Common Patterns

### Pattern 1: Parallel Arrays

```basic
10 DIM NAMES$(10), AGES(10), CITIES$(10)
20 FOR I = 0 TO 9
30   READ NAMES$(I), AGES(I), CITIES$(I)
40 NEXT I
50 DATA "Alice", 25, "Boston"
60 DATA "Bob", 30, "NYC"
...
```

### Pattern 2: Array Initialization

```basic
10 DIM A(100)
20 FOR I = 0 TO 100
30   LET A(I) = 0
40 NEXT I
```

### Pattern 3: Data Validation

```basic
10 READ COUNT
20 DIM VALUES(COUNT)
30 FOR I = 0 TO COUNT - 1
40   READ VALUES(I)
50 NEXT I
60 DATA 5
70 DATA 10, 20, 30, 40, 50
```

---

## Error Conditions

| Error | Cause | Solution |
|-------|-------|----------|
| Subscript out of range | Array index too large | Check DIM size |
| Out of DATA | READ past available data | Add more DATA or RESTORE |
| Type mismatch | Reading string into number | Match variable types |
| Redimensioned array | DIM called twice | Remove duplicate DIM |

---

## Compatibility

### TRS-80 Level II BASIC
✅ **Fully Compatible**
- DATA, DIM, READ, RESTORE work identically
- Zero-based array indexing
- Multi-dimensional arrays supported

### Microsoft BASIC Variants
✅ **Compatible** with:
- GW-BASIC
- QuickBASIC
- IBM BASIC
- Commodore BASIC V2+

### Differences
⚠️ **Array Base**: This interpreter uses 0-based indexing. Some BASICs support `OPTION BASE 1` for 1-based indexing (not implemented here).

---

## Testing

### Test Files

1. **`06_read_data_restore.bas`** - Basic READ/DATA/RESTORE test
2. **`07_dim_arrays.bas`** - DIM and array operations test
3. **`test_data_dim_comprehensive.bas`** - Complete feature test

### Running Tests

```bash
./build/bin/basic-trs80 tests/basic_tests/06_read_data_restore.bas
./build/bin/basic-trs80 tests/basic_tests/07_dim_arrays.bas
./build/bin/basic-trs80 tests/test_data_dim_comprehensive.bas
```

### Expected Results
All tests should pass without errors, displaying correct values.

---

## Quick Reference

| Statement | Purpose | Example |
|-----------|---------|---------|
| `DATA` | Define data values | `DATA 10, 20, "text"` |
| `READ` | Read data into variables | `READ A, B$` |
| `RESTORE` | Reset data pointer | `RESTORE` |
| `DIM` | Declare array | `DIM A(10), B$(5,5)` |

---

## Implementation Status

✅ **All Features Implemented:**
- DATA statement parsing
- READ with type checking
- RESTORE functionality
- DIM for 1D, 2D, 3D arrays
- Numeric and string arrays
- Multiple arrays in one DIM statement

✅ **Tested and Verified:**
- Basic READ/DATA operations
- RESTORE and re-reading
- Single and multi-dimensional arrays
- Mixed data types
- Array operations in loops

---

**Documentation Version**: 1.0  
**Last Updated**: 2026-01-19  
**Interpreter Version**: 1.5.0  
**Feature Status**: ✅ Production Ready
