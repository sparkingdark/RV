#ifndef VALUE_H
#define VALUE_H

#include "common.h"

typedef enum
{
  VAL_BOOL,
  VAL_NONE,
  VAL_NUMBER,
} ValueType;

typedef struct
{
  ValueType type;
  union { // The size of a union is the size of its largest field
    bool boolean;
    double number;
  } as;
} Value;

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NONE(value) ((value).type == VAL_NONE)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NONE_VAL ((Value){VAL_NONE, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})

// typedef double Value;

typedef struct
{
  int numOfAllocated;
  int actuallyInUse;
  Value *values;
} ValueArray;

bool areValuesEqual(Value a, Value b);
void initValueArray(ValueArray *array);
void writeValueArray(ValueArray *array, Value value);
void freeValueArray(ValueArray *array);
void printValue(Value value);

#endif
