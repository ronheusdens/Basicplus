# INPUT

**BASIC Set:** Level II BASIC

## Syntax
```
INPUT ["prompt";] variable [, variable ...]
```

## Description
Prompts the user for input and assigns the entered value(s) to the specified variable(s).

## Parameters
- `"prompt"` (optional): Text to display before input
- `variable`: Variable(s) to assign input values

## Example
```
10 INPUT "Enter a number"; N
20 PRINT N
```

## Notes
- Multiple variables can be read in one statement.
- String and numeric variables are supported.
