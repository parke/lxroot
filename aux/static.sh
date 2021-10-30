#! /bin/bash


#  static.sh  version  20211028


#  this script will:
#    Download an x86 (32 bit) Alpine userland.
#    Build a statically compiled lxroot binary in that userland.


tgz_url='https://dl-cdn.alpinelinux.org/alpine/v3.14/releases/x86/alpine-minirootfs-3.14.2-x86.tar.gz'
tgz_sum='874f6e60047535c08ffcc8f430afec21d323b367269391279f74ff8f87420d83'


set  -o errexit


usage  ()  {    #  ----------------------------------------------------  usage
  echo
  echo  'usage:  sh  static.sh  /path/to/lxroot  /path/to/build_dir'  ;  }


die    ()  {  echo  "die  $*"  ;  exit  1  ;  }    #  -------------------  die

found  ()  {  printf  'found  %-8s  %s\n'  "$1"  "$2"  ;  }    #  -----  found

trace  ()  {  echo  "+  $@"  ;  "$@"  ;  }    #  ----------------------  trace


lxr  ()  {    #  --------------------------------------------------------  lxr
  "$lxroot"  -rn  "$build"/newroot  --  "$@"  ;  }


init  ()  {    #  ------------------------------------------------------  init
  [ "$build" ]      ||  {  usage  ;  exit  1 ;  }
  if  [ -d  "$build" ]
    then  found  init  "$build"
    else  trace  mkdir  -p  "$build"  ;  fi
  cd  "$build"  ;  }


fetch  ()  (    #  ----------------------------------------------------  fetch
  [ -d 'dist'      ]  ||  trace  mkdir  dist
  [ -f dist/"$tgz" ]  &&  {  found  fetch  dist/"$tgz"  ;  return  ;  }
  cd  dist
  trace  wget  -nc  "$tgz_url"  )


checksum  ()  {    #  ----------------------------------------------  checksum
  local  actual="$(  sha256sum  dist/"$tgz"  )"
  actual="${actual%%  dist/*}"
  [ "$actual" = "$tgz_sum" ]  &&  {  found  checksum  valid  ;  return  ;  }
  echo
  echo  'checksum  error'
  echo  "  expect  $tgz_sum"
  echo  "  actual  $actual"
  exit  1  ;  }


extract  ()  {    #  ------------------------------------------------  extract
  [ -d newroot ]  &&  {  found  extract  newroot  ;  return  ;  }
  trace  mkdir  newroot
  trace  tar  xzf  dist/"$tgz"  -C  newroot  ;  }


dns  ()  {    #  --------------------------------------------------------  dns
  local  path='newroot/etc/resolv.conf'
  [ -f "$path" ]  &&  {  found  dns  "$path"  ;  return  ;  }
  trace  cp  /etc/resolv.conf  newroot/etc/resolv.conf  ;  }


apk_update  ()  {    #  ------------------------------------------  apk_update
  local  path="newroot/root/touch_apk_update"
  [ -f "$path" ]  &&  {  found  update  "$path"  ;  return  ;  }
  trace  lxr  apk  update
  trace  touch  "$path"  ;  }


apk_add  ()  {    #  ------------------------------------------------  apk_add
  local  path="newroot/usr/bin/g++"
  [ -f "$path" ]  &&  {  found  add  "$path"  ;  return  ;  }
  trace  lxr  apk  add  build-base  ;  }


build  ()  {    #  ----------------------------------------------------  build

  #  see  http://ptspts.blogspot.com/2013/12/how-to-make-smaller-c-and-c-binaries.html

  local  options=(

    -s
    -Os
    -fomit-frame-pointer
    -fno-stack-protector
    -ffunction-sections
    -fdata-sections
    -Wl,--gc-sections
    -fno-unroll-loops
    -fmerge-all-constants
    -fno-ident
    -Wl,-z,norelro
    -Wl,--build-id=none

    -fno-exceptions
    -fno-rtti

    #  20211028  -fvtable-gc is no longer supported
    #  -fvtable-gc

    )

  echo
  trace  rm  -f  'newroot/root/lxroot-plain'
  trace  rm  -f  'newroot/root/lxroot-small'
  trace  rm  -f  'newroot/root/lxroot-static'
  trace  cp  "$src"/lxroot.cpp  newroot/root/
  trace  cp  "$src"/help.cpp    newroot/root/
  trace  cp  "$src"/str.cpp     newroot/root/

  echo
  trace  lxr  time  g++  lxroot.cpp  -o  lxroot-plain

  echo
  trace  lxr  time  g++  lxroot.cpp  -o  lxroot-small  \
    "${options[@]}"

  echo
  trace  lxr  time  g++  lxroot.cpp  -o  lxroot-static  \
    "${options[@]}"  -static



  return  ;  }


main  ()  {    #  ------------------------------------------------------  main

  local  src="$PWD"  lxroot="$1"  build="$2"
  local  tgz="${tgz_url##*/}"

  lxroot=`  realpath  "$lxroot"  `

  echo
  echo  'static.sh'
  echo  "  src     $src"
  echo  "  lxroot  $lxroot"
  echo  "  bulid   $build"

  echo
  init
  fetch
  checksum
  extract
  dns
  apk_update
  apk_add

  build

  trace  cd  newroot/root

  echo
  echo
  echo  '====  size of elf sections'
  size  lxroot-small  lxroot-plain  lxroot-static

  echo
  echo
  echo  '====  file size in bytes'
  ls  -lSr  lxroot-*

  echo
  echo
  echo  '====  file size in kilobytes'
  ls  -lSrh  lxroot-*

  return  ;  }


main  "$@"
