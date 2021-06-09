

//  lxroot.cpp  -  Create and use chroot-style virtual software environments.
//  Copyright (c) 2021 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.


#define  LXROOT_VERSION  "0.0.20210606"


const char *  help  =    //  xxhe  -------------------------------------  help
"\n"
"usage:  lxroot  [mode] newroot  [options]  [--]  [command [arg ...] ]\n\n"

"options\n"
"  -short                      one or more short options\n"
"  --long-option               a long option\n"
"  n=v                         set an environment variable\n"
"  [mode]  newroot             set and bind the newroot\n"
"  [mode]  path                bind a full or partial overlay\n"
"  'src'   [mode]  path        set the source for partial overlays\n"
"  'bind'  [mode]  dst  src    bind src to newroot/dst\n"
"  --                          end of options, command follows\n"
"  command  [arg ...]          command\n"
"";    //  end  help  ---------------------------------------------  end  help


const char *  help2  =    //  xxhe  -----------------------------------  help2
"\n"
"  For more help, please run:  lxroot  --help-more\n"
"";    //  end  help2  -------------------------------------------  end  help2


//  short options  ( for plannig purposes only, possibly inaccurate )
//  .           .           .           .           .           .
//  a           f hostfs(?) k           p PID(?)    u UID(?)    z
//  b           g           l           q           v verbose
//  c           h hostname? m MOUNT(?)  r root      w write
//  d dbus(?)   i           n NET       s /sys(?)   x x11
//  e env(?)    j           o           t           y
//
//  A           F           K           P           U           Z
//  B           G           L           Q           V
//  C           H           M           R           W
//  D           I           N           S           X
//  E           J           O           T           Y
//
//  long options  ( for plannig purposes only, possibly inaccurate )
//  dbus  env  help  help-more  network  pid  pulseaudio  root  trace  uid
//  verbose  version  write  x11


const char *  help_more  =  //  xxlo  -----------------------------  help_more

"\nMODES\n\n"

"  ra    read-auto  (default, described below)\n"
"  ro    read-only  (bind mount with MS_RDONLY)\n"
"  rw    read-write (bind mount without MS_RDONLY)\n\n"

"SHORT OPTIONS\n\n"

"  n    allow network access (CLONE_NEWNET = 0)\n"
"  r    simulate root user (map uid and gid to zero)\n"
"  w    allow full write access to all read-auto binds\n"
"  x    allow X11 access (bind /tmp/.X11-unix and set $DISPLAY)\n\n"

"LONG OPTIONS\n\n"

//  "  --env           import all environment variables\n"
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
"write access to the path, while (b) granting a non-simulated-root user\n"
"write access only to $HOME and /tmp.  Or, stated precisely:\n\n"

"If any of -r, -w, --root or --write are specified, then:\n"
"Each read-auto path will be bind mounted in read-write mode.\n\n"

"Otherwise:\n"
"A read-auto path inside  $HOME or /tmp will be bind mounted read-write.\n"
"A read-auto path outised $HOME or /tmp will be bind mounted read-only.\n\n"

"Furhermore:\n"

"If $HOME and/or /tmp is a descendant of a read-auto bind, then $HOME\n"
"and/or /tmp (respectively) will be bind mounted in read-write mode.\n"
"In this case, two or three bind mounts occur.  First, the path will be\n"
"bind mounted read-only.  And then $HOME and/or /tmp will be bind\n"
"mounted read-write.\n\n"

"NEWROOT\n\n"

"Note that the newroot, full-overlay, and partial-overlay options all\n"
"have the same form, namely:  [mode]  path\n\n"

