

//  lxroot.cpp  -  Create and use chroot-style virtual software environments.
//  Copyright (c) 2022 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.


#define  LXROOT_VERSION  "0.22.1"    //  version  20220822


//  Welcome to the source code for Lxroot.
//
//  Lxroot's command line interface is documented in help.cpp.
//
//  The classes and structs in Lxroot can be divided into three categories:
//
//    low level data storage classes
//    convenience wrappers around system APIs
//    high level classes
//
//  --  String management classes
//
//  Several types come in mutable vs. immutable pairs.  For example:
//
typedef  const char *  mstr;    //  a  mutable   pointer (to const char)
typedef  const mstr    str;     //  an immutable pointer (to const char)
//
//  In general, if a type name starts with 'm', then it may be the
//  mutable half of a mutable vs. immutable type pair.
//
//  struct   mStr    a mutable string (pointer and length, both mutable).
//  typedef  Str     an immutable string (pointer and length).
//  struct   CatN    represents the concatenation of up to 4 strings.
//  struct   oStr    an dynamicly appendable string that owns its memory.
//  struct   Argv    a convenience and saftey wrapper around mstr[].
//
//  --  Low level data storage classes
//
//  enum    opt       an enumeration that represents various options.
//  struct  Option    represents a parsed command line option.
//  struct  Bind      represents (and specifies) a potential bind mount.
//  struct  Env       a list that specifies the new environment.
//  struct  State     contains shared, mutable, global variables.
//
//  --  Convenience wrappers
//
//  class   Dirent     represents a directory entry (from readdir()).
//  struct  Fgetpwent  parses /etc/passwd.
//  struct  Lib        contains generally useful functions.
//  struct  Syscall    provides error handling and tracing of syscalls.
//
//  --  High level classes
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
#include  <sys/statfs.h>     //  man 2 statfs
#include  <sys/statvfs.h>    //  man 2 statfs  ST_RELATIME
#include  <sys/syscall.h>    //  man 8 pivot_root
#include  <sys/types.h>      //  man 3 opendir
#include  <sys/wait.h>       //  man 2 wait

#include  <vector>           //  class Env .vec
#include  <functional>       //  std :: function

#include  "help.cpp"         //  Lxroot's help strings
#include  "str.cpp"          //  Parke's string library


template < class C >    //  ---------------------------  template  using  sink
using  sink  =  std :: function < void ( const C & ) >;
template < class C >    //  --------------------------  template  using  msink
using  msink  =  std :: function < void ( C & ) >;


//  macro  printe  --------------------------------------------  macro  printe
#define  printe( fmt, ... )  fprintf ( stderr, fmt "\n", ##__VA_ARGS__ );
#define  writee( fmt, ... )  fprintf ( stderr, fmt     , ##__VA_ARGS__ );


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
T  assert2  ( T v, str file, const int line )  {

  //  Discussion of assert2 being a function vs being a macro:

  //  A function ensures that v is evaluated only once.

  //  My hope is that assert2 as a template (rather than a macro) will
  //  result in more readable compile time errors.
  //  However, a template might result in unnecessary copying of a T.

  //  Does assert2 need to be a function so that it can return a value?
  //  Or could a macro "return" a value via the comma operator?  Not sure.

  if  ( (bool) v == false )  {
    printe ( "lxroot  assert2()  failed  %s  %d", file, line );  abort();  }
  return  v;  }


//  macro  assert  --------------------------------------------  macro  assert
#define  assert( v )  assert2 ( v, __FILE__, __LINE__ )


mstr  bash_command[]  =  {    //  ------------------------------  bash_command
  "/bin/bash",  "--norc",  nullptr  };


typedef  const int      cint;     //  xxci  ------------------  typedefg  cint
typedef  unsigned char  Opt_t;    //  xxop  ------------------  typedef  Opt_t


//  20220822  mopt is deprecated, and will eventually be replaced by Opt_t.
enum  mopt  {    //  xxop  ---------------------------------------  enum  mopt
  o_none,
  /*  literal arg types  */
  o_bind,  o_dashdash,  o_cd,  o_ra,  o_ro,  o_rw,  o_src,  o_wd,
  /*  non-literal option types  */
  o_full,  o_newroot,  o_partial,  o_setenv,  o_shortopt,
  /*  literal long options  */
  o_env,    o_help,    o_help_more,    o_network,    o_pulse,
  o_root,    o_trace,    o_uid,  o_version,    o_write,    o_x11,
  };


str  opt_name[]  =  {    //  ---------------------------------------  opt_name
  "0",
  /*  literal arg types  */
  "bind",  "--",        "cd",  "ra",  "ro",  "rw",  "src",  "wd",
  /*  non-literal option types  */
  "full",  "newroot",  "partial",  "setenv",  "shortopt",
  /*  literal long options  */
  "--env",  "--help",  "--help-more",  "--network",  "--pulseaudio",
  "--root",  "--trace",  "--uid",  "--version",  "--write",  "--x11",
  nullptr  };


//  20220822  opt_t is deprecated, and will eventually be replaced by Opt_t.
typedef  const mopt  opt_t;


mopt  operator ||  ( opt_t a, opt_t b )  {    //  --------  global  opt  op ||
  return  a  ?  a  :  b  ;  }


mopt  global_opt_trace  =  o_none;    //  --------------  ::  global_opt_trace


template  < class... Args >    //  --------------------------  template  trace
void  trace  ( str format, Args... args )  {
  if  ( global_opt_trace == o_trace )  {
    fprintf ( stderr, format, args... );
    fputc ( '\n', stderr );   }  }


void  trace  ( str s )  {  trace ( "%s", s );  }    //  -------------  trace


mstr  o2s  ( const opt_t n )  {    //  ------------------------------  ::  o2s
  if  (  0 <= n  &&  n < ( sizeof opt_name / sizeof(char*) )  )  {
    return  opt_name[n];  }
  return  "INVALID_OPTION";  }


mopt  s2o  ( str s )  {    //  xxs2  --------------------------------  ::  s2o
  if  ( s )  {
    for  (  int n = o_none;  opt_name[n];  n++ )  {
      if  ( strcmp ( s, opt_name[n] ) == 0 )  {  return  (mopt) n;  }  }  }
  return  o_none;  }


mopt  s2o  ( Str s )  {    //  xxs2  --------------------------------  ::  s2o
  return  s2o ( s.s );  }


//  end  enum  opt  ------------------------------------------  end  enum  opt


typedef  unsigned long  flags_t;    //  xxfl  --------------  typedef  flags_t




struct  Option  {    //  xxop  -------------------------------  struct  Option


  Argv  a;                  //  points to the first Str of the current option
  Argv  p;                  //  a traveller that advances to the next option

  mopt  type  =  o_none;    //  the type of this option
  mopt  mode  =  o_none;    //  the read-write mode of this option
  mStr  arg0,  arg1;        //  optional args

  mStr  newroot;            //  the newroot
  mStr  overlay;            //  the current overlay


  Option  ( Argv p )  :  p(p)  {}    //  -----------------------  Option  ctor


  operator bool  ()  const  {  return  type;  }    //  ---  Option  cast  bool


  void  print  ( Str m )  const  {    //  ---------------------  Option  print
    printe ( "%-8s  %-7s  %s", m.s, o2s(type), arg0.s );  }


protected:


