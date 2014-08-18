//
//  Created by kaishiqi on 14-8-5.
//  Copyright (c) 2014 kaishiqi. All rights reserved.
//

#ifndef __I18N__I18nUtils__
#define __I18N__I18nUtils__

#include "I18nDefine.h"
#include "MO.h"
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

NS_I18N_BEGIN
using std::string;

class I18nUtils
{
public:
    static std::string DEFAULT_DOMAIN;
    
private:
    /**
     * MO hash map, through domain save.
     * @see addMO
     */
    std::unordered_map<std::string, MO> _moMap;
    
public:
    /** Gets the instance of I18nUtils. */
    static I18nUtils* getInstance();
    
    /** Destroys the instance of I18N. */
    static void destroyInstance();
    
#if (IS_ENABLE_FORMAT_MATCH_ARGS_INDEX == FORMAT_MATCH_ENABLE)
    /**
     * Same as use printf, but format % character can append arguments' index(from to 1 start).
     * e.g., format = "Hello, %s1. Are you free at %d2:%d3?.";
     *
     * @see printf
     */
    static string formatMatch(const char* format, ...);
#else
    /**
     * Same as use printf, but returned std:string.
     * e.g., format = "Hello, %s. Are you free at %d:%d?.";
     *
     * @see printf
     */
    static string formatMatch(const char* format, ...) __attribute__((format(printf, 1, 2)));
#endif
    
    /** The destructor of I18nUtils. */
    virtual ~I18nUtils();
    
    /**
     * Append a MO, through domain string save.
     * @see MO::MO
     */
    void addMO(MO mo, string domain = DEFAULT_DOMAIN);
    
    /**
     * Append a MO, through domain string save.
     * @see MO::MO
     */
    void addMO(string filePath, std::function<int(int n)> onPluralExpression = MO::DEFAULT_PLURAL_EXPRESSION, unsigned int nplurals = MO::DEFAULT_NPLURALS, string domain = DEFAULT_DOMAIN);
    
    /**
     * Append a MO, through domain string save.
     * @see MO::MO
     */
    void addMO(unsigned char* bytes, std::function<int(int n)> onPluralExpression = MO::DEFAULT_PLURAL_EXPRESSION, unsigned int nplurals = MO::DEFAULT_NPLURALS, string domain = DEFAULT_DOMAIN);
    
    /** Remove a MO, through domain string. */
    void removeMO(string domain = DEFAULT_DOMAIN);
    
    /** Clear all domain MO. */
    void removeAllMO();
    
    /** Returns true if this domain has a mo. */
    bool hasMO(string domain = DEFAULT_DOMAIN);
    
    /** Get a MO, through domain string. */
    const MO& getMO(string domain = DEFAULT_DOMAIN);
    
    /**
     * Retrieves the translation of text.
     * If there is no translation, or the domain isn't loaded, the original text is returned.
     *
     * @param text      Text to translate.
     * @param domain    The domain identifier the text should be retrieved in.
     */
    string gettext(string text, string domain = DEFAULT_DOMAIN);
    
    /**
     * Retrieve translated string with context.
     *
     * Quite a few times, there will be collisions with similar translatable text
     * found in more than two places but with different translated context.
     *
     * By including the context in the pot file translators can translate the two
     * strings differently.
     *
     * @param text          Text to translate.
     * @param context       Context information for the translators.
     * @param domain        The domain identifier the text should be retrieved in.
     */
    string xgettext(string text, string context, string domain = DEFAULT_DOMAIN);
    
    /**
     * Retrieve the plural or single form based on the amount.
     * If the domain is not set in the mo hash map, then a comparison will be made
     * and either plural or single parameters returned.
     *
     * @param singular      The text that will be used if number is 1
     * @param plural        The text that will be used if number is not 1
     * @param number        The number to compare against to use either single or plural.
     * @param domain        The domain identifier the text should be retrieved in.
     */
    string ngettext(string singular, string plural, int number, string domain = DEFAULT_DOMAIN);
    
