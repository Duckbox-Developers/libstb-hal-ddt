AC_DEFUN([TUXBOX_APPS], [
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

AC_GNU_SOURCE

AC_ARG_WITH(target,
	AS_HELP_STRING([--with-target=TARGET], [target for compilation [[native,cdk]]]),
	[TARGET="$withval"],
	[TARGET="native"])

AC_ARG_WITH(targetprefix,
	AS_HELP_STRING([--with-targetprefix=PATH], [prefix relative to target root (only applicable in cdk mode)]),
	[TARGET_PREFIX="$withval"],
	[TARGET_PREFIX=""])

AC_ARG_WITH(debug,
	AS_HELP_STRING([--without-debug], [disable debugging code @<:@default=no@:>@]),
	[DEBUG="$withval"],
	[DEBUG="yes"])

if test "$DEBUG" = "yes"; then
	DEBUG_CFLAGS="-g3 -ggdb"
	AC_DEFINE(DEBUG, 1, [enable debugging code])
fi

AC_MSG_CHECKING(target)

if test "$TARGET" = "native"; then
	AC_MSG_RESULT(native)

	if test "$CFLAGS" = "" -a "$CXXFLAGS" = ""; then
		CFLAGS="-Wall -O2 -pipe $DEBUG_CFLAGS"
		CXXFLAGS="-Wall -O2 -pipe $DEBUG_CFLAGS"
	fi
	if test "$prefix" = "NONE"; then
		prefix=/usr/local
	fi
	TARGET_PREFIX=$prefix
	if test "$exec_prefix" = "NONE"; then
		exec_prefix=$prefix
	fi
	targetprefix=$prefix
elif test "$TARGET" = "cdk"; then
	AC_MSG_RESULT(cdk)

	if test "$CC" = "" -a "$CXX" = ""; then
		AC_MSG_ERROR([you need to specify variables CC or CXX in cdk])
	fi
	if test "$CFLAGS" = "" -o "$CXXFLAGS" = ""; then
		AC_MSG_ERROR([you need to specify variables CFLAGS and CXXFLAGS in cdk])
	fi
	if test "$prefix" = "NONE"; then
		AC_MSG_ERROR([invalid prefix, you need to specify one in cdk mode])
	fi
	if test "$TARGET_PREFIX" != "NONE"; then
		AC_DEFINE_UNQUOTED(TARGET_PREFIX, "$TARGET_PREFIX", [The targets prefix])
	fi
	if test "$TARGET_PREFIX" = "NONE"; then
		AC_MSG_ERROR([invalid targetprefix, you need to specify one in cdk mode])
		TARGET_PREFIX=""
	fi
else
	AC_MSG_RESULT(none)
	AC_MSG_ERROR([invalid target $TARGET, choose on from native,cdk]);
fi

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_SYS_LARGEFILE

])

