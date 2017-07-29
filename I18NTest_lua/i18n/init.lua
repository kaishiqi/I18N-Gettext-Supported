--[[
 * author : kaishiqi
 * descpt : create on 2016-03-18
]]

local gtab -- Place to save glogals

if cc then
	gtab = cc.exports -- For cocos2d-x
elseif tonumber(_VERSION:sub(5))>=5.2 then -- For lua 5.2 and higher
	gtab = _ENV
else
	gtab = _G -- For other cases
end

gtab.i18n = i18n or {}


-- require language code defines
i18n.langMap = require 'i18n.LangCode'
for langCode, define in pairs(i18n.langMap) do
	local shortKey = string.gsub(langCode, '-', '_')
	i18n[shortKey] = langCode
end


-- require I18nUtils
i18n.i18nUtils   = require 'i18n.I18nUtils'
i18n.D_DEFAULT   = i18n.i18nUtils.D_DEFAULT
i18n.addMO       = i18n.i18nUtils.addMO
i18n.hasMO       = i18n.i18nUtils.hasMO
i18n.removeMO    = i18n.i18nUtils.removeMO
i18n.removeAllMO = i18n.i18nUtils.removeAllMO


-- general text
function gtab.__(text, domain)
    return i18n.i18nUtils.gettext(text, domain)
end


-- context text
function gtab._x(text, context, domain)
	return i18n.i18nUtils.xgettext(text, context, domain)
end


-- plural text
function gtab._n(singular, plural, number, domain)
	return i18n.i18nUtils.ngettext(singular, plural, number, domain)
end


-- plural + context
function gtab._nx(singular, plural, number, context, domain)
	return i18n.i18nUtils.nxgettext(singular, plural, number, context, domain)
end


function gtab._n_noop(singular, plural, domain)
	return i18n.i18nUtils.ngettextNoop(singular, plural, domain)
end


function gtab._nx_noop(singular, plural, context, domain)
	return i18n.i18nUtils.nxgettextNoop(singular, plural, context, domain)
end


function gtab.t_nooped(noopEntry, number, domain)
	return i18n.i18nUtils.translateNoop(noopEntry, number, domain)
end
