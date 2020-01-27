#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

void
initValueArray(ValueArray* array) {
    array->capacity = 0;
    array->count = 0;
    array->values = NULL;
}

void
writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values =
          GROW_ARRAY(array->values, Value, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void
freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void
printValue(Value value) {
    printf("%s", valueToString(value));
}

bool
isFalsey(Value value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

bool
valuesEqual(Value a, Value b) {
    if (a.type != b.type)
        return false;

    switch (a.type) {
        case VAL_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:
            return true;
        case VAL_NUMBER:
            return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:
            return AS_OBJ(a) == AS_OBJ(b);
    }
}

char*
valueToString(Value value) {
    if (IS_BOOL(value)) {
        char* str = AS_BOOL(value) ? "true" : "false";
        char* boolString = malloc(sizeof(char) * (strlen(str) + 1));
        snprintf(boolString, strlen(str) + 1, "%s", str);
        return boolString;
    } else if (IS_NIL(value)) {
        char* nilString = malloc(sizeof(char) * 4);
        snprintf(nilString, 4, "%s", "nil");
        return nilString;
    } else if (IS_NUMBER(value)) {
        double number = AS_NUMBER(value);
        int numberStringLength = snprintf(NULL, 0, "%.15g", number) + 1;
        char* numberString = malloc(sizeof(char) * numberStringLength);
        snprintf(numberString, numberStringLength, "%.15g", number);
        return numberString;
    } else if (IS_OBJ(value))
        return objectToString(value);

    char* unknown = malloc(sizeof(char) * 8);
    snprintf(unknown, 7, "%s", "unknown");
    return unknown;
}