dnl expand nested ${foo}/bar
AC_DEFUN([TUXBOX_EXPAND_VARIABLE], [
	__$1="$2"
	for __CNT in false false false false true; do dnl max 5 levels of indirection
		$1=`eval echo "$__$1"`
		echo ${$1} | grep -q '\$' || break # 'grep -q' is POSIX, exit if no $ in variable
		__$1="${$1}"
	done
	$__CNT && AC_MSG_ERROR([can't expand variable $1=$2]) dnl bail out if we did not expand
])

AC_DEFUN([TUXBOX_APPS_DIRECTORY_ONE], [
AC_ARG_WITH($1, AS_HELP_STRING([$6], [$7 [[PREFIX$4$5]]], [32], [79]), [
	_$2=$withval
	if test "$TARGET" = "cdk"; then
		$2=`eval echo "$TARGET_PREFIX$withval"` # no indirection possible IMNSHO
	else
		$2=$withval
	fi
	TARGET_$2=${$2}
], [
	# RFC 1925: "you can always add another level of indirection..."
	TUXBOX_EXPAND_VARIABLE($2,"${$3}$5")
	if test "$TARGET" = "cdk"; then
		TUXBOX_EXPAND_VARIABLE(_$2,"${target$3}$5")
	else
		_$2=${$2}
	fi
	TARGET_$2=$_$2
])
dnl AC_SUBST($2)
AC_DEFINE_UNQUOTED($2,"$_$2",$7)
AC_SUBST(TARGET_$2)
])

AC_DEFUN([TUXBOX_APPS_DIRECTORY], [
AC_REQUIRE([TUXBOX_APPS])

if test "$TARGET" = "cdk"; then
	datadir="\${prefix}/share"
	sysconfdir="\${prefix}/etc"
	localstatedir="\${prefix}/var"
	libdir="\${prefix}/lib"
	targetdatadir="\${TARGET_PREFIX}/share"
	targetsysconfdir="\${TARGET_PREFIX}/etc"
	targetlocalstatedir="\${TARGET_PREFIX}/var"
	targetlibdir="\${TARGET_PREFIX}/lib"
fi

TUXBOX_APPS_DIRECTORY_ONE(configdir, CONFIGDIR, localstatedir, /var, /tuxbox/config,
	[--with-configdir=PATH], [where to find config files])

TUXBOX_APPS_DIRECTORY_ONE(datadir, DATADIR, datadir, /share, /tuxbox,
	[--with-datadir=PATH], [where to find data files])

TUXBOX_APPS_DIRECTORY_ONE(fontdir, FONTDIR, datadir, /share, /fonts,
	[--with-fontdir=PATH], [where to find fonts])

TUXBOX_APPS_DIRECTORY_ONE(gamesdir, GAMESDIR, localstatedir, /var, /tuxbox/games,
	[--with-gamesdir=PATH], [where to find games])

TUXBOX_APPS_DIRECTORY_ONE(libdir, LIBDIR, libdir, /lib, /tuxbox,
	[--with-libdir=PATH], [where to find internal libs])

TUXBOX_APPS_DIRECTORY_ONE(plugindir, PLUGINDIR, libdir, /lib, /tuxbox/plugins,
	[--with-plugindir=PATH], [where to find plugins])

TUXBOX_APPS_DIRECTORY_ONE(themesdir, THEMESDIR, datadir, /share, /tuxbox/neutrino/themes,
	[--with-themesdir=PATH], [where to find themes])
])

dnl automake <= 1.6 needs this specifications
AC_SUBST(CONFIGDIR)
AC_SUBST(DATADIR)
AC_SUBST(FONTDIR)
AC_SUBST(GAMESDIR)
AC_SUBST(LIBDIR)
AC_SUBST(PLUGINDIR)
AC_SUBST(THEMESDIR)
dnl end workaround

AC_DEFUN([TUXBOX_BOXTYPE], [
AC_ARG_WITH(boxtype,
	AS_HELP_STRING([--with-boxtype], [valid values: tripledragon, spark, azbox, generic, armbox, duckbox, spark7162, mipsbox]),
	[case "${withval}" in
		tripledragon|azbox|generic|armbox)
			BOXTYPE="$withval"
		;;
		spark|spark7162)
			BOXTYPE="spark"
			BOXMODEL="$withval"
		;;
		dm*)
			BOXTYPE="dreambox"
			BOXMODEL="$withval"
		;;
		ufs*)
			BOXTYPE="duckbox"
			BOXMODEL="$withval"
		;;
		atevio*)
			BOXTYPE="duckbox"
			BOXMODEL="$withval"
		;;
		fortis*)
			BOXTYPE="duckbox"
			BOXMODEL="$withval"
		;;
		octagon*)
			BOXTYPE="duckbox"
			BOXMODEL="$withval"
		;;
		hs7*)
			BOXTYPE="duckbox"
			BOXMODEL="$withval"
		;;
		dp*)
			BOXTYPE="duckbox"
			BOXMODEL="$withval"
		;;
		cuberevo*)
			BOXTYPE="duckbox"
			BOXMODEL="$withval"
		;;
		ipbox*)
			BOXTYPE="duckbox"
			BOXMODEL="$withval"
		;;
		arivalink200)
			BOXTYPE="duckbox"
			BOXMODEL="$withval"
		;;
		tf*)
			BOXTYPE="duckbox"
			BOXMODEL="$withval"
		;;
		hl101)
			BOXTYPE="duckbox"
			BOXMODEL="$withval"
		;;
		hd51|hd60|vusolo4k)
			BOXTYPE="armbox"
			BOXMODEL="$withval"
		;;
		vuduo)
			BOXTYPE="mipsbox"
			BOXMODEL="$withval"
		;;
		*)
			AC_MSG_ERROR([bad value $withval for --with-boxtype])
		;;
	esac],
	[BOXTYPE="generic"])

