#include "collections.h"
#include "memory.h"
#include "vm.h"

// This is needed for list deepCopy
ObjDict *copyDict(ObjDict *oldDict, bool shallow);

static bool pushListItem(int argCount) {
	if (argCount != 2) {
		runtimeError("push() takes 2 arguments (%d given)", argCount);
		return false;
	}

	Value listItem = pop();

	ObjList *list = AS_LIST(pop());
	writeValueArray(&list->values, listItem);
	push(NIL_VAL);

	return true;
}

static bool insertListItem(int argCount) {
	if (argCount != 3) {
		runtimeError("insert() takes 3 arguments (%d given)", argCount);
		return false;
	}

	if (!IS_NUMBER(peek(0))) {
		runtimeError("insert() third argument must be a number");
		return false;
	}

	int index = AS_NUMBER(pop());
	Value insertValue = pop();
	ObjList *list = AS_LIST(pop());

	if (index < 0 || index > list->values.count) {
		runtimeError(
			"Index passed to insert() is out of bounds for the list given");
		return false;
	}

	if (list->values.capacity < list->values.count + 1) {
		int oldCapacity = list->values.capacity;
		list->values.capacity = GROW_CAPACITY(oldCapacity);
		list->values.values = GROW_ARRAY(list->values.values, Value, oldCapacity, list->values.capacity);
	}

	list->values.count++;

	for (int i = list->values.count - 1; i > index; --i)
		list->values.values[i] = list->values.values[i - 1];

	list->values.values[index] = insertValue;
	push(NIL_VAL);

	return true;
}

static bool popListItem(int argCount) {
	if (argCount < 1 || argCount > 2) {
		runtimeError("pop() takes either 1 or 2 arguments (%d  given)", argCount);
		return false;
	}

	ObjList *list;
	Value last;

	if (argCount == 1) {
		if (!IS_LIST(peek(0))) {
			runtimeError("pop() only takes a list as an argument");
			return false;
		}

		list = AS_LIST(pop());

		if (list->values.count == 0) {
			runtimeError("pop() called on an empty list");
			return false;
		}

		last = list->values.values[list->values.count - 1];
	} else {
		if (!IS_LIST(peek(1))) {
			runtimeError("pop() only takes a list as an argument");
			return false;
		}

		if (!IS_NUMBER(peek(0))) {
			runtimeError("pop() index argument must be a number");
			return false;
		}

		int index = AS_NUMBER(pop());
		list = AS_LIST(pop());

		if (list->values.count == 0) {
			runtimeError("pop() called on an empty list");
			return false;
		}

		if (index < 0 || index > list->values.count) {
			runtimeError("Index passed to pop() is out of bounds for the list given");
			return false;
		}

		last = list->values.values[index];

		for (int i = index; i < list->values.count - 1; ++i)
			list->values.values[i] = list->values.values[i + 1];
	}
	list->values.count--;
	push(last);

	return true;
}

static bool containsListItem(int argCount) {
	if (argCount != 2) {
		runtimeError("contains() takes 2 arguments (%d  given)", argCount);
		return false;
	}

	Value search = pop();
	ObjList *list = AS_LIST(pop());

	for (int i = 0; i < list->values.capacity; ++i) {
		if (!list->values.values[i])
			continue;

		if (list->values.values[i] == search) {
			push(TRUE_VAL);
			return true;
		}
	}

	push(FALSE_VAL);
	return true;
}

ObjList *copyList(ObjList *oldList, bool shallow) {
	ObjList *newList = initList();

	for (int i = 0; i < oldList->values.count; ++i) {
		Value val = oldList->values.values[i];

		if (!shallow) {
			if (IS_DICT(val))
				val = OBJ_VAL(copyDict(AS_DICT(val), false));
			else if (IS_LIST(val))
				val = OBJ_VAL(copyList(AS_LIST(val), false));
		}

		writeValueArray(&newList->values, val);
	}

	return newList;
}

static bool copyListShallow(int argCount) {
	if (argCount != 1) {
		runtimeError("copy() takes 1 argument (%d  given)", argCount);
		return false;
	}

	ObjList *oldList = AS_LIST(pop());
	push(OBJ_VAL(copyList(oldList, true)));

	return true;
}

static bool copyListDeep(int argCount) {
	if (argCount != 1) {
		runtimeError("deepCopy() takes 1 argument (%d  given)", argCount);
		return false;
	}

	ObjList *oldList = AS_LIST(pop());
	push(OBJ_VAL(copyList(oldList, false)));

	return true;
}

