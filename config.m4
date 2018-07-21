dnl $Id$
dnl config.m4 for extension ipip

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(ipip, for ipip support,
Make sure that the comment is aligned:
[  --with-ipip             Include ipip support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(ipip, whether to enable ipip support,
Make sure that the comment is aligned:
[  --enable-ipip           Enable ipip support])

if test "$PHP_IPIP" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-ipip -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/ipip.h"  # you most likely want to change this
  dnl if test -r $PHP_IPIP/$SEARCH_FOR; then # path given as parameter
  dnl   IPIP_DIR=$PHP_IPIP
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for ipip files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       IPIP_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$IPIP_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the ipip distribution])
  dnl fi

  dnl # --with-ipip -> add include path
  dnl PHP_ADD_INCLUDE($IPIP_DIR/include)

  dnl # --with-ipip -> check for lib and symbol presence
  dnl LIBNAME=ipip # you may want to change this
  dnl LIBSYMBOL=ipip # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $IPIP_DIR/$PHP_LIBDIR, IPIP_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_IPIPLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong ipip lib version or lib not found])
  dnl ],[
  dnl   -L$IPIP_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(IPIP_SHARED_LIBADD)

  PHP_NEW_EXTENSION(ipip, ipip.c, $ext_shared)
fi