AC_ARG_WITH(boxmodel,
	AS_HELP_STRING([--with-boxmodel], [valid for generic: raspi])
AS_HELP_STRING([], [valid for duckbox: ufs910, ufs912, ufs913, ufs922, atevio7500, fortis_hdbox, octagon1008, hs7110, hs7810a, hs7119, hs7819, dp7000, cuberevo, cuberevo_mini, cuberevo_mini2, cuberevo_250hd, cuberevo_2000hd, cuberevo_3000hd, ipbox9900, ipbox99, ipbox55, arivalink200, tf7700, hl101])
AS_HELP_STRING([], [valid for spark: spark, spark7162])
AS_HELP_STRING([], [valid for armbox: hd51, hd60, vusolo4k])
AS_HELP_STRING([], [valid for mipsbox: vuduo]),
	[case "${withval}" in
		ufs910|ufs912|ufs913|ufs922|atevio7500|fortis_hdbox|octagon1008|hs7110|hs7810a|hs7119|hs7819|dp7000|cuberevo|cuberevo_mini|cuberevo_mini2|cuberevo_250hd|cuberevo_2000hd|cuberevo_3000hd|ipbox9900|ipbox99|ipbox55|arivalink200|tf7700|hl101)
			if test "$BOXTYPE" = "duckbox"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
		;;
		spark|spark7162)
			if test "$BOXTYPE" = "spark"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
		;;
		hd51|hd60|vusolo4k)
			if test "$BOXTYPE" = "armbox"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
		;;
		vuduo)
			if test "$BOXTYPE" = "mipsbox"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
		;;
		raspi)
			if test "$BOXTYPE" = "generic"; then
				BOXMODEL="$withval"
			else
				AC_MSG_ERROR([unknown model $withval for boxtype $BOXTYPE])
			fi
		;;
		*)
			AC_MSG_ERROR([unsupported value $withval for --with-boxmodel])
		;;
	esac])

AC_SUBST(BOXTYPE)
AC_SUBST(BOXMODEL)

AM_CONDITIONAL(BOXTYPE_AZBOX, test "$BOXTYPE" = "azbox")
AM_CONDITIONAL(BOXTYPE_TRIPLE, test "$BOXTYPE" = "tripledragon")
AM_CONDITIONAL(BOXTYPE_SPARK, test "$BOXTYPE" = "spark")
AM_CONDITIONAL(BOXTYPE_GENERIC, test "$BOXTYPE" = "generic")
AM_CONDITIONAL(BOXTYPE_DUCKBOX, test "$BOXTYPE" = "duckbox")
AM_CONDITIONAL(BOXTYPE_ARMBOX, test "$BOXTYPE" = "armbox")
AM_CONDITIONAL(BOXTYPE_MIPSBOX, test "$BOXTYPE" = "mipsbox")

AM_CONDITIONAL(BOXMODEL_UFS910, test "$BOXMODEL" = "ufs910")
AM_CONDITIONAL(BOXMODEL_UFS912, test "$BOXMODEL" = "ufs912")
AM_CONDITIONAL(BOXMODEL_UFS913, test "$BOXMODEL" = "ufs913")
AM_CONDITIONAL(BOXMODEL_UFS922, test "$BOXMODEL" = "ufs922")
AM_CONDITIONAL(BOXMODEL_SPARK, test "$BOXMODEL" = "spark")
AM_CONDITIONAL(BOXMODEL_SPARK7162, test "$BOXMODEL" = "spark7162")
AM_CONDITIONAL(BOXMODEL_ATEVIO7500, test "$BOXMODEL" = "atevio7500")
AM_CONDITIONAL(BOXMODEL_FORTIS_HDBOX, test "$BOXMODEL" = "fortis_hdbox")
AM_CONDITIONAL(BOXMODEL_OCTAGON1008, test "$BOXMODEL" = "octagon1008")
AM_CONDITIONAL(BOXMODEL_HS7110, test "$BOXMODEL" = "hs7110")
AM_CONDITIONAL(BOXMODEL_HS7810A, test "$BOXMODEL" = "hs7810a")
AM_CONDITIONAL(BOXMODEL_HS7119, test "$BOXMODEL" = "hs7119")
AM_CONDITIONAL(BOXMODEL_HS7819, test "$BOXMODEL" = "hs7819")
AM_CONDITIONAL(BOXMODEL_DP7000, test "$BOXMODEL" = "dp7000")

