#! /bin/bash


#  mk-deb.sh  -  Create a Debian chroot environment (for use with Lxroot).
#  Copyright (c) 2023 Parke Bostrom, parke.nexus at gmail.com
#  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.


#  lxroot / aux / mk-deb.sh  version  20231007


set    -o errexit     #  exit on any error.
set    -o pipefail    #  exit on any error in any pipeline.
set    -o errtrace    #  trap errors also inside function calls.
set    -o nounset     #  exit with error upon use of any unset variable.
shopt  -s lastpipe    #  allow 'function | readarray'.


usage  ()  {    #  ----------------------------------------------------  usage

  echo
  echo  'usage:  ./mk-deb.sh  path/to/world  [command]'
  echo
  echo  'commands'
  echo  '  init'
  echo  '  download'
  echo  '  extract'
  echo  '  install'
  echo  '  clean'

  return  ;  }


main  ()  {    #  ------------------------------------------------------  main

  #  usage:  mk-deb  world  [command]  [distro]

  #  20231007  note  [distro] not yet implemented

  readonly  world="${1-}"  ;  local  command="${2-}"

  not_str  "$world"  &&  {  usage  ;  exit  0  ;  }

  init_vars

  case  "$command"  in

    ( ''       )  install   ;;
    ( init     )  init      ;;
    ( download )  download  ;;
    ( extract  )  extract   ;;
    ( install  )  install   ;;
    ( clean    )  clean     ;;

    ( *        )  halt  "halt  mk-deb  bad command  '$command'"

    esac  ;  exit  0  ;  }


#  phase  -------------------------------------------------------------  phase


#  20231007
#
#  The current version of mk-deb.sh bootstraps *all* packages
#  simultaneuosly.
#
#  Therefore, these phases are effectively obsolete.
#
#  I am keeping these phases (and these comments) just in case I ever
#  need them in the future.


#  20200519
#
#  dpkg  --unpack <any>  requires:
#    (program)     (path)              (provided by package)
#    diff      ->  /usr/bin/diff   ->  diffutils
#    ldconfig  ->  /sbin/ldconfig  ->  libc-bin
#    rm        ->  /bin/rm         ->  coreutils
#    sh        ->  /bin/sh         ->  dash
#
#  dpkg  --unpack libc  requires:
#    sed       ->  /bin/sed        ->  sed
#
#  dpkg  --configure dash  requires:
#    awk       ->  /usr/bin/awk    ->  mawk (must be configured!)
#
#  dpkg  --configure passwd  requires:
#    grep      ->  /bin/grep       ->  grep
#
#  apt-get  update  requires:
#    find      ->  /usr/bin/find   ->  findutils
#      ( without find, the result will be gpg errors !?! )


#  phase 1
#
#    note:  mawk must be configured prior to configuring dash
#
readonly  phase1=(  mawk  )


#  phase2
#
#    note:  phase2 installs apt-get and all its dependencies (both
#           specified and de facto).
#
#    note:  dash requires debconf.  debconf recommends apt-utils.
#
readonly  phase2=(  dpkg  coreutils  dash  diffutils  findutils  grep
                    libc-bin  sed  )


#  phase 3
#
#    note:  phase 3 packages may not be strictly required.
#           they are installed mostly for my convenience.
#
readonly  phase3=(
  bash            #  bash
  debianutils     #  which
  fakeroot        #  fakeroot (used to run apt-get inside lxroot)
  less            #  less
  ncurses-base    #  clear
  ncurses-bin     #  clear
  util-linux      #  getopt (used by fakeroot)
  )


readonly  phase123=(  "${phase1[@]}"  "${phase2[@]}"  "${phase3[@]}"  )


#  xbash  -------------------------------------------------------------  xbash


#  These functions are copied from my (unpublished) xbash library.


log             (){  1>&2  echo    "$@"        ;}    #  -----------------  log
trace           (){  log  "+  $*"  ;  "$@"     ;}    #  ---------------  trace

e_log           (){  log   ;  log        "$@"  ;}    #  ---------------  e_log
ee              (){  echo  ;  echo       "$@"  ;}    #  ------------------  ee
et              (){  log   ;  trace      "$@"  ;}    #  ------------------  et

is_dir          (){     test  -d  "$1"  ;}    #  ---------------------  is_dir
is_file         (){     test  -f  "$1"  ;}    #  --------------------  is_file
is_str          (){     test  -n  "$1"  ;}    #  ---------------------  is_str
not_dir         (){  !  test  -d  "$1"  ;}    #  --------------------  not_dir
not_exists      (){  !  test  -e  "$1"  ;}    #  -----------------  not_exists
not_file        (){  !  test  -f  "$1"  ;}    #  -------------------  not_file
not_str         (){  !  test  -n  "$1"  ;}    #  --------------------  not_str

