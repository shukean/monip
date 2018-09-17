/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
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
#include <fcntl.h>
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_ipip.h"

ZEND_DECLARE_MODULE_GLOBALS(ipip)
//static PHP_GINIT_FUNCTION(ipip);

/* True global resources - no need for thread safety here */
static int le_ipip;
zend_class_entry *ipip_ce;
static unsigned char *datx_buf = NULL;
static size_t datx_buf_len = 0;
static int32_t datx_offset = 0;
static unsigned char *datx_index = NULL;
static size_t datx_index_len = 0;

#define L2B(data) ((data)[0] | ((data)[1]<<8) | ((data)[2]<<16) | ((data)[3]<<24))
#define B2L(data) ((data)[3] | ((data)[2]<<8) | ((data)[1]<<16) | ((data)[0]<<24))

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("ipip.datx_file", NULL, PHP_INI_SYSTEM, OnUpdateString, datx_file, zend_ipip_globals, ipip_globals)
PHP_INI_END()
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

static void ipip_split(zval *return_value, const char *str, uint str_len TSRMLS_DC) {
    char *p1, *p2, *endp;

    array_init(return_value);
    endp = str + str_len;       //end of ip_addr
    p1 = str;
    p2 = php_memnstr(p1, ZEND_STRL("\t"), endp);

    if (p2 == NULL) {
        add_next_index_stringl(return_value, p1, str_len, 1);
    } else {
        do{
            if (p2 - p1 > 1) {
                add_next_index_stringl(return_value, p1, (uint)(p2 - p1), 1);
            }
            p1 = p2 + 1;
        } while ((p2 = php_memnstr(p1, ZEND_STRL("\t"), endp)) != NULL);

        if (p1 < endp) {
            add_next_index_stringl(return_value, p1, (uint)(endp-p1), 1);
        }
    }
}

static bool ip_find(const char *ip_str, zval *rsp TSRMLS_DC) {
    if (datx_offset < 4) {
        php_error(E_ERROR, "Invalid 17ipipdb.datx file!");
        return false;
    }
    uint32_t ips[4];
    int n = sscanf(ip_str, "%d.%d.%d.%d", &ips[0], &ips[1], &ips[2], &ips[3]);
    if (n != 4) {
        php_error(E_WARNING, "Invalid ip value!");
        return false;
    }
    if (ips[0] > 256 || ips[1] > 256 | ips[2] > 256 | ips[3] > 256) {
        php_error(E_NOTICE, "INvalid ip!");
        return false;
    }

    uint32_t nip2 = B2L(ips);   // ip2long
    uint32_t pre_offset = (nip2 >> 16) * 4;
    uint32_t start = L2B(datx_index + pre_offset);
    uint32_t max_comp_len = datx_offset - 262144 - 4;
    uint32_t index_offset = 0, index_len = 0;
    for (start = start * 9 + 262144; start < max_comp_len; start = start + 9) {
        if (B2L(datx_index + start) >= nip2) {
            index_offset = L2B(datx_index + start + 4) & 0x00FFFFFF;
            index_len = (datx_index[start + 7] << 8) + datx_index[start + 8];
            break;
        }
    }

    if (index_offset == 0) {
        php_error(E_NOTICE, "Not found ip!");
        return false;
    }

    ipip_split(rsp, datx_buf + datx_offset + index_offset - 262144, index_len TSRMLS_CC);
    return true;
}

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto array ipip_find(string ipv4)
   Return a array to dispaly ip address message */
PHP_FUNCTION(ipip_find){
    char *ipstr;
    ulong ipstr_len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &ipstr, &ipstr_len) == FAILURE){
        RETURN_FALSE;
    }

    if (ipstr_len < 1) {
        RETURN_NULL();
    }

    bool ret = ip_find(ipstr, return_value TSRMLS_CC);
    if (!ret) {
        RETURN_FALSE;
    }
    return;
}
/* }}} */

/* {{{ calss ipip */
PHP_METHOD(ipip_ce, __construct){
    // nothing
}


