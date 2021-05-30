#! /bin/bash


#  unit.sh  version  20210530


set  -o errexit
set  +o xtrace


run  ()  {    #  --------------------------------------------------------  run

  #  usage:  run  expect  env_name  cmd_name  arg...

  local  expect="$1"  env_name="$2"  lxr_name="$3"
  local  -n  env_ref="$env_name"
  local  -n  lxr_ref="$lxr_name"
  local  argv=(  "${@:4}"  )
  local  command=(  "${env_ref[@]}"  "${lxr_ref[@]}"  "${argv[@]}"  )
  local  stdout  status
  set  +o  errexit
  stdout=`  "${command[@]}"  `
  status=$?
  set  -o  errexit

  if  [ "$status" = 0 ]
    then  local  actual="$stdout"       pretty=' '
    else  local  actual="err  $status"  pretty="$status"  ;  fi

  echo  "$env_name  ${BASH_LINENO[1]}  $pretty  ${argv[*]}"

  if  [ "$actual" = "$expect" ]  ;  then  return  ;  fi

  echo
  echo  '--------'
  echo
  echo  'unit2.sh  test fail'
  echo
  echo  "  line     ${BASH_LINENO[1]}"
  echo
  echo  "  expect   $expect"
  echo  "  actual   $actual"

  if  [ ! "$status" = 0 ];  then
    echo
    echo  "  stdout  $stdout"  ;  fi

  echo
  echo  "  cwd      $PWD"
  echo  "  env      ${env_ref[*]}"
  echo  "  lxr      ${lxr_ref[*]}"
  echo  "  argv     ${argv[*]}"
  echo  "  command  ${command[*]}"
  echo
  exit  1  ;  }


die  ()  {  echo  "die  $*"  ;  exit  1  ;  }    #  ---------------------  die

popd   ()  {  builtin  popd   "$@"  >  /dev/null  ;  }    #  -----------  popd
pushd  ()  {  builtin  pushd  "$@"  >  /dev/null  ;  }    #  ----------  pushd

rm_soft     ()  { if  [ -e "$1" ];  then  rm     "$1"  ;  fi  ;  }    #  -----
rmdir_soft  ()  { if  [ -d "$1" ];  then  rmdir  "$1"  ;  fi  ;  }    #  -----

run1  ()  {  run  "$1"  env1  lxr1  "${@:2}"  ;  }    #  ---------------  run1
run2  ()  {  run  "$1"  env2  lxr2  "${@:2}"  ;  }    #  ---------------  run2
run3  ()  {  run  "$1"  env3  lxr3  "${@:2}"  ;  }    #  ---------------  run3


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
  mkdir  -p  "$unit/nr/home/user2"
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

  #  only environment vars
  env2=(  env  -  HOME=/home/user1  LOGNAME=user1  USER=user1  )
  lxr2=(  ./lxr  nr  )

  #  explicit vars should override environment vars
  env3=(  env  -  HOME=/home/user1  LOGNAME=user1  USER=user1  )
  lxr3=(  ./lxr  nr
            HOME=/home/user3  LOGNAME=user3  USER=user3  SHELL=shell3  )

  return;  }


test_no_home  ()  {    #  --------------------------------------  test_no_home

  echo  ;  echo  "-  test_no_home"

  user_activate  none
  home_remove
  phase_one

  #  test  echo and exit

  run1  'foo'            --  echo  'foo'
  run1  'bar'            'foo=bar'  --  /bin/sh  -c 'echo $foo'
  run1  'err  7'         --  /bin/sh -c 'exit 7'

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
  phase_one  ;  lxr1=()

  #  as non-root

  local  hush="2>/dev/null"

  run1  ''          ./lxr  nr     --  sh -c 'touch  /home/user2/foo'
  run1  'err  1'    ./lxr  nr     --  sh -c "touch  /root/foo        $hush"
  run1  'err  1'    ./lxr  nr     --  sh -c "touch  /foo             $hush"

  run1  ''          ./lxr  ra nr  --  sh -c 'touch  /home/user2/foo'
  run1  'err  1'    ./lxr  ra nr  --  sh -c "touch  /root/foo        $hush"
  run1  'err  1'    ./lxr  ra nr  --  sh -c "touch  /foo             $hush"

  run1  'err  1'    ./lxr  ro nr  --  sh -c "touch  /home/user2/foo  $hush"
  run1  'err  1'    ./lxr  ro nr  --  sh -c "touch  /root/foo        $hush"
  run1  'err  1'    ./lxr  ro nr  --  sh -c "touch  /foo             $hush"

  run1  ''          ./lxr  rw nr  --  sh -c 'touch  /home/user2/foo'
  run1  ''          ./lxr  rw nr  --  sh -c 'touch  /root/foo'
  run1  ''          ./lxr  rw nr  --  sh -c 'touch  /foo'

  #  as root

  run1  ''          ./lxr  -r  nr     --  sh -c 'touch  /root/foo'
  run1  ''          ./lxr  -r  nr     --  sh -c 'touch  /foo'

  run1  ''          ./lxr  -r  ra nr  --  sh -c 'touch  /root/foo'
  run1  ''          ./lxr  -r  ra nr  --  sh -c 'touch  /foo'

  run1  'err  1'    ./lxr  -r  ro nr  --  sh -c "touch  /root/foo  $hush"
  run1  'err  1'    ./lxr  -r  ro nr  --  sh -c "touch  /foo       $hush"

  run1  ''          ./lxr  -r  rw nr  --  sh -c 'touch  /root/foo'
  run1  ''          ./lxr  -r  rw nr  --  sh -c 'touch  /foo'

  return  ;  }