  Option  ()  {}


};    //  end  struct  Option  --------------------------  end  struct  Option




struct  Bind  {    //  xxbi  -----------------------------------  struct  Bind


  mopt  type    =  o_none;
  mopt  mode    =  o_none;    //  the specified mode  ro, rw, ra, none
  mopt  actual  =  o_none;    //  the actual mode     ro, rw

  mStr  full;           //  the path of the full overlay
  mStr  dst;            //  20210619  at present, dst never begins with '/'
  oStr  src;
  oStr  newroot_dst;    //  newroot + dst

  const Option *  option  =  nullptr;    //  the source option for this bind


  void  clear  ()  {    //  -------------------------------------  Bind  clear
    * this  =  Bind();  }


  void  print  ( Str s )  const  {    //  -----------------------  Bind  print
    printe ( "%s  bind  %-7s  %-2s  %-2s  '%s'  '%s'  '%s'",
	     s.s,  o2s(type),  o2s(mode),  o2s(actual), dst.s, src.s,
	     newroot_dst.s );  }


  Bind &  set    //  ----------------------------------------------  Bind  set
  ( const Option & o, Str childname = 0 )  {

    //  20210620  note:  a Bind may outlive the Option o.

    Str  ov  =  o.overlay;
    Str  a0  =  o.arg0;
    Str  a1  =  o.arg1;
    Str  cn  =  childname;

    type    =  o.type;
    mode    =  o.mode;
    full    =  nullptr;
    option  =  & o;

    switch  ( o.type )  {
      case  o_newroot:  dst="";  src=a0;  mode=mode||o_ra;    break;
      case  o_bind:     dst=a0;  src=a1;                      break;

	//  20210620  todo  consider setting dst and src to ""
        //  20210620        nope, it appears setting dst and src is required
	//  20210620        I wonder why?
      case  o_full:     dst=cn;  src=a0+"/"+cn;  full=a0;     break;
      //se  o_full:     dst="";  src="";  full=a0;            break;

	//  20210620  todo  set mode (and write unit tests)
      case  o_partial:  dst=a0;  src=ov+"/"+a0;               break;

      default:          dst="error";  src="error";            break;  }

    if  ( dst )  {  newroot_dst  =  o.newroot + "/" + dst;  }
    else         {  newroot_dst  =  o.newroot;  }

    //  20210619
    //  if  ( dst[0] == '/' )  {    //  20210619
    //    die1 ( "bind  set  bad dst  %s", dst.s );  }

    return  * this;  }


  const Bind &  trace  ( Str s )  const  {    //  ---------------  Bind  trace
    if  ( global_opt_trace )  {  print ( s );  }
    return  * this;  }


};    //  end  struct  Bind  ------------------------------  end  struct  Bind




class  Env  {    //  xxen  ---------------------------------------  class  Env


  std :: vector < mstr >  vec;


public:


  str *  data  ()  const  {  return  vec .data();  }    //  -------  Env  data
  bool  not_set  ( Str name )  const  {  return  get(name) == nullptr;  }


  Str  get  ( Str name )  const  {    //  --------------------------  Env  get
    for  (  auto & o  :  vec  )  {
      if  ( Str(o) .env_name() == name )  {
	return  Str(o) .tail ( "=" );  }  }
    return  nullptr;  }




  void  set  ( Str pair )  {    //  --------------------------------  Env  set
    Str  name  =  pair .env_name();
    if  ( name.n == 0 )  {  return;  }
    for  (  auto & o  :  vec  )  {
      if  ( Str(o) .env_name() == name  )  {  o  =  pair.s;  return;  }  }
    if  ( vec .size() == 0 )  {  vec .push_back ( nullptr );  }
    vec .back()  =  pair.s;
    vec .push_back ( nullptr );  }


  void  set  ( Str name, Str value )  {    //  ---------------------  Env  set
    set ( oStr ( name + "=" + value ) .leak() );  }


  void  soft  ( Str pair )  {    //  ------------------------------  Env  soft
    if  ( not_set ( pair .env_name() ) )  {  set ( pair );  }  }


  void  soft  ( Str name, Str value )  {    //  -------------------  Env  soft
    if  ( not_set ( name ) )  {
      set ( oStr ( name + "=" + value ) .leak() );  }  }


  void  soft_copy  ( Str name )  {    //  --------------------  Env  soft_copy
    Str  pair  =  Argv(environ) .env_get ( name );
    if  ( pair )  {  soft ( pair );  }  }


};    //  end  class  Env  ----------------------------------  end  class  Env




struct  Fgetpwent  {    //  xxfg  -------------------------  struct  Fgetpwent


  oStr  dir, name, shell;    //  see man 3 fgetpwent


  void  fgetpwent  ( Str path, uid_t uid )  {    //  --------------  fgetpwent

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
  const uid_t  uid           =  getuid();    //  uid prior to unshare()
  const gid_t  gid           =  getgid();    //  gid prior to unshare()
  Fgetpwent    outside       ;    //  from /etc/passwd outside the lxroot
  Fgetpwent    inside        ;    //  from /etc/passwd inside  the lxroot
  mopt         newroot_mode  =  o_none;
  Opt_t        opts[128]     =  { 0 };    //  command line options
  bool         before_pivot  =  true;
  mStr         newroot;
  mStr         guestname;
  mStr         chdir;         //  from the first and only cd option
  mStr         workdir;       //  from the last wd option
  mStr         xauthority;    //  contents of $HOME/.Xauthority
  Argv         dash;          //  points to '--' in argv ("dash dash")
  Argv         command;       //  points to the command to exec()


Opt_t    opt_get  ( Opt_t k )  const    {  return  opts[k];  }    //  ---- opt
Opt_t &  opt      ( Opt_t k )  {  return  opts[k];  }    //  --


};    //  end  struct  State  ----------------------------  end  struct  State


State  mut;    //  -----------------------------  mutable  global  State   mut
const State & st  =  mut;    //  -----------------  const  global  State &  st
Opt_t  opt  ( Opt_t k )  {  return  st .opt_get (k);  }    //  --  global  opt




class  Dirent  {    //  xxdi  ---------------------------------  class  Dirent


  struct dirent *  p  =  nullptr;


public:


  Dirent &  operator =  ( dirent * pp )   {  p  =  pp;  return  * this;  }
  bool      operator == ( Str s )  const  {  return  name() == s;  }
  operator  bool   ()  const              {  return  p;  }
  ino_t     inode  ()  const              {  return  p -> d_ino;  }
  Str       name   ()  const              {  return  p -> d_name;  }


  bool  is_dir  ()  const  {    //  --------------------------  Dirent  is_dir
    if  ( p -> d_type == DT_UNKNOWN )  { die1("dirent type is DT_UNKNOWN"); }
    return  p -> d_type == DT_DIR;  }


};    //  end  class  Dirent  ----------------------------  end  class  Dirent




struct  Lib  {    //  xxli  -------------------------------------  struct  Lib


  //  20210530  apparent redundancy:  assert_is_dir  vs  directory_require


  static void  assert_is_dir  ( str path, str m )  {    //  ---  assert_is_dir
    if  ( is_dir ( path ) )  {  return;  }
    printe ( "lxroot  %s  directory not found  '%s'", m, path  );
    exit ( 1 );  }


  static void  directory_require     //  -------------  Lib  directory_require
  ( Str path, Str m )  {
    if  ( Lib :: is_dir ( path ) )  {  return;  }
    die1 ( "%s directory not found\n  '%s'", m.s, path.s );  }


