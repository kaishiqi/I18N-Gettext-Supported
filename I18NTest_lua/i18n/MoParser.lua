--[[
 * author : kaishiqi
 * descpt : create on 2016-03-18
]]
local Entry    = require 'i18n.Entry'
local MoParser = {}
MoParser.__index = MoParser

MoParser.ErrMsg = {
    NotFind = 'file load fail.',
    NoValid = 'no valid mo-file',
    Version = 'unsupported version',
}

local MoDefine = {
    MAGIC_BIG       = '\222\018\004\149',  -- 0xde120495
    MAGIC_LITTLE    = '\149\004\018\222',  -- 0x950412de
    BYTE_SIZE       = 4,
    CHUNK_SIZE      = 8,
    CONTEXT_SEP     = '\004',  -- 0x04
    PLURAL_SEP      = '\000',  -- 0x00
    HEADER_SEP      = ': ',
    FORM_INFO_SEP   = ';',
    PLURAL_INFO_KEY = 'Plural-Forms',
    NPLURAL_KEY     = 'nplurals=',
    PLURAL_KEY      = ' plural=',
    LogicSymbolMap  = {
        ['!']  = '~',
        [':']  = 'or',
        ['?']  = 'and',
        ['||'] = 'or',
        ['&&'] = 'and',
    }
}


-------------------------------------------------
-- life cycle

function MoParser.new(filePath)
    local instance = setmetatable({}, MoParser)
    instance:ctor(filePath)
    return instance
end


function MoParser:ctor(filePath)
    self.handerInfos_ = {}
    self.entryMap_    = {}
    self.pluralExp_   = nil
    self.nplurals_    = 0
    self.inited_, self.errMsg_ = self:parseMoFile_(self:loadMoFile_(filePath))
end


-------------------------------------------------
-- get / set

function MoParser:isInited()
    return self.inited_ == true
end


function MoParser:getErrorMsg()
    return self.errMsg_
end


function MoParser:getHanderInfos()
    return self.handerInfos_
end


function MoParser:getEntryMap()
    return self.entryMap_
end


function MoParser:getPlurals()
    return self.nplurals_
end


-------------------------------------------------
-- public method

function MoParser:calculatePlural(n)
    local pluralIndex = 0
    if self.pluralExp_ then
        local expressionStr = string.format('local n = %d \n return %s', n, self.pluralExp_)
        local expressionFun = loadstring(expressionStr)
        local ok, ret = pcall(expressionFun)
        if ok then
            if type(ret) == 'boolean' then
                pluralIndex = ret == true and 1 or 0
            else
                pluralIndex = ret
            end
        end
    end
    return pluralIndex + 1
end


function MoParser:translate(singular, context)
    local keyword     = self:buildEntryKey_(singular, context)
    local entry       = self.entryMap_[keyword]
    local translation = singular
    if entry and entry:getTranslations() then
        translation = entry:getTranslations()[1]
    end
    return translation
end


function MoParser:translatePlural(singular, plural, count, context)
    local keyword     = self:buildEntryKey_(singular, context)
    local entry       = self.entryMap_[keyword]
    local number      = tonumber(count) or 0
    local transIndex  = self:calculatePlural(number)
    local translation = number > 1 and plural or singular
    if entry and entry:getTranslations() and entry:getTranslations()[transIndex] then
        translation = entry:getTranslations()[transIndex]
    end
    return translation
end


-------------------------------------------------
-- private method

function MoParser:loadMoFile_(filePath)
    local moBytes  = nil

    if FTUtils and FTUtils.getFileDataWithoutDec then
        -- use cocos2d-x cc.FileUtils, cross-platform
        moBytes = FTUtils:getFileDataWithoutDec(filePath)

    else
        -- use lua io, cannot use in Android
        local fd, err = io.open(filePath, 'rb')
        if fd then
            moBytes = fd:read('*all')
            fd:close()
        end
    end

    return moBytes
end


