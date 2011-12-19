#---------------------------------*- sh -*-------------------------------------
# License
#
#     This file is part of the branch "get-em" available here:
#         https://github.com/wyldckat/ThirdParty-2.0.x/commits/get-em
#
#     These modifications are not endorsed by OpenCFD Ltd.
#
#     It's licensed under the same license of the target software, namely GPL
#   v3 or above (see <http://www.gnu.org/licenses/>).
#
#     The target software is OpenFOAM(R) <http://www.openfoam.com>. It's
#   trademarked by and Copyright (C) 2004-2011 OpenCFD Ltd.
#
# File
#     defaults.sh
#
# Description
#     Default version definitions for the get-em script. Here are defined all
#   of the version numbers that are not accessible otherwise.
#
# TODO: These definitions should be in OpenFOAM's own etc folder and be
#     sourced by "settings.sh".
#
#------------------------------------------------------------------------------

case "$WM_COMPILER" in
Gcc | Gcc++0x | Gcc46 | Gcc46++0x)
    gcc_version=gcc-4.6.1
    gmp_version=gmp-5.0.2
    mpfr_version=mpfr-3.0.1
    mpc_version=mpc-0.9
    binutils_version="binutils-2.21"
    ;;
Gcc45 | Gcc45++0x)
    gcc_version=gcc-4.5.3
    gmp_version=gmp-5.0.1
    mpfr_version=mpfr-2.4.2
    mpc_version=mpc-0.8.2
    binutils_version="binutils-2.21"
    ;;
Gcc44 | Gcc44++0x)
    gcc_version=gcc-4.4.3
    gmp_version=gmp-5.0.1
    mpfr_version=mpfr-2.4.2
    binutils_version="binutils-2.21"
    ;;
Gcc43)
    gcc_version=gcc-4.3.3
    gmp_version=gmp-4.2.4
    mpfr_version=mpfr-2.4.1
    binutils_version="binutils-2.20.1"
    ;;
Clang)
    #clang_version=llvm-2.9
    clang_version=llvm-svn
    ;;
mingw32 | mingw-w32 | mingw-w64 | i686-w64-mingw32 | x86_64-w64-mingw32)
#     gcc_version="gcc-4.4.6"
#     gmp_version="gmp-4.3.2"
#     mpfr_version="mpfr-2.4.2"
#     binutils_version="binutils-2.21"

    gcc_version="gcc-4.5.3"
    gmp_version="gmp-4.3.2"
    mpfr_version="mpfr-2.4.2"
    mpc_version="mpc-0.8.2"
    binutils_version="binutils-2.21"

#    gcc_version=gcc-4.6.1
#    gmp_version=gmp-5.0.1
#    mpfr_version=mpfr-2.4.2
#    mpc_version=mpc-0.8.2
#    binutils_version="binutils-2.21"

    #MinGW versions
    mingw32rt_version="mingwrt-3.20"
    w32api_version="w32api-3.17-2"
    mingw_w64_version="1.x" #1.x or 2.x
    ;;
*)
    echo
    echo "Warning in $WM_THIRD_PARTY_DIR/etc/defaults.sh:"
    echo "    Unknown OpenFOAM compiler type '$WM_COMPILER'"
    echo "    Please check your settings"
    echo
    ;;
esac

boost_version=boost_1_45_0
cgal_version=CGAL-3.8

make_version=3.82
flex_version=2.5.35

qt_version=4.7.3

cmake_version=2.8.4

tecio_version=2009

libccmio_version=2.6.1

zlib_version=1.2.5
