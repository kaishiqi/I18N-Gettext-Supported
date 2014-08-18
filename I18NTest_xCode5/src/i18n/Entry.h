//
//  Created by kaishiqi on 14-8-11.
//  Copyright (c) 2014 kaishiqi. All rights reserved.
//

#ifndef __I18N__Entry__
#define __I18N__Entry__

#include "I18nDefine.h"
#include <string>
#include <vector>

NS_I18N_BEGIN

#define _SYNTHESIZE_READONLY_(varType, varName, funName)\
protected: varType varName;\
protected: void set##funName(varType var){ varName = var; }\
public: virtual varType get##funName(void) const { return varName; }

#define _SYNTHESIZE_READONLY_PASS_BY_REF_(varType, varName, funName)\
protected: varType varName;\
protected: void set##funName(varType var){ varName = var; }\
public: virtual const varType& get##funName(void) const { return varName; }


class Entry
{
    friend class MO;
    
public:
    /** True is if there is a plural, false is singular only */
    _SYNTHESIZE_READONLY_(bool, _isPlural, IsPural);
    /** The string to translate */
    _SYNTHESIZE_READONLY_(std::string, _singular, Singular);
    /** The plural form of the string, setting this will set {isPlural} to true */
    _SYNTHESIZE_READONLY_(std::string, _plural, Plural);
    /** A string differentiating two equal strings used in different contexts */
    _SYNTHESIZE_READONLY_(std::string, _context, Context);
    
protected:
    std::string _name;
    
public:
    Entry():_isPlural(false), _name("Entry") {};
    virtual ~Entry() {};
    
    /** Generates a unique key for this entry */
    virtual std::string getKey();
    virtual std::string toString();
    
protected:
    static std::string createKey(std::string singular, std::string context = "");
};



class NoopEntry : public Entry
{
    friend class I18nUtils;
    
public:
    /** The domain identifier the text should be retrieved in. */
    _SYNTHESIZE_READONLY_PASS_BY_REF_(std::string, _domain, Domain);
    
public:
    NoopEntry() { _name = "NoopEntry"; };
    virtual std::string toString();
};



class TranslateEntry : public Entry
{
    friend class MO;
    
public:
    /** Ttranslations of the string and possibly -- its plural forms */
    _SYNTHESIZE_READONLY_PASS_BY_REF_(std::vector<std::string>, _translations, Translations);
    
public:
    TranslateEntry() { _name = "TranslateEntry"; };
    virtual std::string toString();
};


NS_I18N_END

#endif /* defined(__I18N__Entry__) */
