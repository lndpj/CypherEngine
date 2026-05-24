/*======================================================================
   File: math_quat.cpp
   Project: rengine
   Author: ksiric <email@example.com>
   Created: 2026-05-24 14:46:51
   Last Modified by: ksiric
   Last Modified: 2026-05-24 14:59:43
   ---------------------------------------------------------------------
   Description:
       
   ---------------------------------------------------------------------
   License: 
   Company: 
   Version: 0.1.0
 ======================================================================
                                                                       */

#include "rengine/math/math_quat.h"

namespace reap::rengine::math 
{

quat_t Math_QuatIdentity() 
{
    return quat_t{ 0.0f, 0.0f, 0.0f, 1.0f };   
}

quat_t Math_QuatMake( rcommon::f32 x, rcommon::f32 y, rcommon::f32 z, rcommon::f32 w ) 
{
    return quat_t{ x, y, z, w };
}

rcommon::f32 Math_QuatDot( const quat_t &q1, const quat_t &q2 ) 
{
    
    
    
}




    
}       // namespace reap::rengine::math
