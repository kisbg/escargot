/*
 * Copyright (c) 2018-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
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
#include "SymbolObject.h"
#include "Context.h"

namespace Escargot {

SymbolObject::SymbolObject(ExecutionState& state, Symbol* value)
    : SymbolObject(state, state.context()->globalObject()->symbolPrototype(), value)
{
}

SymbolObject::SymbolObject(ExecutionState& state, Object* proto, Symbol* value)
    : Object(state, proto, ESCARGOT_OBJECT_BUILTIN_PROPERTY_NUMBER + 1)
    , m_primitiveValue(value)
{
    m_structure = state.context()->defaultStructureForSymbolObject();
}

void* SymbolObject::operator new(size_t size)
{
    static bool typeInited = false;
    static GC_descr descr;
    if (!typeInited) {
        GC_word obj_bitmap[GC_BITMAP_SIZE(SymbolObject)] = { 0 };
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(SymbolObject, m_structure));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(SymbolObject, m_prototype));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(SymbolObject, m_values));
        GC_set_bit(obj_bitmap, GC_WORD_OFFSET(SymbolObject, m_primitiveValue));
        descr = GC_make_descriptor(obj_bitmap, GC_WORD_LEN(SymbolObject));
        typeInited = true;
    }
    return GC_MALLOC_EXPLICITLY_TYPED(size, descr);
}
}
