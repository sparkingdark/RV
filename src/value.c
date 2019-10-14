#include <stdio.h>
#include "memory.h"
#include "value.h"

void initValueArray(ValueArray *array)
{
  array->values = NULL;
  array->numOfAllocated = 0;
  array->actuallyInUse = 0;
}

void writeValueArray(ValueArray *array, Value value)
{
  if (array->numOfAllocated < array->actuallyInUse + 1)
  {
    int oldnumOfAllocated = array->numOfAllocated;

    array->numOfAllocated = GROW_NUM_OF_ALLOCATED(oldnumOfAllocated);
    array->values = GROW_ARRAY(array->values, Value, oldnumOfAllocated, array->numOfAllocated);
  }

  array->values[array->actuallyInUse] = value;
  ++array->actuallyInUse;
}

void freeValueArray(ValueArray *array)
{
  FREE_ARRAY(Value, array->values, array->numOfAllocated);
  initValueArray(array);
}

void printValue(Value value)
{
  switch (value.type)
  {
  case VAL_BOOL:
    printf(AS_BOOL(value) ? "true" : "false");
    break;
  case VAL_NONE:
    printf("none");
    break;
  case VAL_NUMBER:
    printf("%g", AS_NUMBER(value));
    break;
  }
}

bool areValuesEqual(Value a, Value b)
{
  if (a.type != b.type)
    return false;

  switch (a.type)
  {
  case VAL_BOOL:
    return AS_BOOL(a) == AS_BOOL(b);
  case VAL_NONE:
    return true;
  case VAL_NUMBER:
    return AS_NUMBER(a) == AS_NUMBER(b);
  }
}