PHP_METHOD(ipip_ce, __destruct){
    // nothing
}

PHP_METHOD(ipip_ce, find){
    char *ipstr;
    ulong ipstr_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &ipstr, &ipstr_len) == FAILURE) {
        RETURN_NULL();
    }

    if (ipstr_len < 1) {
        RETURN_NULL();
    }
    bool ret = ip_find(ipstr, return_value TSRMLS_CC);
    if (!ret) {
        RETURN_FALSE;
    }
    return;
}

const zend_function_entry ipip_methods[] = {
    PHP_ME(ipip_ce, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME(ipip_ce, __destruct, NULL, ZEND_ACC_DTOR | ZEND_ACC_PUBLIC)
    PHP_ME(ipip_ce, find, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_ALLOW_STATIC | ZEND_ACC_STATIC)
    PHP_FE_END
};
/* }}} */

/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_ipip_init_globals
 */
static void php_ipip_init_globals(zend_ipip_globals *ipip_globals)
{
    ipip_globals->datx_file = NULL;
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(ipip)
{
    zend_class_entry ce;

    REGISTER_INI_ENTRIES();

    INIT_CLASS_ENTRY(ce, "ipip", ipip_methods);
    ipip_ce = zend_register_internal_class(&ce TSRMLS_CC);

    if (!IPIP_G(datx_file)) {
        goto out;
    }
    if (UNEXPECTED(DEUBG)) {
        printf("default ipip datx file path %s\n", IPIP_G(datx_file));
    }
    int fd = open(IPIP_G(datx_file), O_RDONLY);
    if (!fd) {
        php_error(E_ERROR, "failed open default ipip datx file");
        goto out;
    }
    datx_buf_len = 0;
    datx_buf = pemalloc(4096, 1);
    while (true) {
        size_t len_read = read(fd, (void *)(datx_buf + datx_buf_len), 4096);
        if (len_read > 0) {
            datx_buf_len += len_read;
            datx_buf = perealloc(datx_buf, datx_buf_len + 4096, 1);
        } else if (len_read == 0) {
            break;
        } else {
            php_error(E_WARNING, "failed load ipip datx file");
            datx_buf_len = 0;
            goto out;
        }
    }
    if (UNEXPECTED(DEUBG)) {
        printf("load datx file, size %ld\n", datx_buf_len);
    }

    datx_offset = B2L(datx_buf);
    if (UNEXPECTED(DEUBG)) {
        printf("datx offset is %d\n", datx_offset);
    }
    if (datx_offset >= 4) {
        datx_index_len = datx_offset - 4;
        datx_index = datx_buf + 4;
    }
    close(fd);
out:
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(ipip)
{
    UNREGISTER_INI_ENTRIES();
    if (datx_buf) {
        pefree(datx_buf, 1);
    }
    datx_buf = NULL;
    datx_index = NULL;
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ipip)
{
    return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(ipip)
{
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(ipip)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "ipip support", "enabled");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ ipip_functions[]
 *
 * Every user visible function must have an entry in ipip_functions[].
 */

ZEND_BEGIN_ARG_INFO(arginfo_ipip_find, ZEND_SEND_BY_VAL)
    ZEND_ARG_INFO(0, ip_str)
ZEND_END_ARG_INFO()

const zend_function_entry ipip_functions[] = {
    PHP_FE(ipip_find,   arginfo_ipip_find)      /* For testing, remove later. */
    PHP_FE_END  /* Must be the last line in ipip_functions[] */
};
/* }}} */

/* {{{ ipip_module_entry
 */
zend_module_entry ipip_module_entry = {
    STANDARD_MODULE_HEADER,
    "ipip",
    ipip_functions,
    PHP_MINIT(ipip),
    PHP_MSHUTDOWN(ipip),
    PHP_RINIT(ipip),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(ipip),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(ipip),
    PHP_IPIP_VERSION,
    STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_IPIP
ZEND_GET_MODULE(ipip)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