prepare_full_overlay  ()  {    #  ----------------------  prepare_full_overlay
  mkdir  -p  nr/ace  nr/bat  nr/car
  mkdir  -p  f1/ace  f1/bat  f1/car
  mkdir  -p  f2/bat
  echo  'one_ace'  >  f1/ace/ace
  echo  'one_bat'  >  f1/bat/bat
  echo  'one_car'  >  f1/car/car
  echo  'two_bat'  >  f2/bat/bat
  return  ;  }


test_full_overlay  ()  {    #  ----------------------------  test_full_overlay

  echo  ;  echo  "-  test_full_overlay"

  prepare_full_overlay
  local  hush='2>/dev/null'
  env1=(  env  -  )
  lxr1=(  ./lxr  nr  f1  --  sh -c  )

  run1  'one_ace'  "cat  /ace/ace  $hush"
  run1  'one_bat'  "cat  /bat/bat  $hush"
  run1  'one_car'  "cat  /car/car  $hush"

  lxr1=(  ./lxr  nr  f2  --  sh -c  )

  run1  'err  1'   "cat  /ace/ace  $hush"
  run1  'two_bat'  "cat  /bat/bat  $hush"
  run1  'err  1'   "cat  /car/car  $hush"

  lxr1=(  ./lxr  nr  f1  f2  --  sh -c  )

  run1  'one_ace'  "cat  /ace/ace  $hush"
  run1  'two_bat'  "cat  /bat/bat  $hush"
  run1  'one_car'  "cat  /car/car  $hush"

  return  ;  }


test_partial_overlay  ()  {    #  ----------------------  test_partial_overlay

  echo  ;  echo  "-  test_partial_overlay"

  prepare_full_overlay
  local  hush='2>/dev/null'
  env1=(  env  -  )
  lxr1=(  ./lxr  nr  src f1  ace  bat  car  --  sh -c  )

  run1  'one_ace'  "cat  /ace/ace  $hush"
  run1  'one_bat'  "cat  /bat/bat  $hush"
  run1  'one_car'  "cat  /car/car  $hush"

  lxr1=(  ./lxr  nr  src f2  bat  --  sh -c  )

  run1  'err  1'   "cat  /ace/ace  $hush"
  run1  'two_bat'  "cat  /bat/bat  $hush"
  run1  'err  1'   "cat  /car/car  $hush"

  lxr1=(  ./lxr  nr  src f1 ace car  src f2 bat  --  sh -c  )

  run1  'one_ace'  "cat  /ace/ace  $hush"
  run1  'two_bat'  "cat  /bat/bat  $hush"
  run1  'one_car'  "cat  /car/car  $hush"

  return  ;  }


test_bind  ()  {    #  --------------------------------------------  test_bind

  prepare_full_overlay
  local  hush='2>/dev/null'
  env1=(  env  -  )
  lxr1=(  ./lxr  nr  bind ace f1/ace  --  sh -c  )

  run1  'one_ace'  "cat  /ace/ace  $hush"
  run1  'err  1'   "cat  /bat/ace  $hush"

  lxr1=(  ./lxr  nr  bind bat f1/ace  --  sh -c  )

  run1  'err  1'   "cat  /ace/ace  $hush"
  run1  'one_ace'  "cat  /bat/ace  $hush"

  return  ;  }


test_chdir  ()  {    #  ------------------------------------------  test_chdir
  echo  ;  echo  "-    mode $mode    test_chdir"
  #  todo  test preservation of cwd
  echo  'TODO  implement chdir tests'
  return  ;  }


test_x11  ()  {    #  ----------------------------------------------  test_x11

  run1  ''     env -             ./lxr       nr -- sh -c 'echo $DISPLAY'
  run1  ''     env -             ./lxr -x    nr -- sh -c 'echo $DISPLAY'
  run1  ''     env -             ./lxr --x11 nr -- sh -c 'echo $DISPLAY'

  run1  ''     env - DISPLAY=foo ./lxr       nr -- sh -c 'echo $DISPLAY'
  run1  'foo'  env - DISPLAY=foo ./lxr -x    nr -- sh -c 'echo $DISPLAY'
  run1  'foo'  env - DISPLAY=foo ./lxr --x11 nr -- sh -c 'echo $DISPLAY'

  return  ;  }


test_options  ()  {    #  --------------------------------------  test_options

  echo  ;  echo  "-  test_options"

  user_activate  user2
  home_make
  phase_one  ;  env=()  lxr1=()

  run1  'err  1'   env - ./lxr --foo -- true

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
