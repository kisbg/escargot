/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

 #include "Escargot.h"
 #include "runtime/GlobalObject.h"
 #include "runtime/Context.h"
 #include "runtime/Object.h"
 #include "runtime/NativeFunctionObject.h"
 #include "SharedArrayBufferObject.h"
 #include "interpreter/ByteCodeInterpreter.h"
 #include "ArrayObject.h"
 #include "TypedArrayObject.h"


 namespace Escargot {


Value validateSharedIntergerTypedArray(ExecutionState& state, Value* typedArray, bool onlyInt32 = false)
{
  const StaticStrings* strings = &state.context()->staticStrings();
  if (!typedArray->isObject()) {
      ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, strings->Atomics.string(), false, String::emptyString, "%s: The target is not a Object");
  }
  if (!typedArray->asObject()->isTypedArrayObject()) {
    ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, "Target is not a TypedArray");
  }
  auto typedArrayType = typedArray->asObject()->asArrayBufferView()->typedArrayType();
  if(onlyInt32 && typedArrayType != Int32 )
  {
    ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, "Target is not a TypedArray Int32");
  }
  else if (typedArrayType != Int8 && typedArrayType != Uint8 && typedArrayType != Int16 && typedArrayType != Uint16 && typedArrayType != Int32 && typedArrayType != Uint32)
  {
    ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, "Target is wrong type");
  }
  ASSERT(typedArray->asObject()->isArrayBufferView());
  auto buffer = typedArray->asObject()->asArrayBufferView();
  if(!buffer->isSharedArrayBufferObject())
  {
    ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, "Target is not a SharedArrayBufferObject");
  }
  return buffer;
}

Value validateAtomicAccess(ExecutionState& state, Value* typedArray, unsigned requestIndex)
{
  ASSERT(!typedArray->isObject() || typedArray->asObject()->isArrayBufferView());
  unsigned length = typedArray->asObject()->asArrayBufferView()->arrayLength();
  ASSERT(requestIndex != 0);
  if(requestIndex >=  length)
  {
    ErrorObject::throwBuiltinError(state, ErrorObject::RangeError, "requestIndex is greater then typedArray length");
  }
  return Value(requestIndex);
}

Value atomicReadModifyWrite(ExecutionState& state, Value* typedArray, unsigned index, Value* value, unsigned op)
{
  Value buffer = validateSharedIntergerTypedArray(state, typedArray);
  Value i = validateAtomicAccess(state, typedArray, index);
  unsigned v = value->toInteger(state);
  auto elementType = typedArray->asObject()->asArrayBufferView()->typedArrayType();
  int elementSize = ArrayBufferView::getElementSize(typedArray->asObject()->asArrayBufferView()->typedArrayType());
  int offset = typedArray->asObject()->asArrayBufferView()->byteOffset();
  int indexedPosition = i.toNumber(state) * elementSize + offset;
  return buffer.asObject()->asArrayBufferView()->buffer()->getModifySetValueInBuffer(state, &buffer, indexedPosition, elementType, v, op);
}
Value atomicLoad(ExecutionState& state, Value* typedArray, unsigned index)
{
  Value buffer = validateSharedIntergerTypedArray(state, typedArray);
  Value i = validateAtomicAccess(state, typedArray, index);
  auto elementType = typedArray->asObject()->asArrayBufferView()->typedArrayType();
  int elementSize = ArrayBufferView::getElementSize(typedArray->asObject()->asArrayBufferView()->typedArrayType());
  int offset = typedArray->asObject()->asArrayBufferView()->byteOffset();
  int indexedPosition = i.toNumber(state) * elementSize + offset;
  // return buffer.asObject()->asArrayBufferView()->buffer()->getValueFromBuffer(state, )
  return Value();
}

static Value builtinAtomicsAdd(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  return atomicReadModifyWrite(state, &argv[0], argv[1].toNumber(state), &argv[2], 0);
}

static Value builtinAtomicsAnd(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  return atomicReadModifyWrite(state, &argv[0], argv[1].toNumber(state), &argv[2], 1);
}