  static bool  eq  ( str a, str b )  {    //  -----------------------  Lib  eq
    if  ( a == NULL  ||  b == NULL )  return  false;
    return  strcmp ( a, b ) == 0;  }


  static oStr  getcwd  ()  {    //  -----------------------------  Lib  getcwd
    return  oStr :: claim ( get_current_dir_name() );  }


  static Str  getenv  ( Str name )  {    //  --------------------  Lib  getenv
    return  :: getenv ( name.s );  }


  static void  help_print  ( int n = 0 )  {    //  --------  Lib :: help_print
    writee ( "%s%s", help, help2 );  exit ( n );  }


  static void  help_more_print  ()  {    //  ---------  Lib :: help_more_print
    writee ( "%s%s", help, help_more );  exit ( 0 );  }


  static Str  home  ()  {    //  ----------------------------------  Lib  home
    return  getenv ( "HOME" );  }


  static bool  is_dir  ( Str path )  {    //  -------------------  Lib  is_dir
    struct stat  st;
    if  (  path.n  &&
	   stat ( path.s, & st ) == 0  &&  st .st_mode & S_IFDIR  )  {
      return  1;  }
    errno  =  ENOENT;
    return  false;  }


  static bool  is_empty_dir  ( str path )  {    //  ------------  is_empty_dir
    if  ( not is_dir ( path ) )  {  return  false;  }
    DIR * dirp  =  assert ( opendir ( path ) );
    for  (  struct dirent * p;  ( p = readdir ( dirp ) );  )  {
      str  s  =  p -> d_name;
      if  (  eq(s,".")  ||  eq(s,"..")  )  {  continue;  }
      closedir ( dirp );  return  false;  }
    closedir ( dirp );  return  true;  }


  static bool  is_exists ( Str path )  {    //  --------------  Lib  is_exists
    struct stat  st;
    return  path.n  &&  :: lstat ( path.s, & st ) == 0  ;  }


  static bool  is_file  ( Str path )  {    //  -----------------  Lib  is_file
    struct stat  st;
    if  (  path.n  &&  :: stat ( path.s, & st ) == 0
	   &&  st .st_mode & S_IFREG  )  {
      return  true;  }
    errno  =  ENOENT;    //  20210521  so perror() is useful?
    return  false;  }


  static bool  is_link  ( Str path )  {    //  -----------------  Lib  is_link
    struct stat  st;
    if  (  path.s
	   &&  lstat ( path.s, & st ) == 0
	   &&  S_ISLNK ( st .st_mode )  )  {  return  true;  }
    errno  =  ENOENT;
    return  false;  }


  enum  prompt_rv  {  prompt_notty=1,  prompt_timeout=2  };    //  -----------


  static int  prompt  ( Str message, int timeout )  {    //  ----  Lib  prompt
    //  prompt() returns:  prompt_notty  or  prompt_timeout
    //    or  -1 on invalid input  or  the character c on input of "c\n"
    if  (  not isatty(0)  ||  not isatty(1)  )  {  return  prompt_notty;  }
    const int      stdout  =  1;
    const ssize_t  expect  =  strlen ( message.s );
    const ssize_t  actual  =  write ( stdout, message.s, expect );
    if  ( actual not_eq expect )  {  die1 ( "prompt failed" );  }
    fd_set          rfds;  FD_ZERO(&rfds);  FD_SET(0,&rfds);
    struct timeval  tv  =  { timeout, 0 };
    int  rv             =  select ( 1, & rfds, 0, 0, & tv );
    if  ( rv == 0 )  {  return   prompt_timeout;  }
    if  ( rv != 1 )  {  return  -1;  }    //  select() error?
    unsigned char  buf[3];  rv  =  read ( 0, buf, 3 );
    return  ( rv == 2 && buf[1] == '\n' )  ?  buf[0]  :  -1  ;  }


  static Str  readlink  ( Str path )  {    //  ---------------  Lib  readlink
    struct stat  st;
    if  (  lstat ( path.s, & st ) == 0  &&  st .st_mode & S_IFLNK  )  {
      ssize_t  lim  =  st .st_size + 2;
      char *   buf  =  (char*)  malloc ( lim );
      if  ( buf )  {
	memset ( buf, '\0', lim );
	ssize_t  len  =  :: readlink ( path.s, buf, lim );
	if  ( len == lim - 2 )  {  return  buf;  } }
      printe ( "lxroot  readlink  failed  %s", path.s );
      exit ( 1 );  }
    return  nullptr;  }


};    //  end  struct  Lib  --------------------------------  end  struct  Lib




//  try  ----------------------------------------------------------------  try


struct  Try2_Format_String  {  void(*fp)();  str format;  };    //  --  struct


Try2_Format_String  try2_lookup_table[]  =  {    //  ------  try2_lookup_table
#define  m(fn)  ( (void(*)()) fn )
  {  m(chdir),    "  chdir    %s"        },
  {  m(chroot),   "  chroot   %s"        },
  {  m(close),    "  close    %d"        },
  {  m(umount2),  "  umount2  %s  0x%x"  },
  {  m(unshare),  "  unshare  0x%08x"    },
#undef  m
  { nullptr, nullptr  }  };


mstr  try2_format  ( void(*fp)() )  {    //  ---------------------  try2_format
  for  (  const auto & en  :  try2_lookup_table  )  {
    if  ( en.fp == fp )  {  return  en.format;  }  }
  return  "try2_format  unexpected function";  }


template  < typename F >    //  -----------------------  template  try2_format
mstr  try2_format  ( F fp )  {  return  try2_format ( (void(*)()) fp );  }


template  < typename F, class... Args >    //  ---------------  template  try1
int  try1  ( str s, F fn, Args... args )  {
  int  rv  =  fn ( args... );
  if  ( rv == -1 )  {  die1 ( "try1  %s", s );  }
  return  rv;  }


template  < typename F, class... Args >    //  ---------------  template  try2
int  try2  ( str s, F fn, Args... args )  {
  trace ( try2_format(fn), args... );  return  try1 ( s, fn, args... );  }


struct  Syscall  {    //  xxsy  -----------------------------  struct  Syscall

  //  note  all Syscall methods call exit(1) on error.


  pid_t  fork_pid   =  -2;
  pid_t  wstatus    =  0;


  static void  chdir    (Str path ){ try2( "chdir",   ::chdir,   path.s );}
  static void  chroot   (Str path ){ try2( "chroot",  ::chroot,  path.s );}
  static void  close    (int fd   ){ try1( "close",   ::close,   fd     );}
  static void  unshare  (int flags){ try2( "unshare", ::unshare, flags  );}
  static void  umount2  ( Str target, int flags )  {
    try2 ( "umount2", :: umount2, target.s, flags );  }


