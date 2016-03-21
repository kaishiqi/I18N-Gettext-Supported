--[[
 * author : kaishiqi
 * descpt : create on 2016-03-18
]]

i18n = i18n or {}


-- require language code defines
i18n.langCode = require 'i18n.LangCode'


-- require I18nUtils
i18n.i18nUtils   = require 'i18n.I18nUtils'
i18n.D_DEFAULT   = i18n.i18nUtils.D_DEFAULT
i18n.addMO       = i18n.i18nUtils.addMO
i18n.hasMO       = i18n.i18nUtils.hasMO
i18n.removeMO    = i18n.i18nUtils.removeMO
i18n.removeAllMO = i18n.i18nUtils.removeAllMO


-- general text
function __(text, domain)
    return i18n.i18nUtils.gettext(text, domain)
end


-- context text
function _x(text, context, domain)
	return i18n.i18nUtils.xgettext(text, context, domain)
end


-- plural text
function _n(singular, plural, number, domain)
	return i18n.i18nUtils.ngettext(singular, plural, number, domain)
end


-- plural + context
function _nx(singular, plural, number, context, domain)
	return i18n.i18nUtils.nxgettext(singular, plural, number, context, domain)
end


function _n_noop(singular, plural, domain)
	return i18n.i18nUtils.ngettextNoop(singular, plural, domain)
end


function _nx_noop(singular, plural, context, domain)
	return i18n.i18nUtils.nxgettextNoop(singular, plural, context, domain)
end


function t_nooped(noopEntry, number, domain)
	return i18n.i18nUtils.translateNoop(noopEntry, count, domain)
end