static Value builtinAtomicsCompareExchange(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  Value buffer = validateSharedIntergerTypedArray(state, &argv[0]);
  Value i = validateAtomicAccess(state,  &argv[0], argv[1].toNumber(state));
  unsigned expected = argv[2].toInteger(state);
  unsigned replacement = argv[3].toInteger(state);
  auto elementType = argv[0].asObject()->asArrayBufferView()->typedArrayType();
  int elementSize = ArrayBufferView::getElementSize(argv[0].asObject()->asArrayBufferView()->typedArrayType());
  // Value expectedBytes = buffer.asObject()->asArrayBufferView()->buffer()->getValueFromBuffer(state, );
  int offset = argv[0].asObject()->asArrayBufferView()->byteOffset();
  int indexedPosition = i.toNumber(state) * elementSize + offset;
  // compare
  return buffer.asObject()->asArrayBufferView()->buffer()->getModifySetValueInBuffer(state, &buffer, indexedPosition, elementType, replacement, 0);
}

static Value builtinAtomicsExchange(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  return atomicReadModifyWrite(state, &argv[0], argv[1].toNumber(state), &argv[2], 2);
}

static Value builtinAtomicsIsLockFree(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  return Value(false);
}

static Value builtinAtomicsLoad(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  return atomicLoad(state, &argv[0], argv[1].toNumber(state));
}

static Value builtinAtomicsOr(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  return atomicReadModifyWrite(state, &argv[0], argv[1].toNumber(state), &argv[2], 3);
}

static Value builtinAtomicsStore(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  Value buffer = validateSharedIntergerTypedArray(state, &argv[0]);
  Value i = validateAtomicAccess(state,  &argv[0], argv[1].toNumber(state));
  unsigned v = argv[2].toInteger(state);
  auto elementType = argv[0].asObject()->asArrayBufferView()->typedArrayType();
  int elementSize = ArrayBufferView::getElementSize(argv[0].asObject()->asArrayBufferView()->typedArrayType());
  int offset = argv[0].asObject()->asArrayBufferView()->byteOffset();
  int indexedPosition = i.toNumber(state) * elementSize + offset;
  // buffer.SetValueInBuffer(state, )
  return Value(v);
}

static Value builtinAtomicsSub(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  return atomicReadModifyWrite(state, &argv[0], argv[1].toNumber(state), &argv[2], 4);
}

static Value builtinAtomicsWait(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  return Value("ok");
}

static Value builtinAtomicsNotify(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  return Value(0);
}

static Value builtinAtomicsXor(ExecutionState& state, Value thisValue, size_t argc, Value* argv, bool isNewExpression)
{
  return atomicReadModifyWrite(state, &argv[0], argv[1].toNumber(state), &argv[2], 5);
}

void GlobalObject::installAtomics(ExecutionState& state)
 {
     const StaticStrings* strings = &state.context()->staticStrings();
     m_atomics = new Object(state);
     m_atomics->markThisObjectDontNeedStructureTransitionTable();

     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().add),
                                                ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().add, builtinAtomicsAdd, 3, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().and),
                                                ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().and, builtinAtomicsAnd, 3, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().compareExchange),
                                                 ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().compareExchange, builtinAtomicsCompareExchange, 4, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().exchange),
                                                 ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().exchange, builtinAtomicsExchange, 3, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().isLockFree),
                                                 ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().isLockFree, builtinAtomicsIsLockFree, 1, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().Load),
                                                 ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().Load, builtinAtomicsLoad, 2, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().Or),
                                                 ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().Or, builtinAtomicsOr, 3, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().store),
                                                 ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().store, builtinAtomicsStore, 3, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().Sub),
                                                 ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().Sub, builtinAtomicsSub, 3, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().wait),
                                                 ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().wait, builtinAtomicsWait, 4, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().notify),
                                                 ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().notify, builtinAtomicsNotify, 3, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
     m_atomics->defineOwnPropertyThrowsException(state, ObjectPropertyName(state.context()->staticStrings().Xor),
                                                 ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().Xor, builtinAtomicsXor, 3, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));


     defineOwnProperty(state, ObjectPropertyName(strings->Atomics),
                       ObjectPropertyDescriptor(m_reflect, (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

   }
 }
