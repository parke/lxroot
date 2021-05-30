# lxroot

###  About

lxroot is a small CLI tool that allows a non-root user to quickly and easily create a "chroot-style" virtual software environment (technically, a Linux user namespace), and then run one or more programs inside that user namespace.

lxroot can be used to:

  -  run a rolling Linux distro's userland on a non-rolling host system
  -  run a legacy userland on a modern Linux system
  -  run software in an "altered version" of the host system itself
  -  create clean, controlled and isolated environments for installing, building, and/or running software packages
  -  deny software access to the network
  -  limit read and/or write access to specific directories
  -  share directories between multiple separate lxroot environments
  -  and possibly for other purposes, as well.

lxroot's name is a combination of "chroot" and "lx".  "lx" meaning Linux, as lxroot uses Linux-specific system calls.  The name was also inspired by the names of the "lxc" and "lxd" container managers.

###  Installation

    $  git  clone  https://github.com/parke/lxroot.git
    $  cd  lxroot
    $  make  build

###  Unit tests

The unit tests install an Alpine Linux chroot environment.  The tests are then run inside this environment.  This consumes approximately 9MB of disk space in `/tmp/lxroot-unit`.

    $  make  unit
    
If the unit tests complete successfully, then the final line of output should be something like:

    unit.sh  done  all tests passed

###  Demo

The demo will create an Alpine Linux chroot environment, and then run an interactive shell inside that envirnoment.  The demo consumes approximately 9MB of disk space in `/tmp/lxroot-demo`.

    $  make  demo
    
When you run the demo, you should see something like this:

    g++  -g   -fmax-errors=2  -Wall  -Werror  -Wextra  -Wno-unused-parameter  lxroot.cpp  -o bin/lxroot
    cp  bin/lxroot  /tmp/lxroot-demo/lxroot
    bash  demo.sh  alpine  /tmp/lxroot-demo
    
    #  ( Welcome to the demo of lxroot!                           )
    #  ( This demo creates an Alpine Linux chroot environment and )
    #  ( then runs an interactive shell inside that environment.  )
    
    +  cd /tmp/lxroot-demo
    
    #  ( Found an Alpine Linux minirootfs tarball. )
    #  ( The tarball checksum is valid. )
    
    +  mkdir newroot
    +  tar xzf dist/alpine-minirootfs-3.13.5-x86_64.tar.gz -C newroot
    +  cp /etc/resolv.conf newroot/etc/
    
    #  ( We will now run the following command to start an interactive  )
    #  ( shell inside the Alpine Linux chroot environment:              )
    #  (                                                                )
    #  (   ./lxroot  -nr  newroot                                       )
    #  (                                                                )
    #  ( The '-n' option allows network access.                         )
    #  ( The '-r' option maps the uid and gid to zero.  In other words, )
    #  (   '-r' simulates being the root user.                          )
    #  ( 'newroot' is the name of the directory that contains the       )
    #  (   Alpine Linux chroot environment.                             )
    #  (                                                                )
    #  ( The prompt inside the demo should be something like:           )
    #  (                                                                )
    #  (   root  -nr  ./newroot  ~                                      )
    #  (                                                                )
    #  ( 'root'      is the name of the (possibly simulated) user       )
    #  ( '-nr'       is a summary of some of the command line options   )
    #  ( './newroot' is the name of the newroot directory               )
    #  ( '~'         is the name of the current working directory       )
    
    +  exec ./lxroot -nr newroot
    
    root  -nr  ./newroot  ~  


###  Recursion!

To build lxroot inside the demo environment, run the following commands.  These commands will use approximately 221MB of disk space in `/tmp/lxroot-demo`.

    ####  To enter the demo environment, run:
    $  make  demo
    ####  Then, inside the demo environment, run:
    $  apk  update
    $  apk  add  build-base
    $  git  clone  https://github.com/parke/lxroot.git
    $  cd  lxroot
    $  make  build
    ####  Then, to run the unit tests inside the demo environment, run:
    $  apk  add  bash  coreutils
    $  make  unit
    ####  Then, to nest a second demo inside of the first demo, run:
    $  make  demo

###  Documentation

lxroot can be thought of as an alternative to the standard Unix program `chroot`.  (lxroot also has similaries to the Linux program `unshare`.)

However, lxroot differs significantly from chroot.

-  Any user can run lxroot, whereas only the administrative user (root) can run chroot.
-  lxroot can bind-mount various combinations of directories to "cut and paste" together custom software environments.
-  Directories can be bind-mounted in read-only mode to prevent undesired modifications to files.
-  Unless network access is granted with the `-n` option, programs will be denied access to the network.

If you are unfamiliar with chroot, the following pages may provide
useful background information.

