#ifndef CYPHER_COMMON_TIER1_EXPRESSIONEVALUATOR_H
#define CYPHER_COMMON_TIER1_EXPRESSIONEVALUATOR_H
#pragma once

/*
================
CypherCommon Expression Evaluator

Small expression evaluator declarations.
================
*/

#include "CypherCommon_Tier0.h"

namespace cypher::common
{

struct expression_result_t {
    f64 value;
    bool_t valid;
};

expression_result_t ExpressionEvaluator_Evaluate( const char *pExpression );

} // namespace cypher::common

#endif // CYPHER_COMMON_TIER1_EXPRESSIONEVALUATOR_H
