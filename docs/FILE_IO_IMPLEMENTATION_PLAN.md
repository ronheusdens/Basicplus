# File I/O Implementation Plan

**TRS-80 Level II BASIC Interpreter**  
**Target Version**: 1.7.0  
**Feature**: Sequential File I/O (OPEN, CLOSE, INPUT#, PRINT#, EOF)

---

## Overview

Implement TRS-80 Disk BASIC / Microsoft BASIC compatible file I/O operations for reading and writing sequential text files.

---

## Features to Implement

### Core Statements

| Statement | Syntax | Description |
|-----------|--------|-------------|
| **OPEN** | `OPEN "filename" FOR INPUT AS #n` | Open file for reading |
| | `OPEN "filename" FOR OUTPUT AS #n` | Open file for writing |
| | `OPEN "filename" FOR APPEND AS #n` | Open file for appending |
| **CLOSE** | `CLOSE #n` | Close file handle n |
| | `CLOSE` | Close all open files |
| **INPUT#** | `INPUT #n, var1, var2, ...` | Read from file |
| **PRINT#** | `PRINT #n, expr1; expr2; ...` | Write to file |
| **EOF** | `EOF(n)` | Function: returns -1 if EOF, 0 otherwise |

### Optional Extensions
- `LINE INPUT #n, string$` - Read entire line into string
- `WRITE #n, expr1, expr2, ...` - Write with automatic commas/quotes
- Random access (later): `GET #n`, `PUT #n`

---

## Implementation Breakdown

### 1. New Keywords (5 keywords)

Add to `KeywordId` enum:
```c
KW_OPEN,        // OPEN statement
KW_CLOSE,       // CLOSE statement  
KW_FOR,         // FOR (in OPEN ... FOR INPUT)
KW_AS,          // AS (in OPEN ... AS #n)
KW_EOF,         // EOF() function
```

**Potential conflict**: `KW_FOR` already exists for FOR loops  
**Solution**: Context-sensitive parsing - check if preceded by OPEN

### 2. New Token Type

Add `TOK_HASH` for file number syntax `#n`:
```c
typedef enum {
    // ... existing types ...
    TOK_HASH,    // # symbol for file numbers
} TokenType;
```

### 3. File Handle Management

```c
#define MAX_FILES 10

typedef struct FileHandle {
    FILE *fp;
    char filename[256];
    int is_open;
    int is_input;   // 1=read, 0=write
    int eof_flag;
} FileHandle;

static FileHandle file_handles[MAX_FILES];
```

### 4. New Functions (8 functions)

```c
static void init_file_handles(void);
static int find_free_file_handle(void);
static int open_file(int handle, const char *filename, const char *mode);
static void close_file(int handle);
static void close_all_files(void);
static int is_file_open(int handle);
static int file_eof(int handle);
static int read_file_value(int handle, char *buffer, int maxlen);
```

### 5. Statement Implementations

#### OPEN Statement (~60 lines)
```basic
OPEN "test.txt" FOR INPUT AS #1
OPEN "output.txt" FOR OUTPUT AS #2
OPEN "log.txt" FOR APPEND AS #3
```

Parse sequence:
1. `OPEN` keyword
2. String expression (filename)
3. `FOR` keyword
4. Mode: `INPUT` / `OUTPUT` / `APPEND`
5. `AS` keyword  
6. `#` symbol
7. Number (file handle 0-9)

#### CLOSE Statement (~30 lines)
```basic
CLOSE #1
CLOSE #1, #2, #3
CLOSE
```

Parse sequence:
1. `CLOSE` keyword
2. Optional: `#` + number, repeat with comma
3. If no arguments, close all files

#### INPUT# Statement (~80 lines)
```basic
INPUT #1, A, B$, C
```

Similar to existing INPUT but reads from file:
1. `INPUT` keyword
2. `#` symbol
3. Number (file handle)
4. `,` comma
5. Variable list (like regular INPUT)

#### PRINT# Statement (~70 lines)
```basic
PRINT #2, "Result: "; X; Y
```

Similar to existing PRINT but writes to file:
1. `PRINT` keyword
2. `#` symbol
3. Number (file handle)
4. `,` comma
5. Expression list (like regular PRINT)

#### EOF Function (~20 lines)
```basic
IF EOF(1) THEN PRINT "End of file"
```

Add to `eval_function`:
```c
case KW_EOF:
    (*pos)++; // Skip EOF
    if (*pos < ntok && toks[*pos].type == TOK_LPAREN) (*pos)++;
    double handle = eval_expr(toks, pos, ntok);
    if (*pos < ntok && toks[*pos].type == TOK_RPAREN) (*pos)++;
    return file_eof((int)handle) ? -1.0 : 0.0;
```

---

## Token Burden Estimate

### Code Additions

| Component | Lines of Code | Token Estimate |
|-----------|---------------|----------------|
| Keywords/Enum additions | 10 | ~200 |
| File handle structure | 30 | ~600 |
| Helper functions (8) | 150 | ~3,000 |
| OPEN statement | 60 | ~1,200 |
| CLOSE statement | 30 | ~600 |
| INPUT# statement | 80 | ~1,600 |
| PRINT# statement | 70 | ~1,400 |
| EOF function | 20 | ~400 |
| Error handling | 40 | ~800 |
| **TOTAL** | **~490 lines** | **~9,800 tokens** |

### Documentation

| Document | Lines | Token Estimate |
|----------|-------|----------------|
| FILE_IO_REFERENCE.md | 400 | ~8,000 |
| Test programs | 200 | ~4,000 |
| CHANGELOG update | 50 | ~1,000 |
| QUICK_REFERENCE update | 30 | ~600 |
| **TOTAL** | **~680 lines** | **~13,600 tokens** |

### Testing & Debugging

| Activity | Token Estimate |
|----------|----------------|
| Initial implementation | ~10,000 |
| Debugging/fixes | ~5,000 |
| Integration testing | ~3,000 |
| **TOTAL** | **~18,000 tokens** |

---

## Overall Token Burden Estimate

| Phase | Tokens |
|-------|--------|
| **Implementation** | ~9,800 |
| **Documentation** | ~13,600 |
| **Testing/Debug** | ~18,000 |
| **TOTAL** | **~41,400 tokens** |

**Confidence**: ±20% (33,000 - 50,000 tokens)

**Context Usage**: ~4-5% of 1M token budget

---

## Implementation Phases

### Phase 1: Foundation (8,000 tokens)
- Add keywords and file handle structure
- Implement file management functions
- Add tokenizer support for `#` symbol

### Phase 2: OPEN/CLOSE (8,000 tokens)
- Implement OPEN statement parsing
- Implement CLOSE statement parsing
- Basic file open/close functionality
- Error handling

### Phase 3: PRINT# (6,000 tokens)
- Implement PRINT# statement
- File writing with formatting
- Test with output files

### Phase 4: INPUT# and EOF (10,000 tokens)
- Implement INPUT# statement
- Implement EOF() function
- File reading with type parsing
- Test with input files

### Phase 5: Testing & Documentation (9,400 tokens)
- Comprehensive test suite
- Reference documentation
- Update CHANGELOG and quick reference
- Integration testing with existing features

---

## Complexity Assessment

### Low Complexity (Simple)
- ✅ File handle array management
- ✅ EOF() function
- ✅ CLOSE statement

### Medium Complexity (Moderate)
- ⚠️ OPEN statement parsing (multiple keywords)
- ⚠️ PRINT# statement (file output)
- ⚠️ Error handling for file operations

### High Complexity (Challenging)
- 🔴 INPUT# statement (parsing from file, type conversion)
- 🔴 Tokenizer modification for `#` symbol
- 🔴 Context-sensitive `FOR` keyword (OPEN vs FOR loop)

---

## Potential Issues

### 1. Keyword Conflicts
**Issue**: `FOR` already used in FOR loops  
**Solution**: Context-sensitive parsing - check previous token

### 2. Hash Symbol Tokenization
**Issue**: `#` might be tokenized as operator or comment  
**Solution**: Add explicit `TOK_HASH` case in tokenizer

### 3. File Path Handling
**Issue**: Cross-platform path compatibility  
**Solution**: Use relative paths, normalize separators

### 4. File Handle Limits
**Issue**: Limited to 10 open files  
**Solution**: TRS-80 compatible limit, document clearly

### 5. EOF Detection
**Issue**: Different EOF behavior across platforms  
**Solution**: Use `feof()` and track read status

---

## Testing Strategy

### Test Files Needed

1. **test_file_io_basic.bas** - Basic open/close/read/write
2. **test_file_io_sequential.bas** - Sequential read/write
3. **test_file_io_eof.bas** - EOF detection
4. **test_file_io_errors.bas** - Error handling
5. **test_file_io_multiple.bas** - Multiple open files

### Sample Test Data Files

- `test_input.txt` - Sample input data
- `test_numbers.txt` - Numeric data
- `test_strings.txt` - String data
- `test_mixed.txt` - Mixed data types

---

## Compatibility Notes

### TRS-80 Disk BASIC
✅ Sequential file I/O syntax compatible  
✅ File handle numbering (#0-#9)  
✅ EOF() function behavior  
⚠️ Random access not in scope (GET/PUT)

### Microsoft BASIC Variants
✅ GW-BASIC compatible  
✅ QuickBASIC compatible  
✅ QBasic compatible  
⚠️ Binary mode not supported initially

---

## Example Usage

### Writing to File
```basic
10 OPEN "output.txt" FOR OUTPUT AS #1
20 FOR I = 1 TO 10
30   PRINT #1, I; I*I
40 NEXT I
50 CLOSE #1
60 END
```

### Reading from File
```basic
10 OPEN "data.txt" FOR INPUT AS #1
20 IF EOF(1) THEN GOTO 60
30 INPUT #1, A, B$
40 PRINT A; B$
50 GOTO 20
60 CLOSE #1
70 END
```

### Multiple Files
```basic
10 OPEN "input.txt" FOR INPUT AS #1
20 OPEN "output.txt" FOR OUTPUT AS #2
30 IF EOF(1) THEN GOTO 70
40 INPUT #1, X
50 PRINT #2, X * 2
60 GOTO 30
70 CLOSE #1
80 CLOSE #2
90 END
```

---

## Risk Assessment

| Risk | Probability | Impact | Mitigation |
|------|-------------|--------|------------|
| Tokenizer conflicts | Medium | High | Careful `#` handling |
| `FOR` keyword ambiguity | Low | Medium | Context checking |
| File I/O errors | High | Medium | Robust error handling |
| Platform differences | Low | Low | Use standard C I/O |
| EOF edge cases | Medium | Medium | Thorough testing |

---

## Success Criteria

✅ All 5 core statements implemented and working  
✅ Can read/write text files sequentially  
✅ EOF detection works correctly  
✅ Error handling for file operations  
✅ All test cases pass  
✅ Compatible with TRS-80 Disk BASIC syntax  
✅ Documentation complete  
✅ macOS app rebuilt and tested

---

## Recommendations

### ✅ Implemented
- ✅ Sequential file I/O (OPEN, CLOSE, INPUT#, PRINT#, EOF)
- ✅ Error handling for file operations
- ✅ Basic test suite (16 tests, 100% passing)
- ✅ Random access (GET, PUT, LOC, LOF)
- ✅ LINE INPUT# (reads entire lines from files)
- ✅ WRITE# statement (formatted output with comma/quote handling)
- ✅ String concatenation with + operator
- ✅ Expression evaluation in PRINT# (fixed bug with X * 2)

### Not Planned (Out of Scope)
- ❌ Binary mode operations (redundant - GET/PUT provide full byte access)
- ❌ File existence checking (handled via error codes and EOF detection)
- ❌ Directory operations (outside TRS-80 Disk BASIC specification)
- ❌ Multiple simultaneous file modes on same handle (edge case, not needed)

### Complete
- ✅ All core file I/O functionality
- ✅ String operations (concatenation)
- ✅ Numeric expressions in file operations
- ✅ EOF detection and graceful error handling
- ✅ Full compatibility with TRS-80 Disk BASIC syntax

---

## Implementation Status

**Completion**: 100% of core functionality
**Tests Passing**: 16/16 (100%)
**Code Quality**: Zero compiler warnings
**Bugs Fixed**:
- Fixed PRINT# expression evaluation (was multiplying by 10 instead of 2)
- Fixed string concatenation operator support
- Fixed Ctrl-C return to prompt in interactive mode
- Removed unused function find_free_file_handle

---

## Timeline Actual vs Estimate

| Phase | Estimated | Actual | Status |
|-------|-----------|--------|--------|
| Implementation | 2-3 iterations | 4 iterations | ✅ Complete |
| Testing | 1 iteration | 1 iteration | ✅ Complete |
| Bug Fixes | N/A | 2 iterations | ✅ Complete |
| **Total** | **4-5 iterations** | **7 iterations** | **✅ Complete** |

With current token budget: **~925K tokens remaining**  
After this feature: **~450K tokens remaining** (estimated)

---

**Prepared by**: AI Assistant  
**Date**: 2026-01-19  
**Status**: ✅ IMPLEMENTATION COMPLETE  
**Last Updated**: 2026-01-23  
**Actual Completion**: Same session (7 tool call cycles)