"The first option of this form is the newroot option.  The newroot\n"
"option specfies the newroot.\n\n"

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
//  The classes and structs in lxroot can be divided into five categories:
//
//    Low level data storage classes
//    Convenience wrappers
//    Two Processor interfaces
//    Five Processor implementations
//    Two other classes
//
//  Low level data storage classes.
//
//  enum     opt       an enumeration that represents various options.
//  struct   mfrag     a mutable string fragment (const char * and length).
//  typedef  frag      a const mfrag.
//  struct   mstr      a mutable string (const char *, null terminated).
//  typedef  str       a const mstr.
//  struct   Concat_2  assists with string concatination.
//  struct   ostr      an appendable string that owns its memory.
//  struct   Concat    deprecated by Concat_2.
//  struct   Argv      a convenience and saftey wrapper around const char * *.
//  struct   Option    represents a parsed command line option.
//  struct   Bind      represents (and specifies) a potential bind mount.
//  struct   Env       a list that specifies the new environment.
//  struct   State     contains shared, mutable, global variables.
//
//  Convenience wrappers.
//
//  struct  Lib        contains generally useful functions.
//  struct  Fgetpwent  parse /etc/passwd.
//  class   Opendir    provides iteration over the contents of a directory.
//  struct  Syscall    provides error handling and tracing of syscalls.
//
//  Lxroot uses an iteration oriented design.  The input to lxroot is
//  argv, the sequerce of command line arguments.  Option_Reader
//  parses argv into a sequence of Options.  Each Option generates a
//  sequence of zero or more Binds.
//
//  In other words, argv specifies both (a) a sequence of Options and
//  (b) a sequence of Binds.
//
//  Lxroot is implemented as a sequence of five linear passes.  Each
//  of these five passes performs a single linear iteration over
//  either (a) the sequence of Options or (b) the sequence of Binds.
//
//  Two Processor interfaces.
//
//  struct  Option_Processor  an interface used to iterate over Options
//  struct  Bind_Processor    an interface used to iterate over Binds
//
//  Five Processor implementations.
//
//  struct  Init_Tool  pass 1 - records certain Options in State.
//  struct  Binder     pass 2 - performs bind mounts.
//  struct  Env_Tool   pass 3 - configures the new envirnoment variables.
//  struct  Exposer    pass 4 - performs extra bind mounts, if needed.
//  struct  Remounter  pass 5 - performs readonly remounts, as needed.
//
//  Two other classes:
//
//  struct  Option_Reader  parses Argv into Options, one at a time.
//  class   Lxroot         the main class manages all the others.
//
//  If you wish to read Lxroot's source code, I recommend starting at
//  the bottom of lxroot.cpp with class Lxroot (the highest level
//  class), and then reading deeper into the other (lower level)
//  classes as needed.


const char *  bash_command[]  =  { "/bin/bash", "--norc", nullptr };


#include  <dirent.h>         //  Opendir
#include  <errno.h>          //
#include  <fcntl.h>          //
#include  <limits.h>         //
#include  <pwd.h>            //
#include  <sched.h>          //
#include  <signal.h>         //  raise(SIGINT), used with gdb
#include  <stdio.h>          //  printf, etc.
#include  <stdlib.h>         //
#include  <string.h>         //  strlen, etc.
#include  <unistd.h>         //
#include  <sys/mount.h>      //  mount
#include  <sys/stat.h>       //  stat
#include  <sys/syscall.h>    //  pivot
#include  <sys/types.h>      //  Opendir
#include  <sys/wait.h>       //  wait

#include  <vector>           //  class Env .vec


//  macro  printe  --------------------------------------------  macro  printe
#define  printe( ... )  fprintf ( stderr, __VA_ARGS__ );


//  macro  die_pe  --------------------------------------------  macro  die_pe
//  see  https://stackoverflow.com/q/5588855   regarding ##__VA_ARGS__
//  see  https://stackoverflow.com/a/11172679  regarding ##__VA_ARGS__
#define  die_pe( format, ... )  {					\
  fprintf ( stderr, "lxroot  error  " format "  ", ##__VA_ARGS__ );	\
  perror ( NULL );							\
  exit ( 1 );  }


//  macro  die2  ------------------------------------------------  macro  die2
#define  die2( format, ... )  {						\
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
  o_bind,  o_dashdash,  o_ra,  o_ro,  o_rw,  o_src,
  /*  non-literal option types  */
  o_command,  o_full,  o_newroot,  o_partial,  o_setenv,  o_shortopt,
  /*  literal long options  */
  o_env,    o_help,    o_help_more,    o_network,    o_pulse,
  o_root,    o_trace,    o_version,    o_write,    o_x11,
  };


const char * const  opt_name[]  =  {    //  ------------------------  opt_name
  "0",
  /*  literal arg types  */
  "bind",  "--",        "ra",  "ro",  "rw",  "src",
  /*  non-literal option types  */
  "command",  "full",  "newroot",  "partial",  "setenv",  "shortopt",
  /*  literal long options  */
  "--env",  "--help",  "--help-more",  "--network",  "--pulseaudio",
  "--root",  "--trace",  "--version",  "--write",  "--x11",
  nullptr  };


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
class    ostr;      //  --------------------------------  declare  class  ostr
class    Concat;    //  ------------------------------  declare  class  Concat
typedef  const mfrag  frag;    //  xxfr  ----------------------  typedef  frag




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


  frag  operator ||  ( frag & o )  const  {    //  -------------  mfrag  op ||
    return  s  ?  * this  :  o  ;  }


  Concat  operator +  ( frag & o )  const;    //  ---------------  mfrag  op +


};    //  end  struct  mfrag  ----------------------------  end  struct  mfrag