-  https://www.linuxfordevices.com/tutorials/linux/chroot-command-in-linux
-  https://wiki.archlinux.org/title/Chroot
-  https://man.archlinux.org/man/arch-chroot.8
-  https://man.archlinux.org/man/unshare.1
-  https://www.journaldev.com/38044/chroot-command-in-linux
-  https://help.ubuntu.com/community/BasicChroot
-  https://www.howtogeek.com/441534/how-to-use-the-chroot-command-on-linux/

Documentation on lxroot itself is available by running `lxroot --help-more`.
You may also wish to look at the `demo.sh` file.

Below is the output of `lxroot --help-more`:

    usage:  lxroot  [mode] newroot  [options]  [--]  [command [arg ...] ]
    
    options
      -short                      one or more short options
      --long-option               a long option
      n=v                         set an environment variable
      [mode]  newroot             set and bind the newroot
      [mode]  path                bind a full or partial overlay
      'src'   [mode]  path        set the source for partial overlays
      'bind'  [mode]  dst  src    bind src to newroot/dst
      --                          end of options, command follows
      command  [arg ...]          command
    
    MODES
    
      ra    read-auto  (default, described below)
      ro    read-only  (bind mount with MS_RDONLY)
      rw    read-write (bind mount without MS_RDONLY)
    
    SHORT OPTIONS
    
      n    allow network access (CLONE_NEWNET = 0)
      r    simulate root user (map uid and gid to zero)
      w    allow full write access to all read-auto binds
      x    allow X11 access (bind /tmp/.X11-unix and set DISPLAY)
    
    LONG OPTIONS
    
      --help          display help
      --help-more     display more help
      --network       allow network access (CLONE_NEWNET = 0)
      --pulseaudio    allow pulseaudio access (bind $XDG_RUNTIME_DIR/pulse)
      --root          simulate root user (map uid and gid to zero)
      --trace         log major syscalls to stderr
      --version       print version info and exit
      --write         allow full write access to all read-auto binds
      --x11           allow X11 access (bind /tmp/.X11-unix)
    
    READ-AUTO MODE
    
    The purpose of read-auto mode is to (a) grant a simulated-root-user
    write access to the path, while (b) granting a non-simulated-root user
    write access only to $HOME and /tmp.  Or, stated precisely:
    
    If any of -r, -w, --root or --write are specified, then:
    Each read-auto path will be bind mounted in read-write mode.
    
    Otherwise:
    A read-auto path inside  $HOME or /tmp will be bind mounted read-write.
    A read-auto path outised $HOME or /tmp will be bind mounted read-only.
    
    Furhermore:
    If $HOME and/or /tmp is a descendant of a read-auto path, then $HOME
    and/or /tmp (respectively) will be bind mounted in read-write mode.
    In this case, two or three bind mounts occur.  First, the path will be
    bind mounted read-only.  And then $HOME and/or /tmp will be bind
    mounted read-write.
    
    NEWROOT
    
    Note that the newroot, full-overlay, and partial-overlay options all
    have the same form, namely:  [mode]  path
    
    The first option of this form is the newroot option.  The newroot
    option specfies the newroot.
    
    FULL OVERLAY
    
    Zero or more full-overlay options may occur anywhere before the first
    set-source option.
    
    A full-overlay option has the form:  [mode]  path
    
    A full-overlay option will bind each (source) subdirectory inside path
    to an identically named destination subdirectory inside newroot.  If
    the destination subdirectory does not exsit, then the source
    subdirectory will be silently skipped.
    
    For example, a full-overlay option could be used to bind custom /home
    and /tmp directories into a reusable newroot directory.
    
    PARTIAL OVERLAY
    
    Zero or more partial-overlay options may occur anywhere after the
    first set-source option.
    
    A partial-overlay option has the form:  [mode]  path
    
    A partial-overlay option will bind overlay/path to newroot/path, where
    overlay is the overlay source path set by the preceding set-source
    option.
    
    SET SOURCE
    
    A set-source option has the form:  'src'  [mode]  path
    
    'src' is the literal string 'src'.
    
    A set-source option sets the overlay source path and the default
    overlay mode.  These values will be used by any following
    partial-overlay options.
    
    Zero or more set-source options may be specified.
    
    BIND
    
    A bind-option has the form:  'bind'  [mode]  dst  src
    
    'bind' is the literal string 'bind'.
    
    A bind-option will bind src to newroot/dst, using the optionally
    specified mode.
    
    COMMAND
    
    The command option specifies the command that will be executed inside
    the lxroot environment.
    
    If no command is specified, lxroot will attempt to find and execute an
    interactive shell inside the lxroot environment.
    
    Note the following lexical ambiguity: a path-like argument may specify
    either (a) an overlay option or (b) the command option.
    
    lxroot resolves this ambiguity by looking for a directory at the path.
    If a directory exists, lxroot interprets the path as an overlay option.
    If no such directory exists, lxroot interprets the path as a command.
    (lxroot does not verify that the command actually exists inside
    newroot.)
    
    To force a path to be interpreted as a command, proceed the path with
    the option '--'.
    