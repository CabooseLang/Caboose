//> Hash Tables table-h
#ifndef caboose_table_h
#define caboose_table_h

#include "common.h"
#include "value.h"
//> entry

typedef struct {
  ObjString* key;
  Value value;
} Entry;
//< entry

typedef struct {
  int count;
/* Hash Tables table-h < Optimization not-yet
  int capacity;
*/
//> Optimization not-yet
  int capacityMask;
//< Optimization not-yet
  Entry* entries;
} Table;

void initTable(Table* table);
void freeTable(Table* table);
bool tableGet(Table* table, ObjString* key, Value* value);
bool tableSet(Table* table, ObjString* key, Value value);
//< table-set-h
//> table-delete-h
bool tableDelete(Table* table, ObjString* key);
//< table-delete-h
//> table-add-all-h
void tableAddAll(Table* from, Table* to);
//< table-add-all-h
//> table-find-string-h
ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash);
//< table-find-string-h
//> Garbage Collection not-yet

void tableRemoveWhite(Table* table);
void grayTable(Table* table);
//< Garbage Collection not-yet

//< init-table-h
#endif