AM_CONDITIONAL(BOXMODEL_CUBEREVO, test "$BOXMODEL" = "cuberevo")
AM_CONDITIONAL(BOXMODEL_CUBEREVO_MINI, test "$BOXMODEL" = "cuberevo_mini")
AM_CONDITIONAL(BOXMODEL_CUBEREVO_MINI2, test "$BOXMODEL" = "cuberevo_mini2")
AM_CONDITIONAL(BOXMODEL_CUBEREVO_250HD, test "$BOXMODEL" = "cuberevo_250hd")
AM_CONDITIONAL(BOXMODEL_CUBEREVO_2000HD, test "$BOXMODEL" = "cuberevo_2000hd")
AM_CONDITIONAL(BOXMODEL_CUBEREVO_3000HD, test "$BOXMODEL" = "cuberevo_3000hd")
AM_CONDITIONAL(BOXMODEL_IPBOX9900, test "$BOXMODEL" = "ipbox9900")
AM_CONDITIONAL(BOXMODEL_IPBOX99, test "$BOXMODEL" = "ipbox99")
AM_CONDITIONAL(BOXMODEL_IPBOX55, test "$BOXMODEL" = "ipbox55")
AM_CONDITIONAL(BOXMODEL_ARIVALINK200, test "$BOXMODEL" = "arivalink200")
AM_CONDITIONAL(BOXMODEL_TF7700, test "$BOXMODEL" = "tf7700")
AM_CONDITIONAL(BOXMODEL_HL101, test "$BOXMODEL" = "hl101")

AM_CONDITIONAL(BOXMODEL_HD51, test "$BOXMODEL" = "hd51")
AM_CONDITIONAL(BOXMODEL_HD60, test "$BOXMODEL" = "hd60")
AM_CONDITIONAL(BOXMODEL_VUSOLO4K, test "$BOXMODEL" = "vusolo4k")
AM_CONDITIONAL(BOXMODEL_VUDUO, test "$BOXMODEL" = "vuduo")

AM_CONDITIONAL(BOXMODEL_RASPI, test "$BOXMODEL" = "raspi")

if test "$BOXTYPE" = "azbox"; then
	AC_DEFINE(HAVE_AZBOX_HARDWARE, 1, [building for an azbox])
elif test "$BOXTYPE" = "tripledragon"; then
	AC_DEFINE(HAVE_TRIPLEDRAGON, 1, [building for a tripledragon])
elif test "$BOXTYPE" = "spark"; then
	AC_DEFINE(HAVE_SPARK_HARDWARE, 1, [building for a goldenmedia 990 or edision pingulux])
	AC_DEFINE(HAVE_SH4_HARDWARE, 1, [building for a sh4 box])
elif test "$BOXTYPE" = "duckbox"; then
	AC_DEFINE(HAVE_DUCKBOX_HARDWARE, 1, [building for a duckbox])
	AC_DEFINE(HAVE_SH4_HARDWARE, 1, [building for a sh4 box])
elif test "$BOXTYPE" = "generic"; then
	AC_DEFINE(HAVE_GENERIC_HARDWARE, 1, [building for a generic device like a standard PC])
elif test "$BOXTYPE" = "armbox"; then
	AC_DEFINE(HAVE_ARM_HARDWARE, 1, [building for an armbox])
elif test "$BOXTYPE" = "mipsbox"; then
	AC_DEFINE(HAVE_MIPS_HARDWARE, 1, [building for an mipsbox])
fi

# TODO: do we need more defines?
if test "$BOXMODEL" = "ufs910"; then
	AC_DEFINE(BOXMODEL_UFS910, 1, [ufs910])
elif test "$BOXMODEL" = "ufs912"; then
	AC_DEFINE(BOXMODEL_UFS912, 1, [ufs912])
elif test "$BOXMODEL" = "ufs913"; then
	AC_DEFINE(BOXMODEL_UFS913, 1, [ufs913])
elif test "$BOXMODEL" = "ufs922"; then
	AC_DEFINE(BOXMODEL_UFS922, 1, [ufs922])
elif test "$BOXMODEL" = "spark"; then
	AC_DEFINE(BOXMODEL_SPARK, 1, [spark])
elif test "$BOXMODEL" = "spark7162"; then
	AC_DEFINE(BOXMODEL_SPARK7162, 1, [spark7162])
elif test "$BOXMODEL" = "atevio7500"; then
	AC_DEFINE(BOXMODEL_ATEVIO7500, 1, [atevio7500])
elif test "$BOXMODEL" = "fortis_hdbox"; then
	AC_DEFINE(BOXMODEL_FORTIS_HDBOX, 1, [fortis_hdbox])
