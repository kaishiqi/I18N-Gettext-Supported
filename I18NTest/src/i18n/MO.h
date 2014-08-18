//
//  Created by kaishiqi on 14-8-5.
//  Copyright (c) 2014 kaishiqi. All rights reserved.
//

#ifndef __I18N__MO__
#define __I18N__MO__

#include "I18nDefine.h"
#include "Entry.h"
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>


NS_I18N_BEGIN

#define _SYNTHESIZE_READONLY_(varType, varName, funName)\
protected: varType varName;\
protected: void set##funName(varType var){ varName = var; }\
public: virtual varType get##funName(void) const { return varName; }

#define _SYNTHESIZE_READONLY_PASS_BY_REF_(varType, varName, funName)\
protected: varType varName;\
protected: void set##funName(varType var){ varName = var; }\
public: virtual const varType& get##funName(void) const { return varName; }

class MO
{
public:
    static const char CONTEXT_SEPARATOR;
    static const char PLURAL_SEPARATOR;
    static const unsigned int DEFAULT_NPLURALS;
    static const std::function<int(int n)> DEFAULT_PLURAL_EXPRESSION;
    
    /**
     * The nplurals value must be a decimal number which specifies how many different plural forms exist for this language.
     * @see http://www.gnu.org/savannah-checkouts/gnu/gettext/manual/html_node/Plural-forms.html
     * @see http://poedit.net/trac/wiki/Doc/PluralForms
     * @see https://www.drupal.org/node/17564
     */
    unsigned int nplurals;
    
    /**
     * Makes a function, which will return the right translation index, according to the plural forms header.
     * @see http://www.gnu.org/savannah-checkouts/gnu/gettext/manual/html_node/Plural-forms.html
     * @see http://poedit.net/trac/wiki/Doc/PluralForms
     * @see https://www.drupal.org/node/17564
     */
    std::function<int(int n)> onPluralExpression;
    
private:
    bool _isInited;
    std::string _headersInfo;
    
    /**
     * TranslateEntry hash map, through entry.getKey() save.
     * @see TranslateEntry
     */
    std::unordered_map<std::string, TranslateEntry> _entryMap;
    
public:
    /**
     * Creates a mo with a *.mo file path, then parse it.
     * @param filePath              @see MO::loadMoFile
     * @param onPluralExpression    @see MO::onPluralExpression
     * @param nplurals              @see MO::nplurals
     */
    MO(std::string filePath = "", std::function<int(int n)> onPluralExpression = DEFAULT_PLURAL_EXPRESSION, unsigned int nplurals = DEFAULT_NPLURALS);
    
    /**
     * Creates a mo with parse *.mo file bytes pointer.
     * @param bytes                 @see MO::parseMoFile
     * @param onPluralExpression    @see MO::onPluralExpression
     * @param nplurals              @see MO::nplurals
     */
    MO(unsigned char* bytes, std::function<int(int n)> onPluralExpression = DEFAULT_PLURAL_EXPRESSION, unsigned int nplurals = DEFAULT_NPLURALS);
    
    virtual ~MO();
    
    /**
     * Load *.mo file, then parse it.
     * @param filePath A path to mo file, e.g., "languages/zh_CN.mo"
     */
    bool loadMoFile(std::string filePath);
    
    /**
     * Parse *.mo file bytes pointer.
     * @param bytes The buffer pointer.
     */
    void parseMoFile(unsigned char* bytes);
    
    /**
     * Given the number of items, returns index of the plural form to use.
     * @param n     number of items.
     * @return      unsigned int index of the plural form to use.
     */
    unsigned int selectPluralForm(int n);
    
    /**
     * Retrieves the translation of singular. 
     * If there is no translation, the original text is returned.
     * @param singular      Text to translate.
     * @param context       Context information for the translators.
     * @return              Translated text.
     */
    std::string translate(std::string singular, std::string context = "");
    
    /**
     * Retrieve the plural or single form based on the amount.
     * If there is no translation list, then a comparison will be made and either plural or$single parameters returned.
     *
     * @param single        The text that will be used if $number is 1
     * @param plural        The text that will be used if $number is not 1
     * @param count         The number to compare against to use either single or plural
     * @param context       Context information for the translators.
     * @return              Either single or plural translated text
     */
    std::string translatePlural(std::string singular, std::string plural, int count, std::string context = "");
    
    /** parse mo file success */
    bool getIsInited() { return _isInited; };
    
    /** MO file headers info string */
    std::string getHeadersInfo() { return _headersInfo; };
};


NS_I18N_END

#endif /* defined(__I18N__MO__) */
