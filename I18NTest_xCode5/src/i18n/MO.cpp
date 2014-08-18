//
//  Created by kaishiqi on 14-8-5.
//  Copyright (c) 2014 kaishiqi. All rights reserved.
//

#include "MO.h"
#include <fstream>
#include <cstring>
#include <sstream>

NS_I18N_BEGIN

namespace {
    inline const char* findChar(const char* first, const char* last, const char value) {
        for (auto itr = first; (last - itr) > 0; itr++) {
            if (*itr == value) {
                return itr;
            }
        }
        return last;
    }
    
    inline int maxInt(int a, int b) {
        return (a > b ? a : b);
    }
    
    inline int minInt(int a, int b) {
        return (a < b ? a : b);
    }
}

//--------------------------------------------------------------------
//
// MO Class
//
//--------------------------------------------------------------------
//0x04 as original string context separator.
const char MO::CONTEXT_SEPARATOR = 0x04;
//0x00 as original string singular/plural separator or a plural translations separator.
const char MO::PLURAL_SEPARATOR = 0x00;

/** The common logic for English is implemented */
const unsigned int MO::DEFAULT_NPLURALS = 2;
const std::function<int(int n)> MO::DEFAULT_PLURAL_EXPRESSION = [](int n){ return (n!=1); };

MO::MO(std::string filePath/*=""*/, std::function<int(int n)> onPluralExpression/*=DEFAULT_PLURAL_EXPRESSION*/, unsigned int nplurals/*=DEFAULT_NPLURALS*/)
:_isInited(false)
,nplurals(nplurals)
,onPluralExpression(onPluralExpression)
{
    if (!filePath.empty()) {
        this->loadMoFile(filePath);
    }
}

MO::MO(unsigned char* bytes, std::function<int(int n)> onPluralExpression/*=DEFAULT_PLURAL_EXPRESSION*/, unsigned int nplurals/*=DEFAULT_NPLURALS*/)
:_isInited(false)
,nplurals(nplurals)
,onPluralExpression(onPluralExpression)
{
    if (bytes) {
        this->parseMoFile(bytes);
    }
}

MO::~MO()
{
}

bool MO::loadMoFile(std::string filePath)
{
    if (_isInited || filePath.empty()) {
        return false;
    }
    
    std::ifstream fin(filePath, std::ifstream::in | std::ifstream::binary);
    unsigned char* bytes = nullptr;
    unsigned long bytesLen = 0;
    
    if (fin && fin.is_open()) {
        fin.seekg(0, fin.end);
        ssize_t size = fin.tellg();
        fin.seekg(0, fin.beg);
        
        bytesLen = sizeof(unsigned char) * size;
        bytes = new unsigned char[bytesLen];
        fin.read((char*)bytes, size);
        
        fin.close();
    } else {
        return false;
    }
    
    if (!bytes || 0 == bytesLen) {
        if (bytes) {
            delete[] bytes;
            bytes = nullptr;
        }
        
        return false;
    }
    
    // parse *.mo File
    this->parseMoFile(bytes);
    delete[] bytes;
    bytes = nullptr;
    
    return true;
}

