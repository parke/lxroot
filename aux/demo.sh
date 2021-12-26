#! /bin/sh


#  version  20211111


set  -o errexit


usage  ()  {    #  ----------------------------------------------------  usage
  echo
  echo  'usage:  bash  demo.sh  [options]'
  echo
  echo  'options:'
  echo  '  demo1  <path>                #  run demo1 in <path>'
  echo  '  demo3  <path>                #  run demo3 in <path> '
  echo  '  demo3  <path>  root          #  enter the demo3 environment as root'
  return  ;  }


main  ()  {    #  ------------------------------------------------------  main
  case  "$1"  in
    (demo1)            "$@"  ;;
    (demo1_extract)    "$@"  ;;
    (demo3)            "$@"  ;;
    (demo3_u1)         "$@"  ;;
    (demo3_u2)         "$@"  ;;
    (demo3_u3)         "$@"  ;;
    (*)                usage  ;  exit  1   ;;  esac
  exit  ;  }


#  These values are used by demo1 and demo1_extract.
demo1_tgz='alpine-minirootfs-3.13.5-x86_64.tar.gz'
demo1_url='https://dl-cdn.alpinelinux.org/alpine/v3.13/releases/x86_64'
sha256='a103f4f5560d3ae59d53fcc44fe78a42e32e421c0a2981c03c297f23a3965071'


#  These values are used by demo3.
demo3_ver='2021.11.01'
demo3_iso=archlinux-"$demo3_ver"-x86_64.iso
demo3_url=https://mirror.rackspace.com/archlinux/iso/"$demo3_ver"
demo3_md5='e42e562dd005fbe15ade787fe1ddba48'
demo3_sha1='d81bff7f9b05653b048529d741edcce99bc97819'

#  demo3 uses only a single Arch mirror in hopes of avoiding problems
#  caused by skew across mirrors.  You may specify a different mirror
#  below, if you wish.
demo3_mirrorlist='Server = https://mirror.sfo12.us.leaseweb.net/archlinux/$repo/os/$arch'


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
    trace  tar  xzf dist/"$demo1_tgz"  -C demo1  ;  fi

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


fetch  ()  {    #  ----------------------------------------------------  fetch

  local  url="$1"  path="$2"

  [ -f "$path" ]  &&  {  echo  "fetch  found  '$path'"  ;  return  ;  }

  echo
  echo  "fetch  $url"
  echo

  trace  wget  --no-clobber  -O "$path"  "$url"  &&  return

  echo
  echo  'fetch  fetch appears to have failed'
  echo  "  url   $url"
  echo  "  path  $(  realpath  "$path"  )"
  echo
  die  'demo.sh  fetch  die'  ;  }


verify  ()  {    #  --------------------------------------------------  verify

  #  usage:  verify  [-v]  path  algo  expect

  local  verbose=''
  if  [ "$1" = '-v' ]  ;  then  verbose=1  ;  shift  ;  fi
  local  path="$1"  algo="$2"  expect="$3"

  if  [ "$verbose" = '1' ]  ;  then  echo  "$algo  $path"  ;  fi
  local  actual="$(  "$algo"  "$path"  )"
  actual="${actual%% *}"

  if  [ ! "$actual" = "$expect" ]  ||  [ "$verbose" = '1' ]  ;  then
    echo  "verify    $algo  $path"
    echo  "  path    $(  realpath  "$path"  )"
    echo  "  size    $(  stat  --format '%s'  "$path"  )"
    echo  "  expect  $expect"
    echo  "  actual  $actual"
    fi

  if  [ "$actual" = "$expect" ]  ;  then  return  ;  fi
  echo  "verify  failed  exiting..."
  exit  1  ;  }


demo1_verify  ()  {    #  --------------------------------------  demo1_verify

  local  path=dist/"$demo1_tgz"
  mkdir  -p  dist
  if  [ -e "$path" ]  ;  then
    echo
    echo  '#  ( Alpine Linux minirootfs - file found             )'
  else
    echo
    echo  '#  ( Alpine Linux minirootfs - downloading file ...   )'
    wget  --no-clobber  -O "$path"  "$demo1_url"/"$demo1_tgz"  ;  fi

  verify  "$path"  sha256sum  "$sha256"
  echo  "#  ( Alpine Linux minirootfs - file checksum is valid )"  ;  }