  void  bind  ( Str target, Str source )  {    //  ------------  Syscall  bind
    //
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
    //              non-root user, MS_BIND (often? always?) requires MS_REC.
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


  void  execve  ( const Str   pathname,    //  --------------  Syscall  execve
		  const Argv  argv,
		  const Argv  envp )  {

    trace ( "  execve   %s  %s", pathname.s, argv .concat().s );
    if  ( pathname .chr ( '/' ) )  {    //  path is specified, so use execve()
      :: execve ( pathname.s, (char**) argv.p, (char**) envp.p );  }
    else  {    //  only filename is specified, so use execvpe()
      char **  old  =  environ;
      environ       =  (char**) envp.p;
      :: execvpe ( pathname.s, (char**) argv.p, (char**) envp.p );
      environ       =  old;  }
    //  execve only returns on failure, so ...
    die_pe ( "execve  %s", pathname.s );  }


  static void  exit  ( int status )  {    //  -----------------  Syscall  exit
    trace ( "  exit     %d", status );  :: exit ( status );  }


  static void  file_copy  ( Str src, Str dst, const mode_t mode )  {    //  --

    int  src_fd  =  open ( src, O_RDONLY );
    int  dst_fd  =  open ( dst, O_WRONLY | O_CREAT, mode );
    char  buf[1024];
    while  ( true )  {
      int  a  =  try1 ( "read", :: read, src_fd, buf, sizeof(buf) );
      if  ( a == 0 )  {  break;  }
      int  b  =  try1 ( "write", :: write, dst_fd, buf, a );
      assert ( a == b );  }
  try1 ( "close", :: close, src_fd );
  try1 ( "close", :: close, dst_fd );  }


  static Str  file_read  ( Str path )  {    //  ----------  Syscall  file_read
    const auto  st  =  Syscall :: stat ( path );
    const int   fd  =  Syscall :: open ( path, O_RDONLY );
    Str         rv  =  Syscall :: read ( fd, nullptr, st.st_size );
    ;                  Syscall :: close ( fd );
    return  rv;  }


  static void  file_write  ( Str path, Str s, const mode_t mode )  {   //  ---
    const int  fd      =  Syscall :: open ( path, O_WRONLY | O_CREAT, mode );
    const int  actual  =  :: write ( fd, s.s, s.n );
    if  ( actual not_eq s.n )  {  die1 ( "file_write  %s", path.s );  }
    ;                     Syscall :: close(fd);  }


  void  fork  ()  {    //  ------------------------------------  Syscall  fork
    if  ( fork_pid != -2 )  {  die_pe ( "extra fork?" );  }
    if  ( ( fork_pid = :: fork() ) >= 0 )  {
      trace ( "  fork     (fork returned %d)", fork_pid );
      return;  }
    die_pe ( "fork" );  }


  void  mount  ( Str source, Str target, Str filesystemtype )  {    //   mount
    Lib :: directory_require ( target, "target" );
    trace ( "  mount    %s  %s  %s", source.s, target.s, filesystemtype.s );
    if  ( :: mount ( source.s, target.s, filesystemtype.s, 0, 0 ) == 0 )  {
      return;  }
    die_pe ( "mount  %s  %s  %s\n",  source.s, target.s, filesystemtype.s );  }


  static int  open  ( Str path, const int flags, const mode_t mode )  {  //  --
    const int  fd  =  :: open ( path.s, flags, mode );
    if  ( fd == -1 )  {  die1 ( "open  %s  %s", path.s, strerror(errno) );  }
    return  fd;  }


  static int  open  ( Str pathname, const int flags )  {    //  --------  open
    return  open ( pathname, flags, 0 );  }


  static void  open  ( int * fd, Str pathname, const int flags )  {    //  ---
    * fd  =  open ( pathname, flags );  }


  void  pivot  ( Str new_root, Str put_old )  {   //  --------  Syscall  pivot
    trace ( "  pivot    '%s'  '%s'", new_root.s, put_old.s );
    if  ( syscall ( SYS_pivot_root, new_root.s, put_old.s ) == 0 )  {
      mut .before_pivot  =  false;  return;  }
    die_pe ( "pivot  %s  %s", new_root.s, put_old.s );  }


  void  rdonly  ( Str target )  {    //  --------------------  Syscall  rdonly
    struct statfs  st;    //  we will store the current statfs() flags in st.
    if  ( statfs ( target.s, & st ) not_eq 0 )  {
      die_pe ( "rdonly  statfs err  '%s'\n", target.s );  }
    const flags_t  flags  =  (  st_to_ms ( st .f_flags )
				| MS_BIND | MS_REMOUNT | MS_RDONLY  );
    if  ( :: mount ( NULL, target.s, NULL, flags, NULL ) == 0 )  {
      trace ( "  rdonly   %lx  '%s'", flags, target.s );  return;  }
    die_pe ( "rdonly  %lx  '%s'\n", flags,  target.s );  }


  static Str  read  ( cint fd, void * buf, cint count )  {    //  ------  read
    if  ( buf == nullptr )  {  buf  =  malloc(count);  }
    if  ( buf == nullptr )  {  die1 ( "read failed  buf == nullptr" );  }
    const int  actual  =  :: read ( fd, buf, count );
    if  ( actual not_eq count )  {
      die1 ( "read failed  actual not_eq count" );  }
    return  Str ( (char*) buf, actual );  }


  static flags_t  st_to_ms  ( flags_t n )  {    //  -------  Syscall  st_to_ms

    //  convert a statfs() flag to a mount() flag.  (ST_ to MS_ conversion.)

    //  see  / usr / include / x86_64-linux-gnu / bits / statvfs.h
    //  see  / usr / include / linux / mount.h

    //  flags from man 2 statfs    flags from man 2 mount
    //    ST_RDONLY          1        MS_RDONLY          1
    //    ST_NOSUID          2        MS_NOSUID          2
    //    ST_NODEV           4        MS_NODEV           4
    //    ST_NOEXEC          8        MS_NOEXEC          8
    //    ST_SYNCHRONOUS    16        MS_SYNCHRONOUS    16
    //    ST_MANDLOCK       64        MS_MANDLOCK       64
    //    ST_NOATIME      1024        MS_NOATIME      1024
    //    ST_NODIRATIME   2048        MS_NODIRATIME   2048
    //    ST_RELATIME     4096        MS_RELATIME     1<<21

    //    note  on x86_64  ST_RELATIME != MS_RELATIME
    //    note  on x86_64  ST_RELATIME == MS_BIND == 4096

    //  the below verbose yet simple implementation should optimize well.

    #define  c(a)  ( (flags_t) (a) )
    #define  if_equal(     a, b )  (   c(a) == c(b)                ? b : 0 )
    #define  if_not_equal( a, b )  ( ( c(a) != c(b) ) && ( n & a ) ? b : 0 )

    constexpr flags_t  copy_these_bits  =
      if_equal         (  ST_RDONLY,       MS_RDONLY       )
      |  if_equal      (  ST_NOSUID,       MS_NOSUID       )
      |  if_equal      (  ST_NODEV,        MS_NODEV        )
      |  if_equal      (  ST_NOEXEC,       MS_NOEXEC       )
      |  if_equal      (  ST_SYNCHRONOUS,  MS_SYNCHRONOUS  )
      |  if_equal      (  ST_MANDLOCK,     MS_MANDLOCK     )
      |  if_equal      (  ST_NOATIME,      MS_NOATIME      )
      |  if_equal      (  ST_NODIRATIME,   MS_NODIRATIME   )
      |  if_equal      (  ST_RELATIME,     MS_RELATIME     );

    const flags_t  shifted_bits  =
      if_not_equal     (  ST_RDONLY,       MS_RDONLY       )
      |  if_not_equal  (  ST_NOSUID,       MS_NOSUID       )
      |  if_not_equal  (  ST_NODEV,        MS_NODEV        )
      |  if_not_equal  (  ST_NOEXEC,       MS_NOEXEC       )
      |  if_not_equal  (  ST_SYNCHRONOUS,  MS_SYNCHRONOUS  )
      |  if_not_equal  (  ST_MANDLOCK,     MS_MANDLOCK     )
      |  if_not_equal  (  ST_NOATIME,      MS_NOATIME      )
      |  if_not_equal  (  ST_NODIRATIME,   MS_NODIRATIME   )
      |  if_not_equal  (  ST_RELATIME,     MS_RELATIME     );

    #undef  c
    #undef  if_equal
    #undef  if_not_equal

    return  ( n & copy_these_bits ) | shifted_bits;  }


  static struct stat  stat  ( Str path )  {    //  ------------  Syscall  stat
    struct stat  st;
    if  ( :: stat ( path.s, & st ) == -1 )  {  die1 ( "stat  %s", path.s );  }
    return  st;  }


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
    die_pe ( "write  %d  %ld", fd, (long int) count );  }


};    //  end  struct  Syscall  ------------------------  end  struct  Syscall


Syscall  sys;    //  xxsy  --------------------------------------  global  sys




struct  Option_Reader    //  xxop  --------------------  struct  Option_Reader
  :  private  Option  {


  const Option &  ref;    //  const access to the Option base class


  Option_Reader  ( Argv p )  :  Option(p),  ref(*this)  {}    //  ------  ctor


  Option_Reader  ( const Option * other )  :  ref(*this)  {    //  -----  ctor
    * (Option*) this  =  * other;  }


  const Option &  next  ()  {  next_impl();  return  ref;  }    //  ----  next


private:


  void  next_impl  ()  {    //  --------------------  Option_Reader  next_impl

    //    input                  type          mode    arg0     arg1
    //
    //    --<longopt>            o_<longopt>    -       -       -
    //    -short                 o_shortopt     -       -       -
    //    name=value             o_setenv       -       -       -
    //    [mode] path            o_newroot      mode    path    -
    //    [mode] path            o_full         mode    path    -
    //    [mode] path            o_partial      mode    path    -
    //    src [mode] path        o_src          mode    path    -
    //    bind [mode] dst src    o_bind         mode    dst     src
    //    cd path                o_cd           -       path    -
    //    wd path                o_wd           -       path    -
    //    -- cmd [arg ...]       o_dashdash     -       -       -


    a     =  p;    //  advance
    type  =  s2o ( a[0] );
    mode  =  o_none;

    if  (  p.p == nullptr  ||  p[0] == nullptr  )  {  return;  }

    switch  ( type )  {

      case  o_ra:          //  fallthrough to o_rw
      case  o_ro:          //  fallthrough to o_rw
      case  o_rw:          mode_path();  return;

      case  o_src:         p++;  opt_mode();  overlay=*p++;  return;
      case  o_bind:        p++;  opt_mode();  arg0=*p++;  arg1=*p++;  return;

      case  o_cd:          //  fallthrough to o_wd
      case  o_wd:          p++;  arg0=*p++;  return;

      case  o_dashdash:    p=(char**)nullptr;  return;

      default:             break;  }

    if  ( a[0].starts_with("--") )  {
      if  ( type )  {  p++;  return;  }
      else          {  die1 ( "bad option  %s", a[0].s );  }  }
    if  ( a[0].starts_with("-") )  {  type=o_shortopt;  p++;  return;  }
    if  ( is_setenv()           )  {  type=o_setenv;    p++;  return;  }

    path();  }


  bool  is_setenv  ()  {    //  --------------------  Option_Reader  is_setenv
    str  var_name_allowed  =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
    cint  n  =  a[0].spn ( var_name_allowed );
    return  n > 0  &&  n < a[0].n  &&  a[0].s[n] == '=';  }


  void  mode_path  ()  {    //  --------------------  Option_Reader  mode_path
    //  mode  path
    mode=type;  p++;  path();  }


  void  opt_mode  ()  {    //  ----------------------  Option_Reader  opt_mode
    //  [mode]
    opt_t  n  =  s2o(p[0]);  switch  ( n )  {
      case  o_ra:  case  o_ro:  case  o_rw:  mode=n;  p++;  break;
      default:  break;  }  }


  void  path  ()  {    //  ------------------------------  Option_Reader  path
    //  path
    arg0  =  * p ++ ;
    if  ( newroot )  {  type  =  overlay.n  ?  o_partial  :  o_full  ;  }
    else             {  type=o_newroot;  newroot=arg0;  }  }


};    //  end  class  Option_Reader  --------------  end  class  Option_Reader




struct  Logic  :  Lib  {    //  xxlo  -------------------------  struct  Logic


