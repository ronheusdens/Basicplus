# FOR...NEXT

**BASIC Set:** Level II BASIC

## Syntax
```
FOR variable = start TO end [STEP increment]
    ...
NEXT [variable]
```

## Description
Begins a counted loop. The variable is assigned the start value and incremented by the STEP value (default 1) until it exceeds the end value. The loop body executes for each value.

## Parameters
- `variable`: Numeric loop variable (auto-created if not defined)
- `start`: Initial value
- `end`: Final value
- `STEP increment` (optional): Amount to increment each time (default 1)

## Example
```
FOR I = 1 TO 5
  PRINT I
NEXT I
```

## Notes
- The loop variable is updated after each iteration.
- Exiting the loop early (e.g., with GOTO) is allowed but not recommended.
- Nested FOR...NEXT loops are supported.
