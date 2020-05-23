#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef uint16_t mvm_Value;
typedef uint16_t mvm_VMExportID;
typedef uint16_t mvm_HostFunctionID;

typedef enum mvm_TeError {
  MVM_E_SUCCESS,
  MVM_E_UNEXPECTED,
  MVM_E_MALLOC_FAIL,
  MVM_E_ALLOCATION_TOO_LARGE,
  MVM_E_INVALID_ADDRESS,
  MVM_E_COPY_ACROSS_BUCKET_BOUNDARY,
  MVM_E_FUNCTION_NOT_FOUND,
  MVM_E_INVALID_HANDLE,
  MVM_E_STACK_OVERFLOW,
  MVM_E_UNRESOLVED_IMPORT,
  MVM_E_ATTEMPT_TO_WRITE_TO_ROM,
  MVM_E_INVALID_ARGUMENTS,
  MVM_E_TYPE_ERROR,
  MVM_E_TARGET_NOT_CALLABLE,
  MVM_E_HOST_ERROR,
  MVM_E_NOT_IMPLEMENTED,
  MVM_E_HOST_RETURNED_INVALID_VALUE,
  MVM_E_ASSERTION_FAILED,
  MVM_E_INVALID_BYTECODE,
  MVM_E_UNRESOLVED_EXPORT,
  MVM_E_RANGE_ERROR,
  MVM_E_DETACHED_EPHEMERAL,
  MVM_E_TARGET_IS_NOT_A_VM_FUNCTION,
  MVM_E_FLOAT64,
  MVM_E_NAN,
  MVM_E_NEG_ZERO,
  MVM_E_OPERATION_REQUIRES_FLOAT_SUPPORT,
  MVM_E_BYTECODE_CRC_FAIL,
  MVM_E_BYTECODE_REQUIRES_FLOAT_SUPPORT,
} mvm_TeError;

typedef enum mvm_TeType {
  VM_T_UNDEFINED,
  VM_T_NULL,
  VM_T_BOOLEAN,
  VM_T_NUMBER,
  VM_T_STRING,
  VM_T_BIG_INT,
  VM_T_SYMBOL,
  VM_T_FUNCTION,
  VM_T_OBJECT,
  VM_T_ARRAY,
} mvm_TeType;

typedef struct mvm_VM mvm_VM;

typedef mvm_TeError (*mvm_TfHostFunction)(mvm_VM* vm, mvm_HostFunctionID hostFunctionID, mvm_Value* result, mvm_Value* args, uint8_t argCount);

typedef mvm_TeError (*mvm_TfResolveImport)(mvm_HostFunctionID hostFunctionID, void* context, mvm_TfHostFunction* out_hostFunction);

/**
 * A handle holds a value that must not be garbage collected.
 */
typedef struct mvm_Handle { struct mvm_Handle* _next; mvm_Value _value; } mvm_Handle;

#include "microvium_port.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Restore the state of a virtual machine from a snapshot */
mvm_TeError mvm_restore(mvm_VM** result, MVM_PROGMEM_P snapshotBytecode, size_t bytecodeSize, void* context, mvm_TfResolveImport resolveImport);
void mvm_free(mvm_VM* vm);

/**
 * Call a function in the VM
 *
 * @param func The function value to call
 * @param out_result Where to put the result, or NULL if the result is not needed
 * @param args Pointer to arguments array, or NULL if no arguments
 * @param argCount Number of arguments
 */
mvm_TeError mvm_call(mvm_VM* vm, mvm_Value func, mvm_Value* out_result, mvm_Value* args, uint8_t argCount);

void* mvm_getContext(mvm_VM* vm);

void mvm_initializeHandle(mvm_VM* vm, mvm_Handle* handle); // Handle must be released by mvm_releaseHandle
void mvm_cloneHandle(mvm_VM* vm, mvm_Handle* target, const mvm_Handle* source); // Target must be released by mvm_releaseHandle
mvm_TeError mvm_releaseHandle(mvm_VM* vm, mvm_Handle* handle);
static inline mvm_Value mvm_handleGet(mvm_Handle* handle) { return handle->_value; }
static inline void mvm_handleSet(mvm_Handle* handle, mvm_Value value) { handle->_value = value; }

/**
 * Roughly like the `typeof` operator in JS, except with distinct values for
 * null and arrays
 */
mvm_TeType mvm_typeOf(mvm_VM* vm, mvm_Value value);

/**
 * Converts the value to a string encoded as UTF-8.
 *
 * @param out_sizeBytes Returns the length of the string in bytes, or provide NULL if not needed.
 * @return A pointer to the string data in VM memory.
 *
 * Note: for convenience, the returned data has an extra null character appended
 * to the end of it, so that the result is directly usable in printf, strcpy,
 * etc. The returned size in bytes is the size of the original string data,
 * excluding the extra null.
 *
 * The string data itself is permitted to contain nulls or any other data. For
 * example, if the string value is "abc\0", the size returned is "4", and the
 * returned pointer points to the data "abc\0\0" (i.e. with the extra safety
 * null beyond the user-provided data).
 *
 * The memory pointed to by the return value is transient: it is only guaranteed
 * to exist until the next garbage collection cycle. See
 * [memory-management.md](https://github.com/coder-mike/microvium/blob/master/doc/native-vm/memory-management.md)
 * for details.
 */
const char* mvm_toStringUtf8(mvm_VM* vm, mvm_Value value, size_t* out_sizeBytes);

/**
 * Convert the value to a bool based on its truthiness.
 *
 * See https://developer.mozilla.org/en-US/docs/Glossary/Truthy
 */
bool mvm_toBool(mvm_VM* vm, mvm_Value value);

/**
 * Converts the value to a 32-bit signed integer.
 *
 * The result of this should be the same as `value|0` in JavaScript code.
 */
int32_t mvm_toInt32(mvm_VM* vm, mvm_Value value);

#if MVM_SUPPORT_FLOAT
/**
 * Converts the value to a number.
 *
 * The result of this should be the same as `+value` in JavaScript code.
 */
MVM_FLOAT64 mvm_toFloat64(mvm_VM* vm, mvm_Value value);

mvm_Value mvm_newNumber(mvm_VM* vm, MVM_FLOAT64 value);
#endif

bool mvm_isNaN(mvm_Value value);

extern const mvm_Value mvm_undefined;
extern const mvm_Value mvm_null;
mvm_Value mvm_newBoolean(bool value);
mvm_Value mvm_newInt32(mvm_VM* vm, int32_t value);
mvm_Value mvm_newString(mvm_VM* vm, const char* valueUtf8, size_t sizeBytes);

/**
 * Resolves (finds) the values exported by the VM, identified by ID.
 *
 * @param ids An array of `count` IDs to look up.
 * @param results An array of `count` output values that result from each
 * lookup
 *
 * Note: Exports are immutable (shallow immutable), so they don't need to be
 * captured by a mvm_Handle. In typical usage, exports will each be function
 * values, but any value type is valid.
 */
mvm_TeError mvm_resolveExports(mvm_VM* vm, const mvm_VMExportID* ids, mvm_Value* results, uint8_t count);

/** Run the garbage collector to free up memory. (Can only be executed when the VM is idle) */
void mvm_runGC(mvm_VM* vm);

/**
 * Compares two values for equality. The same semantics as JavaScript `===`
 */
bool mvm_equal(mvm_VM* vm, mvm_Value a, mvm_Value b);

#ifdef __cplusplus
}
#endif