elif test "$BOXMODEL" = "octagon1008"; then
	AC_DEFINE(BOXMODEL_OCTAGON1008, 1, [octagon1008])
elif test "$BOXMODEL" = "hs7110"; then
	AC_DEFINE(BOXMODEL_HS7110, 1, [hs7110])
elif test "$BOXMODEL" = "hs7810a"; then
	AC_DEFINE(BOXMODEL_HS7810A, 1, [hs7810a])
elif test "$BOXMODEL" = "hs7119"; then
	AC_DEFINE(BOXMODEL_HS7119, 1, [hs7119])
elif test "$BOXMODEL" = "hs7819"; then
	AC_DEFINE(BOXMODEL_HS7819, 1, [hs7819])
elif test "$BOXMODEL" = "dp7000"; then
	AC_DEFINE(BOXMODEL_DP7000, 1, [dp7000])
elif test "$BOXMODEL" = "cuberevo"; then
	AC_DEFINE(BOXMODEL_CUBEREVO, 1, [cuberevo])
elif test "$BOXMODEL" = "cuberevo_mini"; then
	AC_DEFINE(BOXMODEL_CUBEREVO_MINI, 1, [cuberevo_mini])
elif test "$BOXMODEL" = "cuberevo_mini2"; then
	AC_DEFINE(BOXMODEL_CUBEREVO_MINI2, 1, [cuberevo_mini2])
elif test "$BOXMODEL" = "cuberevo_250hd"; then
	AC_DEFINE(BOXMODEL_CUBEREVO_250HD, 1, [cuberevo_250hd])
elif test "$BOXMODEL" = "cuberevo_2000hd"; then
	AC_DEFINE(BOXMODEL_CUBEREVO_2000HD, 1, [cuberevo_2000hd])
elif test "$BOXMODEL" = "cuberevo_3000hd"; then
	AC_DEFINE(BOXMODEL_CUBEREVO_3000HD, 1, [cuberevo_3000hd])
elif test "$BOXMODEL" = "ipbox9900"; then
	AC_DEFINE(BOXMODEL_IPBOX9900, 1, [ipbox9900])
elif test "$BOXMODEL" = "ipbox99"; then
	AC_DEFINE(BOXMODEL_IPBOX99, 1, [ipbox99])
elif test "$BOXMODEL" = "ipbox55"; then
	AC_DEFINE(BOXMODEL_IPBOX55, 1, [ipbox55])
elif test "$BOXMODEL" = "arivalink200"; then
	AC_DEFINE(BOXMODEL_ARIVALINK200, 1, [arivalink200])
elif test "$BOXMODEL" = "tf7700"; then
	AC_DEFINE(BOXMODEL_TF7700, 1, [tf7700])
elif test "$BOXMODEL" = "hl101"; then
	AC_DEFINE(BOXMODEL_HL101, 1, [hl101])
elif test "$BOXMODEL" = "hd51"; then
	AC_DEFINE(BOXMODEL_HD51, 1, [hd51])
elif test "$BOXMODEL" = "hd60"; then
	AC_DEFINE(BOXMODEL_HD60, 1, [hd60])
elif test "$BOXMODEL" = "vusolo4k"; then
	AC_DEFINE(BOXMODEL_VUSOLO4K, 1, [vusolo4k])
elif test "$BOXMODEL" = "vuduo"; then
	AC_DEFINE(BOXMODEL_VUDUO, 1, [vuduo])
elif test "$BOXMODEL" = "raspi"; then
	AC_DEFINE(BOXMODEL_RASPI, 1, [raspberry pi])
fi
])

dnl backward compatiblity
AC_DEFUN([AC_GNU_SOURCE], [
AH_VERBATIM([_GNU_SOURCE], [
/* Enable GNU extensions on systems that have them. */
#ifndef _GNU_SOURCE
# undef _GNU_SOURCE
#endif
])dnl
AC_BEFORE([$0], [AC_COMPILE_IFELSE])dnl
AC_BEFORE([$0], [AC_RUN_IFELSE])dnl
AC_DEFINE([_GNU_SOURCE])
])

AC_DEFUN([AC_PROG_EGREP], [
AC_CACHE_CHECK([for egrep], [ac_cv_prog_egrep], [
if echo a | (grep -E '(a|b)') >/dev/null 2>&1; then
	ac_cv_prog_egrep='grep -E'
else
	ac_cv_prog_egrep='egrep'
fi
])
EGREP=$ac_cv_prog_egrep
AC_SUBST([EGREP])
])