is_equal        (){  test  "$1"  =    "$2"  ;}    #  ---------------  is_equal
not_equal       (){  test  "$1"  !=   "$2"  ;}    #  --------------  not_equal


dd_soft  ()  {    #  ------------------------------------------------  dd_soft
  #  usage:  dd_soft  dst  text
  local  dst="$1"  text="$2"
  is_exists  "$dst"  &&  return
  echo  "$text"  |  trace  dd  of="$dst"  ;  }


dd_sync  ()  {    #  ------------------------------------------------  dd_sync
  #  usage:  dd_sync  dst  text
  local  dst="$1"  text="$2"
  not_exists  "$dst"  &&  {  dd_soft  "$dst"  "$text"  ;  return  ;  }
  local  actual="$(  cat  "$dst"  )"
  is_equal  "$actual"  "$text"  &&  return
  echo  "$text"  |  trace  dd  of="$dst"  ;  }


fetch  ()  {    #  ----------------------------------------------------  fetch
  #  usage:  fetch  url  path
  local  url="$1"  path="$2"
  local  dir="${path%/*}"
  is_file  "$path"  &&  {  echo  "fetch  found  $path"  ;  return  ;  }
  not_dir  "$dir"   &&  trace  mkdir  -p  "$dir"
  trace  wget  -O "$path"  "$url"  ;  }


is_link  ()  {    #  ------------------------------------------------  is_link
  #  usage:  is_link  path  [text]
  local  path="$1"  text="${2-}"
  test  !  -h  "$path"  &&  return  1
  case  "$#"  in
    ( 1 )  return  0  ;;
    ( 2 )  is_equal  "$( readlink "$path" )"  "$text"  ;;
    ( * )  die  ;;  esac  }


trace_mkdir  ()  {    #    --------------------------------------  trace_mkdir
  #  usage:  trace_mkdir  [-p]  path
  case  "$1"  in
    ( -p )  is_dir  "$2"  &&  return  ;;
    ( *  )  is_dir  "$1"  &&  return  ;;  esac
  trace  mkdir  "$@"  ;  }


trace_symlink_warn  ()  {    #  --------------------------  trace_symlink_warn

  #  usage:  trace_symlink_warn  text  path

  local  text="$1"  path="$2"

  is_link  "$path"  &&  {
    local  actual="$(  readlink  "$path"  ||  true  )"
    is_equal  "$actual"  "$text"  &&  return
    e_log  "trace_symlink_warn  bad link"
    log    "  path    $path"
    log    "  expect  '$text'"
    log    "  actual  '$actual'"  ;  }

  not_exists  "$path"  &&  {
    trace  ln  -sn  "$text"  "$path"  ;  return  ;  }

  e_log  "trace_symlink_warn  not a link"
  log    "  path    $path"
  log    "  expect  '$text'"  ;  }


trace_touch  ()  {    #  ----------------------------------------  trace_touch
  #  usage:  trace_touch  path
  local  path="$1"
  not_exists  "$path"  &&  trace  touch  "$path"
  return  0  ;  }


#  lib  -----------------------------------------------------------------  lib


#  These helper functions are not from my xbash library.


do_once  ()  {    #  ------------------------------------------------  do_once
  #  usage:  do_once  command
  local  command="$1"
  local  once="$newroot/root/.hide/mk-deb-once/$command"
  is_file  "$once"  &&  {  echo  "do_once  found  $command"  ;  return  ;  }
  echo  "do_once  run  $command"
  "$command"
  log  "do_once  done  $command"
  trace  touch  "$once"  ;  }


simulate  ()  {    #  ----------------------------------------------  simulate
  #  usage:  simulate
  is_str  "${simulate-}"  &&  return
  readonly  simulate="$(
    apt-get  install  --simulate  "${apt_opts[@]}"  "${phase123[@]}"  )"
  return  ;  }


symlink  ()  {    #  ------------------------------------------------  symlink
  #  usage:  symlink  path  text
  local  path="$1"  text="$2"
  trace_symlink_warn  "$text"  "$newroot/${path#/}"  ;  }


#  command  ---------------------------------------------------------  command


