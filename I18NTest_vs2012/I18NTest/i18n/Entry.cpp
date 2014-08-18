//
//  Entry.cpp
//  I18NTest
//
//  Created by kaishiqi on 14-8-11.
//  Copyright (c) 2014 kaishiqi. All rights reserved.
//

#include "Entry.h"
#include "MO.h"
#include <sstream>

NS_I18N_BEGIN

//--------------------------------------------------------------------
//
// Entry Class
//
//--------------------------------------------------------------------
std::string Entry::createKey(std::string singular, std::string context/*=""*/)
{
    return (context.empty() ? singular : (context + MO::CONTEXT_SEPARATOR + singular));
}

std::string Entry::getKey()
{
    return Entry::createKey(_singular, _context);
}

std::string Entry::toString()
{
    std::string str = "[" + _name + "]";
    
    if (!_singular.empty()) {
        str += "\n\t";
        str += " Singular:\t\t ";
        str += _singular;
    }
    
    if (_isPlural && !_plural.empty()) {
        str += "\n\t";
        str += " Plural:\t\t ";
        str += _plural;
    }
    
    if (!_context.empty()) {
        str += "\n\t";
        str += " Context:\t\t ";
        str += _context;
    }
    
    return str;
}

//--------------------------------------------------------------------
//
// NoopEntry Class
//
//--------------------------------------------------------------------
std::string NoopEntry::toString()
{
    auto str = Entry::toString();
    
    if (!_domain.empty()) {
        str += "\n\t";
        str += " Domain:\t\t ";
        str += _domain;
    }
    
    return str;
}

//--------------------------------------------------------------------
//
// TranslateEntry Class
//
//--------------------------------------------------------------------
std::string TranslateEntry::toString()
{
    auto str = Entry::toString();
    
    if (!_translations.empty()) {
        for (int i = 0; i < _translations.size(); i++) {
            str += "\n\t";
            str += " Translation";
            if (1 < _translations.size()) {
                std::stringstream ss;
                ss << i;
                str += "[" + ss.str() + "]";
            }
            str += ":";
            if (1 >= _translations.size())
                str += "\t";
            str += " ";
            str += _translations[i];
        }
    }
    
    return str;
}

NS_I18N_END