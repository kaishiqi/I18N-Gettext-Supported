//
//  Created by kaishiqi on 14-8-5.
//  Copyright (c) 2014 kaishiqi. All rights reserved.
//

#ifndef __I18N__I18nDefine_H__
#define __I18N__I18nDefine_H__


/* namespace I18N {} */
#ifdef __cplusplus
    #define NS_I18N_BEGIN                     namespace I18N {
    #define NS_I18N_END                       }
    #define USING_NS_I18N                     using namespace I18N
#else
    #define NS_I18N_BEGIN
    #define NS_I18N_END
    #define USING_NS_I18N
#endif


/* @see I18nUtils::formatMatch */
#define FORMAT_MATCH_ENABLE 1
#define FORMAT_MATCH_UNABLE 0
#define IS_ENABLE_FORMAT_MATCH_ARGS_INDEX FORMAT_MATCH_ENABLE


/* @see I18nUtils::formatMatch */
#define i18nFormat(format, ...) I18nUtils::getInstance()->formatMatch(format, ##__VA_ARGS__)
#define i18nFormatStr(format, ...) I18nUtils::getInstance()->formatMatch(format.c_str(), ##__VA_ARGS__)


#ifdef _MSC_VER
#define snprintf _snprintf
typedef unsigned long ssize_t;
#endif

#endif /* defined(__I18N__I18nDefine_H__) */