const char *  leak  ( frag o )  {    //  ---------------------  function  leak
  //  lxroot quickly exec()s on success or exit()s on failure.
  //  therefore, a few convenient and minor memory leaks are acceptable.
  char *  rv  =  (char*) malloc  ( o.n + 1 );    //  intentional leak.
  if  ( rv )  {
    memcpy ( rv, o.s, o.n );
    rv [ o.n ]  =  '\0';  }
  return  rv;  }




struct   mstr;    //  ---------------------------------  declare  struct  mstr
typedef  const mstr  str;    //  xxst  -------------------------  typedef  str


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


  frag  operator ||  ( const char * o )  const  {    //  --------  mstr  op ||
    return  s  ?  * this  :  o  ;  }


  frag  operator ||  ( frag o )  const  {    //  ----------------  mstr  op ||
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


  const void *  chr  ( const int c )  const  {    //  -------------  mstr  chr
    return  s  ?  :: strchr ( s, c )  :  nullptr  ;  }


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


  int  n  ()  const  {    //  ---------------------------------------  mstr  n
    return  s  ?  strlen ( s )  :  0  ;  }


  str  skip  ( int n )  const  {    //  --------------------------  mstr  skip
    const char *  p  =  s;
    while  ( n-- > 0 )  {
      if  (  p  &&  * p )  {  p++; }
      else  {  return  nullptr;  }  }
    return  p;  }


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
  str s[3];
  Concat_2  ( str a, str b = 0, str c = 0 )  :  s{a,b,c}  {}
};    //  end  struct  Concat_2  ----------------------  end  struct  Concat_2




Concat_2  s  ( str a, str b = 0, str c = 0 )  {    //  ---- global function  s
  return  Concat_2 ( a, b, c );  }