  static mopt  actual  ( Str path, opt_t mode )  {    //  -----  Logic  actual
    return  (  mode == o_rw
	       ||  is_inside_workdir ( path )
	       ||  (  mode == o_ra
		      &&  (  opt('r')
			     ||  opt('w')
			     ||  is_inside_readauto ( path )  )  )
	       ?  o_rw  :  o_ro  );  }


  static void  binds  ( sink<Bind> fn )  {    //  --------------  Logic  binds

    //  Transform each command line Option into zero or more Binds.
    //  Pass each Bind to fn.

    Bind  b;

    auto  single  =  [&]  ()  {
      if  ( is_overbound ( b ) )  { return;  }
      if  ( b.mode == o_none )  {
	mStr  parent;  calculate_parent ( b.dst, parent, b.mode );  }
      b.actual  =  actual ( b.dst, b.mode );
      fn ( b );  };

    auto  full  =  [&]  ( const Option & o )  {
      scandir  (  o.arg0,  [&](auto e)  {
        if  ( e.is_dir() )  {
	  b.set ( o, e.name() );
	  if  ( is_dir ( b.newroot_dst ) )  {  single();  }  }  }  );  };

    options  (  [&]( const Option & o )  {
      switch  ( o.type )  {
	case  o_newroot:    //  fallthrough to o_bind
	case  o_partial:    //  fallthrough to o_bind
	case  o_bind:       b.set(o);  single();  break;
	case  o_full:       full(o);  break;
	default:  break;  }  }  );  }


  static void  calculate_parent    //  --------------  Logic  calculate_parent
  ( Str child, mStr & parent, mopt & mode )  {

    //  Find/calculate the path and specified mode of the Bind that
    //  contains path child.  Since each Bind may inherit from its parent,
    //  we must descend to child, one step at a time.

    //  child is the full dst of the bind.
    //  parent will be set to the full dst of the Bind that contains child.

    auto  is_descendant  =  [&]  ( const Bind & raw )  {
      return  child .is_inside ( raw .dst )
	&&  (  parent == nullptr ||  raw.dst.n > parent.n  );  };

    auto  descend_to  =  [&]  ( const Bind & raw )  {
      parent  =  raw .dst;    //  .dst is eternal becaus .type != o_full
      mode    =  raw .mode  ||  mode;  };

    Bind  b;

    auto  single  =  [&]  ()  {
      if  ( is_overbound  ( b ) )  {  return;  }
      if  ( is_descendant ( b ) )  {  descend_to ( b );  }  };

    oStr  trunk;

    auto  full  =  [&]  ( const Option & o )  {
      ;;;
      trunk  =  child .trunk();    //  trunk is a subdir of root
      b.set ( o, trunk );
      if  ( not is_dir ( b.newroot_dst ) )  {  return;  }
      if  ( is_overbound ( b ) )  {  return;  }
      parent  =  trunk;
      mode    =  b.mode  ||  mode  ;  };

    options  (  [&]( const Option o )  {
      switch  ( o.type )  {
	case  o_newroot:    //  fallthrough to o_bind
	case  o_partial:    //  fallthrough to o_bind
	case  o_bind:       b.set(o);  single();  break;
	case  o_full:       full(o);  break;
	default:            break;  }  }  );  }


