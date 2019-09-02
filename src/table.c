#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table) {
  table->count = 0;
/* Hash Tables table-c < Optimization not-yet
  table->capacity = 0;
*/
  table->capacityMask = -1;
  table->entries = NULL;
}
void freeTable(Table* table) {
/* Hash Tables free-table < Optimization not-yet
  FREE_ARRAY(Entry, table->entries, table->capacity);
*/
  FREE_ARRAY(Entry, table->entries, table->capacityMask + 1);
  initTable(table);
}
/* Hash Tables find-entry < Optimization not-yet
static Entry* findEntry(Entry* entries, int capacity,
                        ObjString* key) {
*/
static Entry* findEntry(Entry* entries, int capacityMask,
                        ObjString* key) {
/* Hash Tables find-entry < Optimization not-yet
  uint32_t index = key->hash % capacity;
*/
  uint32_t index = key->hash & capacityMask;
  Entry* tombstone = NULL;
  
  for (;;) {
    Entry* entry = &entries[index];

/* Hash Tables find-entry < Hash Tables find-tombstone
    if (entry->key == key || entry->key == NULL) {
      return entry;
    }
*/
    if (entry->key == NULL) {
      if (IS_NIL(entry->value)) {
                return tombstone != NULL ? tombstone : entry;
      } else {
                if (tombstone == NULL) tombstone = entry;
      }
    } else if (entry->key == key) {
            return entry;
    }

/* Hash Tables find-entry < Optimization not-yet
    index = (index + 1) % capacity;
*/
    index = (index + 1) & capacityMask;
  }
}
bool tableGet(Table* table, ObjString* key, Value* value) {
  if (table->entries == NULL) return false;

/* Hash Tables table-get < Optimization not-yet
  Entry* entry = findEntry(table->entries, table->capacity, key);
*/
  Entry* entry = findEntry(table->entries, table->capacityMask, key);
  if (entry->key == NULL) return false;

  *value = entry->value;
  return true;
}
/* Hash Tables table-adjust-capacity < Optimization not-yet
static void adjustCapacity(Table* table, int capacity) {
*/
static void adjustCapacity(Table* table, int capacityMask) {
/* Hash Tables table-adjust-capacity < Optimization not-yet
  Entry* entries = ALLOCATE(Entry, capacity);
  for (int i = 0; i < capacity; i++) {
*/
  Entry* entries = ALLOCATE(Entry, capacityMask + 1);
  for (int i = 0; i <= capacityMask; i++) {
    entries[i].key = NULL;
    entries[i].value = NIL_VAL;
  }
  
  table->count = 0;
/* Hash Tables re-hash < Optimization not-yet
  for (int i = 0; i < table->capacity; i++) {
*/
  for (int i = 0; i <= table->capacityMask; i++) {
    Entry* entry = &table->entries[i];
    if (entry->key == NULL) continue;

/* Hash Tables re-hash < Optimization not-yet
    Entry* dest = findEntry(entries, capacity, entry->key);
*/
    Entry* dest = findEntry(entries, capacityMask, entry->key);
    dest->key = entry->key;
    dest->value = entry->value;
    table->count++;
  }

/* Hash Tables free-old-array < Optimization not-yet
  FREE_ARRAY(Entry, table->entries, table->capacity);
*/
  FREE_ARRAY(Entry, table->entries, table->capacityMask + 1);
  table->entries = entries;
/* Hash Tables table-adjust-capacity < Optimization not-yet
  table->capacity = capacity;
*/
  table->capacityMask = capacityMask;
}
bool tableSet(Table* table, ObjString* key, Value value) {
/* Hash Tables table-set-grow < Optimization not-yet
  if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
    int capacity = GROW_CAPACITY(table->capacity);
    adjustCapacity(table, capacity);
*/
  if (table->count + 1 > (table->capacityMask + 1) * TABLE_MAX_LOAD) {
        int capacityMask = GROW_CAPACITY(table->capacityMask + 1) - 1;
    adjustCapacity(table, capacityMask);
  }

/* Hash Tables table-set < Optimization not-yet
  Entry* entry = findEntry(table->entries, table->capacity, key);
*/
  Entry* entry = findEntry(table->entries, table->capacityMask, key);
  
  bool isNewKey = entry->key == NULL;
/* Hash Tables table-set < Hash Tables set-increment-count
  if (isNewKey) table->count++;
*/
  if (isNewKey && IS_NIL(entry->value)) table->count++;

  entry->key = key;
  entry->value = value;
  return isNewKey;
}
bool tableDelete(Table* table, ObjString* key) {
  if (table->count == 0) return false;

  /* Hash Tables table-delete < Optimization not-yet
  Entry* entry = findEntry(table->entries, table->capacity, key);
*/
  Entry* entry = findEntry(table->entries, table->capacityMask, key);
  if (entry->key == NULL) return false;

    entry->key = NULL;
  entry->value = BOOL_VAL(true);

  return true;
}
void tableAddAll(Table* from, Table* to) {
/* Hash Tables table-add-all < Optimization not-yet
  for (int i = 0; i < from->capacity; i++) {
*/
  for (int i = 0; i <= from->capacityMask; i++) {
    Entry* entry = &from->entries[i];
    if (entry->key != NULL) {
      tableSet(to, entry->key, entry->value);
    }
  }
}
ObjString* tableFindString(Table* table, const char* chars, int length,
                           uint32_t hash) {
    if (table->entries == NULL) return NULL;

/* Hash Tables table-find-string < Optimization not-yet
  uint32_t index = hash % table->capacity;
*/
  uint32_t index = hash & table->capacityMask;

  for (;;) {
    Entry* entry = &table->entries[index];

    if (entry->key == NULL) {
            if (IS_NIL(entry->value)) return NULL;
    } else if (entry->key->length == length &&
        entry->key->hash == hash &&
        memcmp(entry->key->chars, chars, length) == 0) {
            return entry->key;
    }

    /* Hash Tables table-find-string < Optimization not-yet
    index = (index + 1) % table->capacity;
*/
    index = (index + 1) & table->capacityMask;
  }
}

void tableRemoveWhite(Table* table) {
/* Garbage Collection not-yet < Optimization not-yet
  for (int i = 0; i < table->capacity; i++) {
*/
  for (int i = 0; i <= table->capacityMask; i++) {
    Entry* entry = &table->entries[i];
    if (entry->key != NULL && !entry->key->obj.isDark) {
      tableDelete(table, entry->key);
    }
  }
}

void grayTable(Table* table) {
/* Garbage Collection not-yet < Optimization not-yet
  for (int i = 0; i < table->capacity; i++) {
*/
  for (int i = 0; i <= table->capacityMask; i++) {
    Entry* entry = &table->entries[i];
    grayObject((Obj*)entry->key);
    grayValue(entry->value);
  }
}
