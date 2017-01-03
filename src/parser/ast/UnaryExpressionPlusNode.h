/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef UnaryExpressionPlusNode_h
#define UnaryExpressionPlusNode_h

#include "ExpressionNode.h"

namespace Escargot {

class UnaryExpressionPlusNode : public ExpressionNode {
public:
    friend class ScriptParser;
    UnaryExpressionPlusNode(Node* argument)
        : ExpressionNode()
    {
        m_argument = argument;
    }

    virtual ASTNodeType type() { return ASTNodeType::UnaryExpressionPlus; }
    virtual void generateExpressionByteCode(ByteCodeBlock* codeBlock, ByteCodeGenerateContext* context)
    {
        m_argument->generateExpressionByteCode(codeBlock, context);
        codeBlock->pushCode(ToNumber(ByteCodeLOC(m_loc.index), context->getLastRegisterIndex()), context, this);
    }

protected:
    Node* m_argument;
};
}

#endif
