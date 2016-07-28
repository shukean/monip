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

ZEND_DECLARE_MODULE_GLOBALS(monip)

/* True global resources - no need for thread safety here */
static int le_monip;

zend_class_entry *monip_ce;

static char machine_little_endian;

static int le_monip_cache_ipdata_p;
#define LE_MONIP_CACHE_IPDATA_NAME "monip_res_cache_ipdata_name"

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
    PHP_ME(monip_ce, info, NULL, ZEND_ACC_PUBLIC)
    PHP_FE_END
};


#define _unpack_n(num)              \
    if (machine_little_endian){     \
        num = ntohl(num);           \
    }

#define _unpack_v(num)              \
    if (!machine_little_endian){     \
        num = ntohl(num);           \
    }


static unsigned int monip_ip2long(zend_string *addr){
#ifdef HAVE_INET_PTON
    struct in_addr ip;
#else
    zend_ulong ip;
#endif

#ifdef HAVE_INET_PTON
    if (ZSTR_LEN(addr) == 0 || inet_pton(AF_INET, ZSTR_VAL(addr), &ip) != 1) {
        return 0;
    }
    return ntohl(ip.s_addr);
#else
    if (ZSTR_LEN(addr) == 0 || (ip = inet_addr(ZSTR_VAL(addr))) == INADDR_NONE) {
        /* The only special case when we should return -1 ourselves,
         * because inet_addr() considers it wrong. We return 0xFFFFFFFF and
         * not -1 or ~0 because of 32/64bit issues. */
        if (ZSTR_LEN(addr) == sizeof("255.255.255.255") - 1 &&
            !memcmp(ZSTR_VAL(addr), "255.255.255.255", sizeof("255.255.255.255") - 1)
        ) {
            return 0xFFFFFFFF;
        }
        return 0;
    }
    return ntohl(ip);
#endif

}

static zend_string *monip_gethostbyname(char *name, int persistent)
{
    struct hostent *hp;
    struct in_addr in;
    char *address;

    hp = gethostbyname(name);

    if (!hp || !*(hp->h_addr_list)) {
        return zend_string_init(name, strlen(name), persistent);
    }

    memcpy(&in.s_addr, *(hp->h_addr_list), sizeof(in.s_addr));

    address = inet_ntoa(in);
    return zend_string_init(address, strlen(address), persistent);
}


static void monip_split(zval *return_value, char *str, zend_long str_len){
    const char *p1, *p2, *endp;

    array_init(return_value);

    endp = str + str_len;		//end of ip_addr
    p1 = str;
    p2 = php_memnstr(p1, ZEND_STRL("\t"), endp);

    if(p2 == NULL){
        add_next_index_stringl(return_value, p1, str_len);
    }else{
        do{
            if (p2 - p1 > 1) {
                add_next_index_stringl(return_value, p1, (uint)(p2 - p1));
            }
            p1 = p2 + 1;
        }while((p2 = php_memnstr(p1, ZEND_STRL("\t"), endp)) != NULL);

        if (p1 < endp){
            add_next_index_stringl(return_value, p1, (uint)(endp-p1));
        }
    }

}

