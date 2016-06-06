/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2014 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifndef PHP_MONIP_H
#define PHP_MONIP_H

extern zend_module_entry monip_module_entry;
#define phpext_monip_ptr &monip_module_entry

#define PHP_MONIP_VERSION "0.1.1" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_MONIP_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_MONIP_API __attribute__ ((visibility("default")))
#else
#	define PHP_MONIP_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#define MOINIP_DEBUG 1
#define MONIP_CACHE_DATA_PER_NAME "monip_cache_data_persistent_name"

#define IP_PRO_NAME_OFFSET "offset"
#define IP_PRO_NAME_INDEX "index"
#define IP_PRO_NAME_STREAM "stream"
#define IP_PRO_NAME_CACHE "cache"


PHP_MINIT_FUNCTION(monip);
PHP_MSHUTDOWN_FUNCTION(monip);
PHP_RINIT_FUNCTION(monip);
PHP_RSHUTDOWN_FUNCTION(monip);
PHP_MINFO_FUNCTION(monip);

extern zend_class_entry *monip_ce;

PHP_METHOD(monip_ce, __construct);
PHP_METHOD(monip_ce, __destruct);
PHP_METHOD(monip_ce, find);
PHP_METHOD(monip_ce, info);

typedef struct {
  HashTable *cache;
  HashTable *cache_expire_time;
} monip_ipdata;

ZEND_BEGIN_MODULE_GLOBALS(monip)
	zend_bool  debug;
	zend_bool  cache_enable;
	long  cache_expire_time;
	char *default_ipdata_file;
ZEND_END_MODULE_GLOBALS(monip)



/* In every utility function you add that needs to use variables
   in php_monip_globals, call TSRMLS_FETCH(); after declaring other
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as MONIP_G(variable).  You are
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define MONIP_G(v) TSRMG(monip_globals_id, zend_monip_globals *, v)
#else
#define MONIP_G(v) (monip_globals.v)
#endif

#endif	/* PHP_MONIP_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
