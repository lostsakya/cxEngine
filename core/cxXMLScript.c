//
//  cxXMLScript.c
//  cxEngine
//
//  Created by xuhua on 10/14/13.
//  Copyright (c) 2013 xuhua. All rights reserved.
//

#include <textures/cxTextureFactory.h>
#include "cxXMLScript.h"
#include "cxHash.h"
#include "cxEventItem.h"
#include "cxEngine.h"

CX_OBJECT_INIT(cxXMLReader, cxObject)
{
    
}
CX_OBJECT_FREE(cxXMLReader, cxObject)
{
    if(this->reader != NULL){
        xmlTextReaderClose(this->reader);
    }
}
CX_OBJECT_TERM(cxXMLReader, cxObject)

CX_OBJECT_INIT(cxTextureAttr, cxObject)
{
    this->box = cxBoxTex2fDefault();
}
CX_OBJECT_FREE(cxTextureAttr, cxObject)
{
    CX_RELEASE(this->texture);
}
CX_OBJECT_TERM(cxTextureAttr, cxObject)

#define PARSE_FUNCTION(vptr,fname,farg)                         \
cxConstChars ptr = vptr;                                        \
cxInt len = (cxInt)strlen(ptr) + 1;                             \
cxInt lpos = 0;                                                 \
cxInt rpos = 0;                                                 \
for(cxInt i=0; i < len; i++){                                   \
    if(ptr[i] == '('){                                          \
        lpos = i;                                               \
    }                                                           \
    if(ptr[i] == ')'){                                          \
        rpos = i;                                               \
    }                                                           \
}                                                               \
cxBool isFunc = (lpos > 0 && rpos > 0);                         \
cxChar fname[len];                                              \
cxChar farg[len];                                               \
if(isFunc){                                                     \
    int len = rpos - lpos - 1;                                  \
    memcpy(farg, ptr + lpos + 1, len);                          \
    farg[len] = '\0';                                           \
    for(int i=0; i < len;i++){                                  \
        if(farg[i] == '\''){                                    \
            farg[i] = '\"';                                     \
        }                                                       \
    }                                                           \
    len = lpos;                                                 \
    memcpy(fname, ptr, len);                                    \
    fname[len] = '\0';                                          \
}else{                                                          \
    memcpy(fname, ptr, len);                                    \
}


#define GET_FUNCTION_ITEM(item)                                 \
cxFuncItem item = NULL;                                         \
if(functions != NULL){                                          \
    item = cxHashGet(functions, cxHashStrKey(fName));           \
}                                                               \
if(item == NULL){                                               \
    item = cxEngineGetFunc(fName);                              \
}                                                               \
CX_ASSERT(item != NULL, "function %s not eixsts",fName);


#define GET_EVENT_ITEM(event)                                   \
cxEventItem event = NULL;                                       \
if(events != NULL){                                             \
    event = cxHashGet(events, cxHashStrKey(eName));             \
}                                                               \
if(event == NULL){                                              \
    event = cxEngineGetEvent(eName);                            \
}

CX_OBJECT_INIT(cxXMLScript, cxObject)
{
}
CX_OBJECT_FREE(cxXMLScript, cxObject)
{
    CX_RELEASE(this->bytes);
}
CX_OBJECT_TERM(cxXMLScript, cxObject)

cxConstChars cxXMLAttrAuto(xmlTextReaderPtr reader,cxConstChars name)
{
    xmlChar *txt = xmlTextReaderGetAttribute(reader, BAD_CAST(name));
    CX_RETURN(txt == NULL || xmlStrlen(txt) == 0, NULL);
    return cxStringBody(cxStringAttachChars((cxChar *)txt));
}

xmlTextReaderPtr cxXMLReaderForString(cxString code,xmlTextReaderErrorFunc error,cxAny arg)
{
    cxXMLReader this = CX_CREATE(cxXMLReader);
    this->reader = xmlReaderForMemory(cxStringBody(code), cxStringLength(code), NULL, "UTF-8", 0);
    if(error != NULL){
        xmlTextReaderSetErrorHandler(this->reader, error, arg);
    }
    return this->reader;
}

xmlTextReaderPtr cxXMLReaderForScript(cxXMLScript this,xmlTextReaderErrorFunc error,cxAny arg)
{
    return cxXMLReaderForString(this->bytes,error,arg);
}

