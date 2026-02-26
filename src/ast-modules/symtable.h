#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "common.h"
#include "ast.h"

/*
 * Symbol table entry
 */
typedef struct
{
    char *name;
    VarType type;
    int is_array;
    int *dimensions;
    int num_dimensions;
    int is_function;
    int line_defined;
} Symbol;

/*
 * Symbol table
 */
typedef struct
{
    Symbol *symbols;
    int num_symbols;
    int capacity;
} SymbolTable;

/* Symbol table functions */

SymbolTable *symtable_create(void);
void symtable_free(SymbolTable *table);

Symbol *symtable_lookup(SymbolTable *table, const char *name);
void symtable_insert(SymbolTable *table, const char *name, VarType type);
void symtable_insert_array(SymbolTable *table, const char *name,
                           VarType type, int *dimensions, int num_dims);

int symtable_analyze_program(SymbolTable *table, Program *prog);

#endif /* SYMTABLE_H */