function MoParser:parseMoFile_(moBytes)
    if moBytes == nil then return false, MoParser.ErrMsg.NotFind end

    -------------------------------------------------
    -- predefine some functions

    local peekLongBig = function(bytes, offs)
        local a, b, c, d = string.byte(bytes, offs + 1, offs + MoDefine.BYTE_SIZE)
        return ((d * 256 + c) * 256 + b) * 256 + a
    end

    local peekLongLittle = function(bytes, offs)
        local a, b, c, d = string.byte(bytes, offs + 1, offs + MoDefine.BYTE_SIZE)
        return ((a * 256 + b) * 256 + c) * 256 + d
    end

    local splitString = function(src, delimiter)
        local pos, strs, idx = 0, {}, 1
        local src, delimiter = tostring(src), tostring(delimiter)
        for st, sp in function() return string.find(src, delimiter, pos, true) end do
            strs[idx] = string.sub(src, pos, st - 1)
            idx = idx + 1
            pos = sp + 1
        end
        strs[idx] = string.sub(src, pos)
        return strs
    end

    -------------------------------------------------
    -- check format in MO file.
    -- @see http://www.gnu.org/savannah-checkouts/gnu/gettext/manual/html_node/MO-Files.html

    local peekLong = nil
    local magic    = string.sub(moBytes, 1, MoDefine.BYTE_SIZE)

    if magic == MoDefine.MAGIC_BIG then
        peekLong = peekLongBig

    elseif magic == MoDefine.MAGIC_LITTLE then
        peekLong = peekLongLittle

    else
        return false, MoParser.ErrMsg.NoValid
    end

    -------------------------------------------------
    -- check version

    local version = peekLong(moBytes, 4)
    if version ~= 0 then
        return false, MoParser.ErrMsg.Version
    end

    -------------------------------------------------
    -- get number of N, o, T, S, H

    local moN = peekLong(moBytes, 8)
    local moO = peekLong(moBytes, 12)
    local moT = peekLong(moBytes, 16)
    local moS = peekLong(moBytes, 20)
    local moH = peekLong(moBytes, 24)

    -------------------------------------------------
    -- translation and get strings

    local pluralSepLen  = string.len(MoDefine.PLURAL_SEP)
    local contextSepLen = string.len(MoDefine.CONTEXT_SEP)
    for i = 1, moN do
        local originalLength    = peekLong(moBytes, moO)
        local originalOffset    = peekLong(moBytes, moO + MoDefine.BYTE_SIZE)
        local translationLength = peekLong(moBytes, moT)
        local translationOffset = peekLong(moBytes, moT + MoDefine.BYTE_SIZE)
        local originalStr       = string.sub(moBytes, originalOffset + 1, originalOffset + originalLength)
        local translationStr    = string.sub(moBytes, translationOffset + 1, translationOffset + translationLength)

        moO = moO + MoDefine.CHUNK_SIZE
        moT = moT + MoDefine.CHUNK_SIZE

        if 0 == originalLength then
            -- parse header infos
            local infoPos, sepLen = 0, string.len(MoDefine.HEADER_SEP)
            for _, info in ipairs(splitString(translationStr, '\n')) do
                local separatorPos = string.find(info, MoDefine.HEADER_SEP)
                if separatorPos then
                    local infoKey   = string.sub(info, 0, separatorPos - 1)
                    local infoValue = string.sub(info, separatorPos + sepLen)
                    self.handerInfos_[infoKey] = infoValue
                end
            end

            -- parse pluralForms
            local pluralFormStr = self:getHanderInfos()[MoDefine.PLURAL_INFO_KEY]
            if pluralFormStr then
                local pluralFormStrs = splitString(pluralFormStr, MoDefine.FORM_INFO_SEP)
                local npluralStr     = string.sub(pluralFormStrs[1], string.len(MoDefine.NPLURAL_KEY) + 1)
                local pluralStr      = string.sub(pluralFormStrs[2], string.len(MoDefine.PLURAL_KEY) + 1)
                for symbol, luaValue in pairs(MoDefine.LogicSymbolMap) do
                    pluralStr = string.gsub(pluralStr, symbol, luaValue)
                end
                self.nplurals_  = tonumber(npluralStr) or 0
                self.pluralExp_ = pluralStr
            end

        else
            --[[
                Build a TranslationEntry from original string and translation strings, found in a MO file.
                original strring contain 0x04 as context separator or 0x00 as singular/plural separator.
                translation string contain 0x00 as a plural translations separator.
            ]]
            local entry = Entry.new()

            -- parse original string
            local contextPos = string.find(originalStr, MoDefine.CONTEXT_SEP)
            if contextPos then
                local original = string.sub(originalStr, contextPos + pluralSepLen)
                entry:setContext(string.sub(originalStr, 0, contextPos - 1))
                entry:setOriginals(splitString(original, MoDefine.PLURAL_SEP))
            else
                entry:setOriginals(splitString(originalStr, MoDefine.PLURAL_SEP))
            end

            -- parse translation string
            entry:setTranslations(splitString(translationStr, MoDefine.PLURAL_SEP))

            -- entry cache append
            local keyword = self:buildEntryKey_(entry:getSingular(), entry:getContext())
            if keyword then
                self.entryMap_[keyword] = entry
            end
        end
    end

    return true
end


function MoParser:buildEntryKey_(singular, context)
    if context then
        return tostring(singular) .. MoDefine.CONTEXT_SEP .. context
    else
        return tostring(singular)
    end
end


return MoParser
