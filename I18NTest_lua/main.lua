require 'i18n.init'


local MO_MAP = {
	[i18n.langCode.Chinese_Simplified]  = 'res/zh_Hans.mo',
	[i18n.langCode.Chinese_Traditional] = 'res/zh_Hant.mo',
	[i18n.langCode.Japanese]            = 'res/ja.mo',
	[i18n.langCode.Korean]              = 'res/ko.mo',
	[i18n.langCode.French]              = 'res/fr.mo',
	[i18n.langCode.German]              = 'res/de.mo',
	[i18n.langCode.Russian]             = 'res/ru.mo',
}


local printText = function()
	-- general text
	print(__('Hello world!'))

	-- context text
	print(_x('post', 'A post.'))
	print(_x('post', 'To post.'))

	-- plural text
	print(_n('There is a comment.', 'There are comments.', 1))
	print(_n('There is a comment.', 'There are comments.', 3))

	-- plural + context
	print(_nx('This apple belongs to them.', 'These apples belong to them.', 30, 'male'))
	print(_nx('This apple belongs to them.', 'These apples belong to them.', 7, 'female'))
	print(_nx('This apple belongs to them.', 'These apples belong to them.', 1, 'animal'))

	-- noop text (don't translate them now)
	local messages = {
	    _n_noop('hero', 'heroes'),
	    _n_noop('book', 'books'),
	    _n_noop('child', 'children'),
	    _n_noop('knife', 'knives'),
	    _n_noop('mouse', 'mice'),
	    _nx_noop('he', 'they', 'male'),
	    _nx_noop('she', 'they', 'female'),
	    _nx_noop('it', 'they', 'object')
	}
	print('noops:')
	for i,v in ipairs(messages) do
		print(i, t_nooped(v, i))
	end
end


print('-- i18n translate usage --')
print('[Engilish] Original Text')
printText()

print('\n[Chinese (Simplified)]')
i18n.addMO(MO_MAP[i18n.langCode.Chinese_Simplified])
printText()

print('\n[Chinese (Traditional)]')
i18n.addMO(MO_MAP[i18n.langCode.Chinese_Traditional])
printText()

print('\n[Japanese')
i18n.addMO(MO_MAP[i18n.langCode.Japanese])
printText()

print('\n[Korean]')
i18n.addMO(MO_MAP[i18n.langCode.Korean])
printText()

print('\n[French]')
i18n.addMO(MO_MAP[i18n.langCode.French])
printText()

print('\n[German]')
i18n.addMO(MO_MAP[i18n.langCode.German])
printText()

print('\n[Russian]')
i18n.addMO(MO_MAP[i18n.langCode.Russian])
printText()


print('-- domain usage --')
print('[load modules...]')
i18n.addMO(MO_MAP[i18n.langCode.Chinese_Simplified], 'module1')
i18n.addMO(MO_MAP[i18n.langCode.Japanese], 'module2')
i18n.addMO(MO_MAP[i18n.langCode.Russian], 'module3')
i18n.addMO(MO_MAP[i18n.langCode.Korean], 'module4')
print(__('Hello world!', 'module1'))
print(__('Hello world!', 'module2'))
print(__('Hello world!', 'module3'))
print(__('Hello world!', 'module4'))

print('[unload modules...]')
i18n.removeAllMO()
print(__('Hello world!', 'module1'))
print(__('Hello world!', 'module2'))
print(__('Hello world!', 'module3'))
print(__('Hello world!', 'module4'))
