//
//  Created by kaishiqi on 14-8-5.
//  Copyright (c) 2014 kaishiqi. All rights reserved.
//

#include "I18nUtils.h"
#include <cstdlib>
#include <cstdarg>

#ifdef __GNUC__
#include <regex.h>
#else
#include <regex>
#endif

NS_I18N_BEGIN

namespace {
    // The max length of formatMatch message.
    static const int MAX_LOG_LENGTH = 100*1024;
    
    /** The singleton pointer of I18nUtils. */
    static I18nUtils* _instance = nullptr;
}

string I18nUtils::DEFAULT_DOMAIN = "default";

I18nUtils* I18nUtils::getInstance()
{
    if (!_instance)
        _instance = new I18nUtils();
    return _instance;
}

void I18nUtils::destroyInstance()
{
    if (_instance) {
        delete _instance;
        _instance = nullptr;
    }
}

I18nUtils::I18nUtils()
{
}

I18nUtils::~I18nUtils()
{
}

void I18nUtils::addMO(MO mo, string domain/*=DEFAULT_DOMAIN*/)
{
    domain = (!domain.empty() ? domain : DEFAULT_DOMAIN);
    _moMap[domain] = mo;
}

void I18nUtils::addMO(string filePath, std::function<int(int n)> onPluralExpression/*=MO::DEFAULT_PLURAL_EXPRESSION*/, unsigned int nplurals/*=MO::DEFAULT_NPLURALS*/, string domain/*=DEFAULT_DOMAIN*/)
{
    if (!filePath.empty()) {
        domain = (!domain.empty() ? domain : DEFAULT_DOMAIN);
        _moMap[domain] = MO(filePath, onPluralExpression, nplurals);
    }
}

void I18nUtils::addMO(unsigned char* bytes, std::function<int(int n)> onPluralExpression/*=MO::DEFAULT_PLURAL_EXPRESSION*/, unsigned int nplurals/*=MO::DEFAULT_NPLURALS*/, string domain/*=DEFAULT_DOMAIN*/)
{
    if (bytes) {
        domain = (!domain.empty() ? domain : DEFAULT_DOMAIN);
        _moMap[domain] = MO(bytes, onPluralExpression, nplurals);
    }
}

void I18nUtils::removeMO(string domain/*=DEFAULT_DOMAIN*/)
{
    auto moItr = _moMap.find(domain);
    if (moItr != _moMap.end()) {
        _moMap.erase(moItr);
    }
}

void I18nUtils::removeAllMO()
{
    _moMap.clear();
}

bool I18nUtils::hasMO(string domain/*=DEFAULT_DOMAIN*/)
{
    auto moItr = _moMap.find(domain);
    return (moItr != _moMap.end());
}

const MO& I18nUtils::getMO(string domain/*=DEFAULT_DOMAIN*/)
{
    if (domain.empty()) {
        domain = DEFAULT_DOMAIN;
    }
    
    if (!hasMO(domain)) {
        _moMap[domain] = MO();
    }
    
    return _moMap[domain];
}

string I18nUtils::translate(string singular, string context/*=""*/, string domain/*=DEFAULT_DOMAIN*/)
{
    auto moItr = _moMap.find(domain);
    auto hasMo = (moItr != _moMap.end());
    return (hasMo ? (*moItr).second.translate(singular, context) : singular);
}

string I18nUtils::translatePlural(string singular, string plural, int count, string context/*=""*/, string domain/*=DEFAULT_DOMAIN*/)
{
    auto moItr = _moMap.find(domain);
    auto hasMo = (moItr != _moMap.end());
    
    if (hasMo) {
        return (*moItr).second.translatePlural(singular, plural, count, context);
    } else {
        return (1==count) ? singular : plural;
    }
}

string I18nUtils::gettext(string text, string domain/*=DEFAULT_DOMAIN*/)
{
    return translate(text, "", domain);
}

string I18nUtils::xgettext(string text, string context, string domain/*=DEFAULT_DOMAIN*/)
{
    return translate(text, context, domain);
}

string I18nUtils::ngettext(string singular, string plural, int number, string domain/*=DEFAULT_DOMAIN*/)
{
    return translatePlural(singular, plural, number, "", domain);
}

string I18nUtils::nxgettext(string singular, string plural, int number, string context, string domain/*=DEFAULT_DOMAIN*/)
{
    return translatePlural(singular, plural, number, context, domain);
}