    /**
     * A hybrid of xgettext() and ngettext(). It supports contexts and plurals.
     *
     * @see xgettext
     * @see ngettext
     */
    string nxgettext(string singular, string plural, int number, string context, string domain = DEFAULT_DOMAIN);
    
    /**
     * Retrieve the plural strings in NoopEntry, but don't translate them.
     *
     * Used when you want to keep structures with translatable plural strings and
     * use them later.
     *
     * Example:
     *  struct Messages {
     *      NoopEntry post;
     *      NoopEntry page;
     *      ...
     *  };
     *  Messages msg;
     *  msg.post = ngettextNoop('%s post', '%s posts');
     *  msg.page = ngettextNoop('%s pages', '%s pages');
     *  ...
     *  NoopEntry selectMessageNoopEntry;
     *  switch (selectIndex) {
     *      case POST: selectMessageNoopEntry = msg.post; break;
     *      case PAGE: selectMessageNoopEntry = msg.page; break;
     *      ...
     *  }
     *  std::string usableText = translateNooped(selectMessageNoopEntry, 3);
     *
     * @return NoopEntry
     * @see ngettext
     */
    NoopEntry ngettextNoop(string singular, string plural, string domain = DEFAULT_DOMAIN);
    
    /**
     * Retrieve the plural strings with context in NoopEntry, but don't translate them.
     *
     * @see ngettextNoop
     */
    NoopEntry nxgettextNoop(string singular, string plural, string context, string domain = DEFAULT_DOMAIN);
    
    /**
     * Translate the result of ngettextNoop() or nxgettextNoop().
     *
     * @param domain        Optional. The domain identifier the text should be retrieved in.
     * If NoopEntry contains a domain passed to ngettextNoop() or nxgettextNoop(), it will override this value.
     *
     * @see ngettextNoop
     * @see nxgettextNoop
     */
    string translateNooped(NoopEntry& NoopEntry, int number, string domain = DEFAULT_DOMAIN);
    
private:
    I18nUtils();
    
    /**
     * Retrieves the translation of singular.
     * If there is no translation, or the domain isn't loaded, the original text is returned.
     * @param domain        Domain to retrieve the translated text.
     * @see MO::translate
     */
    string translate(string singular, string context = "", string domain = DEFAULT_DOMAIN);
    
    /**
     * Retrieve the plural or single form based on the amount.
     * If the domain is not set in the _moMap list, then a comparison will be made and either plural or single parameters returned.
     * @param domain        Domain to retrieve the translated text.
     * @see MO::translatePlural
     */
    string translatePlural(string singular, string plural, int count, string context = "", string domain = DEFAULT_DOMAIN);
};


/***************************************************************
 *
 * Here provide wrappers for translate interface keywords.
 *
 ***************************************************************
 */

inline string __(string text, string domain = I18nUtils::DEFAULT_DOMAIN) {
    return I18nUtils::getInstance()->gettext(text, domain);
}

inline string _x(string text, string context, string domain = I18nUtils::DEFAULT_DOMAIN) {
    return I18nUtils::getInstance()->xgettext(text, context, domain);
}

inline string _n(string singular, string plural, int number, string domain = I18nUtils::DEFAULT_DOMAIN) {
    return I18nUtils::getInstance()->ngettext(singular, plural, number, domain);
}

inline string _nx(string singular, string plural, int number, string context, string domain = I18nUtils::DEFAULT_DOMAIN) {
    return I18nUtils::getInstance()->nxgettext(singular, plural, number, context, domain);
}

inline NoopEntry _n_noop(string singular, string plural, string domain = I18nUtils::DEFAULT_DOMAIN) {
    return I18nUtils::getInstance()->ngettextNoop(singular, plural, domain);
}

inline NoopEntry _nx_noop(string singular, string plural, string context, string domain = I18nUtils::DEFAULT_DOMAIN) {
    return I18nUtils::getInstance()->nxgettextNoop(singular, plural, context, domain);
}

inline string t_nooped(NoopEntry entry, int number, string domain = I18nUtils::DEFAULT_DOMAIN) {
    return I18nUtils::getInstance()->translateNooped(entry, number, domain);
}

NS_I18N_END

#endif /* defined(__I18N__I18nUtils__) */
