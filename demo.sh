#! /bin/bash


#  version  20210608


set  -o errexit


usage  ()  {    #  ----------------------------------------------------  usage
  echo
  echo  'usage:'
  echo  '  bash  demo.sh  demo1  <path>'
  echo  '  bash  demo.sh  demo3  <path>'
  echo  '  bash  demo.sh  alpine_extract  <path>  <subdir>'
  return  ;  }


#  These values are used by demo1 and demo1_extract.
url='https://dl-cdn.alpinelinux.org/alpine/v3.13/releases/x86_64'
file='alpine-minirootfs-3.13.5-x86_64.tar.gz'
sha256='a103f4f5560d3ae59d53fcc44fe78a42e32e421c0a2981c03c297f23a3965071'


demo1_run  ()  {    #  --------------------------------------------  demo1_run
  local  demo="$1"
  echo
  echo  '#  ( Welcome to the demo of lxroot!                       )'
  echo  '#  ( This demo creates an Alpine Linux guest userland and )'
  echo  '#  ( then runs an interactive shell inside it.            )'

  echo
  trace  cd  "$demo"

  if  [ -d demo1 ]  ;  then
    echo
    echo  '#  ( We will use the existing directory /tmp/lxroot-demo/demo1. )'
    echo  "#  ( Run 'make demo-clean' if you wish to delete this direcotry )"
    echo  '#  ( and start fresh.                                           )'
  else
    #  alpine_verify will download and verify the Apline minitrootfs tarball.
    demo1_verify
    echo
    trace  mkdir  demo1
    trace  tar  xzf dist/"$file"  -C demo1  ;  fi

  if  [ ! -f demo1/etc/resolv.conf ]  ;  then
    trace  cp  /etc/resolv.conf  demo1/etc/  ;  fi

  echo
  echo  '#  ( We will now run the following command to start an interactive  )'
  echo  '#  ( shell inside the Alpine Linux guest userland:                  )'
  echo  '#  (                                                                )'
  echo  '#  (   ./lxroot  -nr  demo1                                         )'
  echo  '#  (                                                                )'
  echo  "#  ( The '-n' option allows network access.                         )"
  echo  "#  ( The '-r' option maps the uid and gid to zero.  In other words, )"
  echo  "#  (   '-r' simulates being the root user.                          )"
  echo  "#  ( 'demo1' is the name of the directory that contains the         )"
  echo  '#  (   Alpine Linux guest userland.                                 )'
  echo  '#  (                                                                )'
  echo  '#  ( The prompt inside the demo should be something like:           )'
  echo  '#  (                                                                )'
  echo  '#  (   root  -nr  ./demo1  ~                                        )'
  echo  '#  (                                                                )'
  echo  "#  ( 'root'    is the name of the (possibly simulated) user         )"
  echo  "#  ( '-nr'     is a summary of some of the command line options     )"
  echo  "#  ( './demo1' is the name of the newroot directory                 )"
  echo  "#  ( '~'       is the name of the current working directory         )"

  echo
  trace  exec  ./lxroot  -nr  demo1  ;  }


die  ()  {  echo  "$*"  ;  exit  1  ;  }    #  --------------------------  die
mute  ()  {  >/dev/null  2>/dev/null  "$@"  ;  }    #  -----------------  mute
trace  ()  {   echo  "+  $@"  ;  "$@"  ;  }    #  ---------------------  trace


verify  ()  {    #  --------------------------------------------------  verify

  #  usage:  verify  [-v]  path  algo  expect

  local  verbose=''
  if  [ "$1" = '-v' ]  ;  then  verbose=1  ;  shift  ;  fi
  local  path="$1"  algo="$2"  expect="$3"

  if  [ "$verbose" = '1' ]  ;  then  echo  "$algo  $path"  ;  fi
  local  actual=`  "$algo"  "$path"  `
  actual="${actual%% *}"

  if  [ ! "$actual" = "$expect" ]  ||  [ "$verbose" = '1' ]  ;  then
    echo  "verify    $algo  $path"
    echo  "  expect  $expect"
    echo  "  actual  $actual"
    fi

  if  [ "$actual" = "$expect" ]  ;  then  return  ;  fi
  echo  "verify  failed  exiting..."
  exit  1  ;  }


demo1_verify  ()  {    #  --------------------------------------  demo1_verify

  local  path=dist/"$file"
  mkdir  -p  dist
  if  [ -e "$path" ]  ;  then
    echo
    echo  '#  ( Alpine Linux minirootfs - file found             )'
  else
    echo
    echo  '#  ( Alpine Linux minirootfs - downloading file ...   )'
    wget  --quiet  --no-clobber  -O "$path"  "$url"/"$file"  ;  fi

  verify  "$path"  sha256sum  "$sha256"
  echo  "#  ( Alpine Linux minirootfs - file checksum is valid )"  ;  }


