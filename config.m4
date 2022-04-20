dnl $Id$
dnl config.m4 for extension emicro

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(emicro, for emicro support,
dnl Make sure that the comment is aligned:
dnl [  --with-emicro             Include emicro support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(emicro, whether to enable emicro support,
dnl Make sure that the comment is aligned:
[  --enable-emicro           Enable emicro support])

if test "$PHP_EMICRO" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-emicro -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/emicro.h"  # you most likely want to change this
  dnl if test -r $PHP_EMICRO/$SEARCH_FOR; then # path given as parameter
  dnl   EMICRO_DIR=$PHP_EMICRO
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for emicro files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       EMICRO_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$EMICRO_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the emicro distribution])
  dnl fi

  dnl # --with-emicro -> add include path
  dnl PHP_ADD_INCLUDE($EMICRO_DIR/include)

  dnl # --with-emicro -> check for lib and symbol presence
  dnl LIBNAME=emicro # you may want to change this
  dnl LIBSYMBOL=emicro # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $EMICRO_DIR/$PHP_LIBDIR, EMICRO_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_EMICROLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong emicro lib version or lib not found])
  dnl ],[
  dnl   -L$EMICRO_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(EMICRO_SHARED_LIBADD)

  PHP_NEW_EXTENSION(emicro, emicro.c app/helper.c app/application.c app/request.c app/dispatcher.c app/factory.c app/annotation.c app/command.c app/loader.c app/config.c, $ext_shared)

  PHP_ADD_BUILD_DIR($ext_builddir/app)

fi