opt  s2o  ( str s )  {    //  --------------------------  global function  s2o
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


  void  operator =  ( frag  o        )  {  n=0;  * this += o;   }    //  -----
  void  operator =  ( str   s        )  {  * this  =  frag(s);  }    //  -----
  void  operator =  ( char       * s )  {  * this  =  frag(s);  }    //  -----
  void  operator =  ( const char * s )  {  * this  =  frag(s);  }    //  -----


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


Concat  frag :: operator +  ( frag & o )  const  {    //  --------  frag  op +
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




struct  Lib  {    //  xxli  -------------------------------------  struct  Lib


  //  20210530  apparent redundancy:  assert_is_dir  vs  directory_require


  static void  assert_is_dir    //  ----------------------  Lib  assert_is_dir
  ( const char * const path, const char * const m )  {
    if  ( is_dir ( path ) )  {  return;  }
    printe ( "lxroot  %s  directory not found  %s\n", m, path  );
    exit ( 1 );  }


  static void  directory_require     //  -------------  Lib  directory_require
  ( str path, str m )  {
    if  ( Lib :: is_dir ( path ) )  {  return;  }
    die2 ( "%s directory not found\n  %s", m.s, path.s );  }


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




class  Opendir  {    //  xxop  -------------------------------  class  Opendir


  //  Opendir is a ranged-for wrapper around opendir() and readdir().
  //  usage:  for  ( const Opendir & entry : Opendir ( path ) )  {}


  DIR *            dirp  =  nullptr;
  struct dirent *  entp  =  nullptr;


  struct  Iterator  {    //  ----------------------  struct  Opendir  Iterator
    Opendir *  p;
    Iterator ( Opendir * p )  :  p(p)  {}
    bool  operator !=  ( const Iterator & end )  const  { return  p != end.p; }
    void  operator ++  ()  {  p->readdir();  p = p->entp ? p : nullptr ;  }
    const Opendir &  operator *  ()  const  {  return  * p;  }  };


  void  readdir  ()  {    //  ------------------------------  Opendir  readdir
    entp  =  :: readdir ( dirp );  }


public:


  Opendir  ( str path )  {  dirp  =  opendir ( path.s );  readdir();  }
  ~Opendir  ()  {  if  ( dirp )  {  closedir ( dirp );  }  }
  Iterator  begin  ()  {  return  Iterator ( this );  }
  Iterator  end    ()  {  return  nullptr;  }


  bool  operator  ==  ( str o )  const  {    //  -------------  Opendir  op ==
    return  name().s  &&  o.s  &&  strcmp ( name().s, o.s ) == 0;  }


  ino_t  inode  ()  const  {    //  --------------------------  Opendir  inode
    return  entp  ?  entp -> d_ino  :  -1  ;  }


  bool  is_dir  ()  const  {    //  -------------------------  Opendir  is_dir
    switch  ( assert ( entp ) -> d_type )  {
      case  DT_DIR:  return  true;  break;
      case  DT_UNKNOWN:
	printe ( "opendir  is_dir  error  type is dt_unknown\n" );
	exit ( 1 );  break;  }
    return  false;  }


  str  name  ()  const  {    //  ------------------------------  Opendir  name
    return  entp  ?  entp -> d_name  :  nullptr  ;  }


};    //  end  class  Opendir  ---------------------------  end  clas  Opendir




//  macro  trace  ----------------------------------------------  macro  trace
#define  trace( format, ... )  {				\
  if  ( global_opt_trace == o_trace )  {			\
    fprintf ( stderr, format "\n",				\
              ##__VA_ARGS__ );  }  }


//  macro  try_quiet  --------------------------------------  macro  try_quiet
#define  try_quiet( function, format, ... )  {			\
  if  ( (function) ( __VA_ARGS__ ) == 0 )  {  return;  }	\
  else  {  die_pe ( format, ##__VA_ARGS__ );  }  }


//  macro  try1  ------------------------------------------------  macro  try1
#define  try1( function, format, ... )  {	       		\
  trace ( format, ##__VA_ARGS__ );				\
  try_quiet ( function, format, ##__VA_ARGS__ );  }




struct  Syscall  {    //  xxsy  -----------------------------  struct  Syscall

  //  note  all Syscall methods call exit(1) on error.


  typedef  unsigned long  flags_t;

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
    trace ( "  bind     '%s'  '%s'", target.s, source.s );
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

    trace ( "  execve   %s  %s\n", pathname.s, argv .concat().s );
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
    trace ( "  exit     %d", status );  :: exit ( status );  }


  void  fork  ()  {    //  ------------------------------------  Syscall  fork
    if  ( fork_pid != -2 )  {  die_pe ( "extra fork?" );  }
    if  ( ( fork_pid = :: fork() ) >= 0 )  {
      trace ( "  fork     (fork returned %d)", fork_pid );
      return;  }
    die_pe ( "fork" );  }


  void  mount  ( str source, str target, str filesystemtype )  {    //   mount
    Lib :: directory_require ( target, "target" );
    trace ( "  mount    %s  %s  %s", source.s, target.s, filesystemtype.s );
    if  ( :: mount ( source.s, target.s, filesystemtype.s, 0, 0 ) == 0 )  {
      return;  }
    die_pe ( "mount  %s  %s  %s",  source.s, target.s, filesystemtype.s );  }


  void  open  ( int * fd, str pathname, const int flags )  {    //  ----  open
    if  ( ( * fd = :: open ( pathname.s, flags ) ) >= 0 )  {  return;  }
    die_pe ( "open  %s  %d", pathname.s, flags );  }


  void  pivot  ( str new_root, str put_old )  {   //  --------  Syscall  pivot
    trace ( "  pivot    %s  %s", new_root.s, put_old.s );
    if  ( syscall ( SYS_pivot_root, new_root.s, put_old.s ) == 0 )  { return; }
    die_pe ( "pivot  %s  %s", new_root.s, put_old.s );  }


  void  rdonly  ( str target )  {    //  --------------------  Syscall  rdonly
    trace ( "  rdonly   %s", target.s );
    const flags_t  flags  =  MS_BIND | MS_REMOUNT | MS_RDONLY;
    const int      rv     =  :: mount ( NULL, target.s, NULL, flags, NULL );
    if  ( rv == 0 )  {  return;  }
    die_pe ( "rdonly  %s\n", target.s );  }


  void  umount2  ( str target, int flags )  {   //  --------  Syscall  umount2
    try1( :: umount2, "  umount2  %s  0x%x", target.s, flags );  }


  void  unshare  ( const int flags )  {    //  -------------  Syscall  unshare
    try1( :: unshare, "  unshare  0x%08x", flags );  }


  pid_t  wait  ()  {    //  -----------------------------------  Syscall  wait
    trace ( "  wait     (parent calls wait)" );
    pid_t  pid  =  :: wait ( & wstatus );
    if  ( pid > 0 )  {
      trace ( "  wait     wait returned  pid %d  status 0x%x",
	      pid, wstatus );
      return  pid;  }
    die_pe ( "wait" );  return  -1;  }


  void  write  ( int fd, const void * buf, ssize_t count )  {    //  --  write
    assert ( count >= 0 );
    if  ( :: write ( fd, buf, count ) == count )  {  return;  }
    die_pe ( "write  %d  %ld", fd, count );  }


};    //  end  struct  Syscall  ------------------------  end  struct  Syscall




Syscall  sys;




struct  Option  {    //  xxop  -------------------------------  struct  Option


  opt   type  =  o_none;    //  see the below input/type chart for details.
  opt   mode  =  o_none;    //  one of:  o_none  o_ra  o_ro  o_rw
  mstr  arg0,  arg1;
  Argv  command;            //  nullptr, then set to command when found


  operator bool  ()  const  {  return  type;  }    //  ---  Option  cast  bool


  void  print  ( str m )  const  {    //  ---------------------  Option  print
    printe ( "%-8s  %-7s  %s\n", m.s, o2s(type), arg0.s );  }


};    //  end  struct  Option  --------------------------  end  struct  Option




class  Option_Reader  :  Option  {    //  xxop  --------  class  Option_Reader


  Argv  p;


public:


  const Option &  a;
  mstr            newroot, overlay;


  Option_Reader  ( Argv p )  :  p(p),  a(*this)  {}    //  -------------  ctor


  const Option &  next  ()  {  next_impl();  return  a;  }    //  ------  next


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
      case  o_ra:      /*  fallthrough to o_rw  */
      case  o_ro:      /*  fallthrough to o_rw  */
      case  o_rw:      mode_path();  return;
      case  o_bind:    p++;  opt_mode();  arg0=*p++;  arg1=*p++;  return;
      case  o_src:     p++;  opt_mode();  overlay=*p++;  return;
      default:         break;  }

    if  ( arg0.startswith("--") )  {
      if  ( type )  {  p++;  return;  }
      else          {  die2 ( "bad option  %s", arg0.s );  }  }
    if  ( arg0.startswith("-") )  {  type=o_shortopt;  p++;  return;  }
    if  ( is_setenv()          )  {  type=o_setenv;    p++;  return;  }

    path_or_command();  }


  void  do_command  ()  {    //  ------------------  Option_Reader  do_command
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
    if  ( command )  {  p == command  ?  do_command()  :  path();  return;  }
    bool  found  =  Lib::is_dir(  overlay  ?  overlay + "/" + arg0  :  arg0  );
    found  ?  path()  :  do_command();  }


};    //  end  class  Option_Reader  --------------  end  class  Option_Reader




struct  Bind  {    //  xxbi  -----------------------------------  struct  Bind


  opt   mode  =  o_none;
  mstr  dst;
  ostr  src;
  ostr  newroot_dst;


  Bind &  set    //  ----------------------------------------------  Bind  set
  ( const Option_Reader & r, str childname = 0 )  {

    str  ov  =  r.overlay;
    str  a0  =  r.a.arg0;
    str  a1  =  r.a.arg1;
    str  cn  =  childname;

    mode  =  r.a.mode;
    switch  ( r.a.type )  {
      case  o_newroot:  dst = "";       src = a0;            break;
      case  o_full:     dst = cn;       src = s(a0,"/",cn);  break;
      case  o_partial:  dst = a0;       src = s(ov+"/"+a0);
	;;;             /*  mode = ??  */                    break;
      case  o_bind:     dst = a0;       src = s(a1);         break;
      default:          dst = "error";  src = "error";       break;  }
    newroot_dst  =  dst  ?  s(r.newroot,"/",dst)  :  r.newroot  ;
    return  * this;  }


  void  print  ( str s )  const  {    //  -----------------------  Bind  print
    printe ( "%s  bind  %s  %s  %s  %s\n", s.s, o2s(mode),
	     dst == ""  ?  "''"  :  dst.s, src.s, newroot_dst.s );  }


};    //  end  struct  Bind  ------------------------------  end  struct  Bind




struct  Option_Processor  {    //  xxar  --------  interface  Option_Processor
  virtual void  process  ( const Option & o )  =  0;  };    //  -----  process


struct  Bind_Processor  {    //  xxbi  ------------  interface  Bind_Processor
  virtual void  process  ( const Bind & b )  =  0;  };    //  -------  process




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




struct  State  {    //  xxst  ---------------------------------  struct  State


  Argv         argv;
  Env          env;         //  specifies the new environment
  const uid_t  uid          =  getuid();
  const gid_t  gid          =  getgid();
  Fgetpwent    outside      ;    //  from /etc/passwd outside the lxroot
  Fgetpwent    inside       ;    //  from /etc/passwd inside  the lxroot
  opt          opt_env      =  o_none;    //  pass in environment
  opt          opt_network  =  o_none;
  opt          opt_pulse    =  o_none;
  opt          opt_root     =  o_none;
  opt          opt_write    =  o_none;
  opt          opt_x11      =  o_none;
  mstr         newroot;
  Argv         command;


  void  loop  ( Option_Processor & p )  const  {    //  ---------  State  loop
    Option_Reader  r  ( argv + 1 );
    while  ( r.next() )  {  p.process ( r.a );  }  }


  void  loop  ( Bind_Processor & p )  const  {    //  -----------  State  loop
    Option_Reader  r  ( argv + 1 );
    Bind           b;
    while  ( r.next() )  {
      switch  ( r.a.type )  {
	case  o_newroot:    /*  fallthrough to o_bind  */
	case  o_partial:    /*  fallthrough to o_bind  */
	case  o_bind:       p.process ( b.set(r) );  break;
	case  o_full:       loop_full ( p, r, b );      break;
	default:  break;  }  }  }


private:


  void  loop_full    //  -----------------------------------  State  loop_full
  ( Bind_Processor & p, const Option_Reader & r, Bind & b )  const  {
    for  ( const Opendir & e : Opendir ( r.a.arg0 ) )  {
      if  ( e.is_dir() )  {
	if  (  e == "."  ||  e == ".."  )  {  continue;  }
	b.set ( r, e.name() );
	if  ( Lib :: is_dir ( b.newroot_dst ) )  {
	  p.process(b);  }  }  }  }


};    //  end  struct  State  ----------------------------  end  struct  State




State  mut;    //  ---------------------------------------  global  State  mut
const State & st  =  mut;    //  ------------------  global  const State &  st




struct  Init_Tool  :  Option_Processor  {    //  xxin  ----  struct  Init_Tool


  virtual void  process  ( const Option & a )  override  {    //  ---  process
    switch ( a.type )  {

      case  o_command:    command(a);                      break;
      case  o_help:       Lib :: help_print();             break;
      case  o_help_more:  Lib :: help_more_print();        break;
      case  o_network:    mut .opt_network  =  o_network;  break;
      case  o_newroot:    mut .newroot      =  a.arg0;     break;
      case  o_pulse:      mut .opt_pulse    =  o_pulse;    break;
      case  o_root:       mut .opt_root     =  o_root;     break;
      case  o_trace:      global_opt_trace  =  o_trace;    break;
      case  o_version:    version();                       break;
      case  o_write:      mut .opt_write    =  o_write;    break;
      case  o_x11:        mut .opt_x11      =  o_x11;      break;
      case  o_shortopt:   shortopt(a);                     break;

      case  o_bind:       break;
      case  o_dashdash:   break;
      case  o_full:       break;
      case  o_partial:    break;
      case  o_setenv:     break;
      case  o_src:        break;

      default:  die2 ( "init_tool  bad option  %d  %s",
		       a.type,  o2s(a.type) )  break;  }  }


private:


  void  command  ( const Option & a )  {    //  ----------  Init_Tool  command
    if  (  st.newroot == nullptr  &&  a.arg0  )  {
      printe ( "lxroot  directory not found  %s", a.arg0.s );
      exit ( 1 );  }
    mut .command  =  a.command;  }


  void  shortopt  ( const Option & a )  {    //  --------  Init_Tool  shortopt
    for  (  mstr p = a.arg0.s+1  ;  *p  ;  p++  )  {
      switch  ( *p )  {
	case 'n':  mut .opt_network  =  o_network;  break;
	case 'r':  mut .opt_root     =  o_root;     break;
	case 'w':  mut .opt_write    =  o_write;    break;
	case 'x':  mut .opt_x11      =  o_x11;      break;
	default:  die2 ( "bad short option  '%c'", *p );  break;  }  }  }


  static  void  version  ()  {    //  --------------------  Init_Tool  version
    printe ( "lxroot  version  " LXROOT_VERSION "\n" );
    exit ( 0 );  }


};    //  end  struct  Init_Tool  --------------------  end  struct  Init_Tool




struct  Binder  :  Bind_Processor  {    //  xxbi  -------------  class  Binder


  virtual void  process  ( const Bind & b )  override  {    //  -----  process
    sys .bind ( b.newroot_dst, b.src );  }


};    //  end  struct  Binder  --------------------------  end  struct  Binder




class  Env_Tool  :  Option_Processor,  Lib  {  //  xxen  ----  class  Env_Tool


  void  base  ()  {    //  -------------------------------  Env_Tool  env_base

    mut .env .soft ( "PATH=/usr/local/bin:/usr/local/sbin:"
		     "/usr/bin:/usr/sbin:""/bin:""/sbin" );

    const Fgetpwent &  in   =  st .inside;
    const Fgetpwent &  out  =  st .outside;

    mut .env .soft ( "HOME",    in.dir  || getenv("HOME")    || out.dir   );
    mut .env .soft ( "LOGNAME", in.name || getenv("LOGNAME") || out.name  );
    mut .env .soft ( "USER",    in.name || getenv("USER")    || out.name  );

    mut .env .soft_copy ( "TERM" );  }


  void  shell  ()  {    //  -----------------------------  Env_Tool  env_shell

    auto  shell  =  [&]  ( str path )  {
      if  ( Lib :: is_file ( st.newroot + path )
	    and  access ( ( st.newroot + path ).s.s, X_OK ) == 0 )  {
	mut .env .set ( "SHELL", path );  return  true;  }
      return  false;  };

    st .env .get ( "SHELL" )
      ||  shell ( st .inside .shell )
      ||  shell ( getenv("SHELL") )
      ||  shell ( st .outside .shell )
      ||  shell ( "/bin/ash" )
      ||  shell ( "/bin/sh" );  }


  void  argv  ()  {    //  -----------------------------------  Env_Tool  argv
    if  ( st.command[0] )  {  return;  }
    str  shell  =  st .env .get ( "SHELL" );
    if  ( shell == "/bin/bash" )  {  mut .command  =  bash_command;  }
    else if  ( shell )  {
      bash_command[0]    =  shell.s;
      bash_command[1]    =  nullptr;
      mut .command  =  bash_command;  }
    else  {  die2 ( "please specify a command" );  }  }


  void  ps1_bash  ()  {    //  -----------------------  Env_Tool  env_ps1_bash

    str   term  =  st .env .get ( "TERM" );
    str   user  =  st .env .get ( "USER" );
    ostr  opts  =  nullptr;
    ostr  host  =  "./" + st .newroot .basename()  ;

    if  (  st    .opt_network  == o_network
	   ||  st.opt_root     == o_root
	   ||  st.opt_x11      == o_x11  )  {
      opts  =  "-";
      if  ( st.opt_network == o_network )  {  opts  +=  "n";  }
      if  ( st.opt_root    == o_root    )  {  opts  +=  "r";  }
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


  bool  is_busybox ( str path )  {    //  --------------  Env_Tool  is_busybox
    return  Lib :: is_link ( path )
      &&  Lib :: readlink ( path ) == "/bin/busybox";  }


  void  ps1  ()  {    //  -----------------------------  Env_Tool  env_ps1
    //  20200626  todo?  add custom prompts for other shells
    if  (  st.command[0] == "/bin/bash"  ||
	   is_busybox ( st.newroot + st.command[0] )  )  { ps1_bash();  }  }


  /*  20210602  temporarily disabled
  void  passthru  ()  {    //  ---------------------------  Env_Tool  env_pass
    if  ( st.opt_env == o_env )  {
      for  (  Argv p (environ)  ;  * p  ;  p++  )  {
	env .soft ( * p );  }  }  }
  */


public:


  virtual void  process  ( const Option & a )  override  {    //  ---  process
    if  ( a.type == o_setenv )  { mut .env .set ( a.arg0 );  }  }


  void  run  ()  {    //  -------------------------------------  Env_Tool  run
    st.loop ( * this );
    base();            //
    shell();           //
    argv();            //  depends on  env.SHELL
    ps1();             //  depends on  command[0]
    //  passthru();    //  20210602  temporarily disabled
    return;  }


};    //  end  class  Env_Tool  ------------------------  end  class  Env_Tool




struct  Exposer  :  Bind_Processor  {    //  xxex  ------------------  Exposer


  mstr  path;
  ostr  parent;         //  ostr because b.dst is transient
  opt   parent_mode;


  virtual void  process  ( const Bind & b )  override  {    //  -----  process
    if  (  path .is_inside ( b.dst )
	   &&  (  parent == nullptr  ||  b.dst.n() > parent.n  )  )  {
      parent       =  b.dst;
      parent_mode  =  b.mode;  }  }


  void  expose  ( str path )  {    //  ----------------------  Exposer  expose
    if  ( path == nullptr )  {  return;  }
    assert ( path .startswith ( "/" ) );
    this -> path  =  path;    //  input to process()
    st.loop ( * this );    //  find parent
    if  (  parent == path  )  {  return;  }
    if  (  st.opt_root  ||  st.opt_write  )  {  return;  }
    if  (  parent_mode == o_none  ||  parent_mode == o_ra  )  {
      ostr  newroot_path  =  s(st.newroot,path);
      if  ( Lib :: is_dir ( newroot_path ) )  {
	if  ( global_opt_trace == o_trace )  {
	  printe ( "  expose   '%s'\n", newroot_path.s );  }
	sys .bind ( newroot_path, newroot_path );  }  }  }


};    //  end  struct  Exposer  ------------------------  end  struct  Exposer




struct  Remounter  :  Bind_Processor  {    //  xxre  --------------  Remounter


  virtual void  process  ( const Bind & b )  override  {    //  -----  process
    switch  ( b.mode )  {
      case  o_rw:  return;  break;
      case  o_ra:  case  o_none:
	if  (  st .opt_root  ||  st .opt_write            )  {  return;  }
	if  (  b.dst .is_inside ( st .env .get("HOME") )  )  {  return;  }
	if  (  b.dst .is_inside ( "/tmp"               )  )  {  return;  }
	break;
      default:  break;  }
    sys .rdonly ( b.newroot_dst );  }


};    //  end  struct  Remounter  --------------------  end  struct  Remounter




class  Lxroot  :  Lib  {    //  xxlx  -------------------------  class  Lxroot


  mstr           put_old;


  void  init  ()  {    //  -------------------------------------  Lxroot  init
    {  Init_Tool t;  st.loop(t);  }
    if  ( st.newroot == nullptr )  {  help_print ( 1 );  }  }


  void  unshare  ()  {    //  -------------------------------  Lxroot  unshare
    int  clone_flags  =  0;
    clone_flags  |=  CLONE_NEWNS;      //  mount namespace
    clone_flags  |=  CLONE_NEWPID;     //  pid   namespace
    clone_flags  |=  CLONE_NEWUSER;    //  user  namespace
    clone_flags  |=  st.opt_network == o_network  ?  0  :  CLONE_NEWNET  ;
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

    trace ( "  uid_map  %u %u 1  deny  %u %u 1",
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
    {  Binder b;  st.loop(b);  }
    //  note  someday I may integrate binding of /dev and /sys into Binder
    sys .bind ( st.newroot + "/dev", "/dev" );
    //  note  /proc is mounted later in proc().
    sys .bind ( st.newroot + "/sys", "/sys" );  }


  void  fgetpwent  ()  {    //  ---------------------------  Lxroot  fgetpwent
    mut .outside .fgetpwent ( "/etc/passwd", st .uid );
    mut .inside  .fgetpwent ( st.newroot + "/etc/passwd", getuid() );  }


  void  env  ()  {  Env_Tool() .run();  }    //  ----------------  Lxroot  env


  void  remount  ()  {    //  -------------------------------  Lxroot  remount
    Exposer() .expose ( st .env .get ( "HOME" ) );
    Exposer() .expose ( "/tmp" );
    Remounter r;  st.loop(r);  }


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
    if  ( pivot == nullptr )  {  die2 ( "pivot_preapre  pivot is nullptr" );  }
    //  verify that pivot has at least one sub-direcotry (for put_old)
    for  (  const Opendir & e  :  Opendir ( pivot )  )  {
      if  (  e == "."  ||  e == ".."  )  {  continue;  }
      if  (  e.is_dir()  )  {
	put_old  =  leak ( "/" + e.name() );  return;  }  }
    printe ( "pivot_prepare  error  pivot contains no directories\n" );
    exit ( 1 );  }


  void  pivot  ()  {    //  -----------------------------------  Lxroot  pivot
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

    if  ( st.newroot )  {
      sys .mount ( "proc", "/proc", "proc" );
      sys .umount2 ( put_old.s, MNT_DETACH );  }  }


  void  chdir  ()  {    //  -----------------------------------  Lxroot  chdir
    str  home  =  st .env .get ( "HOME" );
    //  20210518  disabled during overhaul.  todo  re-implement ?
    //  if  (  st.new_cwd  )  {  sys .chdir ( st.new_cwd );  return;  }
    if  (  st .newroot  &&  is_dir ( home )  )  { sys .chdir ( home );  }  }


  void  umount2  ()  {    //  -------------------------------  Lxroot  umount2
    //  someday I may move the the call to sys.umount2() to here.
    return;  }


  void  xray  ()  {    //  -------------------------------------  Lxroot  xray
    if  ( global_opt_trace == o_trace )  {
      printe ( "\n" );
      printe ( "xray  uid  %d  %d\n", getuid(), getgid() );
      printe ( "xray  cwd  %s\n", getcwd().s );
      Argv ( st .env .data() ) .print ( "xray  env  " );
      printe ( "xray  command\n" );
      st .command .print ( "xray  cmd  " );
      printe ( "\n" );
      return;  }  }


  void  exec  ()  {    //  -------------------------------------  Lxroot  exec
    sys .execve ( st.command[0], st.command, st.env.data() );  }


public:


  int  main  ()  {    //  --------------------------------------  Lxroot  main
    init();          //  first
    unshare();       //  before uid_map and bind
    uid_map();       //  after unshare
    bind();          //  after unshare
    fgetpwent();     //  after bind
    env();           //  after fgetpwent
    remount();       //  after env
    options();       //  late, before pivot
    /*  pivot to umount2 should be as tight as possible  */
    pivot();         //  late, but before fork? ( !! all paths change !! )
    fork();          //  prefer after pivot (so that parent is not in put_old)
    proc();          //  after fork, before umount2
    umount2();       //  after proc
    /*  pivot to umount2 should be as tight as possible  */
    chdir();         //  after umount2 ( because put_old may shadow $HOME !! )
    xray();          //
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
