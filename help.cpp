

//  help.cpp  -  Create and use chroot-style virtual software environments.
//  Copyright (c) 2022 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.


//  version  20220822


const char *  help  =    //  xxhe  -------------------------------------  help
"\n"
"usage:  lxroot  [mode] newroot  [options]  [-- command [arg ...]]\n\n"

"options\n"
"  -short                      one or more short options\n"
"  --long-option               a long option\n"
"  name=value                  set an environment variable\n"
"  [mode]  newroot             set and bind the newroot\n"
"  [mode]  path                bind a full or partial overlay\n"
"  'src'   [mode]  path        set the source for partial overlays\n"
"  'bind'  [mode]  dst  src    bind src to newroot/dst\n"
"  'cd'    path                cd to path (inside newroot)\n"
"  'wd'    path                cd to path and make path writable\n"
"  --                          end of options, command follows\n"
"  command  [arg ...]          command\n"
"";    //  end  help  ---------------------------------------------  end  help


const char *  help2  =    //  xxhe  -----------------------------------  help2
"\n"
"  For more help, please run:  lxroot  --help-more\n"
"";    //  end  help2  -------------------------------------------  end  help2


//  short options  --------------------------------------------  short options
//
//  a  -				A  -
//  b  -				B  -
//  c  -				C  -
//  d  ( ? dbus ? )			D  -
//  e  environment			E  -
//  f  -				F  -
//  g  -				G  -
//  h  -				H  -
//  i  -				I  -
//  j  -				J  -
//  k  -				K  -
//  l  -				L  -
//  m  -				M  -
//  n  network				N  -
//  o  ( overlay with lxroot.conf )	O  -
//  p  pulseaudio			P  -
//  q  -				Q  -
//  r  root (i.e. map uid to 0)		R  -
//  s  -				S  -
//  t  -				T  -
//  u  -				U  -
//  v  ( ? verbose ? )			V  -
//  w  full write access		W  -
//  x  x11				X  -
//  y  -				Y  -
//  z  -				Z  -
//
//  Unassigned potential future options:
//    dbus (i.e. pass in dbus access)
//    don't bind /sys
//    don't bind /dev
//    share mount namespace with host
//    share pid   namespace with host
//    unshare hostname (i.e. set a custom hostname)
//    verbose


//  long options  ( for plannig purposes only, possibly inaccurate )
//    dbus  env  help  help-more  network  pid  pulseaudio  root  trace
//    verbose  version  write  x11


const char *  help_more  =  //  xxlo  -----------------------------  help_more

"\nMODES\n\n"

"  ra    read-auto  (default for newroot, described below)\n"
"  ro    read-only  (bind mount with MS_RDONLY)\n"
"  rw    read-write (bind mount without MS_RDONLY)\n\n"

"SHORT OPTIONS\n\n"

"  e     import (almost) all external environment variables\n"
"  n     allow network access (CLONE_NEWNET = 0)\n"
"  r     simulate root user (map uid and gid to zero)\n"
"  w     allow full write access to all read-auto binds\n"
"  x     allow X11 access (bind /tmp/.X11-unix and set $DISPLAY)\n\n"

"LONG OPTIONS\n\n"

"  --env           import (almost) all external environment variables\n"
"  --help          display help\n"
"  --help-more     display more help\n"
"  --network       allow network access (CLONE_NEWNET = 0)\n"
"  --pulseaudio    allow pulseaudio access (bind $XDG_RUNTIME_DIR/pulse)\n"
"  --root          simulate root user (map uid and gid to zero)\n"
"  --trace         log diagnostic info to stderr\n"
"  --version       print version info and exit\n"
"  --write         allow full write access to all read-auto binds\n"
"  --x11           allow X11 access (bind /tmp/.X11-unix and set $DISPLAY)\n\n"

"READ-AUTO MODE\n\n"

"The purpose of read-auto mode is to (a) grant a simulated-root user\n"
"broad or total write access, while (b) granting a non-root user write\n"
"access only to a few select directories, namely: $HOME, /tmp, and\n"
"/var/tmp.\n\n"

"To be precise and complete:\n\n"

"Each bind (including newroot) has a specified mode.  The specified\n"
"mode is one of: 'ra', 'ro', or 'rw'.\n\n"

"If no mode is specified for newroot, then newroot's specified mode\n"
"defaults to 'ra' (read-auto).\n\n"

"If any other bind lacks a specified mode, then that bind simply\n"
"inherits the specified mode of its parent.\n\n"

"Each bind also has an actual mode.  The actual mode is: 'ro' or 'rw'.\n\n"

"A bind's actual mode may be different from its specified mode.  A\n"
"bind's actual mode is determined as follows:\n\n"

"If the specified mode is 'rw', then the actual mode is 'rw'.\n\n"

"If the bind is inside a path specified by a wd-option, then the actual\n"
"mode is 'rw' (even if that bind's specified mode is 'ro').\n\n"

"If the specified mode is 'ra', and furthormore if:\n"
"  a)  the '-r' or '--root' option is specified, or\n"
"  b)  the '-w' or '--write' option is specified, or\n"
"  c)  the bind's destination path is inside $HOME, /tmp, or /var/tmp,\n"
"then the actual mode is 'rw'.\n\n"

"Otherwise the bind's actual mode is 'ro'.\n\n"