bool listMethods(char *method, int argCount) {
	if (strcmp(method, "push") == 0)
		return pushListItem(argCount);
	else if (strcmp(method, "insert") == 0)
		return insertListItem(argCount);
	else if (strcmp(method, "pop") == 0)
		return popListItem(argCount);
	else if (strcmp(method, "contains") == 0)
		return containsListItem(argCount);
	else if (strcmp(method, "copy") == 0)
		return copyListShallow(argCount);
	else if (strcmp(method, "deepCopy") == 0)
		return copyListDeep(argCount);

	runtimeError("List has no method %s()", method);
	return false;
}

static bool getDictItem(int argCount) {
	if (argCount != 3) {
		runtimeError("get() takes 3 arguments (%d  given)", argCount);
		return false;
	}

	Value defaultValue = pop();

	if (!IS_STRING(peek(0))) {
		runtimeError("Key passed to get() must be a string");
		return false;
	}

	Value key = pop();
	ObjDict *dict = AS_DICT(pop());

	Value ret = searchDict(dict, AS_CSTRING(key));

	if (ret == NIL_VAL)
		push(defaultValue);
	else
		push(ret);

	return true;
}

static uint32_t hash(char *str) {
	uint32_t hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;

	return hash;
}

static bool removeDictItem(int argCount) {
	if (argCount != 2) {
		runtimeError("remove() takes 2 arguments (%d  given)", argCount);
		return false;
	}

	if (!IS_STRING(peek(0))) {
		runtimeError("Key passed to remove() must be a string");
		return false;
	}

	char *key = AS_CSTRING(pop());
	ObjDict *dict = AS_DICT(pop());

	int index = hash(key) % dict->capacity;

	while (dict->items[index] && strcmp(dict->items[index]->key, key) != 0) {
		index++;
		if (index == dict->capacity)
			index = 0;
	}

	if (dict->items[index]) {
		dict->items[index]->deleted = true;
		dict->count--;
		push(NIL_VAL);

		if (dict->capacity != 8 && dict->count * 100 / dict->capacity <= 35)
			resizeDict(dict, false);

		return true;
	}

	runtimeError("Key '%s' passed to remove() does not exist", key);
	return false;
}

static bool dictItemExists(int argCount) {
	if (argCount != 2) {
		runtimeError("exists() takes 2 arguments (%d  given)", argCount);
		return false;
	}

	if (!IS_STRING(peek(0))) {
		runtimeError("Key passed to exists() must be a string");
		return false;
	}

	char *key = AS_CSTRING(pop());
	ObjDict *dict = AS_DICT(pop());

	for (int i = 0; i < dict->capacity; ++i) {
		if (!dict->items[i])
			continue;

		if (strcmp(dict->items[i]->key, key) == 0) {
			push(TRUE_VAL);
			return true;
		}
	}

	push(FALSE_VAL);
	return true;
}

ObjDict *copyDict(ObjDict *oldDict, bool shallow) {
	ObjDict *newDict = initDict();

	for (int i = 0; i < oldDict->capacity; ++i) {
		if (oldDict->items[i] == NULL)
			continue;

		Value val = oldDict->items[i]->item;

		if (!shallow) {
			if (IS_DICT(val))
				val = OBJ_VAL(copyDict(AS_DICT(val), false));
			else if (IS_LIST(val))
				val = OBJ_VAL(copyList(AS_LIST(val), false));
		}

		insertDict(newDict, oldDict->items[i]->key, val);
	}

	return newDict;
}

static bool copyDictShallow(int argCount) {
	if (argCount != 1) {
		runtimeError("copy() takes 1 argument (%d  given)", argCount);
		return false;
	}

	ObjDict *oldDict = AS_DICT(pop());
	push(OBJ_VAL(copyDict(oldDict, true)));

	return true;
}

static bool copyDictDeep(int argCount) {
	if (argCount != 1) {
		runtimeError("deepCopy() takes 1 argument (%d  given)", argCount);
		return false;
	}

	ObjDict *oldDict = AS_DICT(pop());
	push(OBJ_VAL(copyDict(oldDict, false)));

	return true;
}

bool dictMethods(char *method, int argCount) {
	if (strcmp(method, "get") == 0)
		return getDictItem(argCount);
	else if (strcmp(method, "remove") == 0)
		return removeDictItem(argCount);
	else if (strcmp(method, "exists") == 0)
		return dictItemExists(argCount);
	else if (strcmp(method, "copy") == 0)
		return copyDictShallow(argCount);
	else if (strcmp(method, "deepCopy") == 0)
		return copyDictDeep(argCount);

	runtimeError("Dict has no method %s()", method);
	return false;
}
