## DIM

**BASIC Set:**  
Level II BASIC

**Syntax:**
```basic
DIM array(size1[, size2, ...])
```

**Description:**  
Declares an array (or multi-dimensional array).

**Parameters:**
- `array`: Name of the array
- `size1, size2, ...`: Size of each dimension

**Example:**
```basic
10 DIM A(10)
20 FOR I = 1 TO 10
30 A(I) = I * I
40 NEXT I
50 PRINT A(5)
```

**Notes:**
- Arrays must be declared before use.
