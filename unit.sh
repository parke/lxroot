#! /bin/bash


#  unit.sh  version  20210606


set  -o errexit
set  +o xtrace


run  ()  {    #  --------------------------------------------------------  run

  #  usage:  run  expect  env_name  cmd_name  arg...

  local  expect="$1"
  local  -n  aa="$2"  bb="$3"  cc="$4"
  local  argv=(  "${@:5}"  )
  local  command=(  "${aa[@]}"  "${bb[@]}"  "${argv[@]}"  "${cc[@]}"  )

  #  echo  "command  ${command[@]}"
  local  stdout  status  actual  pretty
  set  +o  errexit
  stdout=`  "${command[@]}"  2>&1  `
  status=$?
  pretty="$status"
  set  -o  errexit

  if    [ "$status" = 0 ]
    then  actual="$stdout"  pretty=' '
  elif  [ "$expect" = 'err  1-' ]    #  'err  1-' means ignore stdout
    then  actual="err  $status-"
  elif  [ "$stdout" ]
    then  actual="err  $status  $stdout"
    else  actual="err  $status"  ;  fi

  if  [ "$actual" = "$expect" ]  ;  then
    local  pretty2=(  "${bb[@]}"  "${argv[@]}"  "${cc[@]}"  )
    echo  "${BASH_LINENO[1]}  $pretty  ${pretty2[*]}"
    return  ;  fi

  case  "$expect"  in
    ('')        actual='see stdout'  ;;
    ('err  '?)  actual='see stdout'  ;;  esac

  echo
  echo  '--------'
  echo
  #  run_gdb
  echo  'unit2.sh  test fail'
  echo
  echo  "  line     ${BASH_LINENO[1]}"
  echo
  echo  "  expect   '$expect'"
  echo  "  actual   '$actual'"

  local  f='  %-7s  %s\n'
  echo
  printf  "$f"  'cwd'      "$PWD"
  printf  "$f"  "$2"       "${aa[*]}"
  printf  "$f"  "$3"       "${bb[*]}"
  printf  "$f"  'argv'     "${argv[*]}"
  printf  "$f"  "$4"       "${cc[*]}"
  printf  "$f"  'command'  "${command[*]}"
  printf  "$f"  'status'   "$status"

  if  [ ! "$status" = 0 ];  then
    echo
    echo  "  stdout  $stdout"  ;  fi

  echo
  exit  1  ;  }


run_gdb  ()  {    #  ------------------------------------------------  run_gdb
  gdb=(  /usr/bin/gdb  -q  -ex run  --args  )
  local  command=( "${env_ref[@]}" "${gdb[@]}" "${lxr_ref[@]}" "${argv[@]}" )
  "${command[@]}"  ;  }


die  ()  {  echo  "die  $*"  ;  exit  1  ;  }    #  ---------------------  die

popd   ()  {  builtin  popd   "$@"  >  /dev/null  ;  }    #  -----------  popd
pushd  ()  {  builtin  pushd  "$@"  >  /dev/null  ;  }    #  ----------  pushd

rm_soft     ()  { if  [ -e "$1" ];  then  rm     "$1"  ;  fi  ;  }    #  -----
rmdir_soft  ()  { if  [ -d "$1" ];  then  rmdir  "$1"  ;  fi  ;  }    #  -----

run1  ()  {  run  "$1"  env1  lxr1  cmd1  "${@:2}"  ;  }    #  ---------  run1
run2  ()  {  run  "$1"  env2  lxr2  cmd2  "${@:2}"  ;  }    #  ---------  run2
run3  ()  {  run  "$1"  env3  lxr3  cmd3  "${@:2}"  ;  }    #  ---------  run3


trace  ()  {   echo  "+  $@"  ;  "$@"  ;  }    #  ---------------------  trace


prepare  ()  {    #  ------------------------------------------------  prepare
  mkdir  -p  "$unit"/nr/tmp/.X11-unix
  echo  'unit.sh  done  prepare'  ;  }


prepare_users  ()  {    #  ------------------------------------  prepare_users
  pushd  "$unit"/nr/etc
  cp  -n  group   group-orig
  cp  -n  passwd  passwd-orig
  touch           group-none
  touch           passwd-none

  local  uid  ;  uid=`id  -u`
  local  gid  ;  gid=`id  -g`

  echo  "group2:x:$gid:"  >  group-user2
  echo  "user2:x:$uid:$gid::/home/user2:/bin/ash"  >  passwd-user2

  popd
  echo  'unit.sh  done  prepare_users'  ;  }


