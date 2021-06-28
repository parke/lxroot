

//  lxroot.cpp  -  Create and use chroot-style virtual software environments.
//  Copyright (c) 2021 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.


#define  LXROOT_VERSION  "0.0.20210628"


const char *  help  =    //  xxhe  -------------------------------------  help
"\n"
"usage:  lxroot  [[mode] newroot]  [options]  [--]  [command [arg ...] ]\n\n"

"options\n"
"  -short                      one or more short options\n"
"  --long-option               a long option\n"
"  n=v                         set an environment variable\n"
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


//  short options  ( for plannig purposes only, possibly inaccurate )
//  .           .           .           .           .           .
//  a -         f - hostfs  k -         p - PID     u - UID     z
//  b -         g -         l -         q -         v - verbose
//  c -         h - hstname m - MOUNT   r root      w write
//  d - dbus    i -         n NET       s - /sys    x x11
//  e environ   j -         o -         t           y
//
//  A           F           K           P           U           Z
//  B           G           L           Q           V
//  C           H           M           R           W
//  D           I           N           S           X
//  E           J           O           T           Y
//
//  long options  ( for plannig purposes only, possibly inaccurate )
//    dbus  env  help  help-more  network  pid  pulseaudio  root  trace  uid
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
"the lxroot environment.\n\n"

"If no command is specified, lxroot will attempt to find and execute an\n"
"interactive shell inside the lxroot environment.\n\n"

"Note the following lexical ambiguity: a path-like argument may specify\n"
"either (a) an overlay option or (b) the command option.\n\n"

"lxroot resolves this ambiguity by looking for a directory at the path.\n"
"If a directory exists, lxroot interprets the path as an overlay option.\n"
"If no such directory exists, lxroot interprets the path as a command.\n\n"

"Note that lxroot does not verify that the command actually exists\n"
"inside newroot.  If the command does not exist, then the call to\n"
"exec() will will fail and lxroot will exit with status 1.\n\n"

"To force a path to be interpreted as a command, proceed the path with\n"
"the option '--'.\n\n"

""  ;    //  end  help_more  ---------------------------------  end  help_more


//  Welcome to the source code for lxroot.
//
//  The classes and structs in lxroot can be divided into three categories:
//
//    low level data storage classes
//    convenience wrappers around system APIs
//    high level classes
//
//  Low level data storage classes
//
//  enum    opt       an enumeration that represents various options.
//  struct  mfrag     a mutable string fragment (const char * and length).
//  struct  mstr      a mutable string (const char *, null terminated).
//  struct  Concat_2  assists with string concatination.
//  struct  ostr      an appendable string that owns its memory.
//  struct  Concat    possibly deprecated, assists with string concatination
//  struct  Argv      a convenience and saftey wrapper around const char * *.
//  struct  Option    represents a parsed command line option.
//  struct  Bind      represents (and specifies) a potential bind mount.
//  struct  Env       a list that specifies the new environment.
//  struct  State     contains shared, mutable, global variables.
//
//  typedef  frag     a const mfrag.
//  typedef  str      a const mstr.
//
//  Convenience wrappers
//
//  class   Dirent     represents a directory entry (from readdir()).
//  struct  Fgetpwent  parses /etc/passwd.
//  struct  Lib        contains generally useful functions.
//  struct  Syscall    provides error handling and tracing of syscalls.
//
//  High level classes
//
//  struct  Option_Reader  parses Argv to generate Options and/or Binds.
//  struct  Logic          analyzes data and performs loops over data.
//  struct  Init_Tool      records certain Options in State.
//  struct  Env_Tool       configures the new envirnoment variables.
//  class   Lxroot         one class to rule them all.
//
//  If you wish to read Lxroot's source code, I recommend starting at
//  the bottom of lxroot.cpp with class Lxroot (the highest level
//  class), and then reading deeper into the other (lower level)
//  classes as needed.


const char *    bash_command[]  =  { "/bin/bash", "--norc", nullptr };


#include  <dirent.h>         //  man 3 opendir
#include  <errno.h>          //  man 3 errno
#include  <fcntl.h>          //  man 2 open
#include  <limits.h>         //  what uses limits.h ?
#include  <pwd.h>            //  man 3 fgetpwent
#include  <sched.h>          //  man 2 unshare
#include  <signal.h>         //  raise(SIGINT), used with gdb
#include  <stdio.h>          //  man 3 fprintf
#include  <stdlib.h>         //  man 3 malloc
#include  <string.h>         //  man 3 strlen
#include  <unistd.h>         //  man 2 getuid stat
#include  <sys/mount.h>      //  man 2 mount
#include  <sys/stat.h>       //  man 2 stat
#include  <sys/syscall.h>    //  man 8 pivot_root
#include  <sys/types.h>      //  man 3 opendir
#include  <sys/wait.h>       //  man 2 wait

#include  <vector>           //  class Env .vec
#include  <functional>       //  std :: function


template < class C >    //  ---------------------------  template  using  sink
using  sink  =  std :: function < void ( const C & ) >;
template < class C >    //  --------------------------  template  using  msink
using  msink  =  std :: function < void ( C & ) >;


//  macro  printe  --------------------------------------------  macro  printe
#define  printe( ... )  fprintf ( stderr, __VA_ARGS__ );


//  macro  die_pe  --------------------------------------------  macro  die_pe
//  see  https://stackoverflow.com/q/5588855   regarding ##__VA_ARGS__
//  see  https://stackoverflow.com/a/11172679  regarding ##__VA_ARGS__
#define  die_pe( format, ... )  {					\
  fprintf ( stderr, "lxroot  error  " format "  ", ##__VA_ARGS__ );	\
  perror ( NULL );							\
  exit ( 1 );  }


//  macro  die1  ------------------------------------------------  macro  die1
#define  die1( format, ... )  {						\
  fprintf ( stderr, "lxroot  error  " format "\n", ##__VA_ARGS__ );	\
  exit ( 1 );  }    // or call abort() instead of exit?