  static void  options  ( sink<Option> fn )  {   //  ---------  Logic  options
    //  parse the command line options and pass each option to fn().
    Option_Reader  r  ( st .argv + 1 );
    while  ( r.next() )  {  fn ( r.ref );  }  }


  static void  scandir  ( Str path, sink<Dirent> fn )  {    //  Logic  scandir
    DIR *  dirp  =  opendir ( path.s );
    if  ( dirp == nullptr )  {
      die1  ( "opendir  failed  '%s'", path.s );  }
    for  (  Dirent entry;  entry = readdir ( dirp );  )  {
      if  (  entry == "."  ||  entry == ".."  )  {  continue;  }
      fn ( entry );  }
    closedir ( dirp );  }


private:


  static bool  is_inside_readauto    //  ----------  Logic  is_inside_readauto
  ( Str path )  {
    return  path .is_inside ( st .env .get("HOME") )
      ||    path .is_inside ( "/tmp" )
      ||    path .is_inside ( "/var/tmp" );  }


  static bool  is_inside_workdir    //  ------------  Logic  is_inside_workdir
  ( Str path )  {
    bool  rv  =  false;
    options (  [&](auto o)  {
      rv  |=  o.type == o_wd  &&  path .is_inside ( o.arg0 );  }  );
    return  rv;  }


  static bool  is_overbound   //  -----------------------  Logic  is_overbound
  ( const Bind & b )  {
    //  return true iff some later Option overbinds b.dst.
    Option_Reader  r  ( b .option );    //  start after b's option
    while  ( r.next() )  {
      if  ( is_overbound_by ( b, r.ref ) )  {  return  true;  }  }
    return  false;  }


  static bool  is_overbound_by    //  ------------------  Lib  is_overbound_by
  ( const Bind & b, const Option & o )  {

    //  return true iff b is overbound by o.

    auto  do_full  =  [&]  ()  {
      if  ( b.type == o_newroot )  {  return  false;  }
      oStr  full_trunk  =  o.arg0 + "/" + b.dst .trunk();
      return  Lib :: is_dir ( full_trunk );  };

    switch  ( o.type )  {
      case  o_full:       return  do_full();  break;
      case  o_partial:    return  b.dst .is_inside ( o.arg0 );  break;
      case  o_bind:       return  b.dst .is_inside ( o.arg0 );  break;
      default:            return  false;  break;  }  }


};    //  end  struct  Logic  ----------------------------  end  struct  Logic


Logic  q;    //  -------------------------------------------  global  Logic  q




struct  Init_Tool  {    //  xxin  -------------------------  struct  Init_Tool


  static void  process  ( const Option & v )  {    //  ---  Init_Tool  process

    switch ( v.type )  {

      //  long options
      case  o_env:        mut .opt('e')      =  true;    break;
      case  o_network:    mut .opt('n')      =  true;    break;
      case  o_pulse:      mut .opt('p')      =  true;    break;
      case  o_root:       mut .opt('r')      =  true;    break;
      case  o_write:      mut .opt('w')      =  true;    break;
      case  o_x11:        mut .opt('x')      =  true;    break;
      case  o_trace:      global_opt_trace   =  o_trace;     break;

      //  other options
      case  o_cd:         cd(v);                             break;
      case  o_dashdash:   mut .dash          =  v.a;         break;
      case  o_full:       guestname(v);                      break;
      case  o_help:       Lib :: help_print();               break;
      case  o_help_more:  Lib :: help_more_print();          break;
      case  o_newroot:    mut .newroot_mode  =  v.mode;
	;;;               mut .newroot       =  v.arg0;      break;
      case  o_src:        guestname(v);                      break;
      case  o_version:    version();                         break;
      case  o_wd:         mut .workdir       =  v.arg0;      break;
      case  o_shortopt:   shortopt(v);                       break;

      case  o_bind:       break;
      case  o_partial:    break;
      case  o_setenv:     break;

      default:  die1 ( "init_tool  bad option  %d  %s",
		       v.type,  o2s(v.type) )  break;  }  }


private:


  static void  cd  ( const Option & a )  {    //  -------------  Init_Tool  cd
    if  ( st .chdir )  {  die1 ( "extra cd option  %s", a.arg0.s );  }
    mut .chdir  =  a.arg0;  }


  static void  guestname  ( const Option & o )  {    //  ----------  guestname
    if  ( o.arg0 == "/" )  {  return;  }
    mut .guestname  =  o.arg0;  }


  static void  shortopt  ( const Option & a )  {    //  -  Init_Tool  shortopt
    for  (  mstr p = a.a[0].s+1  ;  *p  ;  p++  )  {
      //        ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_'abcdefghijklmnopqrstuvwxyz
      if  ( ( 0b0000000000000000000000000000000000001000000001010100001100
	      >>  ( 'z' - *p ) )  &  0x01 )  {
	mut .opt(*p)  =  true;  continue;  }
      die1 ( "bad short option  '%c'", *p );  break;  }  }


  static  void  version  ()  {    //  --------------------  Init_Tool  version
    printe ( "lxroot  version  " LXROOT_VERSION );
    exit ( 0 );  }


};    //  end  struct  Init_Tool  --------------------  end  struct  Init_Tool




struct  Env_Tool  :  Lib  {  //  xxen  ---------------------  struct  Env_Tool


  static void  option  ( const Option & o )  {    //  ------  Env_Tool  option
    if  ( o.type == o_setenv )  { mut .env .set ( o.a[0] );  }  }


  static void  essential  ()  {    //  ------------------  Env_Tool  essential

    mut .env .soft ( "PATH="  "/usr/local/bin:/usr/local/sbin"
		     ":/usr/bin:/usr/sbin"  ":/bin:/sbin"
		     ":/usr/local/games:/usr/games"  );

    const Fgetpwent &  in   =  st .inside;
    const Fgetpwent &  out  =  st .outside;

    auto  get  =  []  ( Str name )  {
      return  opt('r')  ?  nullptr  :  Lib :: getenv ( name );  };

    mut .env .soft ( "HOME",    in.dir  || get("HOME")    || out.dir  );
    mut .env .soft ( "LOGNAME", in.name || get("LOGNAME") || out.name );
    mut .env .soft ( "USER",    in.name || get("USER")    || out.name );

    auto  vars  =  { "LANG", "LC_COLLATE",  "TERM", "TZ" };
    for  ( auto s : vars )  {  mut .env .soft_copy ( s );  }

    mut .env .soft ( "FAKEROOTDONTTRYCHOWN=1" );  }


  static void  argv  ()  {    //  ----------------------------  Env_Tool  argv
    if  ( st.dash[1] )  {  mut .command  =  st.dash + 1;  return;  }
    Str  shell  =  st .env .get ( "SHELL" );
    if  ( shell == "/bin/bash" )  {  mut .command  =  bash_command;  }
    else if  ( shell )  {
      bash_command[0]  =  shell.s;
      bash_command[1]  =  nullptr;
      mut .command     =  bash_command;  }
    else  {  die1 ( "please specify a command" );  }  }


