dnl $Id$
dnl config.m4 for extension monip



PHP_ARG_ENABLE(monip, whether to enable monip support,
[  --enable-monip           Enable monip support])

if test "$PHP_MONIP" != "no"; then

  PHP_SUBST(MONIP_SHARED_LIBADD)
  PHP_NEW_EXTENSION(monip, monip.c, $ext_shared)
fi