demo1_extract  ()  {    #  ------------------------------------  demo1_extract

  local  demo="$1"  extract="$2"
  local  url="$demo1_url"

  if  [ -d "$extract" ]  ;  then
    echo  "demo1_extract  found  '$extract'"  ;  return  ;  fi
  mkdir  -p  "$demo"  ;  cd  "$demo"  ;  demo1_verify
  mkdir  -p  "$extract"
  trace  tar  xzf dist/"$demo1_tgz"  -C "$extract"  ;  }


demo1 ()  {    #  -----------------------------------------------------  demo1
  local  demo="$1"
  if  [ ! -x "$demo"/lxroot ]  ;  then
    die  "demo.sh  error  executable not found  $demo/lxroot"  ;  fi
  demo1_run  "$demo"  ;  }


#  demo 3  -----------------------------------------------------------  demo 3


demo3_verify  ()  {    #  --------------------------------------  demo3_verify
  [ -f dist/"$iso" ]  ||  fetch  "$url"/"$iso"  dist/"$iso"
  verify  -v  dist/"$iso"  md5sum     "$demo3_md5"
  verify  -v  dist/"$iso"  sha1sum    "$demo3_sha1"
  return  ;  }


demo3_host  ()  {    #  ------------------------------------------  demo3_host

  #  demo3_host is run on the host.
  #  demo3_host creates userland #1 (Alpine).

  echo
  echo  "demo3  $*"

  trace  cd  "$demo"
  demo1_extract  .  demo3
  demo3_verify

  trace  mkdir  -p  demo3/dist/
  trace  cp  /etc/resolv.conf  demo3/etc/
  trace  cp  bin/demo.sh       demo3/dist/
  trace  ln  -f  dist/"$iso"   demo3/dist/"$iso"

  return  ;  }


demo3_u1_extract  ()  {    #  ------------------------------  demo3_u1_extract

  [ -d /userland2 ]  &&  {
    echo  'demo3_u1_extract  found  /userland2'  ;  return  ;  }

  if  [ ! -f /root/update ]  ;  then
    trace  apk  update  ;  touch  /root/update  ;  fi

  trace  apk  add  p7zip  squashfs-tools

  trace  mkdir  -p  /iso-extract
  trace  cd  /iso-extract
  trace  mute  7z  x  -aos  /dist/"$iso"  arch/x86_64/airootfs.sfs
  trace  mute  7z  x  -aos  /dist/"$iso"  arch/x86_64/airootfs.sfs.sig
  trace  mute  7z  x  -aos  /dist/"$iso"  arch/x86_64/airootfs.sha512

  trace  cd  arch/x86_64
  trace  sha512sum  -c airootfs.sha512

  trace  cd  /iso-extract
  [ -d squashfs-root ]  \
    ||  trace  unsquashfs  -no-xattrs  /iso-extract/arch/x86_64/airootfs.sfs  \
    ||  true
  trace  mv  squashfs-root  userland2
  trace  rm  -f  userland2/etc/resolv.conf
  trace  cp  /etc/resolv.conf  userland2/etc/resolv.conf

  trace  cd  /iso-extract/userland2/etc/pacman.d
  if  [ -f mirrorlist ]  &&  [ ! -f mirrorlist-orig ]  ;  then
    trace  cp  mirrorlist  mirrorlist-orig  ;  fi
  echo  "$demo3_mirrorlist"  >  mirrorlist

  trace  cd  /
  trace  mv  /iso-extract/userland2  /userland2
  trace  rm  -rf  /iso-extract
  trace  chmod  -R  u+w  /userland2
  trace  du  -hs  /dist  /userland2

  return  ;  }


demo3_u1  ()  {    #  ----------------------------------------------  demo3_u1

  #  demo3_u1 is run in userland #1 (Alpine).
  #  demo3_u1 creates userland #2 (Arch bootstrap).

  local  iso="$demo3_iso"

  echo
  echo  'demo3_u1'

  demo3_u1_extract

  trace  mkdir  -p  /userland2/dist/
  trace  cp  /dist/demo.sh  /userland2/dist/

  return  ;  }