home_make  ()  {    #  --------------------------------------------  home_make
  mkdir  -p  "$unit/nr/home/$USER"
  mkdir  -p  "$unit/nr/home/user1"
  mkdir  -p  "$unit/nr/home/user2/ape"
  mkdir  -p  "$unit/nr/home/user3"
  mkdir  -p  "$unit/nr/root"
  return  ;  }


home_remove  ()  {    #  ----------------------------------------  home_remove
  rm_soft     "$unit/nr/home/user2/foo"
  rm_soft     "$unit/nr/root/foo"
  rm_soft     "$unit/nr/home/$USER/.ash_history"
  rm_soft     "$unit/nr/home/user1/.ash_history"
  rm_soft     "$unit/nr/home/user2/.ash_history"
  rm_soft     "$unit/nr/home/user3/.ash_history"
  rmdir_soft  "$unit/nr/home/$USER"
  rmdir_soft  "$unit/nr/home/user1"
  rmdir_soft  "$unit/nr/home/user2/ape"
  rmdir_soft  "$unit/nr/home/user2"
  rmdir_soft  "$unit/nr/home/user3"
  rmdir_soft  "$unit/nr/root"
  return  ;  }


user_activate  ()  {    #  ------------------------------------  user_activate
  pushd  "$unit/nr/etc"
  cp  group-"$1"   group
  cp  passwd-"$1"  passwd
  popd  ;  }


phase_one  ()  {    #  --------------------------------------------  phase_one

  #  no vars
  env1=(  env  -  )
  lxr1=(  ./lxr  nr  )
  cmd1=()

  #  only environment vars
  env2=(  env  -  HOME=/home/user1  LOGNAME=user1  USER=user1  )
  lxr2=(  ./lxr  nr  )
  cmd2=()

  #  explicit vars should override environment vars
  env3=(  env  -  HOME=/home/user1  LOGNAME=user1  USER=user1  )
  lxr3=(  ./lxr  nr
            HOME=/home/user3  LOGNAME=user3  USER=user3  SHELL=shell3  )
  cmd3=()

  return;  }


test_no_home  ()  {    #  --------------------------------------  test_no_home

  echo  ;  echo  "-  test_no_home"

  user_activate  none
  home_remove
  phase_one

  #  test  echo and exit

  run1  'foo'                       --  echo  'foo'
  run1  '$foo'           'foo=bar'  --  echo  '$foo'
  run1  'bar'            'foo=bar'  --  /bin/sh  -c 'echo $foo'
  run1  'err  7'                    --  /bin/sh  -c 'exit 7'

  #  test  outside /etc/passwd

  #  we assume HOME, LOGNAME, and USER will match /etc/passwd

  run1  "$HOME"          --  /bin/sh  -c 'echo $HOME'
  run1  "$LOGNAME"       --  /bin/sh  -c 'echo $LOGNAME'
  run1  "$USER"          --  /bin/sh  -c 'echo $USER'
  run1  '/bin/ash'       --  /bin/sh  -c 'echo $SHELL'
  run1  '/'              --  pwd

  #  test  inherited environment variables

  run2  '/home/user1'    --  /bin/sh  -c 'echo $HOME'
  run2  'user1'          --  /bin/sh  -c 'echo $LOGNAME'
  run2  'user1'          --  /bin/sh  -c 'echo $USER'
  run2  '/bin/ash'       --  /bin/sh  -c 'echo $SHELL'
  run2  '/'              --  pwd

  #  test  $unit/nr/etc/passwd-user2

  user_activate  user2

  run2  '/home/user2'    --  /bin/sh  -c 'echo $HOME'
  run2  'user2'          --  /bin/sh  -c 'echo $LOGNAME'
  run2  'user2'          --  /bin/sh  -c 'echo $USER'
  run2  '/bin/ash'       --  /bin/sh  -c 'echo $SHELL'
  run2  '/'              --  pwd

  #  test  explicit environment variables

  user_activate  orig

  run3  '/home/user3'    -- /bin/sh -c 'echo $HOME'
  run3  'user3'          -- /bin/sh -c 'echo $LOGNAME'
  run3  'user3'          -- /bin/sh -c 'echo $USER'
  run3  'shell3'         -- /bin/sh -c 'echo $SHELL'
  run3  '/'              --  pwd

  #  test  various newroots
  lxr1=()
  run1  ''    /bin/sh -c './lxr 2>/dev/null'
  run1  'err  1  lxroot  directory not found  bad'  ./lxr  bad  --  true

  return  ;  }