  static void  ps1_bash  ()  {    //  --------------------  Env_Tool  ps1_bash

    Str   term  =  st .env .get ( "TERM" );
    Str   user  =  st .env .get ( "USER" );
    oStr  opts  =  nullptr;
    oStr  host  =  "./" + ( st .guestname || st .newroot ) .basename()  ;

    opts  =  "-";
    if  ( opt('e') )  {  opts  +=  "e";  }
    if  ( opt('n') )  {  opts  +=  "n";  }
    if  ( opt('p') )  {  opts  +=  "p";  }
    if  ( opt('r') )  {  opts  +=  "r";  }
    if  ( opt('w') )  {  opts  +=  "w";  }
    if  ( opt('x') )  {  opts  +=  "x";  }

    oStr  ps1  =  "PS1=";

    if  ( term  ==  "xterm-256color" )  {    //  set terminal title
      ps1  +=  Str("\\[\\e]0;") + user + "  ";
      if  ( opts.n > 1 )  {  ps1  +=  opts  +  "  ";  }
      ps1  +=  host  +  "  \\W\\007\\]";  }

    //r  bright_cyan  =  "\\[\\e[0;96m\\]";
    Str  bright_red   =  "\\[\\e[0;91m\\]";
    Str  cyan         =  "\\[\\e[0;36m\\]";
    //r  red          =  "\\[\\e[0;31m\\]";
    Str  normal       =  "\\[\\e[0;39m\\]";

    ps1  +=  Str("\n") + cyan + user + "  ";
    if  ( opts.n > 1 )  {  ps1  +=  bright_red + opts + cyan + "  ";  }
    ps1  +=  host + "  \\W" + normal + "  ";

    mut .env .set ( ps1.leak() );  }


  static bool  is_busybox  ( Str path )  {    //  ------  Env_Tool  is_busybox
    return  is_link ( path )  &&  readlink ( path ) == "/bin/busybox";  }


  static void  passthru  ()  {    //  --------------------  Env_Tool  passthru
    if  ( opt('e') )  {
      for  (  Argv p (environ)  ;  p[0].n  ;  p ++  )  {
	trace ( "env_tool  passthru  %s", p[0].s );
	mut .env .soft ( * p );  }  }  }


  static void  ps1  ()  {    //  ------------------------------  Env_Tool  ps1
    //  20200626  todo?  add custom prompts for other shells
    //  20211030  probably not.  use selective env var passthru instead.
    if  (  st.command[0] == "/bin/bash"
	   ||  is_busybox ( st.command[0] )  )  {  ps1_bash();  }  }


  static void  shell  ()  {    //  --------------------------  Env_Tool  shell

    auto  shell  =  [&]  ( Str path )  {
      if  ( is_file ( path )  &&  access ( path.s, X_OK ) == 0 )  {
	mut .env .set ( "SHELL", path );  return  true;  }
      return  false;  };

    st .env .get ( "SHELL" ).n
      ||  shell ( st .inside .shell )
      ||  shell ( getenv("SHELL") )
      ||  shell ( st .outside .shell )
      ||  shell ( "/bin/ash" )
      ||  shell ( "/bin/sh" );  }


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


  oStr  put_old;


  void  init  ()  {    //  -------------------------------------  Lxroot  init
    q.options ( Init_Tool :: process );
    if  ( st.newroot == nullptr )  {
      die1 ( "please specify a newroot directory" );  }  }


  void  unshare  ()  {    //  -------------------------------  Lxroot  unshare
    int  clone_flags  =  0;
    clone_flags  |=  CLONE_NEWNS;      //  mount namespace  ( optional )
    clone_flags  |=  CLONE_NEWPID;     //  pid   namespace  ( optional )
    clone_flags  |=  CLONE_NEWUSER;    //  user  namespace  ( required )
    clone_flags  |=  opt('n')  ?  0  :  CLONE_NEWNET  ;
    trace ( "" );
    sys .unshare ( clone_flags );  }


  void  uid_map  ()  {    //  -------------------------------  Lxroot  uid_map

    //  see  https://lwn.net/Articles/532593/

    const uid_t  un_uid  =  opt('r')  ?  0  :  st.uid  ;
    const gid_t  un_gid  =  opt('r')  ?  0  :  st.gid  ;

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
    if  ( st.newroot == nullptr )  {  return;  }
    q.binds (  [](auto b)  {
      sys .bind ( b.newroot_dst, b.src );  }  );
    //  note  /proc is mounted later on in Lxroot :: proc().
    //  note  someday I may integrate /dev and /sys into q.bind
    sys .bind ( oStr ( st.newroot + "/dev" ), "/dev" );
    sys .bind ( oStr ( st.newroot + "/sys" ), "/sys" );  }


  void  dns_check  ()  {    //  ---------------------------  Lxroot  dns_check
    Str   src  =  "/etc/resolv.conf";
    oStr  dst  =  st.newroot + src;
    if  ( opt('n') == false )  {  return;  }
    if  ( is_file ( dst ) )  {  return;  }
    str  m  =  "\nlxroot:  Copy /etc/resolv.conf into newroot?  [y/n]  ";
    const int  rv  =  prompt ( m, 15 );
    if ( rv == prompt_timeout )  {  printf
	( "\nlxroot:  Timeout.  Continuing without resolv.conf.\n" );  }
    if ( rv == 'y' )  {  sys.file_copy ( src, dst, 0644 );  }  }


  void  fgetpwent  ()  {    //  ---------------------------  Lxroot  fgetpwent
    mut .outside .fgetpwent ( "/etc/passwd", getuid() );
    mut .inside  .fgetpwent ( oStr ( st.newroot + "/etc/passwd" ), getuid() );
    Env_Tool :: before_pivot();  }


  void  env  ()  {  Env_Tool :: after_pivot();  }    //  --------  Lxroot  env


  static void  expose_path    //  -----------------------  Lxroot  expose_path
  ( Str path, opt_t readauto = o_none )  {
    if  (  st.newroot == nullptr  )  {  return;  }
    if  (  path == nullptr        )  {  return;  }
    mStr  parent;
    mopt  mode;
    q.calculate_parent ( path, parent, mode );
    oStr   parent2 ( parent );
    if  (  path .is_same_path_as ( parent2 ) )  {  return;  }
    if  (  q.actual ( parent2, mode ) == o_rw  )  {  return;  }
    if  (  readauto == o_none  ||  mode == o_ra  )  {
      oStr  newroot_path  =  st.newroot + path;
      if  ( is_dir ( newroot_path ) )  {
	trace ( "  expose   '%s'", newroot_path.s );
	sys .bind ( newroot_path, newroot_path );  }  }  }


  void  expose_rw  ()  {    //  ---------------------------  Lxroot  expose_rw
    expose_path ( st .env .get ( "HOME" ), o_ra );
    expose_path ( "/tmp", o_ra );
    expose_path ( "/var/tmp", o_ra );
    q.options  (  [](auto o)  {
      if  ( o.type == o_wd )  {  expose_path ( o.arg0 );  }  }  );  }