demo1_extract  ()  {    #  ------------------------------------  demo1_extract
  local  demo="$1"  extract="$2"
  if  [ -d "$extract" ]  ;  then
    echo  'demo1_extract  already done'  ;  return  ;  fi
  mkdir  -p  "$demo"  ;  cd  "$demo"  ;  demo1_verify
  mkdir  -p  "$extract"
  trace  tar  xzf dist/"$file"  -C "$extract"  ;  }


demo1 ()  {    #  -----------------------------------------------------  demo1
  local  demo="$1"
  if  [ ! -x "$demo"/lxroot ]  ;  then
    die  "demo.sh  error  executable not found  $demo/lxroot"  ;  fi
  demo1_run  "$demo"  ;  }


#  demo 3  -----------------------------------------------------------  demo 3


demo3_u1_create_u2  ()  {    #  --------------------------  demo3_u1_create_u2

  if  [ -d /userland2 ]  ;  then
    echo  'demo3_u1_create_u2  already done'  ;  return  ;  fi

  echo  'demo3_u1_create_u2'

  if  [ ! -f /root/update ]  ;  then
    trace  apk  update  ;  touch  /root/update  ;  fi

  trace  apk  add  p7zip  squashfs-tools

  local  iso='archlinux-2021.06.01-x86_64.iso'

  verify  -v  /dist/"$iso"  md5sum     1bf76d864651cc6454ab273fd3d2226a
  verify  -v  /dist/"$iso"  sha1sum    6c41a22fb3c5eabfb7872970a9b5653ec47c3ad5
  verify  -v  /dist/"$iso"  sha256sum  \
    bd23f81dfb7a224589ccabed7f690c33fbc243bae3f32d295547a64445ae0245

  trace  mkdir  -p  /iso-extract
  trace  cd  /iso-extract
  trace  mute  7z  x  -aos  /dist/"$iso"  arch/x86_64/airootfs.sfs
  trace  mute  7z  x  -aos  /dist/"$iso"  arch/x86_64/airootfs.sfs.sig
  trace  mute  7z  x  -aos  /dist/"$iso"  arch/x86_64/airootfs.sha512

  trace  cd  arch/x86_64
  trace  sha512sum  -c airootfs.sha512

  trace  cd  /iso-extract
  trace  unsquashfs  -no-xattrs  /iso-extract/arch/x86_64/airootfs.sfs
  trace  mv  squashfs-root  userland2
  trace  rm  -f  userland2/etc/resolv.conf
  trace  cp  /etc/resolv.conf  userland2/etc/resolv.conf

  trace  cd  /
  trace  mv  /iso-extract/userland2  /userland2
  trace  rm  -rf  /iso-extract
  trace  chmod  -R  u+w  /userland2
  trace  du  -hs  /dist  /userland2

  return  ;  }


demo3_u2_create_u3  ()  {    #  --------------------------  demo3_u2_create_u3

  if  [ -d /userland3 ]  ;  then
    echo  'demo3_u2_create_u3  already done'  ;  return  ;  fi

  local  mirror='Server = https://mirror.sfo12.us.leaseweb.net/archlinux/$repo/os/$arch'

  echo  'demo3_u2_create_u3'

  trace  pacman-key  --init
  trace  pacman-key  --populate  archlinux
  trace  mkdir  -p  /mnt/userland3/etc/pacman.d
  trace  mkdir  -p  /mnt/userland3/var/lib/pacman/
  trace  cp  /etc/resolv.conf  /mnt/userland3/etc/
  trace  cp  -a  /etc/pacman.d/gnupg  /mnt/userland3/etc/pacman.d/

  echo  "$mirror"  >>  /mnt/userland3/etc/pacman.d/mirrorlist

  trace  pacman  -r /mnt/userland3  -Sy  --noconfirm  pacman

  trace  mv  /mnt/userland3  /userland3

  return  ;  }


demo3_u3_finish  ()  {    #    #  ---------------------------  demo3_u3_finish

  if  [ -f /usr/bin/chromium ]  ;  then
    echo  'demo3_u3_finish  already done'  ;  return  ;  fi

  echo
  echo  'demo3_u3_finish'

  trace  mkdir  -p  /tmp/.X11-unix
  trace  pacman  -Sy  --noconfirm  chromium

  return  ;  }


main  ()  {    #  ------------------------------------------------------  main
  case  "$1"  in
    (demo1)               "$1"  "$2"        ;;
    (demo1_extract)       "$1"  "$2"  "$3"  ;;
    (demo3_u1_create_u2)  "$1"              ;;    #  run in userland1
    (demo3_u2_create_u3)  "$1"              ;;    #  run in userland2
    (demo3_u3_finish)     "$1"              ;;    #  run in userland3
    (*)                 usage  ;  exit  1   ;;  esac
  exit  ;  }


main  "$@"
