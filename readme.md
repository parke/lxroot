# Lxroot - Lightweight software virtualization

###  About

Lxroot is a lightweight alternative to chroot, Docker, and other software virtualization tools.

Lxroot allows a non-root user to quickly and easily create a "chroot-style" virtual software environment (a Linux user namespace), and then run one or more programs (a "guest userland") inside that user namespace.

For example, with Lxroot a non-root user can...

-  simultaneously run multiple, different guest userlands on a single Linux host
-  run an Arch Linux userland on an Ubuntu Linux host
-  run a legacy userland on a modern host
-  run software in an "altered version" of the host system itself
-  run any given graphical X11 client (from any guest userland) on the host's X11 server
-  create clean, controlled, and isolated guest environments for installing, building, and/or running software packages
-  deny software access to the network
-  restrict read and/or write access to specific directories
-  share one or more directories between an Lxroot environment and the host system
-  share one or more directories between multiple Lxroot environments.

All without root access!

The name "Lxroot" is a combination of "Lx" (as in "Linux") and "chroot".

###  Guest userland compatibility

Below are notes on using Lxroot with userlands from the following Linux distributions:

-  **Alpine Linux**  -  The Alpine Linux package manager and userland seem to run well inside Lxroot.  (See Demo #1, below.)
-  **Arch Linux**  -  The Arch Linux package manager and userland seem to run well inside Lxroot.  (See Demo #3, below.)
-  **Debian** and **Ubuntu**  -  Debian's package managers (`apt-get` and `dpkg`) require extensive tweaking and shimming before they will run successfully inside Lxroot.  Consequently, at present I recommend against attempting to create a Debian or Ubuntu guest userland.  If you really want to create a Debian or Ubuntu guest userland, I recommend creating the userland on a native Debian or Ubuntu host.  This will probably require root access. Consider using `debootstrap`.  After you have created the userland, you should be able to use `lxroot` to enter the userland.
-  **Void Linux**  -  It has been a while, but I previously ran Void's package manager and userland inside Lxroot.  Void seemed to work well with Lxroot.  Void Linux provides a rootfs tarball.

Lxroot creates virtual environments via Linux user namespaces.  Linux user namespaces, by their very nature, have inherent and unavoidable limitations.  In some cases, these limitations may create compatibility issues, depending on exactly what software you try to run inside Lxroot.

It is also possible that your Linux distribution provides a Linux kernel that was compiled with user namespaces disabled.  In this case, Lxroot will fail to run on your kernel.  I suspect most Linux kernels today are compiled with user namespaces enabled, but I have not attempted to verify this.

###  Installation

    $  git  clone  https://github.com/parke/lxroot.git
    $  cd  lxroot
    $  make  build

###  Unit tests

The unit tests install an Alpine Linux guest userland.  The tests are then run inside this guest userland.  This consumes approximately 9MB of disk space: 3MB in `/tmp/lxroot-demo`, plus 6MB in `/tmp/lxroot-unit`.  Run the unit tests as follows:

    $  make  unit
    
If the final line of output is something like the below, then the unit tests have completed successfully:

    unit.sh  done  all tests passed

###  Demo #1 - Alpine Linux

Demo #1 will create an Alpine Linux guest userland, and then run an interactive shell inside the guest userland.  Demo #1 consumes approximately 9MB of disk space in `/tmp/lxroot-demo`.  Creating the Alpine Linux userland is very simple.  All Demo #1 needs to do is download and untar an official Alpine Linux `minirootfs` tarball, and then copy in an appropriate `/etc/resolv.conf` file.

Run Demo #1 as follows:

    $  make  demo
    
When you run the demo, you should see something like this:

    mkdir  -p  /tmp/lxroot-demo/bin
    g++  -g   -fmax-errors=2  -Wall  -Werror  -Wextra  -Wno-unused-parameter  lxroot.cpp  -o /tmp/lxroot-demo/bin/lxroot
    cp  bin/lxroot  /tmp/lxroot-demo/lxroot
    bash  demo.sh  demo1  /tmp/lxroot-demo

    #  ( Welcome to the demo of lxroot!                       )
    #  ( This demo creates an Alpine Linux guest userland and )
    #  ( then runs an interactive shell inside it.            )

    +  cd /tmp/lxroot-demo

    #  ( Alpine Linux minirootfs - file found             )
    #  ( Alpine Linux minirootfs - file checksum is valid )

    +  mkdir demo1
    +  tar xzf dist/alpine-minirootfs-3.13.5-x86_64.tar.gz -C demo1
    +  cp /etc/resolv.conf demo1/etc/

    #  ( We will now run the following command to start an interactive  )
    #  ( shell inside the Alpine Linux guest userland:                  )
    #  (                                                                )
    #  (   ./lxroot  -nr  demo1                                         )
    #  (                                                                )
    #  ( The '-n' option allows network access.                         )
    #  ( The '-r' option maps the uid and gid to zero.  In other words, )
    #  (   '-r' simulates being the root user.                          )
    #  ( 'demo1' is the name of the directory that contains the         )
    #  (   Alpine Linux guest userland.                                 )
    #  (                                                                )
    #  ( The prompt inside the demo should be something like:           )
    #  (                                                                )
    #  (   root  -nr  ./demo1  ~                                        )
    #  (                                                                )
    #  ( 'root'    is the name of the (possibly simulated) user         )
    #  ( '-nr'     is a summary of some of the command line options     )
    #  ( './demo1' is the name of the newroot directory                 )
    #  ( '~'       is the name of the current working directory         )

    +  exec ./lxroot -nr demo1

    root  -nr  ./demo1  ~  

###  Demo #2 - Nesting a second Lxroot environment inside Demo #1

Lxroot makes it easy to create a child Lxroot nested inside a parent Lxroot.

The below commands will:

- enter the guest userland created by Demo #1
- install the GCC compiler inside the Demo #1 userland
- use GCC to compile Lxroot inside the Demo #1 userland
- create a nested child Alpine userland inside the Demo #1 userland
- lxroot into the nested child Alpine userland.

These commands will use approximately 221MB of disk space in `/tmp/lxroot-demo`.

To run Demo #2, please run the following commands:

    ####  To enter the demo #1 userland, run:
    $  make  demo
    ####  Then, inside the demo #1 userland, run:
    $  apk  update
    $  apk  add  build-base  git
    $  git  clone  https://github.com/parke/lxroot.git
    $  cd  lxroot
    $  make  build
    ####  Then, to run the unit tests inside the demo #1 userland, run:
    $  apk  add  bash  coreutils
    $  make  unit
    ####  Then, to nest a second userland inside of the demo #1 userland, run:
    $  make  demo

###  Demo #3 - Arch Linux plus the Chromium web browser

Demo #3 will create an Arch Linux userland that contains the Chromium web browser.  Demo #3 will then run an interactive shell in this userland.  Run `chromium` in this shell to run Chromium.  Demo #3 was developed and tested on an Ubuntu 20.04 host.  Demo #3 may or may not run on other hosts.

Chromium's access to the filesystem will be limited to the Lxroot environment.  Furthermore, only `$HOME` and `/tmp` will be writable; all other directories will be bind mounted in read-only mode.

Demo #3 will need to create an Arch Linux userland from scratch, as Arch Linux does not provide a `minirootfs` tarball.  Therefore, Demo #3 will do the following:

-  download the Arch Linux installion CD `.iso` file
-  create userland #1 (Alpine Linux, not Arch)
-  `lxroot` into userland #1 and then:
   -  install file decompression utitilies
   -  extract the `airootfs.sfs` file from the Arch `.iso` file
   -  extract userland #2 from the `airootfs.sfs` file
-  `lxroot` into userland #2 and then:
   -  run `pacman` to create userland #3
-  `lxroot` into userland #3 and then:
   -  run `pacman` to install Chromium
-  `lxroot` into userland #3 as non-root and then:
   -  run an interactive shell 
   -  you should be able to run `chromium` in this shell

To run Demo #3, please run:

    make  demo3

After downloading the `.iso` file (which may take several minutes), Demo #3 takes about two minutes to build the three userlands on my computer.  I have a low TDP CPU, but all files are read and written to a fast ramdisk.    Userland #3, which contains everything Chromium needs to run, is 1.7GB.  (This 1.7GB could probably be pruned down significantly, if desired.)  Demo #3's total disk usage is approximately 5GB of disk space.  By default, all files will be written inside `/tmp/lxroot-demo`.  I think you can change this location by editing one line in `Makefile`.

At present, sound is disabled in Demo #3.  If you wish to attempt to enable sound (via PulseAudio), then edit `Makefile` and add `--pulseaudio` to the final line of the `demo3` recipe, as shown below.  You probably also need to create manually create the `$XDG_RUNTIME_DIR/pulse` directory inside `userland3`.

    $(bin)/lxroot  -nx  --pulseaudio  $(demo)/demo3/userland2/userland3

###  Documentation

Lxroot can be thought of as an alternative to the standard Unix program `chroot`.  (Lxroot also has similaries to the Linux program `unshare`.)

However, Lxroot differs significantly from `chroot`.

-  Any user can run `lxroot`, whereas only the administrative user (root) can run `chroot`.
-  Lxroot can bind-mount various combinations of directories to "cut and paste" together custom userlands.
-  Directories can be bind-mounted in read-only mode to prevent undesired modifications to files.
-  Unless network access is granted with the `-n` option, programs will be denied access to the network.

If you are unfamiliar with `chroot`, the following pages may provide useful background information.

-  https://www.linuxfordevices.com/tutorials/linux/chroot-command-in-linux
-  https://wiki.archlinux.org/title/Chroot
-  https://man.archlinux.org/man/arch-chroot.8
-  https://man.archlinux.org/man/unshare.1
-  https://www.journaldev.com/38044/chroot-command-in-linux
-  https://help.ubuntu.com/community/BasicChroot
-  https://www.howtogeek.com/441534/how-to-use-the-chroot-command-on-linux/

Documentation on Lxroot itself is available by running `lxroot --help-more`.
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
      x    allow X11 access (bind /tmp/.X11-unix and set $DISPLAY)
    
    LONG OPTIONS
    
      --help          display help
      --help-more     display more help
      --network       allow network access (CLONE_NEWNET = 0)
      --pulseaudio    allow pulseaudio access (bind $XDG_RUNTIME_DIR/pulse)
      --root          simulate root user (map uid and gid to zero)
      --trace         log diagnostic info to stderr
      --version       print version info and exit
      --write         allow full write access to all read-auto binds
      --x11           allow X11 access (bind /tmp/.X11-unix and set $DISPLAY)
    
    READ-AUTO MODE
    
    The purpose of read-auto mode is to (a) grant a simulated-root user
    write access to the path, while (b) granting a non-simulated-root user
    write access only to $HOME and /tmp.  Or, stated precisely:
    
    If any of -r, -w, --root or --write are specified, then:
    Each read-auto path will be bind mounted in read-write mode.
    
    Otherwise:
    A read-auto path inside  $HOME or /tmp will be bind mounted read-write.
    A read-auto path outised $HOME or /tmp will be bind mounted read-only.
    
    Furhermore:
    If $HOME and/or /tmp is a descendant of a read-auto bind, then $HOME
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
    
    A full-overlay option will attempt to bind all the subdirectories
    inside path to identically named subdirectories inside newroot.
    
    For example, if my_overlay contains the subdirectories 'home', 'run',
    and 'tmp', then the full-overlay option 'rw my_overlay' will attempt
    to bind the following:
    
      my_overlay/home  to  newroot/home  in read-write mode
      my_overlay/run   to  newroot/run   in read-write mode
      my_overlay/tmp   to  newroot/tmp   in read-write mode
    
    If any newroot/subdir does not exist, then that my_overlay/subdir will
    be silently skipped.
    
    SET SOURCE
    
    A set-source option has the form:  'src'  [mode]  path
    
    'src' is the literal string 'src'.
    
    A set-source option sets the overlay-source-path and the default
    overlay-mode.  These values will be used by any following
    partial-overlay options.
    
    Zero or more set-source options may be specified.
    
    PARTIAL OVERLAY
    
    Zero or more partial-overlay options may occur anywhere after the
    first set-source option.
    
    A partial-overlay option has the form:  [mode]  path
    
    A partial-overlay option will bind overlay/path to newroot/path, where
    overlay is the overlay-source-path set by the preceding set-source
    option.
    
    For example, the two options 'src my_overlay home/my_username' will do
    the following:
    
      1)  first, the overlay-source-path will be set to 'my_overlay'
      2)  then, the following bind will occur:
    
            my_overlay/home/my_username  to  newroot/home/my_username
    
    If either directory does not exist, lxroot will exit with status 1.
    
    Successive partial-overlay options may be used to bind a selected
    subset of the descendants of an overlay into newroot.  (Whereas a
    single full-overlay option attempts to bind all of the full-overlay's
    immediate subdirectories into newroot.)
    
    BIND
    
    A bind-option has the form:  'bind'  [mode]  dst  src
    
    'bind' is the literal string 'bind'.
    
    A bind-option will bind src to newroot/dst, using the optionally
    specified mode.
    
    Note that dst precedes src.  This hopefully improves readibilty in
    scripts where: (a) many binds may be specified, (b) dst is tyically
    shorter than src, and (c) src may vary greatly in length from bind to
    bind.
    
    COMMAND
    
    The command-option specifies the command that will be executed inside
    the lxroot environment.
    
    If no command is specified, lxroot will attempt to find and execute an
    interactive shell inside the lxroot environment.
    
    Note the following lexical ambiguity: a path-like argument may specify
    either (a) an overlay option or (b) the command option.
    
    lxroot resolves this ambiguity by looking for a directory at the path.
    If a directory exists, lxroot interprets the path as an overlay option.
    If no such directory exists, lxroot interprets the path as a command.
    
    Note that lxroot does not verify that the command actually exists
    inside newroot.  If the command does not exist, then the call to
    exec() will will fail and lxroot will exit with status 1.
    
    To force a path to be interpreted as a command, proceed the path with
    the option '--'.
    
###  Other software virtualization tools

For reference, here is a partial list of some other software virtualization tools, in approximate(?) order from lightest to heaviest:
-  `chroot`
-  `unshare`
-  ( Lxroot )
-  Firejail
-  Bubblewrap
-  podman.io
-  Docker
