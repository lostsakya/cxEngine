//
//  cxParabola.c
//  cxEngine
//
//  Created by xuhua on 1/9/14.
//  Copyright (c) 2014 xuhua. All rights reserved.
//

#include "cxParabola.h"

static void cxParabolaInit(cxAny pav)
{
    cxParabola this = pav;
    cxAny view = cxActionView(pav);
    cxVec2f n;
    kmVec2Normalize(&n, &this->speed);
    this->angle = cxVec2fAngle(n);
    this->time = 0;
    this->pos = cxViewPosition(view);
}

static void cxParabolaStep(cxAny pav,cxFloat dt,cxFloat time)
{
    cxParabola this = pav;
    cxView view = cxActionView(pav);
    cxVec2f tmp;
    cxVec2f gv;
    cxFloat tv = this->time * this->time / 2.0f;
    kmVec2Scale(&tmp, &this->speed, this->time);
    kmVec2Scale(&gv, &this->gravity, tv);
    kmVec2Add(&tmp, &tmp, &gv);
    kmVec2Add(&tmp, &tmp, &this->pos);
    cxViewSetPos(view, tmp);
    this->time += dt;
    this->super.duration -= dt;
}

static cxBool cxParabolaExit(cxAny pav)
{
    cxParabola this = pav;
    return this->super.duration != -1 && this->time >= this->super.duration;
}

static void cxParabolaReadAttr(cxReaderAttrInfo *info)
{
    cxActionReadAttr(info);
    cxParabola this = info->object;
    this->speed = cxXMLReadVec2fAttr(info, "cxParabola.speed", cxVec2fv(0, 0));
    cxActionSetDuration(this, cxXMLReadFloatAttr(info, "cxAction.time", -1));
}

CX_OBJECT_INIT(cxParabola, cxAction)
{
    cxObjectSetReadAttrFunc(this, cxParabolaReadAttr);
    CX_METHOD_OVERRIDE(this->super.Exit, cxParabolaExit);
    CX_METHOD_OVERRIDE(this->super.Init, cxParabolaInit);
    CX_METHOD_OVERRIDE(this->super.Step, cxParabolaStep);
    this->gravity = cxVec2fv(0, -1000);
    this->super.duration = -1;
}
CX_OBJECT_FREE(cxParabola, cxAction)
{
    
}
CX_OBJECT_TERM(cxParabola, cxAction)

cxParabola cxParabolaCreate(cxVec2f speed)
{
    cxParabola this = CX_CREATE(cxParabola);
    this->speed = speed;
    return this;
}
