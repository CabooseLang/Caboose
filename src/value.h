#ifndef caboose_value_h
#define caboose_value_h

#include "common.h"

typedef struct sObj Obj;
typedef struct sObjString ObjString;

typedef enum { VAL_BOOL, VAL_NIL, VAL_NUMBER, VAL_OBJ } ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#define BOOL_VAL(value) ((Value){ VAL_BOOL, { .boolean = value } })
#define NIL_VAL ((Value){ VAL_NIL, { .number = 0 } })
#define NUMBER_VAL(value) ((Value){ VAL_NUMBER, { .number = value } })
#define OBJ_VAL(object) ((Value){ VAL_OBJ, { .obj = (Obj*)object } })

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

void
initValueArray(ValueArray* array);
void
writeValueArray(ValueArray* array, Value value);
void
freeValueArray(ValueArray* array);
void
printValue(Value value);
bool
isFalsey(Value value);
bool
valuesEqual(Value a, Value b);

#endif