"NEWROOT\n\n"

"Note that the newroot, full-overlay, and partial-overlay options all\n"
"have the same form, namely:  [mode]  path\n\n"

"The first option of this form is the newroot-option.  The newroot-\n"
"option specfies the newroot.\n\n"

"If no newroot-option is specified, then lxroot will neither bind,\n"
"chroot, nor pivot.  This is useful to simulate root or deny network\n"
"access while retaining the current mount namespace.\n\n"

"FULL OVERLAY\n\n"

"Zero or more full-overlay options may occur anywhere before the first\n"
"set-source option.\n\n"

"A full-overlay option has the form:  [mode]  path\n\n"

"A full-overlay option will attempt to bind all the subdirectories\n"
"inside path to identically named subdirectories inside newroot.\n\n"

"For example, if my_overlay contains the subdirectories 'home', 'run',\n"
"and 'tmp', then the full-overlay option 'rw my_overlay' will attempt\n"
"to bind the following:\n\n"

"  my_overlay/home  to  newroot/home  in read-write mode\n"
"  my_overlay/run   to  newroot/run   in read-write mode\n"
"  my_overlay/tmp   to  newroot/tmp   in read-write mode\n\n"

"If any newroot/subdir does not exist, then that my_overlay/subdir will\n"
"be silently skipped.\n\n"

"SET SOURCE\n\n"

"A set-source option has the form:  'src'  [mode]  path\n\n"

"'src' is the literal string 'src'.\n\n"

"A set-source option sets the overlay-source-path and the default\n"
"overlay-mode.  These values will be used by any following\n"
"partial-overlay options.\n\n"

"Zero or more set-source options may be specified.\n\n"

"PARTIAL OVERLAY\n\n"

"Zero or more partial-overlay options may occur anywhere after the\n"
"first set-source option.\n\n"

"A partial-overlay option has the form:  [mode]  path\n\n"

"A partial-overlay option will bind overlay/path to newroot/path, where\n"
"overlay is the overlay-source-path set by the preceding set-source\n"
"option.\n\n"

"For example, the two options 'src my_overlay home/my_username' will do\n"
"the following:\n\n"

"  1)  first, the overlay-source-path will be set to 'my_overlay'\n"
"  2)  then, the following bind will occur:\n\n"

"        my_overlay/home/my_username  to  newroot/home/my_username\n\n"

"If either directory does not exist, lxroot will exit with status 1.\n\n"

"Successive partial-overlay options may be used to bind a selected\n"
"subset of the descendants of an overlay into newroot.  (Whereas a\n"
"single full-overlay option attempts to bind all of the full-overlay's\n"
"immediate subdirectories into newroot.)\n\n"

"BIND\n\n"

"A bind-option has the form:  'bind'  [mode]  dst  src\n\n"

"'bind' is the literal string 'bind'.\n\n"

"A bind-option will bind src to newroot/dst, using the optionally\n"
"specified mode.\n\n"

"Note that dst precedes src.  This hopefully improves readibilty in\n"
"scripts where: (a) many binds may be specified, (b) dst is tyically\n"
"shorter than src, and (c) src may vary greatly in length from bind to\n"
"bind.\n\n"

"CD\n\n"

"A cd-option has the form:  'cd'  path\n\n"

"'cd' is the literal string 'cd'.  One or zero cd-options may be\n"
"specified.\n\n"

"A cd-option tells lxroot to cd into path (in the new environment)\n"
"before executing the command.\n\n"

"path does not include newroot, as a cd-option is processed after the\n"
"pivot.\n\n"

"WD\n\n"

"A wd-option has the form:  'wd'  path\n\n"

"'wd' is the literal string 'wd'.  Zero or more wd-options may be\n"
"specified.\n\n"

"Lxroot will bind path (and all of path's descendants) in read-write\n"
"mode.  So a wd-option is used to make writeable a specific path (and\n"
"its descendants) inside the new environment.\n\n"

"path does not include newroot, as wd-options are processed after the\n"
"pivot.\n\n"

"Additionally, if no cd-option is specified, then lxroot will cd into\n"
"the path of the last wd-option prior to executing the command.\n\n"

"Note: Any path that is already mounted in read-only mode in the\n"
"outside environment (i.e. before lxroot runs) will still be read-only\n"
"inside the new environment.  This is because non-root namespaces can\n"
"only impose new read-only restricitons.  Non-root namespaces cannot\n"
"remove preexsiting read-only restrictions.\n\n"

"COMMAND\n\n"

"The command-option specifies the command that will be executed inside\n"
"the lxroot environment.  The command-option must be preceded by '--'.\n\n"

"If no command is specified, lxroot will attempt to find and execute an\n"
"interactive shell inside the lxroot environment.\n\n"

""  ;    //  end  help_more  ---------------------------------  end  help_more


//  help.cpp  -  Create and use chroot-style virtual software environments.
//
//  Copyright (c) 2022 Parke Bostrom, parke.nexus at gmail.com
//
//  This program is free software: you can redistribute it and/or
//  modify it under the terms of version 3 of the GNU General Public
//  License as published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See version
//  3 of the GNU General Public License for more details.
//
//  You should have received a copy of version 3 of the GNU General
//  Public License along with this program.  If not, see
//  <https://www.gnu.org/licenses/>.