  void  remount_ro  ()  {    //  -------------------------  Lxroot  remount_ro
    q.binds  (  [&](auto b)  {
      if ( b.actual == o_ro )  { sys .rdonly ( b.newroot_dst );  }  }  );  }


  void  option_pulse  ()  {    //  ---------------------  Lxroot  option_pulse
    if  ( opt('p') )  {
      Str  xdg_dir  =  getenv ( "XDG_RUNTIME_DIR" );
      if  ( xdg_dir )  {
	mut .env .soft_copy ( "XDG_RUNTIME_DIR" );
        oStr  pulse_dir  =  xdg_dir + "/pulse";
        sys .bind ( oStr ( st .newroot + pulse_dir ), pulse_dir );  }
      else  {
        warn ( "bind_opt_pulse()  XDG_RUNTIME_DIR not set" );  }  }  }


  void  require_env  ( Str k )  {    //  ----------------  Lxroot  require_env
    if  ( getenv(k).n == 0 )  {  die1 ( "$%s not set", k.s );  }  }


  void  option_x11  ()  {    //  -------------------------  Lxroot  option_x11

    if  ( not opt('x') )  {  return;  }
    require_env ("DISPLAY");

    sys .bind ( oStr ( st.newroot + "/tmp/.X11-unix" ), "/tmp/.X11-unix" );
    mut .env .soft_copy ("DISPLAY");

    oStr  xauthority  =  getenv("HOME") + "/.Xauthority";
    if  (  getenv("HOME").n  &&  is_file(xauthority)  )  {
      mut .xauthority  =  sys .file_read ( xauthority );  }  }


  void  options  ()  {    //  -------------------------------  Lxroot  options
    option_pulse();
    option_x11();  }


  void  pivot_prepare  ( Str pivot )  {    //  --------  Lxroot  pivot_prepare
    //  verify that pivot has at least one sub-direcotry (for put_old)
    assert ( put_old.n == 0 );
    assert ( put_old.s == nullptr );
    q.scandir  (  pivot,  [&](auto e)  {
      if  (  put_old.s == nullptr
	     &&  e.is_dir()  &&  e.name() not_eq "proc"  )  {
	put_old  =  "/" + e.name();  }  }  );
    if  ( put_old.n == 0 )  {
      die1 ( "pivot_prepare  no directory in newroot" );  }  }


  void  pivot  ()  {    //  -----------------------------------  Lxroot  pivot
    if  ( st.newroot == nullptr )  {  return;  }
    pivot_prepare ( st.newroot );
    sys .pivot  ( st.newroot,  oStr ( st.newroot + put_old ) );
    sys .chdir  ( "/" );
    sys .chroot ( "/" );  }


  void  fork  ()  {    //  -------------------------------------  Lxroot  fork
    sys .fork();
    if  ( sys .fork_pid == 0 )  {  return;  }    //  child returns
    if  ( sys .fork_pid >  0 )  {    //  parent waits for child to exit
      sys .wait();
      if  ( WIFEXITED ( sys .wstatus ) )  {
	exit ( WEXITSTATUS ( sys .wstatus ) );  }
      printe ( "lxroot  warning  child exited abnormally" );
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

    auto  chdir_home  =  [&]  ()  {
      Str  home  =  st .env .get ( "HOME" );
      if  (  st .newroot  &&  is_dir ( home )  )  { sys .chdir ( home );  }  };

    auto  chdir2  =  [&]  ( Str s )  {
      if  ( s && s.s[0] != '/' )  {  chdir_home();  }
      sys .chdir ( s );  };

    if  ( st .chdir   )  {  chdir2 ( st .chdir   );  return;  }
    if  ( st .workdir )  {  chdir2 ( st .workdir );  return;  }
    chdir_home();  }


  int  xset_q  ()  {    //  ----------------------------------  Lxroot  xset_q

    //  xset_q() runs after pivot, fork, and env.

    if  ( not is_file ( "/usr/bin/xset" ) )  {
      die1 ( "file not found  /usr/bin/xset\n" );  }

    const int  child  =  :: fork();
    if  ( child == -1 )  {  return  -1;  }
    if  ( child ==  0 )  {
      const char * const  argv[]  =  { "/usr/bin/xset", "q", nullptr };
      close(1);                          //  close  fd 1 (stdout)
      open ( "/dev/null", O_WRONLY );    //  reopen fd 1 (stdout)
      execve ( argv[0], (char**) argv, (char**) st.env.data() );
      printe ( "lxroot  xset_q  child  execve failed" );
      exit ( 1 );  }
    int wstatus = 0;  wait(&wstatus);  return  wstatus;  }


  void  x11_check  ()  {    //  ------------------------------------  x11_check

    //  x11_check() runs after pivot, fork, and env.

    if  ( not opt('x')  )  {  return;  }
    if  ( xset_q() == 0 )  {  return;  }

    if  ( st.xauthority )  {
      str  m  =  "\nlxroot:  Copy ~/.Xauthority into newroot?  [y/n]  ";
      switch  ( prompt ( m, 15 ) )  {
	case  'y':  {
	  oStr  path  =  st .env .get("HOME") + "/.Xauthority";
	  sys .file_write ( path, st.xauthority, 0600 );
	  if  ( xset_q() == 0 )  {  return;  }  }  break;
	case  prompt_timeout:
	  printe ( "lxroot:  prompt timeout.  continuing..." );  break;  }  }

    printe ( "lxroot  error  '/usr/bin/xset q' failed." );
    die1 ( "X server connection failed." );  }


  void  xray  ()  {    //  -------------------------------------  Lxroot  xray

    if  ( global_opt_trace == o_trace )  {

      printe ( "" );
      printe ( "xray  uid  %d  %d", getuid(), getgid() );
      printe ( "xray  cwd  %s", getcwd().s );
      printe ( "xray  environment" );
      Argv ( st .env .data() ) .print ( "xray    env  " );
      printe ( "xray  dash_dash  %s  %d",
               st .dash[0] .s, st .dash .len() );
      printe ( "xray  command" );
      st .command .print ( "xray    cmd  " );

      if  ( st .before_pivot )  {
	printe ( "" );
	printe ( "xray  binds" );
	q.binds  (  [](auto b)  { b.trace ( "  " ); }  );  }

      printe ( "" );
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
    dns_check();     //  after bind, before remount
    fgetpwent();     //  after bind, before expose
    expose_rw();     //  after fgetpwent, before remount
    remount_ro();    //  after expose
    options();       //  late, before pivot, after remount
    /*  pivot to umount2 should be as tight as possible  */
    pivot();         //  late, but before fork? ( !! all paths change !! )
    fork();          //  prefer after pivot (so that parent is not in put_old)
    proc();          //  after fork, before umount2
    umount2();       //  after proc
    /*  pivot to umount2 should be as tight as possible  */
    env();           //  after pivot, therefore after umount2
    chdir();         //  after env
    x11_check();     //  after pivot, fork, env.  after chdir (why?)
    xray();
    exec();          //  last
    return  1;  }    //  exec failed!


};    //  end  class  Lxroot  ----------------------------  end  class  Lxroot




#ifndef  LXROOT_MAIN_SKIP
int  main  ( const int argc, str argv[] )  {    //  --------------------  main
  if  ( argc < 2 )  {  Lib :: help_print();  }
  mut .argv  =  argv;
  return  Lxroot() .main();  }
#endif




//  lxroot.cpp  -  Create and use chroot-style virtual software environments.
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
