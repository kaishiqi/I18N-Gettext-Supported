--[[
 * author : kaishiqi
 * descpt : create on 2016-03-18
]]
local Entry = {}
Entry.__index = Entry


-------------------------------------------------
-- life cycle

function Entry.new()
    local instance = setmetatable({}, Entry)
    instance:ctor()
    return instance
end


function Entry:ctor()
	self.context_      = nil
	self.originals_    = nil
	self.translations_ = nil
end


-------------------------------------------------
-- get / set

function Entry:getContext()
	return self.context_
end
function Entry:setContext(str)
	self.context_ = str
end


function Entry:getOriginals()
	return self.originals_
end
function Entry:setOriginals(strs)
	self.originals_ = strs
end


function Entry:getTranslations()
	return self.translations_
end
function Entry:setTranslations(strs)
	self.translations_ = strs
end


function Entry:isPlural()
	local originals = self:getOriginals() or {}
	return #originals > 1
end


function Entry:getSingular()
	local originals = self:getOriginals() or {}
	return originals[1]
end


function Entry:getPlural()
	local originals = self:getOriginals() or {}
	return originals[2]
end


function Entry:__tostring()
	local str = '[Entry]'
	if self:getContext() then
		str = str .. '\n  context:\n\t' .. self:getContext()
	end
	if self:getOriginals() then
		str = str .. '\n  original:\n\t' .. table.concat(self:getOriginals(), '\n\t')
	end
	if self:getTranslations() then
		str = str .. '\n  translation:\n\t' .. table.concat(self:getTranslations(), '\n\t')
	end
	return str
end


return Entry