void MO::parseMoFile(unsigned char* bytes)
{
    if (!bytes || _isInited) {
        return;
    }
    
    /** precache some functions */
    unsigned long (*peekLong)(unsigned char* bytes, unsigned long offs) = nullptr;
    
    auto peekLongBig = [](unsigned char* bytes, unsigned long offs)->unsigned long {
        unsigned char a = bytes[offs];
        unsigned char b = bytes[offs+1];
        unsigned char c = bytes[offs+2];
        unsigned char d = bytes[offs+3];
        return ((d*256+c)*256+b)*256+a;
    };
    
    auto peekLongLittle = [](unsigned char* bytes, unsigned long offs)->unsigned long {
        unsigned char a = bytes[offs];
        unsigned char b = bytes[offs+1];
        unsigned char c = bytes[offs+2];
        unsigned char d = bytes[offs+3];
        return ((a*256+b)*256+c)*256+d;
    };
    
    auto copyBytes = [](char* srcBytes, int len) {
        char* chars = new char[len+1];// +1 is null-terminated byte string pointed
        strncpy(chars, srcBytes, len);
        chars[len] = 0;
        std::string str(chars);
        delete [] chars;
        return str;
    };
    
    /**
     * Check format in MO file.
     * @see http://www.gnu.org/savannah-checkouts/gnu/gettext/manual/html_node/MO-Files.html
     */
    
    /** magic */
    const unsigned long MAGIC_BIG = 0xde120495;
    const unsigned long MAGIC_LITTLE = 0x950412de;
    const int BYTE_SIZE = 4;
    
    unsigned char magicBytes[BYTE_SIZE];
    memcpy(magicBytes, bytes, BYTE_SIZE);
    
    unsigned long magicNumber = 0;
    for (int i = 0; i < BYTE_SIZE; i++)
        magicNumber += (unsigned long)magicBytes[i] << ((BYTE_SIZE-1-i)*8);
    
    switch (magicNumber) {
        case MAGIC_BIG:
            peekLong = peekLongBig;
            break;
            
        case MAGIC_LITTLE:
            peekLong = peekLongLittle;
            break;
            
        default:
            //no valid mo-file
            return;
            break;
    }
    
    
    /** revision */
    unsigned long revision = peekLong(bytes, 4);
    // support revision 0 of MO format specs, only
    if (0 != revision) {
        //unsupported version
        return;
    }
    
    
    /** get number of N, O, T, S, H */
    unsigned long moN = peekLong(bytes, 8);
    unsigned long moO = peekLong(bytes, 12);
    unsigned long moT = peekLong(bytes, 16);
    unsigned long moS = peekLong(bytes, 20);
    unsigned long moH = peekLong(bytes, 24);
    
    // skip hash table
    (void)moS;
    (void)moH;
    
    
    /** traverse and get strings */
    const int CHUNK_SIZE = 8;
    for (unsigned long i = 0; i < moN; i++) {
        auto originalLength = peekLong(bytes, moO);
        auto originalOffset = peekLong(bytes, moO + BYTE_SIZE);
        
        auto translationLength = peekLong(bytes, moT);
        auto translationOffset = peekLong(bytes, moT + BYTE_SIZE);
        
        char* originalBytes = new char[originalLength];
        memcpy(originalBytes, bytes + originalOffset, originalLength);
        
        char* translationBytes = new char[translationLength];
        memcpy(translationBytes, bytes + translationOffset, translationLength);
        
        moO += CHUNK_SIZE;
        moT += CHUNK_SIZE;
        
        /** mo file headers info */
        if (0 == originalLength && _headersInfo.empty()) {
            _headersInfo = copyBytes(translationBytes, (int)translationLength);
        } else {
            /**
             * Build a TranslationEntry from original string and translation strings,
             * found in a MO file.
             *
             * char[] {originalBytes} original string to translate from MO file. Might contain
             * 	0x04 as context separator or 0x00 as singular/plural separator.
             * char[] {translationBytes} translation string from MO file. Might contain
             * 	0x00 as a plural translations separator.
             */
            TranslateEntry entry;
            
            // check original
            do {
                auto originalItr = findChar(originalBytes, originalBytes + originalLength, CONTEXT_SEPARATOR);
                size_t bytesLen = originalItr - originalBytes;
                
                // look for context
                if (bytesLen < originalLength) {
                    entry.setContext(copyBytes(originalBytes, (int)bytesLen));
                    bytesLen += sizeof(CONTEXT_SEPARATOR);
                } else {
                    bytesLen = 0;
                }
                
                // look for singular original
                originalItr = findChar(originalBytes + bytesLen, originalBytes + originalLength, PLURAL_SEPARATOR);
                entry.setSingular(copyBytes(originalBytes + bytesLen, (int)(originalItr - originalBytes - bytesLen)));
                bytesLen = originalItr - originalBytes + sizeof(PLURAL_SEPARATOR);
                
                // look for plural original
                if (bytesLen < originalLength) {
                    entry.setPlural(copyBytes(originalBytes + bytesLen, (int)(originalLength - (originalItr - originalBytes))));
                }
            }while (0);
            
            // check translation
            do {
                size_t bytesLen = 0;
                std::vector<std::string> translations;
                
                // plural translations are also separated by \0
                while (true) {
                    auto translationItr = findChar(translationBytes + bytesLen, translationBytes + translationLength, PLURAL_SEPARATOR);
                    translations.push_back(copyBytes(translationBytes + bytesLen, (int)(translationItr - translationBytes - bytesLen)));
                    bytesLen = translationItr - translationBytes + sizeof(PLURAL_SEPARATOR);
                    
                    if (bytesLen >= translationLength) {
                        break;
                    }
                }
                entry.setTranslations(translations);
            } while (0);
            
            // save
            _entryMap[entry.getKey()] = entry;
            
            // debug
            //printf("%s\n", entry.toString().c_str());
        }
        
        delete [] originalBytes;
        delete [] translationBytes;
    }
    
    _isInited = true;
}

unsigned int MO::selectPluralForm(int n)
{
    auto index = onPluralExpression ? onPluralExpression(n) : MO::DEFAULT_PLURAL_EXPRESSION(n);
    return maxInt(0, minInt(index, nplurals-1));
}

std::string MO::translate(std::string singular, std::string context/*=""*/)
{
    auto entryItr = _entryMap.find(Entry::createKey(singular, context));
    auto isTranslated = (entryItr != _entryMap.end());
    return (isTranslated ? (*entryItr).second.getTranslations().at(0) : singular);
}

std::string MO::translatePlural(std::string singular, std::string plural, int count, std::string context/*=""*/)
{
    auto entryItr = _entryMap.find(Entry::createKey(singular, context));
    auto pluralFormsIndex = this->selectPluralForm(count);
    auto isTranslated = (entryItr != _entryMap.end());
    
    if (isTranslated && pluralFormsIndex < (*entryItr).second.getTranslations().size()) {
        return (*entryItr).second.getTranslations().at(pluralFormsIndex);
    } else {
        return (1==count) ? singular : plural;
    }
}

NS_I18N_END