test_home  ()  {    #  --------------------------------------------  test_home

  echo  ;  echo  "-  test_home"

  user_activate  none    #  nr/etc/passwd miss
  home_make
  phase_one

  run1  "$HOME"        -- /bin/sh -c 'echo $HOME'       #  /etc/passwd
  run2  "/home/user1"  -- /bin/sh -c 'echo $HOME'       #  host env
  run3  '/home/user3'  -- /bin/sh -c 'echo $HOME'       #  explicit n=v
  run1  "$HOME"        -- pwd                           #  /etc/passwd
  run2  '/home/user1'  -- pwd                           #  host env
  run3  '/home/user3'  -- pwd                           #  explicit n=v

  user_activate  user2    #  nr/etc/passwd hit

  run1  '/home/user2'  -- /bin/sh -c 'echo $HOME'       #  nr/etc/passwd
  run2  '/home/user2'  -- /bin/sh -c 'echo $HOME'       #  nr/etc/passwd
  run3  '/home/user3'  -- /bin/sh -c 'echo $HOME'       #  explicit n=v
  run1  "/home/user2"  -- pwd                           #  nr/etc/passwd
  run2  '/home/user2'  -- pwd                           #  nr/etc/passwd
  run3  '/home/user3'  -- pwd                           #  explicit n=v

  #  as root,  nr/etc/passwd hit

  user_activate  orig

  run1  '/root'        -r -- /bin/sh -c 'echo $HOME'    #  $nr/etc/passwd
  run2  '/root'        -r -- /bin/sh -c 'echo $HOME'    #  $nr/etc/passwd
  run3  '/home/user3'  -r -- /bin/sh -c 'echo $HOME'    #  explicit n=v
  run1  '/root'        -r -- pwd                        #  $nr/etc/passwd
  run2  '/root'        -r -- pwd                        #  $nr/etc/passwd
  run3  '/home/user3'  -r -- pwd                        #  explicit n=v

  return  ;  }


test_readauto  ()  {    #  ------------------------------------  test_readauto

  echo  ;  echo  "-  test_readauto"

  user_activate  user2
  home_make
  env1=(  env  -  )  lxr1=()  cmd1=()

  #  as non-root

  run1  ''           ./lxr  nr         --  touch  /home/user2/foo
  run1  'err  1-'    ./lxr  nr         --  touch  /root/foo
  run1  'err  1-'    ./lxr  nr         --  touch  /foo

  run1  ''           ./lxr  ra nr      --  touch  /home/user2/foo
  run1  'err  1-'    ./lxr  ra nr      --  touch  /root/foo
  run1  'err  1-'    ./lxr  ra nr      --  touch  /foo

  run1  'err  1-'    ./lxr  ro nr      --  touch  /home/user2/foo
  run1  'err  1-'    ./lxr  ro nr      --  touch  /root/foo
  run1  'err  1-'    ./lxr  ro nr      --  touch  /foo

  run1  ''           ./lxr  rw nr      --  touch  /home/user2/foo
  run1  ''           ./lxr  rw nr      --  touch  /root/foo
  run1  ''           ./lxr  rw nr      --  touch  /foo

  #  as root

  run1  ''           ./lxr  -r  nr     --  touch  /root/foo
  run1  ''           ./lxr  -r  nr     --  touch  /foo

  run1  ''           ./lxr  -r  ra nr  --  touch  /root/foo
  run1  ''           ./lxr  -r  ra nr  --  touch  /foo

  run1  'err  1-'    ./lxr  -r  ro nr  --  touch  /root/foo
  run1  'err  1-'    ./lxr  -r  ro nr  --  touch  /foo

  run1  ''           ./lxr  -r  rw nr  --  touch  /root/foo
  run1  ''           ./lxr  -r  rw nr  --  touch  /foo

  return  ;  }


