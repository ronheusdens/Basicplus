# IF...THEN

**BASIC Set:** Level II BASIC

## Syntax
```
IF condition THEN statement
```

## Description
Executes the statement if the condition is true. The statement can be a line number (for GOTO) or any valid BASIC statement.

## Parameters
- `condition`: An expression that evaluates to true or false
- `statement`: Statement or line number to execute if condition is true

## Example
```
10 INPUT A
20 IF A > 0 THEN PRINT "Positive"
```

## Notes
- Only a single statement is allowed after THEN.
- ELSE is not supported in Level II BASIC.
