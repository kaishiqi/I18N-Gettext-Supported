--[[
 * author : kaishiqi
 * descpt : create on 2016-03-18
]]
local MoParser  = require 'i18n.MoParser'
local Entry     = require 'i18n.Entry'
local moCache   = {}
local I18nUtils = {}

I18nUtils.D_DEFAULT = 'default'


-------------------------------------------------
-- moCache
-------------------------------------------------

-- Append a mo file.
-- @param string filePath 		The .mo file path.
-- @param string domain 		The domain is cache key, default is I18nUtils.D_DEFAULT. (optional)
-- @see I18nUtils.D_DEFAULT
-- @return boolean, string 		It's the mo file init succeed. error msg @see MoParser.ErrMsg.
function I18nUtils.addMO(filePath, domain)
	local mo = MoParser.new(filePath)
	if mo:isInited() then
		local domain = domain and tostring(domain) or I18nUtils.D_DEFAULT
		moCache[domain] = mo
	end
	return mo:isInited(), mo:getErrorMsg()
end


-- Remove a mo file.
-- @param string domain 		The domain is cache key, default is I18nUtils.D_DEFAULT. (optional)
function I18nUtils.removeMO(domain)
	local domain = domain and tostring(domain) or I18nUtils.D_DEFAULT
	moCache[domain] = nil
end


-- Clear all mo cache.
function I18nUtils.removeAllMO()
	moCache = {}
end


-- Cache the domain is exists.
-- @param string domain 		The domain is cache key, default is I18nUtils.D_DEFAULT. (optional)
function I18nUtils.hasMO(domain)
	local domain = domain and tostring(domain) or I18nUtils.D_DEFAULT
	return moCache[domain] ~= nil
end



-------------------------------------------------
-- gettext
-------------------------------------------------

-- Retrieves the translation of text.
-- If there is no translation, or the domain isn't loaded, the original text is returned.
-- @param string text      		Text to translate.
-- @param string domain    		The domain is cache key, default is I18nUtils.D_DEFAULT. (optional)
-- @return string 				Translated text.
function I18nUtils.gettext(text, domain)
	return I18nUtils.translate(text, nil, domain)
end


-- Retrieve translated string with context.
-- Quite a few times, there will be collisions with similar translatable text
-- found in more than two places but with different translated context.
-- By including the context in the pot file translators can translate the two
-- strings differently.
-- @param string text      		Text to translate.
-- @param string context 		Context information for the translators.
-- @param string domain    		The domain is cache key, default is I18nUtils.D_DEFAULT. (optional)
-- @return string 				Translated text.
function I18nUtils.xgettext(text, context, domain)
	return I18nUtils.translate(text, context, domain)
end


-- Retrieve the plural or single form based on the amount.
-- If the domain is not set in the mo hash map, then a comparison will be made
-- and either plural or single parameters returned.
-- @param string singular 		The text that will be used if number is 1
-- @param string plural 		The text that will be used if number is not 1
-- @param int number        	The number to compare against to use either single or plural.
-- @param string domain    		The domain is cache key, default is I18nUtils.D_DEFAULT. (optional)
-- @return string 				Translated text.
function I18nUtils.ngettext(singular, plural, number, domain)
	return I18nUtils.translatePlural(singular, plural, number, nil, domain)
end


-- A hybrid of xgettext() and ngettext(). It supports contexts and plurals.
-- @see I18nUtils.xgettext
-- @see I18nUtils.ngettext
function I18nUtils.nxgettext(singular, plural, number, context, domain)
	return I18nUtils.translatePlural(singular, plural, number, context, domain)
end


-- Retrieve the plural strings in NoopEntry, but don't translate them.
-- Used when you want to keep structures with translatable plural strings and
-- use them later.
--[[Example:
local Messages = {
	'post' = ngettextNoop('%s post', '%s posts'),
	'page' = ngettextNoop('%s pages', '%s pages'),
	...
}
local getTranslationMsg = function(msgKey, number)
	local noopEntry   = Messages[msgKey]
	local translation = ''
	if noopEntry then
		translation = translateNooped(noopEntry, number);
	end
	return translation
end
local usableText = getTranslationMsg('post', 3)
...
]]
-- @return Entry 				@see i18n.Entry
-- @see I18nUtils.ngettext
function I18nUtils.ngettextNoop(singular, plural, domain)
	local noopEntry = Entry.new()
	noopEntry:setOriginals({singular, plural})
	return noopEntry
end


-- Retrieve the plural strings with context in NoopEntry, but don't translate them.
-- @see I18nUtils.ngettextNoop
-- @see I18nUtils.nxgettext
function I18nUtils.nxgettextNoop(singular, plural, context, domain)
	local noopEntry = Entry.new()
	noopEntry:setContext(context)
	noopEntry:setOriginals({singular, plural})
	return noopEntry
end


-------------------------------------------------
-- translate
-------------------------------------------------

function I18nUtils.translate(singular, context, domain)
	local domain      = domain and tostring(domain) or I18nUtils.D_DEFAULT
	local moParser    = moCache[domain]
	local translation = singular
	if moParser then
		translation = moParser:translate(singular, context)
	end

	-- if default domain, and not find, then check all
	if domain == I18nUtils.D_DEFAULT and translation == singular then
		for _, moParser in pairs(moCache) do
			translation = moParser:translate(singular, context)
			if translation ~= singular then
				break
			end
		end
	end
	return translation
end


function I18nUtils.translatePlural(singular, plural, count, context, domain)
	local domain      = domain and tostring(domain) or I18nUtils.D_DEFAULT
	local number      = tonumber(count) or 0
	local moParser    = moCache[domain]
	local translation = number > 1 and plural or singular
	if moParser then
		translation = moParser:translatePlural(singular, plural, count, context)
	end

	-- if default domain, and not find, then check all
	if domain == I18nUtils.D_DEFAULT and (number > 1) and (translation == plural) or (translation == singular) then
		for _, moParser in pairs(moCache) do
			translation = moParser:translatePlural(singular, plural, count, context)
			if (number > 1) and (translation ~= plural) or (translation ~= singular) then
				break
			end
		end
	end
	return translation
end


-- Translate the result of ngettextNoop() or nxgettextNoop().
-- If NoopEntry contains a domain passed to ngettextNoop() or nxgettextNoop(), it will override this value.
-- @param string domain    		The domain is cache key, default is I18nUtils.D_DEFAULT. (optional)
-- @see I18nUtils.ngettextNoop
-- @see I18nUtils.nxgettextNoop
-- @return string 				Translated text.
function I18nUtils.translateNoop(noopEntry, count, domain)
	local translation = ''
	if noopEntry then
		local context  = noopEntry:getContext()
		local singular = noopEntry:getSingular()
		local plural   = noopEntry:getPlural()
		translation    = I18nUtils.translatePlural(singular, plural, count, context, domain)
	end
	return translation
end


return I18nUtils
