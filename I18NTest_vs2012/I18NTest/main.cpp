//
//  Created by kaishiqi on 14-8-5.
//  Copyright (c) 2014 kaishiqi. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "i18n/I18nUtils.h"
#include <windows.h>
#include <shellapi.h>

USING_NS_I18N;

int main(int argc, const char * argv[])
{
    const char MO_FILE_ZH_HANS[] = "res/zh_Hans.mo";
    const char MO_FILE_ZH_HANT[] = "res/zh_Hant.mo";
    const char MO_FILE_JA[] = "res/ja.mo";
    const char MO_FILE_KO[] = "res/ko.mo";
    const char MO_FILE_FR[] = "res/fr.mo";
    const char MO_FILE_DE[] = "res/de.mo";
    const char MO_FILE_RU[] = "res/ru.mo";
	const char OUTPUT_FILE[] = "output.txt";
	const wchar_t OUTPUT_FILE_W[] = L"output.txt";

	std::ofstream fout(OUTPUT_FILE, std::ios::out|std::ios::binary);
	if (fout.is_open()) {
		//UTF-8 dom
		char domChars[3] = {0xEF, 0xBB, 0xBF};
		fout.write(domChars, sizeof(domChars));
	} else {
		printf("output file error. (Enter key to close)\n");
		getchar();
		return 0;
	}

	auto output = [&fout](std::string str) {
		fout << str << "\r\n";
	};
    
    /**
     *
     * i18n translate usage:
     *
     */
    output("\t-- i18n translate usage --");
    
    auto showText = [&](){
        // general text
        output(__("Hello world!"));
        
        // context text
        output(_x("post", "A post."));
        output(_x("post", "To post."));
        
        // plural text
        output(_n("There is a comment.", "There are comments.", 1));
        output(_n("There is a comment.", "There are comments.", 3));
        
        // plural + context
        output(_nx("This apple belongs to them.", "These apples belong to them.", 30, "male"));
        output(_nx("This apple belongs to them.", "These apples belong to them.", 7, "female"));
        output(_nx("This apple belongs to them.", "These apples belong to them.", 1, "animal"));
        
        // noop text (don't translate them now)
        std::vector<NoopEntry> messages;
		messages.push_back(_n_noop("hero", "heroes"));
        messages.push_back(_n_noop("book", "books"));
        messages.push_back(_n_noop("child", "children"));
        messages.push_back(_n_noop("knife", "knives"));
        messages.push_back(_n_noop("mouse", "mice"));
        messages.push_back(_nx_noop("he", "they", "male"));
        messages.push_back(_nx_noop("she", "they", "female"));
        messages.push_back(_nx_noop("it", "they", "object"));
        
        // translate noop (when using translation)
        auto mo = I18nUtils::getInstance()->getMO();
        output("noops:");
        
        for (int i = 0; i < messages.size(); i++) {
            auto isSingular = (mo.onPluralExpression(i) == 0);
            std::string prestr;
#if (IS_ENABLE_FORMAT_MATCH_ARGS_INDEX == FORMAT_MATCH_ENABLE)
            prestr = i18nFormatStr(__("\ti=%d1 is %8s2: "), i, (isSingular ? __("singular").c_str() : __("plural").c_str()));
#else
            prestr = i18nFormatStr(__("\ti=%d is %8s: "), i, (isSingular ? __("singular").c_str() : __("plural").c_str()));
#endif
            output(prestr + t_nooped(messages.at(i), i));
        }
        
        // use arguments
        output(i18nFormatStr(__("You have chosen the %s1 hat."), __("red").c_str()));
        output(i18nFormatStr(__("You have chosen the %s1 hat."), __("blue").c_str()));
        output(i18nFormatStr(__("You have chosen the %s1 hat."), __("yellow").c_str()));
        
        // repeated use arguments
        std::string heroName("xxx");
        output(i18nFormatStr(__("Hi %s1! Welcome to %s1's city."), heroName.c_str()));
        
        /**
         * Sometimes the arguments' index in the translation is very important, such as grammatical inverted.
         * e.g., 
         * english: There are birds singing in the tree.
         * chinese: 小鸟在树上唱歌。
         *
         * original text: There are birds [%s1] in the [%s2]. 
         * translation text: 小鸟在[%s2]上[%s1]。
         */
        output(i18nFormatStr(__("There are birds %s1 in the %s2."), __("singing").c_str(), __("tree").c_str()));
    };
    
    /**
     * Note: onPluralExpression and nplurals,
     * need to keep consistent with *.po file Plural-Forms setting.
     */
    
    // English Plural-Forms: nplurals=2; plural=(n!=1);
    output("[Engilish] Original Text");
    showText();
    
    // Chinese Plural-Forms: nplurals=1; plural=0;
    output("\n[Chinese (Simplified)]");
    I18nUtils::getInstance()->addMO(MO_FILE_ZH_HANS, [](int){return 0;}, 1);
    showText();
    
    // Chinese Plural-Forms: nplurals=1; plural=0;
    output("\n[Chinese (Traditional)]");
    I18nUtils::getInstance()->addMO(MO_FILE_ZH_HANT, [](int){return 0;}, 1);
    showText();
    
    // Japanese Plural-Forms: nplurals=1; plural=0;
    output("\n[Japanese");
    I18nUtils::getInstance()->addMO(MO_FILE_JA, [](int){return 0;}, 1);
    showText();
    
    // Korean Plural-Forms: nplurals=1; plural=0;
    output("\n[Korean]");
    I18nUtils::getInstance()->addMO(MO_FILE_KO, [](int){return 0;}, 1);
    showText();
    
    // French Plural-Forms: nplurals=2; plural=(n > 1);
    output("\n[French]");
    I18nUtils::getInstance()->addMO(MO_FILE_FR, [](int n){return n>1;}, 2);
    showText();
    
    // Germen Plural-Forms: nplurals=2; plural=(n != 1);
    output("\n[Germen]");
    I18nUtils::getInstance()->addMO(MO_FILE_DE, [](int n){return (n!=1);}, 2);
    showText();
    
    // Russian Plural-Forms: nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);
    output("\n[Russian]");
    auto russianExpression = [](int n){
        return n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2;
    };
    I18nUtils::getInstance()->addMO(MO_FILE_RU, russianExpression, 3);
    showText();
    
    //...
    output("");
    
    /** Cocos2d-x 3.x usage */
    //Data moData = FileUtils::getInstance()->getDataFromFile(MO_FILE_ZH_HANS);
    //I18nUtils::getInstance()->addMO(moData.getBytes(), [](int){return 0;}, 1);
    
    
    
    /**
     *
     * domain argument usage:
     * e.g., multiple module project.
     *
     */
    output("\t-- domain usage --");
    
    output("[load modules...]");
    I18nUtils::getInstance()->addMO(MO_FILE_ZH_HANS, [](int){return 0;}, 1, "game1");
    I18nUtils::getInstance()->addMO(MO_FILE_JA, [](int){return 0;}, 1, "game2");
    I18nUtils::getInstance()->addMO(MO_FILE_KO, [](int){return 0;}, 1, "game3");
    I18nUtils::getInstance()->addMO(MO_FILE_FR, [](int n){return n>1;}, 2, "game4");
    I18nUtils::getInstance()->addMO(MO_FILE_DE, [](int n){return (n!=1);}, 2, "game5");
    output(__("Hello world!", "game1"));
    output(__("Hello world!", "game2"));
    output(__("Hello world!", "game3"));
    output(__("Hello world!", "game4"));
    output(__("Hello world!", "game5"));
    
    output("[unload modules...]");
    I18nUtils::getInstance()->removeAllMO();
    output(__("Hello world!", "game1"));
    output(__("Hello world!", "game2"));
    output(__("Hello world!", "game3"));
    output(__("Hello world!", "game4"));
    output(__("Hello world!", "game5"));
    
    output("");
    
    
    
    /**
     *
     * i18nFormat usage:
     *
     */
    output("\t-- i18nFormat usage --");
    
#if (IS_ENABLE_FORMAT_MATCH_ARGS_INDEX == FORMAT_MATCH_ENABLE)
    /** Same as use printf, but format % character can append arguments' index(from to 1 start). */
    
    output(i18nFormat("[enable] c:%c1 d:%d2 f:%.2f3 s:%s4 %:%", '@', 30, 3.1415, "str"));
    //output([enable] c:@ d:30 f:3.14 s:str %:%)
    
    output(i18nFormat("[enable] %s1.a = %s1.b = %s1.c = %s1.d = %d2", "foo", 7));
    //output([enable] foo.a = foo.b = foo.c = foo.d = 18)
    
    output(i18nFormat("[enable] %d1 < %d2; %d2 > %d3; %d3 > %d1", 1, 3, 2));
    //output([enable] 1 < 3; 3 > 2; 2 > 1)
#else
    /** Same as use printf, but returned std:string. */
    
    output(i18nFormat("[unable] c:%c d:%d f:%.2f s:%s %%:%%", '@', 30, 3.1415, "str"));
    //output([unable] c:@ d:30 f:3.14 s:str %:%)
#endif
    
    output("");
	fout.close();
	
	printf("output file succeed. (Enter key to close)\n");
	getchar();

	ShellExecute(NULL, L"open", OUTPUT_FILE_W, NULL, NULL, SW_SHOWNORMAL);

    return 0;
}