prepare_full_overlay  ()  {    #  ----------------------  prepare_full_overlay

  mkdir  -p  nr/ape   nr/bug  nr/cow
  mkdir  -p  f1/ape   f1/bug  f1/cow
  mkdir  -p  f2/ape   f2/bug  f2/cow  f2/dog
  mkdir  -p           f3/bug
  mkdir  -p  f4/home  f4/tmp

  echo  'one_ape'  >  f1/ape/ape
  echo  'one_bug'  >  f1/bug/bug
  echo  'one_cow'  >  f1/cow/cow

  echo  'two_ape'  >  f2/ape/ape
  echo  'two_bug'  >  f2/bug/bug
  echo  'two_cow'  >  f2/cow/cow

  echo  'three_bug'  >  f3/bug/bug

  return  ;  }


test_full_overlay  ()  {    #  ----------------------------  test_full_overlay

  echo  ;  echo  "-  test_full_overlay"

  prepare_full_overlay
  env1=(  env  -  )  lxr1=()  cmd1=()

  lxr1=(  ./lxr  nr  f1  --  cat  )    #  a full overlay
  run1  'one_ape'      /ape/ape
  run1  'one_bug'      /bug/bug
  run1  'one_cow'      /cow/cow

  lxr1=(  ./lxr  nr  f2  --  cat  )    #  a larger full overlay
  run1  'two_ape'      /ape/ape
  run1  'two_bug'      /bug/bug
  run1  'two_cow'      /cow/cow
  run1  'err  1-'      /dog/dog

  lxr1=(  ./lxr  nr  f3  --  cat  )    #  a smaller full overlay
  run1  'err  1-'      /ape/ape
  run1  'three_bug'    /bug/bug
  run1  'err  1-'      /cow/cow

  lxr1=(  ./lxr  nr  f1  f3  --  cat  )    #  two overlapping full overlays
  run1  'one_ape'      /ape/ape
  run1  'three_bug'    /bug/bug
  run1  'one_cow'      /cow/cow

  lxr1=()    #  readauto on a full overlay

  run1  'err  1-'    ./lxr      nr     f1  --  touch  /ape/ape
  run1  'err  1-'    ./lxr      nr  ra f1  --  touch  /ape/ape
  run1  'err  1-'    ./lxr      nr  ro f1  --  touch  /ape/ape
  run1  ''           ./lxr      nr  rw f1  --  touch  /ape/ape

  run1  ''           ./lxr  -r  nr     f1  --  touch  /ape/ape
  run1  ''           ./lxr  -r  nr  ra f1  --  touch  /ape/ape
  run1  'err  1-'    ./lxr  -r  nr  ro f1  --  touch  /ape/ape
  run1  ''           ./lxr  -r  nr  rw f1  --  touch  /ape/ape

  run1  ''           ./lxr  -w  nr     f1  --  touch  /ape/ape
  run1  ''           ./lxr  -w  nr  ra f1  --  touch  /ape/ape
  run1  'err  1-'    ./lxr  -w  nr  ro f1  --  touch  /ape/ape
  run1  ''           ./lxr  -w  nr  rw f1  --  touch  /ape/ape

  lxr1=()    #  readauto on /tmp in a full overlay

  run1  ''           ./lxr      nr     f4  --  touch  /tmp/foo
  run1  ''           ./lxr      nr  ra f4  --  touch  /tmp/foo
  run1  'err  1-'    ./lxr      nr  ro f4  --  touch  /tmp/foo
  run1  ''           ./lxr      nr  rw f4  --  touch  /tmp/foo

  run1  ''           ./lxr  -r  nr     f4  --  touch  /tmp/foo
  run1  ''           ./lxr  -r  nr  ra f4  --  touch  /tmp/foo
  run1  'err  1-'    ./lxr  -r  nr  ro f4  --  touch  /tmp/foo
  run1  ''           ./lxr  -r  nr  rw f4  --  touch  /tmp/foo

  run1  ''           ./lxr  -w  nr     f4  --  touch  /tmp/foo
  run1  ''           ./lxr  -w  nr  ra f4  --  touch  /tmp/foo
  run1  'err  1-'    ./lxr  -w  nr  ro f4  --  touch  /tmp/foo
  run1  ''           ./lxr  -w  nr  rw f4  --  touch  /tmp/foo

  #  20210604  todo  what additional tests should I add?

  return  ;  }


