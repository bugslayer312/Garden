//
//  File.cpp
//  Garden
//
//  Created by Bugslayer on 10.11.15.
//  Copyright (c) 2015 Bugslayer. All rights reserved.
//

#include "GUtil.h"


// ParamStringReader

ParamStringReader::ParamStringReader(String paramString)
{
    this->paramString = paramString;
    curPos = 0;
    readParam();
}

void ParamStringReader::init(String paramString)
{
    this->paramString = paramString;
    curPos = 0;
    readParam();
}

void ParamStringReader::readParam()
{
    hasParam = false;
    int paramdel = paramString.indexOf(";", curPos);
    if(paramdel < 0)
        paramdel = paramString.length();
    if(paramdel > curPos)
    {
        String param = paramString.substring(curPos, paramdel);
        //Serial.println("param:" + param);
        int paramL = param.length();
        if(param[paramL-1] == '?')
        {
            if(paramL > 1)
            {
                currentParam.isRead = true;
                currentParam.name = param.substring(0, paramL-1);
                currentParam.value = "";
                hasParam = true;
            }
        }
        else
        {
            currentParam.isRead = false;
            int setDel = param.indexOf("=");
            if(setDel >= 0)
            {
                //Serial.println("setparam");
                if(setDel > 0 && setDel < paramL - 1)
                {
                    currentParam.name = param.substring(0, setDel);
                    currentParam.value = param.substring(setDel+1, paramL);
                    hasParam = true;
                }
            }
            else
            {
                currentParam.name = param;
                currentParam.value = "";
                hasParam = true;
            }
        }
    }
    curPos = paramdel + 1;
}

ParamStringElement* ParamStringReader::getCurrentParam()
{
    return (hasParam ? &currentParam : NULL);
}

void ParamStringReader::next()
{
    readParam();
}

void ParamStringReader::addParam(String name, String val)
{
    curPos = 0;
    if(paramString.length() > 0)
    {
        curPos = paramString.length() + 1;
        paramString += ";";
    }
    paramString += name + (val.length() > 0 ? ("=" + val) : "");
}

void ParamStringReader::addQueryParam(String name)
{
    curPos = 0;
    if(paramString.length() > 0)
    {
        curPos = paramString.length() + 1;
        paramString += ";";
    }
    paramString += name + "?";
}

String ParamStringReader::getParamString()
{
    return paramString;
}

String ParamStringReader::getParamValue(String& paramName)
{
    if(curPos > paramString.length())
        curPos = 0;
    int startSearchPos = curPos;
    do{
        readParam();
        if(hasParam && !currentParam.isRead && currentParam.name == paramName){
            return currentParam.value;
        }
        if(curPos > paramString.length())
            curPos = 0;
    }while (startSearchPos != curPos);
    return "";
}

// ParamArrayReader

ParamArrayReader::ParamArrayReader(String& paramString, String delimiter, bool back){
    this->delimiter = delimiter;
    this->paramString = paramString;
    if(back){
        curPos = paramString.length() + delimiter.length();
        prev();
    }
    else{
        next();
    }
}

void ParamArrayReader::next(){
    curParam = "";
    _hasParam = paramString.length() > 0 && curPos <= paramString.length();
    if(!_hasParam)
        return;
    if (curPos < 0)
        curPos = 0;
    int findPos = paramString.indexOf(delimiter, curPos);
    if (findPos < 0)
        findPos = paramString.length();
    if(findPos > curPos)
        curParam = paramString.substring(curPos, findPos);
    curPos = findPos + delimiter.length();
}

void ParamArrayReader::prev(){
    curParam;
    _hasParam = paramString.length() > 0 && curPos > delimiter.length();
    if(!_hasParam)
        return;
    int endPos = curPos - delimiter.length() - 1;
    int findPos = paramString.lastIndexOf(delimiter, endPos);
    curPos = findPos < 0 ? 0 : findPos + delimiter.length();
    if(curPos <= endPos)
        curParam = paramString.substring(curPos, endPos + 1);
}

bool ParamArrayReader::hasParam(){
    return _hasParam;
}

String ParamArrayReader::getParam(){
    return curParam;
}

void ParamArrayReader::flush(bool back){
    if(back){
        curPos = paramString.length() + 1;
        prev();
    }
    else{
        curPos = 0;
        next();
    }
}

// PairFromString

PairFromString::PairFromString(String str)
{
    init(str);
}

void PairFromString::init(String str)
{
    int pos = str.indexOf(":");
    First = (pos >= 0 ? str.substring(0, pos) : "");
    Second = (pos >= 0 ? str.substring(pos+1, str.length()) : "");
}