demo3_u2_bootstrap  ()  {    #  --------------------------  demo3_u2_bootstrap

  [ -d /userland3 ]  &&  {
    echo  "demo3_u2_bootstrap  found  /userland3"  ;  return  ;  }

  trace  pacman-key  --init
  trace  pacman-key  --populate  archlinux
  trace  mkdir  -p  /mnt/userland3/etc/pacman.d
  trace  mkdir  -p  /mnt/userland3/var/lib/pacman/
  trace  cp  /etc/resolv.conf  /mnt/userland3/etc/
  trace  cp  -a  /etc/pacman.d/gnupg  /mnt/userland3/etc/pacman.d/

  trace  cd  /mnt/userland3/etc/pacman.d
  echo  "$demo3_mirrorlist"  >  mirrorlist

  trace  cd  /
  trace  pacman  -r /mnt/userland3  -Sy  --noconfirm  pacman

  trace  mv  /mnt/userland3  /userland3

  return  ;  }


demo3_u2  ()  {    #  ----------------------------------------------  demo3_u2

  #  demo3_u2 is run in userland #2 (Arch bootstrap).
  #  demo3_u2 creates userland #$3 (Arch actual).

  echo
  echo  'demo3_u2'

  demo3_u2_bootstrap

  trace  mkdir  -p  /userland3/dist/
  trace  cp  /dist/demo.sh  /userland3/dist/

  return  ;  }


demo3_u3  ()  {    #  ----------------------------------------------  demo3_u3

  #  demo3_3 is run in userland #3 (Arch actual).

  echo
  echo  'demo3_u3'

  [ -f /usr/bin/chromium ]  &&  {
    echo  'demo3_u3  found  /usr/bin/chromium'  ;  return  ;  }

  trace  mkdir  -p  /tmp/.X11-unix
  trace  pacman  -Sy  --noconfirm  chromium

  return  ;  }


demo3_u3_shell  ()  {    #  ----------------------------------  demo3_u3_shell

  mkdir  -p  demo3/userland2/userland3/home/"$USER"  ||  true

  echo
  echo
  echo  "(  The Demo #3 guest userland has been created.       )"
  echo  "(                                                     )"
  echo  "(  Make will now launch an interactive shell in the   )"
  echo  "(  Demo #3 guest userland.                            )"
  echo  "(                                                     )"
  echo  "(  Please run 'chromium' in this shell to attempt to  )"
  echo  "(  run Chromium.                                      )"
  echo  "(                                                     )"
  echo  "(  Please press ENTER when you are ready to proceed.  )"

  read  discard
  bin/lxroot  -nx  demo3/userland2/userland3

  return  ;  }


demo3_u3_shell_root  ()  {    #  ------------------------  demo3_u3_shell_root
  trace  cd  "$demo"
  trace  bin/lxroot  -nr  demo3/userland2/userland3  ;  }


demo3  ()  {    #  ----------------------------------------------------  demo3

  local  demo="$1"  mode="$2"
  local  iso="$demo3_iso"
  local  url="$demo3_url"

  if  [ "$root" = 'root' ]  ;  then
    echo
    echo  'demo3  root'
    return  ;  fi

  local  u1='demo3'
  local  u2='demo3/userland2'
  local  u3='demo3/userland2/userland3'

  demo3_host
  trace  bin/lxroot  -nr  "$u1"  --  /bin/sh  /dist/demo.sh  demo3_u1
  trace  bin/lxroot  -nr  "$u2"  --  /bin/sh  /dist/demo.sh  demo3_u2
  trace  bin/lxroot  -nr  "$u3"  --  /bin/sh  /dist/demo.sh  demo3_u3

  case  "$mode"  in
    (''    )  demo3_u3_shell       ;;
    ('root')  demo3_u3_shell_root  ;;
    (*)       die  "demo3  bad mode  '$mode'"  ;;  esac

  return  ;  }


#  main  ---------------------------------------------------------------  main


main  "$@"