test_partial_overlay  ()  {    #  ----------------------  test_partial_overlay

  echo  ;  echo  "-  test_partial_overlay"

  prepare_full_overlay
  env1=(  env  -  )  lxr1=()  cmd1=()

  lxr1=(  ./lxr  nr  src f1  ape  bug  cow  --  cat  )
  run1  'one_ape'    /ape/ape
  run1  'one_bug'    /bug/bug
  run1  'one_cow'    /cow/cow

  lxr1=(  ./lxr  nr  src f2  bug  --  cat  )
  run1  'err  1-'    /ape/ape
  run1  'two_bug'    /bug/bug
  run1  'err  1-'    /cow/cow

  lxr1=(  ./lxr  nr  src f1 ape cow  src f2 bug  --  cat  )
  run1  'one_ape'    /ape/ape
  run1  'two_bug'    /bug/bug
  run1  'one_cow'    /cow/cow

  #  pwh  todo  test readauto when dst is a descendant of $HOME

  #  pwh  todo  test mode inheritance


  return  ;  }


test_bind  ()  {    #  --------------------------------------------  test_bind

  echo  ;  echo  "-  test_bind"

  prepare_full_overlay

  env1=(  env  -  )
  lxr1=(  ./lxr  nr  bind  ape  f1/ape  )    #  a single bind
  cmd1=(  )

  run1  'one_ape'    --  cat  /ape/ape
  run1  'err  1-'    --  cat  /bug/ape

  lxr1=(  ./lxr  nr  bind  bug  f1/ape  )    #  a single bind

  run1  'err  1-'    --  cat  /ape/ape
  run1  'one_ape'    --  cat  /bug/ape

  lxr1=()  cmd1=(  touch  /home/user2/ape/foo  )    #  bind inside $HOME

  run1  ''           ./lxr  nr  bind      /home/user2/ape  f1/ape  --
  run1  ''           ./lxr  nr  bind      home/user2/ape   f1/ape  --
  run1  ''           ./lxr  nr  bind  ra  home/user2/ape   f1/ape  --
  run1  'err  1-'    ./lxr  nr  bind  ro  home/user2/ape   f1/ape  --
  run1  ''           ./lxr  nr  bind  rw  home/user2/ape   f1/ape  --

  lxr1=()  cmd1=()    #  attempting to bind to / is an error

  #  20210605  the below test is correct, but not yet implemneted in lxroot
  #  run1  'err  1'  ./lxr  nr  bind  /  nr  --  /bin/sh -c 'exit 2'

  return  ;  }


test_chdir  ()  {    #  ------------------------------------------  test_chdir
  echo  ;  echo  "-    mode $mode    test_chdir"
  #  todo  test preservation of cwd
  echo  'TODO  implement chdir tests'
  return  ;  }


test_x11  ()  {    #  ----------------------------------------------  test_x11

  env1=(  env  -  )  lxr1=()  cmd1=(  /bin/sh -c  'echo $DISPLAY'  )

  run1  ''                    ./lxr         nr  --
  run1  ''                    ./lxr  -x     nr  --
  run1  ''                    ./lxr  --x11  nr  --

  run1  ''       DISPLAY=foo  ./lxr         nr  --
  run1  'foo'    DISPLAY=foo  ./lxr  -x     nr  --
  run1  'foo'    DISPLAY=foo  ./lxr  --x11  nr  --

  return  ;  }


test_options  ()  {    #  --------------------------------------  test_options

  echo  ;  echo  "-  test_options"

  user_activate  user2
  home_make
  env1=()  lxr1=()  cmd1=()

  run1  'err  1  lxroot  error  bad option  --foo'  env - ./lxr --foo -- true

  if  [ -d '/tmp/.X11-unix' ]  ;  then  test_x11  ;  fi

  return  ;  }


main  ()  {    #  ------------------------------------------------------  main

  unit="$1"

  echo

  if  [ ! -x "$unit"/lxr ]  ;  then
    die  "unit.sh  error  executable not found  $unit/lxr"  ;  fi

  prepare
  prepare_users

  cd  "$unit"

  test_no_home
  test_home
  test_readauto
  test_options
  test_full_overlay
  test_partial_overlay
  test_bind
  #  test_chdir

  #  echo  ;  echo  'unit.sh  additional tests disabled'  ;  exit  1

  echo  ;  echo  'unit.sh  done  all tests passed'  ;  exit  ;  }


main  "$@"
