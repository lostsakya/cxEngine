//
//  cxSpline.h
//  cxEngine
//
//  Created by xuhua on 12/12/13.
//  Copyright (c) 2013 xuhua. All rights reserved.
//

#ifndef cxEngine_cxSpline_h
#define cxEngine_cxSpline_h

#include <core/cxArray.h>
#include <core/cxAction.h>

CX_C_BEGIN

CX_OBJECT_DEF(cxSpline, cxAction)
    cxArray points;
    cxFloat tension;
    cxFloat delta;
    cxInt index;
    CX_EVENT_ALLOC(onIndex);
CX_OBJECT_END(cxSpline)

void cxSplineAppend(cxAny pav,cxVec2f pos);

CX_C_END

#endif