//method({json}
cxEventItem cxXMLReadEvent(cxHash events,cxConstChars name,xmlTextReaderPtr reader)
{
    //xml view event event(arg)
    cxConstChars eventName = cxXMLAttr(name);
    CX_RETURN(eventName == NULL,NULL);
    PARSE_FUNCTION(eventName, eName, eArg);
    GET_EVENT_ITEM(event);
    //set event arg
    if(isFunc && event != NULL){
        CX_RETAIN_SWAP(event->arg, cxEventArgCreate(eArg));
    }
    return event;
}

cxFloat cxXMLReadFloatAttr(xmlTextReaderPtr reader,cxHash functions, cxConstChars name,cxFloat value)
{
    cxFloat rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberFloat(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToFloat(num);
    }else{
        rv = atof(svalue);
    }
    return rv;
}

cxAssist4f cxXMLReadAssist4fAttr(xmlTextReaderPtr reader,cxHash functions, cxConstChars name,cxAssist4f value)
{
    cxAssist4f rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberAssist4f(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToAssist4f(num);
    }else{
        cxReadFloats(svalue, &rv.v1);
    }
    return rv;
}

cxColor4f cxXMLReadColor4fAttr(xmlTextReaderPtr reader,cxHash functions, cxConstChars name,cxColor4f value)
{
    cxColor4f rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberColor4f(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToColor4f(num);
    }else{
        cxReadFloats(svalue, &rv.r);
    }
    return rv;
}

cxVec2f cxXMLReadVec2fAttr(xmlTextReaderPtr reader,cxHash functions,cxConstChars name,cxVec2f value)
{
    cxVec2f rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberVec2f(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToVec2f(num);
    }else{
        cxReadFloats(svalue, &rv.x);
    }
    return rv;
}

cxVec3f cxXMLReadVec3fAttr(xmlTextReaderPtr reader,cxHash functions,cxConstChars name,cxVec3f value)
{
    cxVec3f rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberVec3f(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToVec3f(num);
    }else{
        cxReadFloats(svalue, &rv.x);
    }
    return rv;
}

cxFloatRange cxXMLReadFloatRangeAttr(xmlTextReaderPtr reader,cxHash functions,cxConstChars name,cxFloatRange value)
{
    cxFloatRange rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberFloatRange(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToFloatRange(num);
    }else{
        cxReadFloats(svalue, &rv.v);
    }
    return rv;
}

cxVec2fRange cxXMLReadVec2fRangeAttr(xmlTextReaderPtr reader,cxHash functions,cxConstChars name,cxVec2fRange value)
{
    cxVec2fRange rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberVec2fRange(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToVec2fRange(num);
    }else{
        cxReadFloats(svalue, &rv.v.x);
    }
    return rv;
}

cxColor4fRange cxXMLReadColor4fRangeAttr(xmlTextReaderPtr reader,cxHash functions,cxConstChars name,cxColor4fRange value)
{
    cxColor4fRange rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberColor4fRange(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToColor4fRange(num);
    }else{
        cxReadFloats(svalue, &rv.v.r);
    }
    return rv;
}

cxVec2i cxXMLReadVec2iAttr(xmlTextReaderPtr reader,cxHash functions, cxConstChars name,cxVec2i value)
{
    cxVec2i rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberVec2i(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToVec2i(num);
    }else{
        cxReadInts(svalue, &rv.x);
    }
    return rv;
}

cxBox4i cxXMLReadBox4iAttr(xmlTextReaderPtr reader,cxHash functions, cxConstChars name,cxBox4i value)
{
    cxBox4i rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberBox4i(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToBox4i(num);
    }else{
        cxReadInts(svalue, &rv.l);
    }
    return rv;
}

cxBox4f cxXMLReadBox4fAttr(xmlTextReaderPtr reader,cxHash functions, cxConstChars name,cxBox4f value)
{
    cxBox4f rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberBox4f(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToBox4f(num);
    }else{
        cxReadFloats(svalue, &rv.l);
    }
    return rv;
}

cxTextureAttr cxXMLReadTextureAttr(xmlTextReaderPtr reader,cxHash functions,cxConstChars name)
{
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, NULL);
    cxTextureAttr rv = CX_CREATE(cxTextureAttr);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreate(fArg);
        rv = item->func(args);
    }else{
        cxUrlPath path = cxUrlPathParse(svalue);
        cxTexture texture = NULL;
        if(path->count > 0){
            texture = cxTextureFactoryLoadFile(path->path);
            rv->size = texture->size;
            CX_RETAIN_SWAP(rv->texture, texture);
        }
        if(path->count > 1 && texture != NULL){
            rv->box = cxTextureBox(texture, path->key);
            rv->size = cxTextureSize(texture, path->key);
        }
    }
    return rv;
}

