dnl $Id$
dnl config.m4 for extension monip

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(monip, for monip support,
Make sure that the comment is aligned:
[  --with-monip             Include monip support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(monip, whether to enable monip support,
Make sure that the comment is aligned:
[  --enable-monip           Enable monip support])

if test "$PHP_MONIP" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-monip -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/monip.h"  # you most likely want to change this
  dnl if test -r $PHP_MONIP/$SEARCH_FOR; then # path given as parameter
  dnl   MONIP_DIR=$PHP_MONIP
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for monip files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       MONIP_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$MONIP_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the monip distribution])
  dnl fi

  dnl # --with-monip -> add include path
  dnl PHP_ADD_INCLUDE($MONIP_DIR/include)

  dnl # --with-monip -> check for lib and symbol presence
  dnl LIBNAME=monip # you may want to change this
  dnl LIBSYMBOL=monip # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $MONIP_DIR/$PHP_LIBDIR, MONIP_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_MONIPLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong monip lib version or lib not found])
  dnl ],[
  dnl   -L$MONIP_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  PHP_SUBST(MONIP_SHARED_LIBADD)

  PHP_NEW_EXTENSION(monip, monip.c, $ext_shared)
fi