static void php_monip_cache_ipdata_res_dtor(zend_resource *res){
    monip_ipdata * ipdata;
    HashTable *cahce;
    HashTable *cache_expire_time;
    if (MONIP_G(cache_enable)){
        ipdata = res->ptr;
        cahce = ipdata->cache;
        zend_hash_destroy(cahce);
        if (MONIP_G(cache_expire_time)){
            cache_expire_time = ipdata->cache_expire_time;
            zend_hash_destroy(cache_expire_time);
        }
        pefree(ipdata, 1);
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
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("monip.debug", "0", PHP_INI_ALL, OnUpdateBool, debug, zend_monip_globals, monip_globals)
    STD_PHP_INI_ENTRY("monip.cache_enable", "0", PHP_INI_SYSTEM, OnUpdateBool, cache_enable, zend_monip_globals, monip_globals)
    STD_PHP_INI_ENTRY("monip.cache_expire_time", "0", PHP_INI_SYSTEM, OnUpdateLong, cache_expire_time, zend_monip_globals, monip_globals)
    STD_PHP_INI_ENTRY("monip.file_type", "0", PHP_INI_SYSTEM, OnUpdateBool, file_type, zend_monip_globals, monip_globals)
	STD_PHP_INI_ENTRY("monip.default_ipdata_file", NULL, PHP_INI_SYSTEM, OnUpdateString, default_ipdata_file, zend_monip_globals, monip_globals)
PHP_INI_END()
/* }}} */

/* {{{ php_monip_init_globals
 */
static void php_monip_init_globals(zend_monip_globals *monip_globals)
{
	monip_globals->debug = 0;
	monip_globals->cache_enable = 0;
	monip_globals->cache_expire_time = 0;
    monip_globals->file_type = 0;
	monip_globals->default_ipdata_file = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(monip)
{
    zend_class_entry ce;
    int machine_endian_check = 1;

	INIT_CLASS_ENTRY(ce, "Yk\\Ip", monip_methods);
    /*INIT_CLASS_ENTRY(ce, "monip", monip_methods);*/
    monip_ce = zend_register_internal_class(&ce TSRMLS_CC);
	zend_register_class_alias("monip", monip_ce);

    machine_little_endian = ((char *)&machine_endian_check)[0];

	REGISTER_INI_ENTRIES();

	if (MONIP_G(cache_enable)){
		le_monip_cache_ipdata_p = zend_register_list_destructors_ex(NULL, php_monip_cache_ipdata_res_dtor, LE_MONIP_CACHE_IPDATA_NAME, module_number);
    }

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(monip)
{
	UNREGISTER_INI_ENTRIES();

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

	DISPLAY_INI_ENTRIES();
}
/* }}} */


PHP_METHOD(monip_ce, __construct){
    zend_string *ip_file = NULL;
    zend_bool   file_type = 0, retval_type = 0, use_conf_ip_file = 0;
    char *index;
    uint offset, index_len;
    php_stream *stream;
    zval *this_ptr = getThis();
    zval pro_stream, pro_index, pro_data;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|Sbb", &ip_file, &file_type, &retval_type) == FAILURE) {
        RETURN_FALSE;
    }
    
    if (MONIP_G(cache_enable) && ZSTR_LEN(ip_file) > 0) {
        php_error_docref(NULL, E_NOTICE, "Invalid set ip file, if cache opened !");
        ip_file = NULL;
    }

    if (ZSTR_LEN(ip_file) < 1){
        if (!MONIP_G(default_ipdata_file) || strlen(MONIP_G(default_ipdata_file)) < 1){
            php_error_docref(NULL, E_ERROR, "Must set ip file in php.ini");
            RETURN_FALSE;
        }
        ip_file = zend_string_init(MONIP_G(default_ipdata_file), strlen(MONIP_G(default_ipdata_file)), 0);
        use_conf_ip_file = 1;
    }else{
        MONIP_G(file_type) = file_type ? 1 : 0;
    }

    stream = php_stream_open_wrapper(ZSTR_VAL(ip_file), "rb", USE_PATH | REPORT_ERRORS, NULL);
    if (!stream) {
        php_error_docref(NULL, E_ERROR, "Can't open ip file : %s !", ZSTR_VAL(ip_file));
        RETURN_FALSE;
    }

    if (use_conf_ip_file){
        zend_string_release(ip_file);
    }

    php_stream_rewind(stream);
    php_stream_read(stream, (void*)&offset, 4);

    _unpack_n(offset);

    index_len = offset - 4;
    //php_printf("%d", index_len);
    index = (char *)emalloc(sizeof(char) * index_len + 1);
    php_stream_read(stream, index, index_len);
    index[index_len] = '\0';

    php_stream_to_zval(stream, &pro_stream);
    ZVAL_STRINGL(&pro_index, index, index_len);

    add_property_long(this_ptr, IP_PRO_NAME_OFFSET, offset);
    add_property_zval(this_ptr, IP_PRO_NAME_INDEX, &pro_index);
    add_property_zval(this_ptr, IP_PRO_NAME_STREAM, &pro_stream);
    add_property_bool(this_ptr, IP_PRO_NAME_RETVAL_TYPE, retval_type);

	if (!MONIP_G(cache_enable)){
		array_init(&pro_data);
		add_property_zval(this_ptr, IP_PRO_NAME_CACHE, &pro_data);
		zval_ptr_dtor(&pro_data);
    }else{
        if (zend_hash_str_find_ptr(&EG(persistent_list), MONIP_CACHE_DATA_PER_NAME, sizeof(MONIP_CACHE_DATA_PER_NAME)-1) == NULL) {
            zend_resource res;
            zend_string *res_key;
            monip_ipdata *ipdata;
            HashTable *cache_ht;
            HashTable *cache_expire_time_ht;
            
            ipdata = pemalloc(sizeof(ipdata), 1);
            
            cache_ht = (HashTable *) pemalloc(sizeof(HashTable), 1);
            zend_hash_init(cache_ht, 64, NULL, ZVAL_PTR_DTOR, 1);
            
            ipdata->cache = cache_ht;
            
            if (MONIP_G(cache_expire_time)){
                cache_expire_time_ht = (HashTable *) pemalloc(sizeof(HashTable), 1);
                zend_hash_init(cache_expire_time_ht, 64, NULL, ZVAL_PTR_DTOR, 1);
                ipdata->cache_expire_time = cache_expire_time_ht;
            }
            
            res.type = le_monip_cache_ipdata_p;
            res.ptr = ipdata;
            
            res_key = zend_string_init(MONIP_CACHE_DATA_PER_NAME, sizeof(MONIP_CACHE_DATA_PER_NAME) - 1, 1);
            if (zend_hash_update_mem(&EG(persistent_list), res_key, (void *)&res, sizeof(res)) == NULL) {
                php_error_docref(NULL, E_ERROR, "Register persistent list fail");
            }
        }
    }

    efree(index);
    zval_ptr_dtor(&pro_stream);
    zval_ptr_dtor(&pro_index);

}


PHP_METHOD(monip_ce, __destruct){
    zval *arg;
    zval rv;
    php_stream *stream = NULL;
    zval *this_ptr = getThis();

	if (!MONIP_G(cache_enable)){
		arg = zend_read_property(monip_ce, this_ptr, ZEND_STRL(IP_PRO_NAME_CACHE), 1, &rv);
		if (Z_TYPE_P(arg) == IS_ARRAY) {
			zend_hash_destroy(Z_ARR_P(arg));
		}
	}

    arg = zend_read_property(monip_ce, this_ptr, ZEND_STRL(IP_PRO_NAME_STREAM), 1, &rv);
    if (Z_TYPE_P(arg) != IS_NULL) {
        php_stream_from_zval_no_verify(stream, arg);
        if (stream != NULL) {
            if ((stream->flags & PHP_STREAM_FLAG_NO_FCLOSE) != 0) {
                php_error_docref(NULL, E_WARNING, "Ip file stream is closed");
                return;
            }
            php_stream_close(stream);
        }
    }
}


PHP_METHOD(monip_ce, info){
    zval *c_data;
    zval rv;
    zval *this_ptr = getThis();
    HashTable *ipdata_cache;
    HashPosition pos;

    if (MONIP_G(cache_enable)){
        zend_resource *le;
        if ((le = zend_hash_str_find_ptr(&EG(persistent_list), MONIP_CACHE_DATA_PER_NAME, sizeof(MONIP_CACHE_DATA_PER_NAME)-1)) != NULL) {
            monip_ipdata *ipdata;
            if (le->type != le_monip_cache_ipdata_p){
                php_error_docref(NULL, E_ERROR, "Cache data type error!");
                RETURN_FALSE;
            }
            ipdata = (monip_ipdata *)le->ptr;
            ipdata_cache = ipdata->cache;
        }else{
            php_error_docref(NULL, E_ERROR, "Cache data not found!");
            RETURN_FALSE;
        }

        if (MONIP_G(debug)){
            php_error_docref(NULL, E_NOTICE, "Cache data Info from global!");
        }

        array_init(return_value);
        zend_hash_internal_pointer_reset_ex(ipdata_cache, &pos);
        for (;; zend_hash_move_forward_ex(ipdata_cache, &pos)) {
            zval _val, *item;
            zend_ulong intindex = 0;
            zend_string *strindex;

            item = zend_hash_get_current_data_ex(ipdata_cache, &pos);
            if (!item){
                break;
            }
            zend_hash_get_current_key_ex(ipdata_cache, &strindex, &intindex, &pos);

            // ZVAL_STR_COPY(&_key, strindex);
            ZVAL_DUP(&_val, item);

            zend_hash_update(Z_ARRVAL_P(return_value), strindex, &_val);
        }

    }else{

        if (MONIP_G(debug)){
            php_error_docref(NULL, E_NOTICE, "Cache data Info from class object!");
        }

        c_data = zend_read_property(monip_ce, this_ptr, ZEND_STRL(IP_PRO_NAME_CACHE), 0, &rv);
        RETVAL_ZVAL(c_data, 1, 0);
    }

    return;
}


PHP_METHOD(monip_ce, find){
    zend_string *str_ip;
    zval *this_ptr = getThis();
    zval *retval_type, *pro_offset, *pro_index, *pro_stream, *pro_data, rv;
    HashTable *cache_ip = NULL, *cache_ip_time = NULL;
    php_stream *stream;
    zval *ip, ip_val;
    unsigned int ip_idx = 0;
    unsigned int tmp_offset = 0, start = 0, inc_num = 0, index_offset = 0, found_result = 0;
    zend_long max_comp_len = 0;
    unsigned int index_length = 0;
    char *location;
    zend_long cur_time = (zend_long)time(NULL);
    zend_bool ip_file_type = MONIP_G(file_type);

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "S", &str_ip) == FAILURE) {
        RETURN_NULL();
    }

    if (ZSTR_LEN(str_ip) < 1) {
        php_error_docref(NULL, E_NOTICE, "ip str length lg 1");
        RETURN_NULL();
    }
    
    retval_type = zend_read_property(monip_ce, this_ptr, ZEND_STRL(IP_PRO_NAME_RETVAL_TYPE), 0, &rv);

	if (MONIP_G(cache_enable)){
		zend_resource *le;
		if ((le = zend_hash_str_find_ptr(&EG(persistent_list), MONIP_CACHE_DATA_PER_NAME, sizeof(MONIP_CACHE_DATA_PER_NAME)-1)) != NULL) {
			monip_ipdata *ipdata;
            if (le->type != le_monip_cache_ipdata_p){
				php_error_docref(NULL, E_ERROR, "Cache data type error!");
				RETURN_FALSE;
			}
            if (MONIP_G(debug)) {
                php_error_docref(NULL, E_NOTICE, "cache data from global cache");
            }
			ipdata = (monip_ipdata *)le->ptr;
            cache_ip = ipdata->cache;
            if (MONIP_G(cache_expire_time)){
                cache_ip_time = ipdata->cache_expire_time;
            }
		}else{
			php_error_docref(NULL, E_ERROR, "Cache data not found!");
			RETURN_FALSE;
		}
	}else{
		pro_data = zend_read_property(monip_ce, this_ptr, ZEND_STRL(IP_PRO_NAME_CACHE), 0, &rv);
        cache_ip = Z_ARR_P(pro_data);
	}

    str_ip = monip_gethostbyname(ZSTR_VAL(str_ip), MONIP_G(cache_enable));
//    printf("%s\n", ZSTR_VAL(str_ip));
    ip = zend_hash_find(cache_ip, str_ip);
    if (ip) {
        if (MONIP_G(debug)) {
            php_error_docref(NULL, E_NOTICE, "this ip found in cache");
        }
        if (MONIP_G(cache_expire_time) > 0){
            zval *expire;
            expire = zend_hash_find(cache_ip_time, str_ip);
            ZEND_ASSERT(expire != NULL);
            // printf("%lldd\n", Z_LVAL_P(expire));
            if (cur_time > Z_LVAL_P(expire)){
                zend_hash_del(cache_ip, str_ip);
                zend_hash_del(cache_ip_time, str_ip);
                if (MONIP_G(debug)) {
                    php_error_docref(NULL, E_NOTICE, "this ip is out expire time");
                }
                goto RE_SEAECH;
            }
        }
        zend_string_release(str_ip);
        if (Z_TYPE_P(retval_type) == IS_TRUE) {
            ZVAL_COPY(return_value, ip);
        }else{
            monip_split(return_value, Z_STRVAL_P(ip), Z_STRLEN_P(ip));
        }
        return;
    }

RE_SEAECH:    

    pro_offset = zend_read_property(monip_ce, this_ptr, ZEND_STRL(IP_PRO_NAME_OFFSET), 0, &rv);
    pro_index = zend_read_property(monip_ce, this_ptr, ZEND_STRL(IP_PRO_NAME_INDEX), 0, &rv);
    pro_stream = zend_read_property(monip_ce, this_ptr, ZEND_STRL(IP_PRO_NAME_STREAM), 0, &rv);
    php_stream_from_zval(stream, pro_stream);

    ip_idx = monip_ip2long(str_ip);

    if (!ip_file_type){
        tmp_offset = (ip_idx >> 24)*4;
    }else{
        tmp_offset = (ip_idx >> 16)*4;
    }

    memcpy((void *)&start, (void *)Z_STRVAL_P(pro_index) + tmp_offset, 4);

    if (!ip_file_type){
        max_comp_len = Z_LVAL_P(pro_offset) - 1028;
        start = start * 8 + 1024;
        inc_num = 8;
    }else{
        max_comp_len = Z_LVAL_P(pro_offset) - 262148;
        start = start * 9 + 262144;
        inc_num = 9;
    }

//    php_printf("%ld -- %ld\n", start, max_comp_len);

    for(; start < max_comp_len; start += inc_num){
        unsigned int cip = 0;
        memcpy(&cip, Z_STRVAL_P(pro_index) + start, 4);
        _unpack_n(cip);
        if(cip >= ip_idx){
            memcpy(&index_offset, Z_STRVAL_P(pro_index) + start + 4, 3);
            _unpack_v(index_offset);
            found_result = 1;
            if (!ip_file_type){
//                printf("start %d\n", start);
                memcpy(&index_length, Z_STRVAL_P(pro_index) + start + 7, 1);
            }else{
                memcpy(&index_length, Z_STRVAL_P(pro_index) + start + 8, 2);
            }
            break;
        }
    }

    if (found_result == 0) {
        RETURN_FALSE;
    }

//    php_printf("%d -- %d\n", index_offset, index_length);
    if (!ip_file_type){
        php_stream_seek(stream, Z_LVAL_P(pro_offset) + index_offset - 1024, SEEK_SET);
    }else{
        php_stream_seek(stream, Z_LVAL_P(pro_offset) + index_offset - 262144, SEEK_SET);
    }

    location = (char *) emalloc(sizeof(char) * index_length);
    php_stream_read(stream, location, index_length);

    if (MONIP_G(cache_enable)){
        ZVAL_PSTRINGL(&ip_val, location, index_length);
        // Z_ADDREF(ip_val);
    }else{
        ZVAL_STRINGL(&ip_val, location, index_length);
    }

    zend_hash_update(cache_ip, str_ip, &ip_val);

    if (MONIP_G(cache_expire_time) > 0){
        zval ip_exipre_val;
        ZVAL_LONG(&ip_exipre_val, cur_time + MONIP_G(cache_expire_time));
        zend_hash_update(cache_ip_time, str_ip, &ip_exipre_val);
    }

    if (Z_TYPE_P(retval_type) == IS_TRUE) {
        ZVAL_COPY(return_value, &ip_val);
    }else{
        monip_split(return_value, location, index_length);
    }

    efree(location);

    if (!MONIP_G(cache_enable)){
        zend_string_release(str_ip);
    }

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
