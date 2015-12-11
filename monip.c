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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_monip.h"

#ifdef NETWARE
#include <netinet/in.h>
#endif

#ifndef PHP_WIN32
#include <netdb.h>
#else
#include "win32/inet.h"
#endif

#if HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

/* If you declare any globals in php_monip.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(monip)
*/

/* True global resources - no need for thread safety here */
static int le_monip;

zend_class_entry *monip_ce;

static char machine_little_endian;

/* {{{ monip_functions[]
 *
 * Every user visible function must have an entry in monip_functions[].
 */
const zend_function_entry monip_functions[] = {
	PHP_FE_END	/* Must be the last line in monip_functions[] */
};
/* }}} */

const zend_function_entry monip_methods[] = {
    PHP_ME(monip_ce, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(monip_ce, __destruct, NULL, ZEND_ACC_DTOR | ZEND_ACC_PUBLIC)
    PHP_ME(monip_ce, find, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};


static uint lb_reverse(uint a){
    union {
        uint i;
        char c[4];
    }u, r;
    
    u.i = a;
    r.c[0] = u.c[3];
    r.c[1] = u.c[2];
    r.c[2] = u.c[1];
    r.c[3] = u.c[0];
    
    return r.i;
}


static char *monip_gethostbyname(char *name)
{
    struct hostent *hp;
    struct in_addr in;
    
    hp = gethostbyname(name);
    
    if (!hp || !*(hp->h_addr_list)) {
        return estrdup(name);
    }
    
    if(hp->h_addrtype != AF_INET){
        return estrdup(name);
    }
    
    memcpy(&in.s_addr, *(hp->h_addr_list), sizeof(in.s_addr));
    
    return estrdup(inet_ntoa(in));
}

static void monip_split(zval *return_value, char *str, uint str_len TSRMLS_DC){
    char *p1, *p2, *endp;
    
    array_init(return_value);
    
    endp = str + str_len;		//end of ip_addr
    p1 = str;
    p2 = php_memnstr(p1, ZEND_STRL("\t"), endp);
    
    if(p2 == NULL){
        add_next_index_stringl(return_value, p1, str_len, 1);
    }else{
        do{
            if (p2 - p1 > 1) {
                add_next_index_stringl(return_value, p1, (uint)(p2 - p1), 1);
            }
            p1 = p2 + 1;
        }while((p2 = php_memnstr(p1, ZEND_STRL("\t"), endp)) != NULL);
        
        if (p1 < endp){
            add_next_index_stringl(return_value, p1, (uint)(endp-p1), 1);
        }
    }
    
}


/* {{{ monip_module_entry
 */
zend_module_entry monip_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"monip",
	monip_functions,
	PHP_MINIT(monip),
	PHP_MSHUTDOWN(monip),
	PHP_RINIT(monip),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(monip),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(monip),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_MONIP_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_MONIP
ZEND_GET_MODULE(monip)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("monip.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_monip_globals, monip_globals)
    STD_PHP_INI_ENTRY("monip.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_monip_globals, monip_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_monip_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_monip_init_globals(zend_monip_globals *monip_globals)
{
	monip_globals->global_value = 0;
	monip_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(monip)
{
    zend_class_entry ce;
    int machine_endian_check = 1;
    
    INIT_CLASS_ENTRY(ce, "monip", monip_methods);
    monip_ce = zend_register_internal_class(&ce TSRMLS_CC);
    
    machine_little_endian = ((char *)&machine_endian_check)[0];
    
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(monip)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(monip)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(monip)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(monip)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "monip support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


PHP_METHOD(monip_ce, __construct){
    char *ip_file;
    uint ip_file_len;
    uint offset, index_len;
    char *index;
    zval *zindex;
    zval *stream;
    zval *cache;
    php_stream *f_stream;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &ip_file, &ip_file_len) == FAILURE) {
        RETURN_FALSE;
    }
    
    f_stream = php_stream_open_wrapper(ip_file, "rb", USE_PATH | REPORT_ERRORS, NULL);
    if (!f_stream) {
        php_error(E_NOTICE, "Invalid 17monipdb.dat file!");
        RETURN_FALSE;
    }
    
    php_stream_rewind(f_stream);
    php_stream_read(f_stream, (void*)&offset, 4);
    
    if (machine_little_endian) {
        offset = lb_reverse(offset);
    }
    
    if (offset < 4) {
        php_stream_close(f_stream);
        RETURN_FALSE;
    }
    
    index_len = offset - 4;
    //php_printf("%d", index_len);
    index = (char *)emalloc(sizeof(char) * index_len + 1);
    php_stream_read(f_stream, index, index_len);
    index[index_len] = 0;
    MAKE_STD_ZVAL(stream);
    php_stream_to_zval(f_stream, stream);
    
    MAKE_STD_ZVAL(zindex);
    ZVAL_STRINGL(zindex, index, index_len, 1);
    
    MAKE_STD_ZVAL(cache);
    array_init(cache);
    
    add_property_long(getThis(), "offset", offset);
    add_property_zval_ex(getThis(), ZEND_STRS("index"), zindex TSRMLS_CC);
    add_property_zval_ex(getThis(), ZEND_STRS("stream"), stream TSRMLS_CC);
    add_property_zval_ex(getThis(), ZEND_STRS("cache"), cache TSRMLS_CC);
    
    efree(index);
    zval_ptr_dtor(&stream);
    zval_ptr_dtor(&zindex);
    zval_ptr_dtor(&cache);
    
}


PHP_METHOD(monip_ce, __destruct){
    zval * arg;
    php_stream *f_stream;
    
    arg = zend_read_property(monip_ce, getThis(), ZEND_STRL("stream"), 0 TSRMLS_CC);
    if (IS_NULL != Z_TYPE_P(arg)) {
        php_stream_from_zval_no_verify(f_stream, &arg);
        if (f_stream != NULL) {
            if ((f_stream->flags & PHP_STREAM_FLAG_NO_FCLOSE) != 0) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "%d is not a valid stream resource", f_stream->rsrc_id);
                return;
            }
            php_stream_close(f_stream);
        }
    }
}


PHP_METHOD(monip_ce, find){
    char *ipstr;
    ulong ipstr_len;
    
    char *ip;
    uint ip_len;
    struct in_addr in;
    uint ip_h;
    
    zval *p, *cache, **cache_val;
    ulong offset;
    char *index;
    php_stream *stream;
    
    char *ip_repeat, *tmp;
    char *ip_dot;
    
    int tmp_offset = 0, start = 0;
    uint index_offset = 0, index_length = 0, max_comp_len = 0;
    
    char *location;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &ipstr, &ipstr_len) == FAILURE) {
        RETURN_NULL();
    }
    
    if (ipstr_len < 1) {
        RETURN_NULL();
    }
    
    p = zend_read_property(monip_ce, getThis(), ZEND_STRL("offset"), 0 TSRMLS_CC);
    //php_printf("%ld\n", Z_LVAL_P(p));
    offset = Z_LVAL_P(p);
    
    p = zend_read_property(monip_ce, getThis(), ZEND_STRL("index"), 0 TSRMLS_CC);
    index = Z_STRVAL_P(p);
    
    p = zend_read_property(monip_ce, getThis(), ZEND_STRL("stream"), 0 TSRMLS_CC);
    php_stream_from_zval(stream, &p);
    
    cache = zend_read_property(monip_ce, getThis(), ZEND_STRL("cache"), 0 TSRMLS_CC);
    
    ip = monip_gethostbyname(ipstr);
    ip_len = (uint)strlen(ip);
    //php_printf("\n%s\n", ip);
    if (inet_pton(AF_INET, ip, &in) != 1) {
        efree(ip);
        RETURN_NULL();
    }
    
    ip_h = ntohl(in.s_addr);
    if(machine_little_endian){
        ip_h = lb_reverse(ip_h);
    }
    
    if (zend_hash_index_find(Z_ARRVAL_P(cache), ip_h, (void **)&cache_val) == SUCCESS) {
        //php_printf("\ncahce find, type %d\n", Z_TYPE_PP(cache_val));
        monip_split(return_value, Z_STRVAL_PP(cache_val), Z_STRLEN_PP(cache_val) TSRMLS_CC);
        efree(ip);
        return;
    }
    
    ip_repeat = estrndup(ip, ip_len);
    ip_dot = php_strtok_r(ip_repeat, ".", &tmp);
    tmp_offset = atoi(ip_dot);
    efree(ip_repeat);
    
    efree(ip);
    
    if (tmp_offset < 0 || tmp_offset > 255) {
        RETURN_NULL();
    }
    tmp_offset *= 4;
    
    max_comp_len = (uint)offset - 1024;
    
    for(start = start * 8 + 1024; start < max_comp_len; start += 8){
        
        if(memcmp(index + start, (char *)&ip_h, 4) >= 0){
            memcpy(&index_offset, index + start + 4, 3);
            if(!machine_little_endian){
                index_offset = lb_reverse(index_offset);
            }
            index_length = *(index + start + 7);
            break;
        }
    }
    
    if (index_offset < 1) {
        RETURN_NULL();
    }
    
    php_stream_seek(stream, offset + index_offset - 1024, SEEK_SET);
    
    location = (char *) emalloc(sizeof(char) * index_length + 1);
    php_stream_read(stream, location, index_length);
    location[index_length] = 0;
    
    MAKE_STD_ZVAL(p);
    ZVAL_STRINGL(p, location, index_length, 1);
    zend_hash_index_update(Z_ARRVAL_P(cache), ip_h, &p, sizeof(zval *), NULL);
    
    monip_split(return_value, location, index_length TSRMLS_CC);
    
    efree(location);
    
    return;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