clean  ()  {    #  ----------------------------------------------------  clean

  #  usage:  clean

  ee  '--  clean'

  local  trash="$newroot/.trash"

  trace_mkdir  "$trash"
  trace  mv  "$newroot"/*  "$trash"  ||  true

  init_skel

  trace  rmdir    "$newroot"/var/cache/apt/archives/partial
  trace  rmdir    "$newroot"/var/cache/apt/archives
  trace  rmdir    "$newroot"/var/cache/apt

  trace  rmdir    "$newroot"/var/lib/apt/lists/partial
  trace  rmdir    "$newroot"/var/lib/apt/lists
  trace  rmdir    "$newroot"/var/lib/apt

  trace  mv  -T   "$trash"/var/cache/apt  "$newroot"/var/cache/apt
  trace  mv  -T   "$trash"/var/lib/apt    "$newroot"/var/lib/apt

  trace  rm  -rf  "$newroot/.trash"

  return  ;  }


download  ()  {    #  ----------------------------------------------  download
  #  usage:  download
  init
  et  apt-get  update  "${apt_opts[@]}"
  et  apt-get  install  --download-only  --yes  \
        "${apt_opts[@]}"  "${phase123[@]}"  ;  }


extract_line  ()  {    #  --------------------------------------  extract_line

  #  usage:  extract_line

  case  "$line"  in
    ( 'Inst '* )  true    ;;
    ( *        )  return  ;;  esac

  local  pkg="$line"
  local  pkg="${pkg#Inst }"
  local  pkg="${pkg%% *}"

  local  deb=(  "$newroot"/var/cache/apt/archives/"$pkg"_*.deb  )
  local  count="${#deb[@]}"

  not_equal  "$count"  '1'  &&
    halt  "halt  extract_line  bad count  $count  $pkg"

  echo  "extract  ${deb##*/}"
  dpkg  --extract  "$deb"  "$newroot"

  return  ;  }


extract  ()  {    #  ------------------------------------------------  extract
  #  usage:  extract
  echo
  simulate
  local  line
  echo  "$simulate"  |  {
    local  line
    while  read  line
    do     extract_line  ;  done  }  }


init_apt_trusted  ()  {    #  ------------------------------  init_apt_trusted

  #  usage:  init_apt_trusted

  local  dir="$newroot"/etc/apt/trusted.gpg.d
  is_dir  "$dir"  &&  {  echo  "init  found  $dir"  ;  return  ;  }

  local  url='https://mirrors.edge.kernel.org/debian/pool/main/d/debian-archive-keyring/debian-archive-keyring_2023.4_all.deb'

  local  path="$newroot"/var/cache/apt/mk-deb/"${url##*/}"

  fetch  "$url"  "$path"
  #  20231007  todo  verify checksum
  trace  dpkg  --extract  "$path"  "$newroot"

  return  0  ;  }


init_skel  ()  {    #  --------------------------------------------  init_skel

  #  usage:  init_skel

  local  mkdir_list=(
    /bin
    /dev
    /etc/alternatives
    /etc/apt/apt.conf.d
    /etc/apt/preferences.d
    /etc/apt/sources.list.d
    "$HOME"
    /proc
    /root/.hide/mk-deb-once
    /sys
    /tmp/
    /usr/bin
    /usr/local
    /usr/sbin
    /var/cache/apt/archives/partial
    /var/lib/apt/lists/partial
    /var/lib/dpkg
    /var/lib/dpkg/info
    /var/lib/dpkg/updates
    /var/log

    /run        #  20231004  Only Lxroot needs /run.      A bug in Lxroot?
    /var/tmp    #  20231004  Only Lxroot needs /var/tmp.  A bug in Lxroot?

    )

  local  touch_list=(
    /etc/gshadow
    /etc/shadow
    /var/lib/dpkg/available
    /var/lib/dpkg/status  )

  local  path
  for    path  in  "${mkdir_list[@]}"
  do     trace_mkdir  -p  "$newroot/${path#/}"  ;  done
  for    path  in  "${touch_list[@]}"
  do     trace_touch  "$newroot/${path#/}"  ;  done

  symlink  /bin/env                    /usr/bin/env
  symlink  /usr/bin/fakeroot           /etc/alternatives/fakeroot
  symlink  /usr/bin/pager              /etc/alternatives/pager
  symlink  /usr/bin/which              /etc/alternatives/which

  symlink  /etc/alternatives/fakeroot  /usr/bin/fakeroot-sysv
  symlink  /etc/alternatives/pager     /usr/bin/less
  symlink  /etc/alternatives/which     /usr/bin/which.debianutils

  return  ;  }