//  macro  warn  ------------------------------------------------  macro  warn
#define  warn( format, ... )  {						\
  fprintf ( stderr, "lxroot  warn   " format "\n", ##__VA_ARGS__ );  }


template < typename T >    //  ----------------------------  template  assert2
T  assert2  ( T v, const char * file, const int line )  {

  //  Discussion of assert2 being a function vs being a macro:

  //  A function ensures that v is evaluated only once.

  //  My hope is that assert2 as a template (rather than a macro) will
  //  result in more readable compile time errors.
  //  However, a template might result in unnecessary copying of a T.

  //  Does assert2 need to be a function so that it can return a value?
  //  Or could a macro "return" a value via the comma operator?  Not sure.

  if  ( (bool) v == false )  {
    printe ( "lxroot  assert2()  failed  %s  %d\n", file, line );  abort();  }
  return  v;  }


//  macro  assert  --------------------------------------------  macro  assert
#define  assert( v )  assert2 ( v, __FILE__, __LINE__ )




enum  opt  {    //  xxop  -----------------------------------------  enum  opt
  o_none,
  /*  literal arg types  */
  o_bind,  o_dashdash,  o_cd,  o_ra,  o_ro,  o_rw,  o_src,  o_wd,
  /*  non-literal option types  */
  o_command,  o_full,  o_newroot,  o_partial,  o_setenv,  o_shortopt,
  /*  literal long options  */
  o_env,    o_help,    o_help_more,    o_network,    o_pulse,
  o_root,    o_trace,    o_version,    o_write,    o_x11,
  };


const char * const  opt_name[]  =  {    //  ------------------------  opt_name
  "0",
  /*  literal arg types  */
  "bind",  "--",        "cd",  "ra",  "ro",  "rw",  "src",  "wd",
  /*  non-literal option types  */
  "command",  "full",  "newroot",  "partial",  "setenv",  "shortopt",
  /*  literal long options  */
  "--env",  "--help",  "--help-more",  "--network",  "--pulseaudio",
  "--root",  "--trace",  "--version",  "--write",  "--x11",
  nullptr  };


opt  operator ||  ( opt a, opt b )  {    //  ---------------------  opt  op ||
  return  a  ?  a  :  b  ;  }


opt  global_opt_trace  =  o_none;    //  --------------  opt  global_opt_trace


const char *  o2s  ( const opt n )  {    //  ----------------------------  o2s
  if  (  0 <= n  &&  n < ( sizeof opt_name / sizeof(char*) )  )  {
    return  opt_name[n];  }
  return  "INVALID_OPTION";  }


opt  s2o  ( const char * const s )  {    //  ----------------------------  s2o
  if  ( s )  {
    for  (  int n = o_none;  opt_name[n];  n++ )  {
      if  ( strcmp ( s, opt_name[n] ) == 0 )  {  return  (opt) n;  }  }  }
  return  o_none;  }


//  end  enum  opt  ------------------------------------------  end  enum  opt




struct   mfrag;     //  ------------------------------  declare  struct  mfrag
struct   mstr;      //  -------------------------------  declare  struct  mstr
class    ostr;      //  --------------------------------  declare  class  ostr
class    Concat;    //  ------------------------------  declare  class  Concat
typedef  const mstr     str;        //  xxst  ------------------  typedef  str
typedef  const mfrag    frag;       //  xxfr  -----------------  typedef  frag
typedef  unsigned long  flags_t;    //  xxfl  --------------  typedef  flags_t


struct  mfrag  {    //  xxmf  ---------------------------------  struct  mfrag


  const char *  s  =  nullptr;
  int  n           =  0;


  mfrag  ()  {}    //  ------------------------------------------  mfrag  ctor
  mfrag  ( const char * s        )  :  s(s),  n(s?strlen(s):0)  {}
  mfrag  ( const char * s, int n )  :  s(s),  n(n)              {}
  mfrag  ( const char * a,
	   const char * b )         :  s(a),  n( b - a + 1 )    {}


  explicit operator bool  ()  const  {  return  s;  }    //  --------  op bool


  bool  operator ==  ( frag & o )  const  {    //  -------------  mfrag  op ==
    return  n == o.n  &&  memcmp ( s, o.s, n ) == 0;  }


  /*  20210623  used next() instead.
  frag  operator ++  ( int )  {  //  ---------------------------  mfrag  op ++
    if  ( n > 0 )  {  s ++;  n --;  }  return  * this;  }
  */


  Concat  operator +  ( frag & o )  const;    //  ---------------  mfrag  op +


  char  c  ()  const  {  return  n > 0  ?  * s  :  '\0'  ;  }    //  ------  c
  void  next  ()  {  if  ( n > 0 )  {  s ++;  n --;  }  }    //  -------  next


  mfrag  capture_until  ( const char accept )  {    //  -------  capture_until
    const char * const  a  =  s;
    return  frag ( a, find(accept).s - a );  }


  mfrag &  find  ( const char accept )  {    //  ----------------  mfrag  find
    while  (  c()  &&  c() not_eq accept )  {  next();  }  return  * this;  }


  mfrag &  find_skip  ( const char accept )  {    //  ------  mfrag  find_skip
    return  find ( accept ) .skip ( accept );  }


  mfrag &  skip  ( const char accept )  {    //  ----------------  mfrag  skip
    while  (  c()  &&  c() == accept  )  {  next();  }  return  * this;  }


  void  trace  ( const char * const message )  const  {    // --  mfrag  trace
    if  ( global_opt_trace == o_trace )  {
      printe ( "%s  >", message );
      fwrite ( s, n, 1, stderr );
      printe ( "<\n" );  }  }


};    //  end  struct  mfrag  ----------------------------  end  struct  mfrag




const char *  leak  ( frag o )  {    //  -------------  global  function  leak
  //  lxroot quickly exec()s on success or exit()s on failure.
  //  therefore, a few convenient and minor memory leaks are acceptable.
  char *  rv  =  (char*) malloc  ( o.n + 1 );    //  intentional leak.
  if  ( rv )  {
    memcpy ( rv, o.s, o.n );
    rv [ o.n ]  =  '\0';  }
  return  rv;  }




struct  mstr  {    //  xxms  -----------------------------------  struct  mstr

  //  mstr is a convenience wrapper around a const char *.
  //  s points to either (a) nullptr or (b) a null-terminated string.
  //  an mstr does not own s.  (But see also derived class ostr.)
  //  an mstr makes no guarantee about the lifetime of *s.


  const char *  s  =  nullptr;   //  a pointer to the string.


  mstr  ()  {}    //  --------------------------------------------  mstr  ctor
  mstr  ( const char * s )  :  s(s)  {}    //  -------------------  mstr  ctor


  operator bool  ()  const  {  return  s;  }    //  --------  mstr  cast  bool
  operator frag  ()  const  {  return  s;  }    //  --------  mstr  cast  frag


  bool  operator ==  ( str & o )  const  {    //  ---------------  mstr  op ==
    if  ( s == nullptr  &&  o.s == nullptr )  {  return  true;   }
    if  ( s == nullptr  ||  o.s == nullptr )  {  return  false;  }
    return  strcmp ( s, o.s ) == 0;  }


  mstr  operator ||  ( const mstr & o )  const  {    //  --------  mstr  op ||
    return  s  ?  * this  :  o  ;  }


  char  operator *  ()  const  {    //  --------------------------  mstr  op *
    return  s  ?  *s  :  '\0';  }


  char  operator []  ( int index )  const  {    //  -------------  mstr  op []
    if  ( s == nullptr )  {  return  0;  }
    return  skip ( index ) .s[0];  }


  mstr  operator ++  ( int )  {    //  --------------------------  mstr  op ++
    return  s && *s  ?  s++  :  s  ;  }


  Concat  operator +  ( frag o )  const;    //  ---------  declare  mstr  op +
  Concat  operator +  ( str  o )  const;    //  ---------  declare  mstr  op +
  Concat  operator +  ( const char * const o )  const;    //  ----  mstr  op +


  frag  basename  ()  const  {    //  -------------------------  mstr  basename
    if  ( s == nullptr )  {  return  nullptr;  }
    //    /foo/bar
    //     a  bc
    //      a is the first character of    this basename
    //      b is the first character after this basename  ( '/' or '\0' )
    //      c is the first non-slash after b
    //    return the last basename
    const char * a  =  s;
    while  ( a[0] == '/' && a[1] )  {  a++;  }
    for  (;;)  {
      const char *  b  =  a + ( a[0] ? 1 : 0 );
      while  ( b[0] && b[0] != '/' )  {  b++;  }
      if     ( b[0] == '\0'        )  {	 return  frag ( a, b-1 );  }
      const char *  c  =  b + 1;
      while  ( c[0] == '/'         )  {  c++;  }
      if     ( c[0] == '\0'        )  {  return  frag ( a, b-1 );  }
      a  =  c;  }  }


  static void  basename_test  ( str s, str expect );    //  ------------------


  frag  capture_until  ( char c )  const  {    //  ------  mstr  capture_until
    mstr  p  =  s;  while  (  p && * p  &&  * p != c  )  {  p ++;  }
    return  frag ( s, p.s - s );  }


  const void *  chr  ( const int c )  const  {    //  -------------  mstr  chr
    return  s  ?  :: strchr ( s, c )  :  nullptr  ;  }


  bool  contains  ( char c )  const  {    //  ----------------  mstr  contains
    if  ( s == nullptr )  {  return  false;  }
    for  (  const char * p = s;  * p;  p ++  )  {
      if  ( * p == c )  {  return  true;  }  }
    return  false;  }


  frag  env_name  ()  const  {    //  ------------------------  str  env_name
    return  head ( "=" );  }


  frag  head  ( const char * sep, int start = 0 )  const  {    //  -----  head
    if  ( s == nullptr )  {  return  frag();  }
    const char *  p  =  s;
    while  (  * p  &&  start-- > 0  )  {  p++;  }
    const char *  found  =  :: strstr ( p, sep );
    if  ( found )  {  return  frag ( p, found - p );  }
    return  frag();  }


  bool  is_inside  ( str path )  const  {    //  ------------  mstr  is_inside

    //  return true iff s is equal to or a descendant of path.

    mstr  descendant  =  s;
    mstr  ancestor    =  path;
    bool  is_inside   =  false;

    auto  skip_slash  =  [&]  ()  {
      while  ( * ancestor    == '/'  )  {  ancestor   ++;       }
      while  ( * descendant  == '/'  )  {  descendant ++;       }
      if     ( * ancestor    == '\0' )  {  is_inside  =  true;  }  };

    if  ( path == nullptr )  {  return  false;  }
    skip_slash();
    while  ( * ancestor == * descendant )  {
      switch ( * ancestor ) {
	case '\0':  return  true;  break;
	case '/':   skip_slash();  break;
	default:    ancestor ++;  descendant ++;  break;  }  }

    return  is_inside  ||  ( * ancestor == '\0' && * descendant == '/' );  }


  bool  is_same_path_as  ( str path )  const  {    //  ------  is_same_path_as
    return  is_inside(path)  &&  path.is_inside(*this);  }


  int  n  ()  const  {    //  ---------------------------------------  mstr  n
    return  s  ?  strlen ( s )  :  0  ;  }


  str  skip  ( int n )  const  {    //  --------------------------  mstr  skip
    const char *  p  =  s;
    while  ( n-- > 0 )  {
      if  (  p  &&  * p )  {  p++; }
      else  {  return  nullptr;  }  }
    return  p;  }


  str  skip_all  ( char c )  const  {    //  -----------------  mstr  skip_all
    mstr  p  =  s;
    while  (  p  &&  * p == c )  {  p ++;  };  return  p;  }


  int  spn  ( str accept )  const  {    //  -----------------------  mstr  spn
    return  s  ?  :: strspn ( s, accept.s )  :  0  ;  }


  bool  startswith  ( frag o )  const  {    //  ------------  mstr  starstwith
    return  s  &&  strncmp ( s, o.s, o.n ) == 0;  }


  str  tail  ( str sep )  const  {    //  ------------------------  mstr  tail
    str  found  =  :: strstr ( s, sep.s );
    return  found  ?  found.s + sep.n()  :  nullptr  ;  }


  static void  unit_test  ()  {    //  ----------------------  mstr  unit_test

    basename_test ( "",          ""    );
    basename_test ( "/",         "/"   );
    basename_test ( "//",        "/"   );
    basename_test ( "///",       "/"   );
    basename_test ( "a",         "a"   );
    basename_test ( "/b",        "b"   );
    basename_test ( "c/",        "c"   );
    basename_test ( "/d/",       "d"   );
    basename_test ( "e/f",       "f"   );
    basename_test ( "/g/h",      "h"   );
    basename_test ( "abc",       "abc" );
    basename_test ( "/def",      "def" );
    basename_test ( "ghi/",      "ghi" );
    basename_test ( "/jkl/",     "jkl" );
    basename_test ( "mno/pqr",   "pqr" );
    basename_test ( "/stu/vwx",  "vwx" );
    basename_test ( "./xyz",     "xyz" );

    return;  }


};    //  end  struct  mstr  ------------------------------  end  struct  mstr




struct  Concat_2  {    //  xxco  ---------------------------  struct  Concat_2
  mfrag s[3];
  Concat_2  ( frag a, frag b = 0, frag c = 0 )  :  s{a,b,c}  {}
};    //  end  struct  Concat_2  ----------------------  end  struct  Concat_2




Concat_2  s  ( frag a, frag b = 0, frag c = 0 )  {    //  xxs  ----  global  s
  return  Concat_2 ( a, b, c );  }


Concat_2  s  ( str a, str b = 0, str c = 0 )  {    //  xxs  -------  global  s
  return  Concat_2 ( a, b, c );  }


opt  s2o  ( str s )  {    //  xxs2  --------------------  global function  s2o
  return  s2o ( s.s );  }




struct  ostr  :  mstr   {    //  xxos  -------------------------  struct  ostr


  int  n  =  0;


  ostr  ()  {}    //  --------------------------------------------  ostr  ctor
  ostr  ( frag          o )  :  mstr()  {  * this  +=  o;  }
  ostr  ( const char *  o )  :  mstr()  {  * this  +=  o;  }
  ostr  ( const str &   o )  :  mstr()  {  * this  +=  o;  }
  ostr  ( const ostr &  o )  :  mstr()  {  * this  +=  o;  }
  ostr  (       ostr && o )  :  mstr()  {  s=o.s; n=o.n; o.s=nullptr; o.n=0; }
  ostr  ( const Concat_2 & o )  :  mstr()  { * this = o; }


  ~ostr  ()  {    //  --------------------------------------------  ostr  dtor
    free ( (char*) s );  }


  void  operator =  ( frag         o )  {  n=0;  * this += o;   }    //  -----
  void  operator =  ( const ostr & o )  {  * this  =  frag(o);  }    //  -----
  void  operator =  ( str          o )  {  * this  =  frag(o);  }    //  -----
  void  operator =  ( char       * o )  {  * this  =  frag(o);  }    //  -----
  void  operator =  ( const char * o )  {  * this  =  frag(o);  }    //  -----


  void  operator =  ( const Concat_2 & t )  {
    ostr & r  =  * this;  r = t.s[0];  r += t.s[1];  r += t.s[2];  }


  ostr &  operator +=  ( frag o )  {    //  ---------------------  ostr  op +=
    char *  p  =  (char*) assert ( realloc ( (char*) s, n + o.n + 1 ) );
    memcpy ( p+n, o.s, o.n );
    n     +=  o.n;
    p[n]  =   '\0';
    s     =   p;
    return  * this;  }


  static ostr  claim  ( str s )  {    //  -----------------------  ostr  claim
    //  return an ostr that owns s.  s must be an unowned, malloc()ed string.
    ostr  rv;    rv.s=s.s;    rv.n=s.n();    return rv;  }


  static void  unit_test  ();    //  ------------------------  ostr  unit_test


};    //  end  struct  ostr  ------------------------------  end  struct  ostr




struct  Concat  {    //  xxco  --------------------------------  class  Concat

  ostr  s;

  Concat  ( frag o )  :  s(o)  {}    //  -----------------------  Concat  ctor

  operator frag          ()  const  {  return  s;  }    //  Concat  cast  frag
  operator str           ()  const  {  return  s;  }    //  Concat  cast  str
  operator ostr          ()  const  {  return  s;  }    //  Concat  cast  ostr
  bool  operator ==  ( str o )  const  {  return  s == o.s;  }    //  --------
  Concat &  operator +   ( frag o )  {  s  +=  o;  return  * this;  }    //  -
  Concat &  operator +=  ( frag o )  {  s  +=  o;  return  * this;  }    //  -


};    //  end  struct  Concat


Concat  mfrag :: operator +  ( frag & o )  const  {    //  ------  mfrag  op +
  //  written as three statements to (1) avoid infinite recursion, and
  //  (2) allow named return value optimization.
  Concat rv (*this);    rv += o;    return rv;  }


Concat  mstr :: operator +  ( frag o )  const  {    //  ----------  mstr  op +
    return  ((frag)*this) + o;  }


Concat  mstr :: operator +  ( str  o )  const  {    //  ----------  mstr  op +
    return  ((frag)*this) + o;  }


Concat  mstr :: operator +  ( const char * const o )  const  {    //  --  op +
    return  ((frag)*this) + o;  }


Concat  operator +  ( const char * a, frag b )  {    //  -----------  ::  op +
  return  frag(a) + b;  }


Concat  operator +  ( const char * a, str b )  {    //  ------------  ::  op +
  return  frag(a) + b;  }


void  mstr :: basename_test  ( str s, str expect )  {    //  --  basename_test
    if  ( s.basename() == expect )  {  return;  }
    ostr  actual  =  s.basename();
    printe ( "basename_test  failed\n" );
    printe ( "  s       %s\n", s.s      );
    printe ( "  expect  %s\n", expect.s );
    printe ( "  actual  %s\n", actual.s );
    abort();  }


void  ostr :: unit_test  ()  {    //  -----------------------  ostr  unit_test
    str   a   =  "a";
    str   b   =  "b";
    ostr  ab  =  a + b;
    assert  ( ab == "ab" );
    assert  ( ab == a+b  );
    frag  c   =  "c";
    frag  d   =  "d";
    ostr  cd  =  c + "=" + d;
    assert  ( cd == "c=d" );
    ostr  ef  =  cd + "ef" + "gh";
    assert  ( ef == "c=defgh" );
    assert  ( cd == "c=d" );
    ;;;  }


//  end  struct  Concat  --------------------------------  end  struct  Concat




struct  Argv  {    //  xxar  -----------------------------------  struct  Argv


  const char * const *  p  =  nullptr;


  Argv  ()  {}    //  --------------------------------------------  Argv  ctor
  Argv  ( const char * const * const p )  :  p(p)  {}


  Argv &  operator |=  ( const Argv & o )  {    //  -------------  Argv  op |=
    if  ( p == nullptr )  {  p  =  o.p;  }  return  * this;  }


  bool  operator ==  ( const Argv & o )  const  {    //  --------  Argv  op ==
    return  p == o.p;  }


  explicit  operator bool  ()  const  {    //  ----------------  Argv  op bool
    return  p;  }


  str  operator *  ()  const  {    //  ---------------------------  Argv  op *
    return  p  ?  * p  :  nullptr  ;  }


  Argv  operator ++  ()  {    //  -------------------------------  Argv  op ++
    return  p  &&  * p  ?  ++p  :  nullptr  ;  }


  Argv  operator ++  (int)  {    //  ----------------------------  Argv  op ++
    return  p  &&  * p  ?  p++  :  nullptr  ;  }


  Argv  operator +  ( int n )  const  {    //  -------------------  Argv  op +
    Argv  rv  ( * this );
    while  (  rv.p  &&  rv.p[0]  &&  n-- > 0  )  {  rv++;  }
    return  rv;  }


  Argv &  operator +=  ( int n )  {    //  ----------------------  Argv  op +=
    * this  =  ( * this ) + n;  return  * this;  }


  str  operator []  ( int n )  const  {    //  ------------------  Argv  op []
    return  * ( ( * this ) + n );  }


  ostr  concat  ( frag sep = "  " )  const  {    //  -----------  Argv  concat
    Argv  o  ( * this );
    ostr  rv;
    if ( *(o.p) )  {  rv  +=  * o;    o++;  }
    for  (  ;  *(o.p)  ;  o++  )  {  rv  +=  sep;    rv  +=  * o;  }
    return  rv;  }


  str  env_get  ( str k )  const  {    //  --------------------  Argv  env_get
    for  (  Argv o (*this)  ;  *(o.p)  ;  o++  )  {
      if  ( (*o).env_name() == k )  {  return  (*o);  }  }
    return  nullptr;  }


  void  print  ( str s )  const  {    //  -----------------------  Argv  print
    for  (  Argv o (*this)  ;  o  &&  o.p[0]  ;  o++  )  {
      printe  ( "%s%s\n", s.s, o[0].s );  }  }


  static void  unit_test  ()  {    //  ----------------------  Argv  unit_test

    const char *  p[]  =  { "a=1", "b=2", "c=3", nullptr };
    Argv  a(p);

    assert ( a[0] == p[0] );
    assert ( a[1] == p[1] );
    assert ( a[2] == p[2] );
    assert ( a[3] == p[3] );

    assert ( a[0] == a.p[0] );
    assert ( a[1] == a.p[1] );
    assert ( a[2] == a.p[2] );
    assert ( a[3] == a.p[3] );
    assert ( a.concat() == "a=1  b=2  c=3" );

    assert ( a.env_get("a") == p[0] );
    assert ( a.env_get("b") == p[1] );
    assert ( a.env_get("c") == p[2] );

    ;;;  }


};    //  end  struct  Argv  ------------------------------  end  struct  Argv




struct  Option  {    //  xxop  -------------------------------  struct  Option


  opt   type  =  o_none;    //  see the below input/type chart for details.
  opt   mode  =  o_none;    //  one of:  o_none  o_ra  o_ro  o_rw
  mstr  arg0,  arg1;
  Argv  command;            //  nullptr, then set to command when found

  Argv  p;                  //  points to the next option
  mstr  newroot;            //  the newroot
  mstr  overlay;            //  the current overlay


  Option  ( Argv p )  :  p(p)  {}    //  -----------------------  Option  ctor


  operator bool  ()  const  {  return  type;  }    //  ---  Option  cast  bool


  void  print  ( str m )  const  {    //  ---------------------  Option  print
    printe ( "%-8s  %-7s  %s\n", m.s, o2s(type), arg0.s );  }


};    //  end  struct  Option  --------------------------  end  struct  Option




struct  Bind  {    //  xxbi  -----------------------------------  struct  Bind


  opt   type    =  o_none;
  opt   mode    =  o_none;    //  the specified mode  ro, rw, ra, none
  opt   actual  =  o_none;    //  the actual mode     ro, rw
  mstr  full;
  mstr  dst;            //  20210619  at present, dst never begins with '/'
  ostr  src;
  ostr  newroot_dst;    //  newroot + dst
  Argv  p;              //  points to the next option after this Bind.


  void  clear  ()  {    //  -------------------------------------  Bind  clear
    * this  =  Bind();  }


  void  print  ( str s )  const  {    //  -----------------------  Bind  print
    printe ( "%s  bind  %-7s  %-2s  %-2s  '%s'  '%s'  '%s'\n",
	     s.s,  o2s(type),  o2s(mode),  o2s(actual), dst.s, src.s,
	     newroot_dst.s );  }


  Bind &  set    //  ----------------------------------------------  Bind  set
  ( const Option & o, str childname = 0 )  {

    //  20210620  note:  a Bind my outlive the Option o.

    str  ov  =  o.overlay;
    str  a0  =  o.arg0;
    str  a1  =  o.arg1;
    str  cn  =  childname;

    type  =  o.type;
    mode  =  o.mode;
    full  =  nullptr;
    p     =  o.p;
    switch  ( o.type )  {
      case  o_newroot:  dst="";  src=a0;  mode=mode||o_ra;    break;
      case  o_bind:     dst=a0;  src=s(a1);                   break;

	//  20210620  todo  consider setting dst and src to ""
        //  20210620        nope, it appears setting dst and src is required
	//  20210620        I wonder why?
      case  o_full:     dst=cn;  src=s(a0,"/",cn);  full=a0;  break;
      //se  o_full:     dst="";  src="";  full=a0;            break;

	//  20210620  todo  set mode (and write unit tests)
      case  o_partial:  dst=a0;  src=s(ov,"/",a0);            break;

      default:          dst="error";  src="error";            break;  }

    newroot_dst  =  dst  ?  s(o.newroot,"/",dst)  :  s(o.newroot)  ;

    //  20210619
    //  if  ( dst[0] == '/' )  {    //  20210619
    //    die1 ( "bind  set  bad dst  %s", dst.s );  }

    return  * this;  }


  const Bind &  trace  ( str s )  const  {    //  ---------------  Bind  trace
    if  ( global_opt_trace )  {  print ( s );  }
    return  * this;  }


};    //  end  struct  Bind  ------------------------------  end  struct  Bind




class  Env  {    //  xxen  ---------------------------------------  class  Env


  std :: vector < const char * >  vec;


public:


  const char * const *  data  ()  const  {    //  -----------------  Env  data
    return  vec .data();  }


  str  get  ( frag name )  const  {    //  -------------------------  Env  get
    for  (  auto & o  :  vec  )  {
      if  ( str(o) .env_name() == name )  {
	return  str(o) .tail ( "=" );  }  }
    return  nullptr;  }


  void  set  ( str pair )  {    //  --------------------------------  Env  set
    frag  name  =  pair .env_name();
    if  ( name.n == 0 )  {  return;  }
    for  (  auto & o  :  vec  )  {
      if  ( str(o) .env_name() == name  )  {  o  =  pair.s;  return;  }  }
    if  ( vec .size() == 0 )  {  vec .push_back ( nullptr );  }
    vec .back()  =  pair.s;
    vec .push_back ( nullptr );  }


  void  set  ( frag name, frag value )  {    //  -------------------  Env  set
    set ( leak ( name + "=" + value ) );  }


  void  soft  ( str pair )  {    //  ------------------------------  Env  soft
    if  ( not get ( pair .env_name() ) )  {  set ( pair );  }  }


  void  soft  ( frag name, frag value )  {    //  -----------------  Env  soft
    if  ( not get ( name ) )  {  set ( leak ( name + "=" + value ) );  }  }


  void  soft_copy  ( str name )  {    //  --------------------  Env  soft_copy
    str  pair  =  Argv ( environ ) .env_get ( name );
    if  ( pair )  {  soft ( pair );  }  }


};    //  end  class  Env  ----------------------------------  end  class  Env




struct  Fgetpwent  {    //  xxfg  -------------------------  struct  Fgetpwent


  ostr  dir, name, shell;    //  see man 3 fgetpwent


  void  fgetpwent  ( str path, uid_t uid )  {    //  --------------  fgetpwent

    FILE *  f  =  fopen ( path.s, "r" );

    if  ( f == nullptr )  {
      warn ( "fopen failed  %s", path.s );  return;  }

    struct passwd *  pwent  =  nullptr;
    while  ((  pwent  =  :: fgetpwent ( f )  ))  {    //  parse next entry
      if ( pwent -> pw_uid == uid )  {
	dir    =  pwent -> pw_dir;
	name   =  pwent -> pw_name;
	shell  =  pwent -> pw_shell;  break;  }  }

    endpwent();
    fclose ( f );  }


};    //  end  struct  Fgetpwent  --------------------  end  struct  Fgetpwent




struct  State  {    //  xxst  ---------------------------------  struct  State


  Argv         argv;
  Env          env;         //  specifies the new environment
  const uid_t  uid           =  getuid();
  const gid_t  gid           =  getgid();
  Fgetpwent    outside       ;    //  from /etc/passwd outside the lxroot
  Fgetpwent    inside        ;    //  from /etc/passwd inside  the lxroot
  opt          opt_env       =  o_none;    //  pass in environment
  opt          opt_network   =  o_none;
  opt          opt_pulse     =  o_none;
  opt          opt_root      =  o_none;
  opt          opt_write     =  o_none;
  opt          opt_x11       =  o_none;
  opt          newroot_mode  =  o_none;
  mstr         newroot;
  mstr         guestname;
  mstr         chdir;       //  from the first and only cd option
  mstr         workdir;     //  from the last wd option
  Argv         command;


};    //  end  struct  State  ----------------------------  end  struct  State


State  mut;    //  ---------------------------------------  global  State  mut
const State & st  =  mut;    //  ------------------  global  const State &  st




class  Dirent  {    //  xxdi  ---------------------------------  class  Dirent


  struct dirent *  p  =  nullptr;


public:


  Dirent &  operator =  ( dirent * pp )   {  p  =  pp;  return  * this;  }
  bool      operator == ( str s )  const  {  return  name() == s;  }
  operator  bool   ()  const              {  return  p;  }
  ino_t     inode  ()  const              {  return  p -> d_ino;  }
  str       name   ()  const              {  return  p -> d_name;  }


  bool  is_dir  ()  const  {    //  --------------------------  Dirent  is_dir
    if  ( p -> d_type == DT_UNKNOWN )  { die1("dirent type is DT_UNKNOWN"); }
    return  p -> d_type == DT_DIR;  }


};    //  end  class  Dirent  ----------------------------  end  class  Dirent




struct  Lib  {    //  xxli  -------------------------------------  struct  Lib


  //  20210530  apparent redundancy:  assert_is_dir  vs  directory_require


  static void  assert_is_dir    //  ----------------------  Lib  assert_is_dir
  ( const char * const path, const char * const m )  {
    if  ( is_dir ( path ) )  {  return;  }
    printe ( "lxroot  %s  directory not found  '%s'\n", m, path  );
    exit ( 1 );  }


  static void  directory_require     //  -------------  Lib  directory_require
  ( str path, str m )  {
    if  ( Lib :: is_dir ( path ) )  {  return;  }
    die1 ( "%s directory not found\n  '%s'", m.s, path.s );  }


  static bool  eq  ( const char * a, const char * b )  {    //  -----  Lib  eq
    if  ( a == NULL  ||  b == NULL )  return  false;
    return  strcmp ( a, b ) == 0;  }


  static ostr  getcwd  ()  {    //  -----------------------------  Lib  getcwd
    return  ostr :: claim ( get_current_dir_name() );  }


  static str  getenv  ( str name )  {    //  --------------------  Lib  getenv
    return  :: getenv ( name.s );  }


  static void  help_print  ( int n = 0 )  {    //  --------  Lib :: help_print
    printe ( "%s%s", help, help2 );  exit ( n );  }


  static void  help_more_print  ()  {    //  ---------  Lib :: help_more_print
    printe ( "%s%s", help, help_more );  exit ( 0 );  }


  static str  home  ()  {    //  ----------------------------------  Lib  home
    return  getenv ( "HOME" );  }


  static bool  is_dir  ( str path )  {    //  -------------------  Lib  is_dir
    struct stat  st;
    if  (  path.s  &&  path.n()  &&
	   stat ( path.s, & st ) == 0  &&  st .st_mode & S_IFDIR  )  {
      return  1;  }
    errno  =  ENOENT;
    return  false;  }


  static bool  is_empty_dir  ( const char * path )  {    //  ---  is_empty_dir
    if  ( not is_dir ( path ) )  {  return  false;  }
    DIR * dirp  =  assert ( opendir ( path ) );
    for  (  struct dirent * p;  ( p = readdir ( dirp ) );  )  {
      const char *  s  =  p -> d_name;
      if  (  eq(s,".")  ||  eq(s,"..")  )  {  continue;  }
      closedir ( dirp );  return  false;  }
    closedir ( dirp );  return  true;  }


  static bool  is_file  ( str path )  {    //  -----------------  Lib  is_file
    struct stat  st;
    if  (  path  &&  :: stat ( path.s, & st ) == 0
	   &&  st .st_mode & S_IFREG  )  {
      return  true;  }
    errno  =  ENOENT;    //  20210521  so perror() is useful?
    return  false;  }


  static bool  is_link  ( str path )  {    //  -----------------  Lib  is_link
    struct stat  st;
    if  (  path.s
	   &&  lstat ( path.s, & st ) == 0
	   &&  S_ISLNK ( st .st_mode )  )  {  return  true;  }
    errno  =  ENOENT;
    return  false;  }


  static str  readlink  ( str path )  {    //  ---------------  Lib  readlink
    struct stat  st;
    if  (  lstat ( path.s, & st ) == 0  &&  st .st_mode & S_IFLNK  )  {
      ssize_t  lim  =  st .st_size + 2;
      char *   buf  =  (char*)  malloc ( lim );
      if  ( buf )  {
	memset ( buf, '\0', lim );
	ssize_t  len  =  :: readlink ( path.s, buf, lim );
	if  ( len == lim - 2 )  {  return  buf;  } }
      printe ( "lxroot  readlink  failed  %s\n", path.s );
      exit ( 1 );  }
    return  nullptr;  }


  /*  20201217  realpath() is not used at present?
  static str  realpath  ( const char * const path )  {    //  ------  realpath
    return  ostr :: claim ( :: realpath ( path, nullptr ) );  }
  */


};    //  end  struct  Lib  --------------------------------  end  struct  Lib




//  macro  trace1  --------------------------------------------  macro  trace1
#define  trace1( format, ... )  {				\
  if  ( global_opt_trace == o_trace )  {			\
    fprintf ( stderr, format "\n",				\
              ##__VA_ARGS__ );  }  }


//  macro  try_quiet  --------------------------------------  macro  try_quiet
#define  try_quiet( function, format, ... )  {			\
  if  ( (function) ( __VA_ARGS__ ) == 0 )  {  return;  }	\
  else  {  die_pe ( format, ##__VA_ARGS__ );  }  }


//  macro  try1  ------------------------------------------------  macro  try1
#define  try1( function, format, ... )  {	       		\
  trace1 ( format, ##__VA_ARGS__ );				\
  try_quiet ( function, format, ##__VA_ARGS__ );  }




struct  Syscall  {    //  xxsy  -----------------------------  struct  Syscall

  //  note  all Syscall methods call exit(1) on error.


  pid_t  fork_pid   =  -2;
  pid_t  wstatus    =  0;


  void  bind  ( str target, str source )  {    //  ------------  Syscall  bind
    //  from  /usr/include/linux/mount.h
    //    MS_REC        0x 4000
    //    MS_BIND       0x 1000
    //    MS_REMOUNT    0x 0020
    //    MS_RDONLY     0x 0001
    //
    //  20210520  I seem to remember that if MS_BIND is set, then MS_RDONLY
    //              has no effect.  Therefore, to make a bind mount
    //              readonly, first do the bind mount, and then call
    //              mount() a second time with MS_REMOUNT | MS_RDONLY.
    //
    //  20210520  I also seem to remember that, when mount is called by a
    //              non-root user, MS_BIND (often) requires MS_REC.
    //
    //  20210520  It appears that non-root users can only set MS_RDONLY.
    //              Non-root users are not allowed to clear MS_RDONLY.
    //
    Lib :: directory_require ( source, "source" );
    Lib :: directory_require ( target, "target" );
    trace1 ( "  bind     '%s'  '%s'", target.s, source.s );
    const flags_t  flags  =  MS_BIND | MS_REC;
    const int      rv     =  :: mount ( source.s, target.s, 0, flags, 0 );
    if  ( rv == 0 )  {  return;  }
    die_pe ( "bind  %s  %s\n", source.s, target.s );  }


  void  chdir  ( str path )  {    //  ------------------------  Syscall  chdir
    try1( :: chdir, "  chdir    %s", path.s );  }


  void  chroot  ( str new_root )  {    //  ------------------  Syscall  chroot
    try1( :: chroot, "  chroot   %s", new_root.s );  }


  void  close  ( int fd )  {    //  --------------------------  Syscall  close
    try_quiet ( :: close, "  close  %d", fd );  }


  void  execve  ( const str   pathname,    //  --------------  Syscall  execve
		  const Argv  argv,
		  const Argv  envp )  {

    trace1 ( "  execve   %s  %s", pathname.s, argv .concat().s );
    if  ( pathname .chr ( '/' ) )  {    //  path is specified, so use execve()
      :: execve ( pathname.s, (char**) argv.p, (char**) envp.p );  }
    else  {    //  only filename is specified, so use execvpe()
      char **  old  =  environ;
      environ       =  (char**) envp.p;
      :: execvpe ( pathname.s, (char**) argv.p, (char**) envp.p );
      environ       =  old;  }
    //  execve only returns on failure, so ...
    die_pe ( "execve  %s", pathname.s );  }


  void  exit  ( int status )  {    //  ------------------------  Syscall  exit
    trace1 ( "  exit     %d", status );  :: exit ( status );  }


  void  fork  ()  {    //  ------------------------------------  Syscall  fork
    if  ( fork_pid != -2 )  {  die_pe ( "extra fork?" );  }
    if  ( ( fork_pid = :: fork() ) >= 0 )  {
      trace1 ( "  fork     (fork returned %d)", fork_pid );
      return;  }
    die_pe ( "fork" );  }


  void  mount  ( str source, str target, str filesystemtype )  {    //   mount
    Lib :: directory_require ( target, "target" );
    trace1 ( "  mount    %s  %s  %s", source.s, target.s, filesystemtype.s );
    if  ( :: mount ( source.s, target.s, filesystemtype.s, 0, 0 ) == 0 )  {
      return;  }
    die_pe ( "mount  %s  %s  %s",  source.s, target.s, filesystemtype.s );  }


  void  open  ( int * fd, str pathname, const int flags )  {    //  ----  open
    if  ( ( * fd = :: open ( pathname.s, flags ) ) >= 0 )  {  return;  }
    die_pe ( "open  %s  %d", pathname.s, flags );  }


  void  pivot  ( str new_root, str put_old )  {   //  --------  Syscall  pivot
    trace1 ( "  pivot    '%s'  '%s'", new_root.s, put_old.s );
    if  ( syscall ( SYS_pivot_root, new_root.s, put_old.s ) == 0 )  { return; }
    die_pe ( "pivot  %s  %s", new_root.s, put_old.s );  }


  void  rdonly_20210624  ( str target )  {    //  ---  Syscall  rdonly_20210624
    trace1 ( "  rdonly   '%s'", target.s );
    const flags_t  flags  =  MS_BIND | MS_REMOUNT | MS_RDONLY;
    const int      rv     =  :: mount ( NULL, target.s, NULL, flags, NULL );
    if  ( rv == 0 )  {  return;  }
    die_pe ( "rdonly  %lx  %s\n", flags, target.s );  }


  void  rdonly  ( str target )  {    //  --------------------  Syscall  rdonly
    //  20210624  I do not know a terse way to determine which flags
    //            are already set.  So we iterate through the relevant
    //            possibilities until one works.  Maybe someday I will
    //            implement a more refined solution.  See the file
    //            fs/namespace.c in the Linux kernel, specifaclly the
    //            do_reconfigure_mnt() and can_change_locked_flags()
    //            functions.
    const char  d = MS_NODEV,  e = MS_NOEXEC,  s = MS_NOSUID;
    const unsigned char  masks[]  =  { 0, d, s, d|s, e, d|e, e|s, d|e|s };
    for  ( const auto guess : masks )  {
      const flags_t  flags  =  guess | MS_BIND | MS_REMOUNT | MS_RDONLY;
      const int      rv     =  :: mount ( NULL, target.s, NULL, flags, NULL );
      if  ( rv == 0 )  {
	trace1 ( "  rdonly  %lx  '%s'", flags, target.s );  return;  }  }
    die_pe ( "rdonly  '%s'\n", target.s );  }


  void  umount2  ( str target, int flags )  {   //  --------  Syscall  umount2
    try1( :: umount2, "  umount2  %s  0x%x", target.s, flags );  }


  void  unshare  ( const int flags )  {    //  -------------  Syscall  unshare
    try1( :: unshare, "  unshare  0x%08x", flags );  }


  pid_t  wait  ()  {    //  -----------------------------------  Syscall  wait
    trace1 ( "  wait     (parent calls wait)" );
    pid_t  pid  =  :: wait ( & wstatus );
    if  ( pid > 0 )  {
      trace1 ( "  wait     wait returned  pid %d  status 0x%x",
	      pid, wstatus );
      return  pid;  }
    die_pe ( "wait" );  return  -1;  }


  void  write  ( int fd, const void * buf, ssize_t count )  {    //  --  write
    assert ( count >= 0 );
    if  ( :: write ( fd, buf, count ) == count )  {  return;  }
    die_pe ( "write  %d  %ld", fd, count );  }


};    //  end  struct  Syscall  ------------------------  end  struct  Syscall


Syscall  sys;    //  xxsy  --------------------------------------  global  sys




struct  Option_Reader    //  xxop  --------------------  struct  Option_Reader
  :  private  Option  {


  const Option &  o;    //  const access to Option


  Option_Reader  ( Argv p )  :  Option(p),  o(*this)  {}  //  ----------  ctor


  const Option &  next  ()  {    //  --------------------  Option_Reader  next
    next_impl();  return  o;  }


private:


  void  next_impl  ()  {    //  --------------------  Option_Reader  next_impl

    //    input                  type          mode    arg0            arg1
    //
    //    --<longopt>            o_<longopt>    -       --<longopt>    -
    //    -short                 o_shortopt     -       -short         -
    //    n=v                    o_setenv       -       n=v            -
    //    [mode] path            o_newroot      mode    path           -
    //    [mode] path            o_full         mode    path           -
    //    [mode] path            o_partial      mode    path           -
    //    src [mode] path        o_src          mode    path           -
    //    bind [mode] dst src    o_bind         mode    dst            src
    //    cd path                o_cd           -       path           -
    //    wd path                o_wd           -       path           -
    //    --                     o_dashdash     -       "--"           -
    //    command [arg ...]      o_command      -       command        -

    mode  =  o_none;
    arg0  =  p[0];

    //  note  arg0 is the *current* arg, but ...
    //          type is the type of the *previous*(!) arg.
    if  ( type == o_dashdash )  {  do_command();     return;  }
    if  ( type == o_command  )  {  type  =  o_none;  return;  }
    if  ( arg0 == nullptr    )  {  do_command();     return;  }

    type  =  s2o ( arg0 );    //  type is now the type of the *current* arg.
    switch  ( type )  {
      case  o_ra:      //  fallthrough to o_rw
      case  o_ro:      //  fallthrough to o_rw
      case  o_rw:      mode_path();  return;
      case  o_src:     p++;  opt_mode();  arg0=overlay=*p++;  return;
      case  o_bind:    p++;  opt_mode();  arg0=*p++;  arg1=*p++;  return;
      case  o_cd:      //  fallthrough to o_wd
      case  o_wd:      p++;  arg0=*p++;  return;
      default:         break;  }

    if  ( arg0.startswith("--") )  {
      if  ( type )  {  p++;  return;  }
      else          {  die1 ( "bad option  %s", arg0.s );  }  }
    if  ( arg0.startswith("-") )  {  type=o_shortopt;  p++;  return;  }
    if  ( is_setenv()          )  {  type=o_setenv;    p++;  return;  }

    path_or_command();  }


  void  do_command  ()  {    //  ------------------  Option_Reader  do_command
    assert ( command == nullptr );
    type  =  o_command;  command  |=  p;  };


  bool  is_setenv  ()  {    //  --------------------  Option_Reader  is_setenv
    const char *  var_name_allowed  =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
    const int  n  =  arg0.spn ( var_name_allowed );
    return  n > 0  &&  n < arg0.n()  &&  arg0[n] == '=';  }


  void  mode_path  ()  {    //  --------------------  Option_Reader  mode_path
    //  mode  path
    mode=type;  p++;  path();  }


  void  opt_mode  ()  {    //  ----------------------  Option_Reader  opt_mode
    //  [mode]
    opt  n  =  s2o(p[0]);  switch  ( n )  {
      case  o_ra:  case  o_ro:  case  o_rw:  mode=n;  p++;  break;
      default:  break;  }  }


  void  path  ()  {    //  ------------------------------  Option_Reader  path
    //  path
    arg0  =  * p ++ ;
    if  ( newroot )  {  type  =  overlay  ?  o_partial  :  o_full  ;  }
    else             {  type=o_newroot;  newroot=arg0;  }  }


  void  path_or_command  ()  {    //  --------  Option_Reader  path_or_command
    //  path_or_command
    bool  found  =  Lib::is_dir(  overlay  ?  overlay + "/" + arg0  :  arg0  );
    found  ?  path()  :  do_command();  }


};    //  end  class  Option_Reader  --------------  end  class  Option_Reader




struct  Logic  :  Lib  {    //  xxlo  -------------------------  struct  Logic


  static opt  actual  ( str path, const opt mode )  {    //  --  Logic  actual
    return  (  mode == o_rw
	       ||  is_inside_workdir ( path )
	       ||  (  mode == o_ra
		      &&  (  st.opt_root
			     ||  st.opt_write
			     ||  is_inside_readauto ( path )  )  )
	       ?  o_rw  :  o_ro  );  }


  static void  binds  ( sink<Bind> fn )  {    //  --------------  Logic  binds
    auto  fn2  =  [&]  ( Bind & raw )  {
      if  ( raw .type == o_full )  {  binds_full ( fn, raw );  return;  }
      if  ( is_shadowed ( raw.dst, raw.p ) )  {  return;  }
      mfrag  parent;
      if  ( raw .mode == o_none )  {
	//  raw .trace  ( "binds 1    " );
	calculate_parent ( raw .dst, parent, raw .mode );
	//  raw .trace  ( "binds 2    " );
	;;;  }
      raw .actual  =  actual ( raw.dst, raw.mode );
      fn ( raw );  };
    binds_raw ( fn2 );  }


  static void  calculate_parent    //  --------------  Logic  calculate_parent
  ( str child, mfrag & parent, opt & mode )  {

    //  Find/calculate the path and specified mode of the Bind that
    //  contains path child.  Since each Bind may inherit from its parent,
    //  we must descend to child, one step at a time.

    auto  is_descendant  =  [&]  ( const Bind & raw )  {
      return  child .is_inside ( raw .dst )
	&&  (  parent == nullptr ||  raw.dst.n() > parent.n  );  };

    auto  descend_to  =  [&]  ( const Bind & raw )  {
      parent  =  raw .dst;    //  .dst is eternal becaus .type != o_full
      mode    =  raw .mode  ||  mode;  };

    auto  full  =  [&]  ( const Bind & full )  {
      if  ( parent .n > 0 )  {  return;  }
      frag  basedir  =  child .skip_all('/') .capture_until('/');
      if  ( not is_dir ( ostr(s( full.full, "/", basedir )) ) )  {  return;  }
      if  ( is_shadowed ( basedir, full.p ) )  {  return;  }
      parent  =  basedir;
      mode    =  full.mode  ||  mode  ;  };

    auto  consider  =  [&]  ( const Bind & raw )  {
      if  ( raw .type == o_full              )  {  return  full ( raw );  }
      if  ( is_shadowed   ( raw.dst, raw.p ) )  {  return;  }
      if  ( is_descendant ( raw )            )  {  descend_to ( raw );  }  };

    binds_raw ( consider );  }


  static void  options  ( sink<Option> fn )  {    //  --------  Logic  options
    Option_Reader  r  ( st .argv + 1 );
    while  ( r.next() )  {  fn ( r.o );  }  }


  static void  scandir  ( str path, sink<Dirent> fn )  {    //  Logic  scandir
    DIR *   dirp  =  assert ( opendir ( path.s ) );
    for  (  Dirent entry;  entry = readdir ( dirp );  )  {
      if  (  entry == "."  ||  entry == ".."  )  {  continue;  }
      fn ( entry );  }
    closedir ( dirp );  }


private:


  static void  binds_full    //  --------------------------  Logic  binds_full
  ( sink<Bind> fn, Bind & raw )  {
    raw .mode  =  raw .mode  ||  st.newroot_mode  ||  o_ra  ;
    scandir  (  raw .full,  [&](auto e)  {
      if  ( e.is_dir() )  {
	raw .dst          =  e.name();
	raw .newroot_dst  =  s( st.newroot, "/", raw.dst );
	if  ( is_dir ( raw .newroot_dst ) )  {
	  raw .src     =  s( raw .full,  "/", e.name() );
	  raw .actual  =  actual ( raw.dst, raw.mode );
	  fn ( raw );  }  }  }  );  }


  static void  binds_raw  ( msink<Bind> fn )  {    //  -----  Logic  binds_raw
    Option_Reader  r  ( st .argv + 1 );
    Bind           raw;
    while  ( r.next() )  {
      raw .clear();
      switch  ( r.o.type )  {
	case  o_newroot:    //  fallthrough to o_bind
	case  o_full:       //  fallthrough to o_bind
	case  o_partial:    //  fallthrough to o_bind
	case  o_bind:       fn ( raw.set(r.o) );  break;
	default:  break;  }  }  }


  static bool  is_inside_readauto    //  ----------  Logic  is_inside_readauto
  ( str path )  {
    return  path .is_inside ( st .env .get("HOME") )
      ||    path .is_inside ( "/tmp" )
      ||    path .is_inside ( "/var/tmp" );  }


  static bool  is_inside_workdir    //  ------------  Logic  is_inside_workdir
  ( str path )  {
    bool  rv  =  false;
    options (  [&](auto o)  {
      rv  |=  o.type == o_wd  &&  path .is_inside ( o.arg0 );  }  );
    return  rv;  }


  static bool  is_shadowed    //  ------------------------  Logic  is_shadowed
  ( frag dst, Argv p )  {
    //  20210620  todo  implement
    //  trace1 ( "is_shadowed  %d  '%s'", dst.n, ostr(dst).s );
    //  20210624  returning false is acceptable for now.
    return  false;  }


};    //  end  struct  Logic  ----------------------------  end  struct  Logic


Logic  q;    //  -------------------------------------------  global  Logic  q




struct  Init_Tool  {    //  xxin  -------------------------  struct  Init_Tool


  static void  process  ( const Option & a )  {    //  ---  Init_Tool  process

    switch ( a.type )  {

      case  o_cd:         cd(a);                            break;
      case  o_command:    command(a);                       break;
      case  o_env:        mut .opt_env       =  o_env;      break;
      case  o_full:       mut .guestname     =  a.arg0;     break;
      case  o_help:       Lib :: help_print();              break;
      case  o_help_more:  Lib :: help_more_print();         break;
      case  o_network:    mut .opt_network   =  o_network;  break;
      case  o_newroot:    mut .newroot_mode  =  a.mode;
	;;;               mut .newroot       =  a.arg0;     break;
      case  o_pulse:      mut .opt_pulse     =  o_pulse;    break;
      case  o_root:       mut .opt_root      =  o_root;     break;
      case  o_src:        mut .guestname     =  a.arg0;     break;
      case  o_trace:      global_opt_trace   =  o_trace;    break;
      case  o_version:    version();                        break;
      case  o_wd:         mut .workdir       =  a.arg0;     break;
      case  o_write:      mut .opt_write     =  o_write;    break;
      case  o_x11:        mut .opt_x11       =  o_x11;      break;
      case  o_shortopt:   shortopt(a);                      break;

      case  o_bind:       break;
      case  o_dashdash:   break;
      case  o_partial:    break;
      case  o_setenv:     break;

      default:  die1 ( "init_tool  bad option  %d  %s",
		       a.type,  o2s(a.type) )  break;  }  }


private:


  static void  cd  ( const Option & a )  {    //  -------------  Init_Tool  cd
    if  ( st .chdir )  {  die1 ( "extra cd option  %s", a.arg0.s );  }
    mut .chdir  =  a.arg0;  }


  static void  command  ( const Option & a )  {    //  ---  Init_Tool  command

    /*  20210626  obsolete
    if  (  st.newroot == nullptr  &&  a.arg0  )  {
      printe ( "lxroot  directory not found  %s", a.arg0.s );
      exit ( 1 );  }
    */

    mut .command  =  a.command;  }


  static void  shortopt  ( const Option & a )  {    //  -  Init_Tool  shortopt
    for  (  mstr p = a.arg0.s+1  ;  *p  ;  p++  )  {
      switch  ( *p )  {
	case 'e':  mut .opt_env      =  o_env;      break;
	case 'n':  mut .opt_network  =  o_network;  break;
	case 'r':  mut .opt_root     =  o_root;     break;
	case 'w':  mut .opt_write    =  o_write;    break;
	case 'x':  mut .opt_x11      =  o_x11;      break;
	default:  die1 ( "bad short option  '%c'", *p );  break;  }  }  }


  static  void  version  ()  {    //  --------------------  Init_Tool  version
    printe ( "lxroot  version  " LXROOT_VERSION "\n" );
    exit ( 0 );  }


};    //  end  struct  Init_Tool  --------------------  end  struct  Init_Tool




struct  Env_Tool  :  Lib  {  //  xxen  ---------------------  struct  Env_Tool


  static void  option  ( const Option & o )  {    //  ------  Env_Tool  option
    if  ( o.type == o_setenv )  { mut .env .set ( o.arg0 );  }  }


  static void  essential  ()  {    //  ------------------  Env_Tool  essential

    mut .env .soft ( "PATH=/usr/local/bin:/usr/local/sbin:"
		     "/usr/bin:/usr/sbin:""/bin:""/sbin" );

    const Fgetpwent &  in   =  st .inside;
    const Fgetpwent &  out  =  st .outside;

    mut .env .soft ( "HOME",    in.dir  || getenv("HOME")    || out.dir  );
    mut .env .soft ( "LOGNAME", in.name || getenv("LOGNAME") || out.name );
    mut .env .soft ( "USER",    in.name || getenv("USER")    || out.name );

    mut .env .soft_copy ( "TERM" );  }


  static void  shell  ()  {    //  --------------------------  Env_Tool  shell

    auto  shell  =  [&]  ( str path )  {
      //  20210623
      //  trace1 ( "shell  %s", path.s );
      if  ( is_file ( path )  &&  access ( path.s, X_OK ) == 0 )  {
	mut .env .set ( "SHELL", path );  return  true;  }
      return  false;  };

    st .env .get ( "SHELL" )
      ||  shell ( st .inside .shell )
      ||  shell ( getenv("SHELL") )
      ||  shell ( st .outside .shell )
      ||  shell ( "/bin/ash" )
      ||  shell ( "/bin/sh" );  }


  static void  argv  ()  {    //  ----------------------------  Env_Tool  argv
    if  ( st.command[0] )  {  return;  }
    str  shell  =  st .env .get ( "SHELL" );
    if  ( shell == "/bin/bash" )  {  mut .command  =  bash_command;  }
    else if  ( shell )  {
      bash_command[0]    =  shell.s;
      bash_command[1]    =  nullptr;
      mut .command  =  bash_command;  }
    else  {  die1 ( "please specify a command" );  }  }


  static void  ps1_bash  ()  {    //  --------------------  Env_Tool  ps1_bash

    str   term  =  st .env .get ( "TERM" );
    str   user  =  st .env .get ( "USER" );
    ostr  opts  =  nullptr;
    ostr  host  =  "./" + ( st .guestname || st .newroot ) .basename()  ;

    if  (  st    .opt_network  == o_network
	   ||  st.opt_root     == o_root
	   ||  st.opt_x11      == o_x11  )  {
      opts  =  "-";
      if  ( st.opt_env     == o_env     )  {  opts  +=  "e";  }
      if  ( st.opt_network == o_network )  {  opts  +=  "n";  }
      if  ( st.opt_root    == o_root    )  {  opts  +=  "r";  }
      if  ( st.opt_write   == o_write   )  {  opts  +=  "w";  }
      if  ( st.opt_x11     == o_x11     )  {  opts  +=  "x";  }  }

    ostr  ps1  =  "PS1=";

    if  ( term  ==  "xterm-256color" )  {    //  set terminal title
      ps1  +=  str("\\[\\e]0;") + user + "  ";
      if  ( opts )  {  ps1  +=  opts  +  "  ";  }
      ps1  +=  host  +  "  \\W\\007\\]";  }

    //r  bright_cyan  =  "\\[\\e[0;96m\\]";
    str  bright_red   =  "\\[\\e[0;91m\\]";
    str  cyan         =  "\\[\\e[0;36m\\]";
    //r  red          =  "\\[\\e[0;31m\\]";
    str  normal       =  "\\[\\e[0;39m\\]";

    ps1  +=  str("\n") + cyan + user + "  ";
    if  ( opts )  {  ps1  +=  bright_red + opts + cyan + "  ";  }
    ps1  +=  host + "  \\W" + normal + "  ";

    mut .env .set ( leak ( ps1 ) );  }


  static bool  is_busybox  ( str path )  {    //  ------  Env_Tool  is_busybox
    return  is_link ( path )  &&  readlink ( path ) == "/bin/busybox";  }


  static void  ps1  ()  {    //  ------------------------------  Env_Tool  ps1
    //  20200626  todo?  add custom prompts for other shells
    if  (  st.command[0] == "/bin/bash"
	   ||  is_busybox ( st.command[0] )  )  {  ps1_bash();  }  }


  static void  passthru  ()  {    //  --------------------  Env_Tool  passthru
    if  ( st.opt_env == o_env )  {
      for  (  Argv p (environ)  ;  * p  ;  p ++  )  {
	trace1 ( "env_tool  passthru  %s", p[0].s );
	mut .env .soft ( * p );  }  }  }


public:


  static void  before_pivot  ()  {    //  ------------  Env_Tool  before_pivot
    q.options ( option );
    essential();  }    //  Lxroot :: expose() depends on $HOME


  static void  after_pivot  ()  {    //  --------------  Env_Tool  after_pivot
    shell();     //
    argv();      //  depends on  env.SHELL
    ps1();       //  depends on  command[0]
    passthru();  }


};    //  end  class  Env_Tool  ------------------------  end  class  Env_Tool




class  Lxroot  :  Lib  {    //  xxlx  -------------------------  class  Lxroot


  ostr  put_old;


  void  init  ()  {    //  -------------------------------------  Lxroot  init
    q.options ( Init_Tool :: process );
    //  20210626  obsolete
    //  if  ( st.newroot == nullptr )  {  help_print ( 1 );  }  }
    return;  }


  void  unshare  ()  {    //  -------------------------------  Lxroot  unshare
    int  clone_flags  =  0;
    clone_flags  |=  CLONE_NEWNS;      //  mount namespace  ( optional )
    clone_flags  |=  CLONE_NEWPID;     //  pid   namespace  ( optional )
    clone_flags  |=  CLONE_NEWUSER;    //  user  namespace  ( required )
    clone_flags  |=  st.opt_network == o_network  ?  0  :  CLONE_NEWNET  ;
    trace1 ( "" );
    sys .unshare ( clone_flags );  }


  void  uid_map  ()  {    //  -------------------------------  Lxroot  uid_map

    //  see  https://lwn.net/Articles/532593/

    uid_t  un_uid  =  st.opt_root == o_root  ?  0  :  st.uid  ;
    gid_t  un_gid  =  st.opt_root == o_root  ?  0  :  st.gid  ;

    char  u_map[80];
    char  g_map[80];
    int   fd;

    snprintf ( u_map, sizeof u_map, "%u %u 1\n", un_uid, st.uid );
    snprintf ( g_map, sizeof g_map, "%u %u 1\n", un_gid, st.gid );

    trace1 ( "  uid_map  %u %u 1  deny  %u %u 1",
	     un_uid,  st.uid,  un_gid,  st.gid );

    sys .open   (  & fd,  "/proc/self/uid_map",  O_RDWR    );
    sys .write  (  fd,    u_map,  strlen ( u_map )         );
    sys .close  (  fd                                      );
    sys .open   (  & fd,  "/proc/self/setgroups",  O_RDWR  );
    sys .write  (  fd,    "deny", 4                        );
    sys .close  (  fd                                      );
    sys .open   (  & fd,  "/proc/self/gid_map",  O_RDWR    );
    sys .write  (  fd,     g_map, strlen ( g_map )         );
    sys .close  (  fd                                      );  }


  void  bind  ()  {    //  -------------------------------------  Lxroot  bind
    if  ( st.newroot == nullptr )  {  return;  }
    q.binds (  [](auto b)  {
      sys .bind ( b.newroot_dst, b.src );  }  );
    //  note  /proc is mounted later on in Lxroot :: proc().
    //  note  someday I may integrate /dev and /sys into q.bind
    sys .bind ( st.newroot + "/dev", "/dev" );
    sys .bind ( st.newroot + "/sys", "/sys" );  }


  void  fgetpwent  ()  {    //  ---------------------------  Lxroot  fgetpwent
    mut .outside .fgetpwent ( "/etc/passwd", st .uid );
    mut .inside  .fgetpwent ( st.newroot + "/etc/passwd", getuid() );
    Env_Tool :: before_pivot();  }


  void  env  ()  {  Env_Tool :: after_pivot();  }    //  --------  Lxroot  env


  static void  expose_path    //  -----------------------  Lxroot  expose_path
  ( str path, opt readauto = o_none )  {
    if  (  st.newroot == nullptr  )  {  return;  }
    if  (  path == nullptr        )  {  return;  }
    mfrag  parent;
    opt    mode;
    q.calculate_parent ( path, parent, mode );
    ostr   parent2 ( parent );
    if  (  path .is_same_path_as ( parent2 ) )  {  return;  }
    if  (  q.actual ( parent2, mode ) == o_rw  )  {  return;  }
    if  (  readauto == o_none  ||  mode == o_ra  )  {
      ostr  newroot_path  =  s( st.newroot, path );
      if  ( is_dir ( newroot_path ) )  {
	trace1 ( "  expose   '%s'", newroot_path.s );
	sys .bind ( newroot_path, newroot_path );  }  }  }


  void  expose  ()  {    //  ---------------------------------  Lxroot  expose
    expose_path ( st .env .get ( "HOME" ), o_ra );
    expose_path ( "/tmp", o_ra );
    expose_path ( "/var/tmp", o_ra );
    q.options  (  [](auto o)  {
      if  ( o.type == o_wd )  {  expose_path ( o.arg0 );  }  }  );  }


  void  remount  ()  {    //  -------------------------------  Lxroot  remount
    q.binds  (  [&](auto b)  {
      if ( b.actual == o_ro )  { sys .rdonly ( b.newroot_dst );  }  }  );  }


  void  option_pulse  ()  {    //  ---------------------  Lxroot  option_pulse
    if  ( st .opt_pulse == o_pulse )  {
      str  xdg_dir  =  getenv ( "XDG_RUNTIME_DIR" );
      if  ( xdg_dir )  {
	mut .env .soft_copy ( "XDG_RUNTIME_DIR" );
        ostr  pulse_dir  =  xdg_dir + "/pulse";
        sys .bind ( st .newroot + pulse_dir, pulse_dir );  }
      else  {
        warn ( "bind_opt_pulse()  XDG_RUNTIME_DIR not set" );  }  }  }


  void  option_x11  ()  {    //  -------------------------  Lxroot  option_x11
    if  ( st .opt_x11 == o_x11 )  {
      sys .bind ( st.newroot + "/tmp/.X11-unix", "/tmp/.X11-unix" );
      mut .env .soft_copy ("DISPLAY");  }  }


  void  options  ()  {    //  -------------------------------  Lxroot  options
    option_pulse();
    option_x11();  }


  void  pivot_prepare  ( str pivot )  {    //  --------  Lxroot  pivot_prepare

    //  20210626  obsolete
    //if( pivot == nullptr )  {  die1 ( "pivot_preapre  pivot is nullptr" );  }


    //  verify that pivot has at least one sub-direcotry (for put_old)
    assert ( put_old == nullptr );
    q.scandir  (  pivot,  [&](auto e)  {
      if  (  put_old == nullptr  &&  e.is_dir()  )  {
	put_old  =  s( "/", e.name() );  }  }  );
    if  ( put_old == nullptr )  {
      die1 ( "pivot_prepare  pivot contains no directories" );  }  }


  void  pivot  ()  {    //  -----------------------------------  Lxroot  pivot
    if  ( st.newroot == nullptr )  {  return;  }
    pivot_prepare ( st.newroot );
    sys .pivot  ( st.newroot,  st.newroot + put_old );
    sys .chdir  ( "/" );
    sys .chroot ( "/" );  }


  void  fork  ()  {    //  -------------------------------------  Lxroot  fork
    sys .fork();
    if  ( sys .fork_pid == 0 )  {  return;  }    //  child returns
    if  ( sys .fork_pid >  0 )  {    //  parent waits for child to exit
      sys .wait();
      if  ( WIFEXITED ( sys .wstatus ) )  {
	exit ( WEXITSTATUS ( sys .wstatus ) );  }
      printe ( "lxroot  warning  child exited abnormally\n" );
      exit ( 1 );  }  }


  void  proc  ()  {    //  -------------------------------------  Lxroot  proc

    //  We try to do as much as possible before calling fork().  Howewer...
    //  It seems /proc can only be mounted:
    //    1) after fork() (which sort of makes sense), and
    //    2) before unmounting put_old (which makes sense for security).

    //  Therefore, we mount() /proc and umount2() put_old here, in the
    //  child process, to ensure both calls finish before execve() is
    //  called.

    //  20201114  Contrarywise:  Forking as early as possible would
    //  free all memory used by the child.

    //  20201213  fork()ing early complicates debugging with gdb.
    //  Perhaps I should implement fork()ing both early and late?

    if  ( st.newroot )  {  sys .mount ( "proc", "/proc", "proc" );  }  }


  void  umount2  ()  {    //  -------------------------------  Lxroot  umount2
    if  ( st.newroot )  {  sys .umount2 ( put_old.s, MNT_DETACH );  }  }


  void  chdir  ()  {    //  -----------------------------------  Lxroot  chdir

    //  20210518  disabled during overhaul.  todo  re-implement ?
    //  if  (  st.new_cwd  )  {  sys .chdir ( st.new_cwd );  return;  }

    if  (  st .chdir    )  {  sys .chdir ( st .chdir   );  return;  }
    if  (  st .workdir  )  {  sys .chdir ( st .workdir );  return;  }
    str  home  =  st .env .get ( "HOME" );
    if  (  st .newroot  &&  is_dir ( home )  )  { sys .chdir ( home );  }  }


  void  xray  ()  {    //  -------------------------------------  Lxroot  xray

    if  ( global_opt_trace == o_trace )  {

      printe ( "\n" );
      printe ( "xray  uid  %d  %d\n", getuid(), getgid() );
      printe ( "xray  cwd  %s\n", getcwd().s );
      printe ( "xray  environment\n" );
      Argv ( st .env .data() ) .print ( "xray    env  " );
      printe ( "xray  command\n" );
      st .command .print ( "xray    cmd  " );

      printe ( "\n" );
      printe ( "xray  binds\n" );
      q.binds  (  [](auto b)  { b.trace ( "  " ); }  );

      printe ( "\n" );
      //  exit ( 2 );
      return;  }  }


  void  exec  ()  {    //  -------------------------------------  Lxroot  exec
    sys .execve ( st.command[0], st.command, st.env.data() );  }


public:


  int  main  ()  {    //  --------------------------------------  Lxroot  main
    init();          //  first
    unshare();       //  before uid_map and bind
    uid_map();       //  after unshare
    bind();          //  after unshare
    fgetpwent();     //  after bind, before expose
    expose();        //  after fgetpwent, before remount
    remount();       //  after expose
    options();       //  late, before pivot, after remount
    /*  pivot to umount2 should be as tight as possible  */
    pivot();         //  late, but before fork? ( !! all paths change !! )
    fork();          //  prefer after pivot (so that parent is not in put_old)
    proc();          //  after fork, before umount2
    umount2();       //  after proc
    /*  pivot to umount2 should be as tight as possible  */
    env();           //  after pivot, therefore after umount2
    chdir();         //  after umount2 ( because put_old may shadow $HOME !! )
    xray();
    exec();          //  last
    return  1;  }    //  exec failed!


};    //  end  class  Lxroot  ----------------------------  end  class  Lxroot




void  unit_test  ()  {    //  -------------------------------------  unit_test
  mstr :: unit_test();
  ostr :: unit_test();
  Argv :: unit_test();  }


int  main  ( const int argc, const char * const argv[] )  {    //  -----  main
  unit_test();
  if  ( argc < 2 )  {  Lib :: help_print();  }
  mut .argv  =  argv;
  return  Lxroot() .main();  }




//  lxroot.cpp  -  Create and use chroot-style virtual software environments.
//
//  Copyright (c) 2021 Parke Bostrom, parke.nexus at gmail.com
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
