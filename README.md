I18N Gettext C++ Supported
==========================

[Gettext][1] is an internationalization and localization (i18n) system, commonly used for writing multilingual programs.
However, here's supported classes working on parse binary .mo (Machine Object) files only.
So, the supported classes is lightweight, free, easy to use. And It uses C++.
It works on iOS, Android, OS X, Windows and Linux. And cocos2d-x supported.
Finally, it is recommended to use [Poedit][2] to compiled with into binary .mo files. (It is also a multiple platforms)



#Usage Tests
--------------------

* precache test function 
```
void output(std::string str) {
    std::cout << str << std::endl;
}
```

* general text
```
output(__("Hello world!"));
```

* context text
```
output(_x("post", "A post."));
output(_x("post", "To post."));
```
   
* plural text
```
output(_n("There is a comment.", "There are comments.", 1));
output(_n("There is a comment.", "There are comments.", 3));
```

* plural + context
```
output(_nx("This apple belongs to them.", "These apples belong to them.", 30, "male"));
output(_nx("This apple belongs to them.", "These apples belong to them.", 7, "female"));
output(_nx("This apple belongs to them.", "These apples belong to them.", 1, "animal"));
```

* formatMatch function
```
output(i18nFormat("c:%c1 d:%d2 f:%.2f3 s:%s4 %:%", '@', 30, 3.1415, "str"));
//c:@ d:30 f:3.14 s:str %:%)
output(i18nFormat("%s1.a = %s1.b = %s1.c = %s1.d = %d2", "foo", 7));
//foo.a = foo.b = foo.c = foo.d = 18)
output(i18nFormat("%d1 < %d2; %d2 > %d3; %d3 > %d1", 1, 3, 2));
//1 < 3; 3 > 2; 2 > 1)
```

* formatMatch + gettext
```
i18nFormatStr(__("There are birds %s1 in the %s2."), __("singing").c_str(), __("tree").c_str());
```

* load MO file
```
I18nUtils::getInstance()->addMO("res/zh_Hans.mo", [](int){return 0;}, 1);
```

* Cocos2d-x Supported
```
Data moData = FileUtils::getInstance()->getDataFromFile("res/zh_Hans.mo");
I18nUtils::getInstance()->addMO(moData.getBytes(), [](int){return 0;}, 1);
```



#How to use Poedit
--------------------
### Presetting
1. Download [Poedit][2], install and open it.
2. First, click `File – Preferences`, fill identity info.
3. Switch to `Editor` tab, select `Automatically compile .mo file on save` and `Show summary after catalog update`.

### New File
1. Click `File - New…`, then select `Lanaguage of the translation`.
2. Click `Catalogue - Properties...`, select `Translation properties` tab:
	* `Project name and version`: I18N Test 1.0 (example)
	* `Plural Forms`：If there is no special requirements then select `Use default rules for this language`.
	Note: Plural Forms is important, it is input class to used.
	```
	//nplural=INTEGER; plural=EXPRESSION
	I18nUtils::getInstance()->addMO(..., [](int n){return EXPRESSION;}, INTEGER);
	```
	* `Charset`：UTF-8 (recommended)
	* `Source code charset`：UTF-8 (recommended)
3. Then save to you project resource directory. (Because after the Settings required to save first)
4. Click `Catalogue - Properties...`, select `Sources paths` tab:
	* `Base path`: `../src` (example)
	Note: ".." is parent directory, here is required to input relative path.
	* `Paths`: click add new, then input `.` .
5. Click `Catalogue - Properties...`, select `Sources keywords` tab, add all supported keywords:
	* `__`
	* `_x:1,2c`
	* `_n:1,2`
	* `_nx:1,2,4c`
	* `_n_noop:1,2`
	* `_nx_noop:1,2,3c`
6. Confirm and Save.

### Usage Poedit
1. First, use the keywords in the source file wrap string need to be translated.
2. Open .po file, click on the Poedit main UI of `Update` button to parsing keywords in the source file.
3. Translation the string and save, automatically compile .mo file on save.
4. Load the .mo file in you project to be used. 



#Contact
--------------------
E-mail: <kaishiqi@icloud.com>



[1]: http://en.wikipedia.org/wiki/Gettext "Gettext"
[2]: http://poedit.net/ "Poedit"