init_etc_group  ()  {    #  ----------------------------------  init_etc_group
  #  usage:  init_etc_group
  local  group="$(  id  -gn  )"  gid="$(  id  -g  )"
  local  groups=(
    root:x:0:           #  root
    tty:x:5:syslog      #  required by what ?
    mail:x:8:           #  required by what ?
    man:x:12:           #  required by what ?
    dip:x:30:           #  required by what ?
    shadow:x:42:        #  required by what ?
    utmp:x:43:          #  required by what ?
    staff:x:50:         #  required by what ?
    nogroup:x:65534:    #  required by what ?
    "$group:x:$gid"     #  self
    )
  init_etc_passwd_impl  groups  "$newroot"/etc/group  ;  }


init_etc_passwd  ()  {    #  --------------------------------  init_etc_passwd
  #  usage:  init_etc_passwd
  local  user="$(  id  -un  )"  uid="$(  id  -u  )"  gid="$(  id  -g  )"
  local  users=(
    root:x:0:0:root:/root:/bin/bash
    man:x:6:12:man:/var/cache/man:/usr/sbin/nologin  #  required by what ?
    "$user:x:$uid:$gid::/home/$user:/bin/bash"
    )
  init_etc_passwd_impl  users  "$newroot"/etc/passwd  ;  }


init_etc_passwd_impl  ()  {    #  ----------------------  init_etc_passwd_impl

  #  usage:  init_etc_passwd  array_name  path

  local     -n  expect_list="$1"  ;  local  path="$2"

  not_file  "$path"  &&  trace  touch  "$path"
  local     actual_list   ;  readarray  actual_list  <  "$path"

  declare   -A  actual_table
  local     line
  for       line   in  "${actual_list[@]}"
  do        local  user="${line%%:*}"
            actual_table["$user"]="$line"
            done

  local     not_found=()
  for       line  in  "${expect_list[@]}"
  do        local  user="${line%%:*}"
            is_str  "${actual_table[$user]-}"  &&  continue
            not_found+=(  "$line"  )
            done

  is_equal  "${#not_found[@]}"  '0'  &&  return
  echo      "appending ${#not_found[@]} lines to  $path"
  printf    '%s\n'  "${not_found[@]}"  >>  "$path"

  return  ;  }


init_sources_list  ()  {    #  ----------------------------  init_sources_list

  #  usage:  init_sources_list

  local  suite='bookworm'

  local  path="$newroot"/etc/apt/sources.list.d/debian.list

  is_file  "$path"  &&  {  echo  "init  found  $path"  ;  return  ;  }

  local  comps='main  contrib  non-free-firmware'

  local  text=`cat  <<EOF
#  This file was created by mk-deb.sh.

deb      http://deb.debian.org/debian/           $suite            $comps
deb-src  http://deb.debian.org/debian/           $suite            $comps

deb      http://deb.debian.org/debian-security/  $suite-security   $comps
deb-src  http://deb.debian.org/debian-security/  $suite-security   $comps

deb      http://deb.debian.org/debian/           $suite-updates    $comps
deb-src  http://deb.debian.org/debian/           $suite-updates    $comps

deb      http://deb.debian.org/debian/           $suite-backports  $comps
deb-src  http://deb.debian.org/debian/           $suite-backports  $comps
EOF`

  dd_sync  "$path"  "$text"

  return  ;  }


init_vars  ()  {    #  --------------------------------------------  init_vars

  #  usage:  init_vars

  readonly  newroot="$world/newroot"    #  a global variable

  local     abs_newroot="$(  realpath  --canonicalize-missing  "$newroot"  )"

  readonly  apt_opts=(    #  a global variable
    "-o=Dir=${abs_newroot}"
    "-o=Dir::State::status=${abs_newroot}/var/lib/dpkg/status"  )

  return  ;  }


init  ()  {    #  ------------------------------------------------------  init
  #  usage:  init
  init_skel
  init_apt_trusted
  init_etc_group
  init_etc_passwd
  init_sources_list
  return  ;  }


install  ()  {    #  ------------------------------------------------  install
  #  usage:  install
  init
  do_once  download
  do_once  extract
  et  lxroot  -w  "$world"  --  \
    fakeroot  apt-get  install  --assume-yes  "${phase123[@]}"

  ee  'mk-deb  all done!'

  return  ;  }


#  main  ---------------------------------------------------------------  main


main  "$@"  ;  exit  0




#  mk-deb.sh  -  Create a Debian chroot environment (for use with Lxroot).
#
#  Copyright (c) 2023 Parke Bostrom, parke.nexus at gmail.com
#
#  This program is free software: you can redistribute it and/or
#  modify it under the terms of version 3 of the GNU General Public
#  License as published by the Free Software Foundation.
#
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See version 3
#  of the GNU General Public License for more details.
#
#  You should have received a copy of version 3 of the GNU General
#  Public License along with this program.  If not, see
#  <https://www.gnu.org/licenses/>.
