#! /bin/bash


#  version  20210526


set  -o errexit


usage  ()  {    #  ----------------------------------------------------  usage
  echo
  echo  'usage:'
  echo  '  bash  demo.sh  alpine  <path>'
  echo  '  bash  demo.sh  alpine_extract  <path>  <subdir>'
  return  ;  }


url='https://dl-cdn.alpinelinux.org/alpine/v3.13/releases/x86_64'
file='alpine-minirootfs-3.13.5-x86_64.tar.gz'
sha256='a103f4f5560d3ae59d53fcc44fe78a42e32e421c0a2981c03c297f23a3965071'


demo_run  ()  {    #  ----------------------------------------------  demo_run
  local  demo_dir="$1"
  echo
  echo  '#  ( Welcome to the demo of lxroot!                           )'
  echo  '#  ( This demo creates an Alpine Linux chroot environment and )'
  echo  '#  ( then runs an interactive shell inside that environment.  )'

  echo
  trace  cd  "$demo_dir"

  if  [ -d newroot ]  ;  then
    echo
    echo  '#  ( We will use the existing directory /tmp/lxroot-demo/newroot. )'
    echo  "#  ( Run 'make demo-clean' if you wish to delete this direcotry   )"
    echo  '#  ( and start fresh.                                             )'
  else
    #  alpine_verify will download and verify the Apline minitrootfs tarball.
    alpine_verify
    echo
    trace  mkdir  newroot
    trace  tar  xzf dist/"$file"  -C newroot  ;  fi

  if  [ ! -f newroot/etc/resolv.conf ]  ;  then
    trace  cp  /etc/resolv.conf  newroot/etc/  ;  fi

  echo
  echo  '#  ( We will now run the following command to start an interactive  )'
  echo  '#  ( shell inside the Alpine Linux chroot environment:              )'
  echo  '#  (                                                                )'
  echo  '#  (   ./lxroot  -nr  newroot                                       )'
  echo  '#  (                                                                )'
  echo  "#  ( The '-n' option allows network access.                         )"
  echo  "#  ( The '-r' option maps the uid and gid to zero.  In other words, )"
  echo  "#  (   '-r' simulates being the root user.                          )"
  echo  "#  ( 'newroot' is the name of the directory that contains the       )"
  echo  '#  (   Alpine Linux chroot environment.                             )'
  echo  '#  (                                                                )'
  echo  '#  ( The prompt inside the demo should be something like:           )'
  echo  '#  (                                                                )'
  echo  '#  (   root  -nr  ./newroot  ~                                      )'
  echo  '#  (                                                                )'
  echo  "#  ( 'root'      is the name of the (possibly simulated) user       )"
  echo  "#  ( '-nr'       is a summary of some of the command line options   )"
  echo  "#  ( './newroot' is the name of the newroot directory               )"
  echo  "#  ( '~'         is the name of the current working directory       )"

  echo
  trace  exec  ./lxroot  -nr  newroot  ;  }


die  ()  {  echo  "$*"  ;  exit  1  ;  }    #  --------------------------  die
trace  ()  {   echo  "+  $@"  ;  "$@"  ;  }    #  ---------------------  trace


alpine_verify  ()  {    #  ----------------------------------  alpine_verify
  local  path=dist/"$file"
  mkdir  -p  dist
  if  [ -e "$path" ]  ;  then
    echo
    echo  '#  ( Found an Alpine Linux minirootfs tarball. )'
  else
    echo
    echo  '#  ( Downloading minirootfs tarball ... )'
    wget  --quiet  --no-clobber  -O "$path"  "$url"/"$file"  ;  fi
  local  actual
  actual=`  sha256sum  "$path"  `
  actual="${actual%% *}"
  if  [ "$actual" = "$sha256" ]  ;  then
    echo  "#  ( The tarball checksum is valid. )"  ;  return  ;  fi
  echo
  echo  "demo.sh  alpine_verify  checksum error"
  echo
  echo  "file    $file"
  echo  "expect  ($sha256)"
  echo  "actual  ($actual)"
  echo
  echo  "demo.sh  exiting"
  exit  1  ;  }


alpine_extract  ()  {    #  ----------------------------------  alpine_extract
  local  demo_dir="$1"  subdir="$2"
  if  [ -d "$demo_dir"/"$subdir" ]  ;  then
    echo  "demo.sh  found  $demo_dir/$subdir"  ;  return  ;  fi
  mkdir  -p  "$demo_dir"/"$subdir"
  cd  "$demo_dir"
  alpine_verify
  trace  tar  xzf dist/"$file"  -C "$subdir"  ;  }


alpine_main  ()  {    #  ----------------------------------------  alpine_main
  local  demo_dir="$1"
  if  [ ! -x "$demo_dir"/lxroot ]  ;  then
    die  "demo.sh  error  executable not found  $demo_dir/lxroot"  ;  fi
  demo_run  "$demo_dir"  ;  }


main  ()  {    #  ------------------------------------------------------  main
  case  "$1"  in
    (alpine_extract)  alpine_extract  "$2"  "$3"  ;;
    (alpine)          alpine_main     "$2"        ;;
    (*)               usage                       ;;  esac
  exit  ;  }


main  "$@"