NoopEntry I18nUtils::ngettextNoop(string singular, string plural, string domain/*=DEFAULT_DOMAIN*/)
{
    NoopEntry entry;
    entry.setIsPural(true);
    entry.setSingular(singular);
    entry.setPlural(plural);
    entry.setDomain(domain);
    return entry;
}

NoopEntry I18nUtils::nxgettextNoop(string singular, string plural, string context, string domain/*=DEFAULT_DOMAIN*/)
{
    NoopEntry entry;
    entry.setIsPural(true);
    entry.setSingular(singular);
    entry.setPlural(plural);
    entry.setContext(context);
    entry.setDomain(domain);
    return entry;
}

string I18nUtils::translateNooped(NoopEntry& entry, int number, string domain/*=DEFAULT_DOMAIN*/)
{
    auto domainString = entry.getDomain().empty() ? domain : entry.getDomain();
    return translatePlural(entry.getSingular(), entry.getPlural(), number, entry.getContext(), domainString);
}

#if (IS_ENABLE_FORMAT_MATCH_ARGS_INDEX == FORMAT_MATCH_ENABLE)
#ifdef __GNUC__
std::string I18nUtils::formatMatch(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    string result(format);
    
    /** precache some functions */
    auto getRegMatchStr = [](regmatch_t match, const char* target) {
        auto len = match.rm_eo - match.rm_so;
        char* chars = new char[len+1];// +1 is null-terminated byte string pointed
        strncpy(chars, &target[match.rm_so], len);
        chars[len] = 0;
        std::string str(chars);
        return str;
    };
    
    //printf style format regex: %[align][flags][width][.prec][type]specifier
    static const string PRINTF_FORMAT_PATTERN = "%(\\-*)([\\+\\# 0]*)([0-9]*)(\\.?[0-9]*)([jztL]?|[hl]{0,2})([csdioxXufFeEaAgGp])";
    static const size_t PRINTF_FORMAT_MACTH_NUM = 6+1;//sub match num + 1(total self)
    
    static const string FORMAT_MATCH_PATTERN = PRINTF_FORMAT_PATTERN + "([0-9]+)";
    static const size_t FORMAT_MATCH_MATCH_NUM = PRINTF_FORMAT_MACTH_NUM + 1;
    
    struct FormatMatchResultInfo {
        string format;  // printf style format string
        string type;    // printf style format type
        char specifier; // printf style format specifier
        string numStr;  // formatMatch format args number string
        int index;      // formatMatch format args number
    };
    
    
    /** match all conversion specifier */
    regmatch_t formatMatchMatches[FORMAT_MATCH_MATCH_NUM];
    regex_t formatMatchRegex;
    regcomp(&formatMatchRegex, FORMAT_MATCH_PATTERN.c_str(), REG_EXTENDED);
    
    //[FormatMatchResultInfo.index] = FormatMatchResultInfo
    std::unordered_map<int, FormatMatchResultInfo> matchItrsMap;
    
    string formatClone(format);
    while (true) {
        auto formatMatchStatic = regexec(&formatMatchRegex, formatClone.c_str(), FORMAT_MATCH_MATCH_NUM, formatMatchMatches, 0);
        if (0 == formatMatchStatic) {//static 0 = find
            auto formatMatch = formatMatchMatches[0];
            auto argNumMatch = formatMatchMatches[FORMAT_MATCH_MATCH_NUM-1];
            auto specifierMatch = formatMatchMatches[FORMAT_MATCH_MATCH_NUM-2];
            auto typeMatch = formatMatchMatches[FORMAT_MATCH_MATCH_NUM-3];
            
            FormatMatchResultInfo matchInfo;
            matchInfo.numStr = getRegMatchStr(argNumMatch, formatClone.c_str());
            matchInfo.specifier = getRegMatchStr(specifierMatch, formatClone.c_str())[0];
            matchInfo.type = getRegMatchStr(typeMatch, formatClone.c_str());
            matchInfo.index = std::atoi(matchInfo.numStr.c_str()) - 1;
            
            auto matchFormat = getRegMatchStr(formatMatch, formatClone.c_str());
            matchInfo.format = matchFormat.substr(0, matchFormat.size() - matchInfo.numStr.size());
            
            if (0 <= matchInfo.index) {
                auto numItr = matchItrsMap.find(matchInfo.index);
                
                if (numItr == matchItrsMap.end()) {
                    matchItrsMap[matchInfo.index] = matchInfo;
                }
            }
            
            for (auto i = formatMatch.rm_so; i < formatMatch.rm_eo; i++) {
                formatClone[i] = '_';
            }
        } else {
            break;
        }
    }
    regfree(&formatMatchRegex);
    
    
    /** replace the all conversion format with corresponding index argument */
    for (int i = 0; i < matchItrsMap.size(); i++) {
        auto matchItr = matchItrsMap.find(i);
        if (matchItr != matchItrsMap.end()) {
            auto matchInfo = matchItrsMap[i];
            char buf[MAX_LOG_LENGTH];
            
            // get a format argument
            switch (matchInfo.specifier) {
                case 'c':
                    if ("l" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (wchar_t)va_arg(args, int));
                    } else {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (char)va_arg(args, int));
                    }
                    break;
                    
                case 's':
                    if ("l" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, wchar_t*));
                    } else {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, char*));
                    }
                    break;
                    
                case 'd':
                case 'i':
                case 'o':
                case 'x':
                case 'X':
                case 'u':
                    if ("hh" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (unsigned char)va_arg(args, int));
                    } else if ("h" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (unsigned short)va_arg(args, int));
                    } else if ("l" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (unsigned long)va_arg(args, long));
                    } else if ("ll" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (unsigned long long)va_arg(args, long long));
                    } else {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (unsigned int)va_arg(args, int));
                    }
                    break;
                    
                case 'f':
                case 'F':
                    if ("L" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, long double));
                    } else {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, double));
                    }
                    break;
                    
                case 'e':
                case 'E':
                case 'a':
                case 'A':
                case 'g':
                case 'G':
                    if ("L" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, long double));
                    } else {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, double));
                    }
                    break;
                    
                case 'p':
                    snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, void*));
                    break;
                    
                default:
                    //unsupported specifier
                    break;
            }
            
            // replace the conversion format with argument
            string replacePattern(PRINTF_FORMAT_PATTERN + "(" + matchInfo.numStr + "[0-9]*)");
            regmatch_t replaceMatchMatches[FORMAT_MATCH_MATCH_NUM];
            regex_t replaceMatchRegex;
            regcomp(&replaceMatchRegex, replacePattern.c_str(), REG_EXTENDED);
            
            string resultClone = result;
            bool hasReplace = false;
            do {
                hasReplace = false;
                
                while (true) {
                    auto replaceMatchStatic = regexec(&replaceMatchRegex, resultClone.c_str(), FORMAT_MATCH_MATCH_NUM, replaceMatchMatches, 0);
                    if (0 == replaceMatchStatic) {//static 0 = find
                        auto replaceMatch = replaceMatchMatches[0];
                        auto argNumMatch = replaceMatchMatches[FORMAT_MATCH_MATCH_NUM - 1];
                        auto argNumStr = getRegMatchStr(argNumMatch, result.c_str());
                        
                        if (argNumStr == matchInfo.numStr) {
                            auto resultLen = resultClone.size();
                            resultLen -= replaceMatch.rm_eo - replaceMatch.rm_so;
                            resultLen += strlen(buf);
                            
                            char* resultCloneStr = new char[resultLen+1];
                            strncpy(resultCloneStr, &resultClone[0], replaceMatch.rm_so);
                            strcpy(&resultCloneStr[replaceMatch.rm_so], buf);
                            strcpy(&resultCloneStr[replaceMatch.rm_so + strlen(buf)], &resultClone[replaceMatch.rm_eo]);
                            resultCloneStr[resultLen] = 0;
                            resultClone = resultCloneStr;
                            delete [] resultCloneStr;
                            
                            char* resultStr = new char[resultLen+1];
                            strncpy(resultStr, &result[0], replaceMatch.rm_so);
                            strcpy(&resultStr[replaceMatch.rm_so], buf);
                            strcpy(&resultStr[replaceMatch.rm_so + strlen(buf)], &result[replaceMatch.rm_eo]);
                            resultStr[resultLen] = 0;
                            result = resultStr;
                            delete [] resultStr;
                            
                            hasReplace = true;
                            break;
                        }
                        
                        for (auto i = replaceMatch.rm_so; i < replaceMatch.rm_eo; i++) {
                            resultClone[i] = '_';
                        }
                    } else {
                        break;
                    }
                }
            } while (hasReplace);
            regfree(&replaceMatchRegex);
        }
    }
    
    va_end(args);
    return result;
}
#else
string I18nUtils::formatMatch(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    string result(format);
    
    //printf style format regex: %[align][flags][width][.prec][type]specifier
    static const string PRINTF_FORMAT_PATTERN("%(\\-*)([\\+\\# 0]*)(\\d*)(\\.?\\d*)([jztL]?|[hl]{0,2})([csdioxXufFeEaAgGp])");
    std::regex formatMatchPattern(PRINTF_FORMAT_PATTERN + "(\\d+)");
    
    struct FormatMatchResultInfo {
        string format;  // printf style format string
        string type;    // printf style format type
        char specifier; // printf style format specifier
        string numStr;  // formatMatch format args number string
        int index;      // formatMatch format args number
    };
    
    /** match all conversion specifier */
    auto formatBegin = std::sregex_iterator(result.begin(), result.end(), formatMatchPattern);
    auto formatEnd = std::sregex_iterator();
    
    //[FormatMatchResultInfo.index] = FormatMatchResultInfo
    std::unordered_map<int, FormatMatchResultInfo> matchItrsMap;
    for (auto itr = formatBegin; itr != formatEnd; ++itr) {
        auto matchResult = (*itr);
        auto matchFormat = matchResult.str();
        
        FormatMatchResultInfo matchInfo;
        matchInfo.numStr = matchResult[matchResult.size() - 1].str();
        matchInfo.specifier = matchResult[matchResult.size() - 2].str()[0];
        matchInfo.type = matchResult[matchResult.size() - 3].str();
        matchInfo.index = std::atoi(matchInfo.numStr.c_str()) - 1;
        matchInfo.format = matchFormat.substr(0, matchFormat.size() - matchInfo.numStr.size());
        
        if (0 <= matchInfo.index) {
            auto numItr = matchItrsMap.find(matchInfo.index);
            
            if (numItr == matchItrsMap.end()) {
                matchItrsMap[matchInfo.index] = matchInfo;
            }
        }
    }
    
    /** replace the all conversion format with corresponding index argument */
    for (int i = 0; i < matchItrsMap.size(); i++) {
        auto matchItr = matchItrsMap.find(i);
        if (matchItr != matchItrsMap.end()) {
            auto matchInfo = matchItrsMap[i];
            char buf[MAX_LOG_LENGTH];
            
            // get a format argument
            switch (matchInfo.specifier) {
                case 'c':
                    if ("l" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (wchar_t)va_arg(args, int));
                    } else {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (char)va_arg(args, int));
                    }
                    break;
                    
                case 's':
                    if ("l" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, wchar_t*));
                    } else {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, char*));
                    }
                    break;
                    
                case 'd':
                case 'i':
                case 'o':
                case 'x':
                case 'X':
                case 'u':
                    if ("hh" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (unsigned char)va_arg(args, int));
                    } else if ("h" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (unsigned short)va_arg(args, int));
                    } else if ("l" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (unsigned long)va_arg(args, long));
                    } else if ("ll" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (unsigned long long)va_arg(args, long long));
                    } else {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), (unsigned int)va_arg(args, int));
                    }
                    break;
                    
                case 'f':
                case 'F':
                    if ("L" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, long double));
                    } else {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, double));
                    }
                    break;
                    
                case 'e':
                case 'E':
                case 'a':
                case 'A':
                case 'g':
                case 'G':
                    if ("L" == matchInfo.type) {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, long double));
                    } else {
                        snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, double));
                    }
                    break;
                    
                case 'p':
                    snprintf(buf, MAX_LOG_LENGTH-3, matchInfo.format.c_str(), va_arg(args, void*));
                    break;
                    
                default:
                    //unsupported specifier
                    break;
            }
            
            // replace the conversion format with argument
            string subPattern("(" + matchInfo.numStr + "\\d*)");
            std::regex replacePattern(PRINTF_FORMAT_PATTERN + subPattern);
            
            bool hasReplace = false;
            do {
                auto words_begin = std::sregex_iterator(result.begin(), result.end(), replacePattern);
                auto words_end = std::sregex_iterator();
                hasReplace = false;
                
                for (auto itr = words_begin; itr != words_end; ++itr) {
                    auto replaceResult = *itr;
                    auto replaceMatch = replaceResult[0];
                    auto resultNumStr = replaceResult[replaceResult.size() - 1].str();
                    
                    if (resultNumStr == matchInfo.numStr) {
                        result.replace(replaceMatch.first, replaceMatch.second, buf);
                        hasReplace = true;
                        break;
                    }
                }
            } while (hasReplace);
        }
    }
    
    va_end(args);
    return result;
}
#endif
#else
string I18nUtils::formatMatch(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    char buf[MAX_LOG_LENGTH];
    vsnprintf(buf, MAX_LOG_LENGTH-3, format, args);
    string result(buf);
    
    va_end(args);
    return result;
}
#endif

NS_I18N_END