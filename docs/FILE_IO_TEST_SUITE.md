# File I/O Test Suite for TRS-80 BASIC Interpreter

## Overview
Comprehensive test suite for file I/O keywords in the BASIC interpreter, including OPEN, CLOSE, PRINT#, INPUT#, and EOF.

## Test Files Created

### Test Data Files (Input)
- `test_input.txt` - Three sample text lines
- `test_numbers.txt` - Comma-separated numeric data
- `test_strings.txt` - String values (HELLO, WORLD, BASIC)
- `test_mixed.txt` - Mixed numeric and string data

### Test Programs & Expected Output

#### 09_file_io_basic.bas ✅
**Tests:** Basic OPEN/CLOSE/PRINT#/INPUT# operations
- Opens file for OUTPUT mode
- Writes string and numeric data
- Opens file for INPUT mode
- Reads and prints data back
- **Status:** PASSING

#### 10_file_io_sequential.bas ✅
**Tests:** Sequential file read/write operations
- Writes sequential numeric values in a loop (1-5, x10)
- Reads them back in a loop
- Verifies proper sequencing
- **Status:** PASSING

#### 11_file_io_multiple.bas ✅
**Tests:** Multiple simultaneously open files
- Opens two different files for output
- Writes to each file alternately
- Opens both files for input
- Reads from each file in sequence
- **Status:** PASSING

#### 12_file_io_eof.bas ✅
**Tests:** EOF (End-Of-File) detection
- Creates a test file with 3 values
- Reads values in a loop
- Uses EOF(filenum) function to detect end of file
- Counts how many values were read
- **Status:** PASSING

## Test Results Summary

```
PASS 09_file_io_basic.bas
PASS 10_file_io_sequential.bas
PASS 11_file_io_multiple.bas
PASS 12_file_io_eof.bas
```

**File I/O Tests: 4/4 PASSING**

## Overall Test Suite Status
**11/12 total tests passing**
- 1 pre-existing failure: 07_dim_arrays.bas (array value printing issue)

## Keywords Tested

### File Operations
- `OPEN filename FOR OUTPUT AS #handle` - Open file for writing
- `OPEN filename FOR INPUT AS #handle` - Open file for reading
- `CLOSE #handle` - Close open file

### File I/O
- `PRINT #handle, expression` - Write to file
- `INPUT #handle, variable` - Read from file

### File Status
- `EOF(handle)` - Check if at end of file

## Usage

Run the full test suite:
```bash
bash tests/basic_tests/run_tests.sh build/bin/basic-trs80
```

Run a specific test:
```bash
./build/bin/basic-trs80 tests/basic_tests/09_file_io_basic.bas
```

## Test Strategy Notes

The tests follow a progression from basic functionality to advanced features:
1. **Basic** - Simple open/write/read/close operations
2. **Sequential** - Multiple operations in loop patterns
3. **Multiple Files** - Handling multiple file handles simultaneously
4. **EOF Detection** - Proper end-of-file handling

Each test includes:
- Clear output for visual verification
- Expected output file (.out) for automated comparison
- Comments explaining what's being tested
- Assertion-like checks (e.g., "Read 3 values")