cxInt cxReadFloats(cxConstChars ptr,cxFloat *values)
{
    CX_RETURN(ptr == NULL || values == NULL, 0);
    cxInt len = (cxInt)strlen(ptr) + 1;
    cxInt argLen = 0;
    cxChar arg[16]={0};
    cxInt c = 0;
    CX_RETURN(len < 2, 0);
    for(int i=0; i < len; i++){
        if(ptr[i] == ',' || ptr[i] == '\0'){
            arg[argLen] = '\0';
            (*values++) = atof(arg);
            c++;
            argLen = 0;
        }else{
            arg[argLen++] = ptr[i];
        }
    }
    return c;
}

cxInt cxReadInts(cxConstChars ptr,cxInt *values)
{
    CX_RETURN(ptr == NULL || values == NULL, 0);
    cxInt len = (cxInt)strlen(ptr) + 1;
    cxInt argLen = 0;
    cxChar arg[16]={0};
    cxInt c = 0;
    CX_RETURN(len < 2, 0);
    for(int i=0; i < len; i++){
        if(ptr[i] == ',' || ptr[i] == '\0'){
            arg[argLen] = '\0';
            c++;
            (*values++) = atoi(arg);
            argLen = 0;
        }else{
            arg[argLen++] = ptr[i];
        }
    }
    return c;
}

cxConstChars cxXMLReadElementName(xmlTextReaderPtr reader)
{
    return (cxConstChars)xmlTextReaderConstName(reader);
}

cxBool cxXMLHasAttr(xmlTextReaderPtr reader,cxConstChars name)
{
    cxConstChars s = cxXMLAttr(name);
    return s != NULL;
}

cxString cxXMLReadString(xmlTextReaderPtr reader)
{
    xmlChar *str = xmlTextReaderReadString(reader);
    return cxStringAttachChars((cxChar *)str);
}

cxInt cxXMLReadIntAttr(xmlTextReaderPtr reader,cxHash functions,cxConstChars name,cxInt value)
{
    cxInt rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberInt(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToInt(num);
    }else{
        rv = atoi(svalue);
    }
    return rv;
}

cxUInt cxXMLReadUIntAttr(xmlTextReaderPtr reader,cxHash functions, cxConstChars name,cxUInt value)
{
    cxUInt rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberUInt(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToUInt(num);
    }else{
        rv = atol(svalue);
    }
    return rv;
}

cxString cxXMLReadStringAttr(xmlTextReaderPtr reader,cxHash functions,cxConstChars name)
{
    cxString rv = NULL;
    cxConstChars value = cxXMLAttr(name);
    CX_RETURN(value == NULL, NULL);
    PARSE_FUNCTION(value, fName, fArg);
    cxFuncItem item = NULL;
    if(functions != NULL){
        item = cxHashGet(functions, cxHashStrKey(fName));
    }
    if(item == NULL){
        item = cxEngineGetFunc(fName);
    }
    if(isFunc && item != NULL){
        cxEventArg args = cxEventArgCreate(fArg);
        rv = item->func(args);
    }
    if(rv == NULL){
        rv = cxStringConstChars(value);
    }
    CX_ASSERT(cxObjectIsType(rv, cxStringTypeName), "get string error");
    return rv;
}

cxString cxXMLReaderReadOuterXml(xmlTextReaderPtr reader)
{
    xmlChar *xml = xmlTextReaderReadOuterXml(reader);
    cxString rv = NULL;
    if(xml != NULL){
        rv = cxStringConstChars((cxConstChars)xml);
    }
    xmlFree(xml);
    return rv;
}

cxBool cxXMLReadBoolAttr(xmlTextReaderPtr reader,cxHash functions,cxConstChars name,cxBool value)
{
    cxBool rv = value;
    cxConstChars svalue = cxXMLAttr(name);
    CX_RETURN(svalue == NULL, rv);
    PARSE_FUNCTION(svalue, fName, fArg);
    if(isFunc){
        GET_FUNCTION_ITEM(item);
        cxEventArg args = cxEventArgCreateWithNumber(fArg,cxNumberBool(rv));
        cxNumber num = item->func(args);
        CX_ASSERT(cxObjectIsType(num, cxNumberTypeName), "%s must return cxNumber type",fName);
        rv = cxNumberToBool(num);
    }else{
        rv = cxConstCharsEqu(svalue, "true");
    }
    return rv;
}








