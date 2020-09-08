

//  -*- mode: c++ -*-


//  Copyright (c) 2020 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv2 (see end of file) WITHOUT ANY WARRANTY.


#define  LXROOT_VERSION  "0.0.20200908.0000"


//  compile with:  g++  -Wall  -Werror  lxroot.cpp  -o lxroot


//  Welcome to the source code for lxroot.
//
//  The classes and structs in lxroot can be divided into three categories:
//
//  Wrappers and interfaces:
//
//  struct  Lib      contains generally useful functions.
//  class   Opendir  allows iteration over the contents of a directory.
//  struct  Syscall  provides error handling and tracing of syscalls.
//
//  Data storage classes:
//
//  class    opt      stores an option flag (represented by a string).
//  struct   mstr     a mutable reference to an immutable string.
//  typedef  str      a const mstr.
//  class    Tokens   can tokenize argv or a text config file.
//  class    Vec      std :: vector < mstr >, with augmentations.
//  class    Env      configures the variables of the new environment.
//  struct   PBind    specifies a single bind mount (src to dst).
//  struct   Profile  specifies all the characteristics of the namespace
//                     environment that lxroot will create.
//
//  Tool classes that modify or process data storage objects:
//
//  class   Profile_Parser    parses the next profile from a config file.
//  class   Profile_Loader    loads a specific profile from a config file.
//  class   Argv_Parser       configures a Profile as specified by argv.
//  class   Profile_Finisher  finishes the configuration of a Profile.
//  class   Chroot_Preparer   creates subdirectories and symlinks, if needed.
//  class   Launcher          creates the namespace environment specified by a
//                              Profile, and executes the specified command.


#include  <dirent.h>
#include  <errno.h>
#include  <fcntl.h>
#include  <limits.h>
#include  <pwd.h>
#include  <sched.h>
#include  <signal.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>
#include  <sys/mount.h>
#include  <sys/stat.h>
#include  <sys/syscall.h>
#include  <sys/types.h>
#include  <sys/wait.h>


#include  <map>
#include  <string>
#include  <vector>


//  macro  printe  --------------------------------------------  macro  printe
#define  printe( ... )  fprintf ( stderr, __VA_ARGS__ );




void  usage  ()  {    //  ---------------------------------------------  usage
  printe (
"\n"
"usage:  lxroot  [-agnr]  newroot   [--]  [n=v ...]  [command [arg ...]]\n"
"  or    lxroot  [-agnr]  @profile  [--]  [n=v ...]  [command [arg ...]]\n"
"  or    lxroot  [-agnr]  -- | n=v        [n=v ...]  [command [arg ...]]\n"
"  or    lxroot  [-agnr]\n"
"\n"
"lxroot allows a non-root user to run one or more commands inside a\n"
"chroot-style environment (specifically, inside a Linux user namespaces).\n"
"\n"
"common options\n"
" -a     provide audio access (mount --bind /run/user/$UID/pulse)\n"
" -g     provide GUI access (mount --bind /tmp/.X11-unix)\n"
" -n     provide network access (via CLONE_NEWNET == 0)\n"
" -r     simulate root user (map uid and gid to zero)\n"
//  todo  -w is not yet implemented
//  " -w     bind mount the current directory as read-write (if possible)\n"
" n=v    set one or more environment variables\n"
"\n"
"other options\n"
" --help          display this help information\n"
//" --pulseaudio    provide access to pulseaudio (experimental)\n"
" --trace         display Profile information and trace syscalls\n"
" --version       display version information\n"
	  );  }


//  a audio     f           k           p (pid?)    u (user?)   z           |
//  b           g gui       l           q           v                       |
//  c           h           m           r root      w (write?)              |
//  d           i           n net       s           x                       |
//  e           j           o           t           y                       |




//  macro  die  --------------------------------------------------  macro  die
//  see  https://stackoverflow.com/q/5588855   regarding ##__VA_ARGS__
//  see  https://stackoverflow.com/a/11172679  regarding ##__VA_ARGS__
#define  die_pe( format, ... )  {					\
  fprintf ( stderr, "lxroot  error  " format "  ", ##__VA_ARGS__ );	\
  perror(NULL);								\
  exit ( 1 );  }


//  macro  die2  ------------------------------------------------  macro  die2
#define  die2( format, ... )  {						\
  fprintf ( stderr, "lxroot  error  " format "\n", ##__VA_ARGS__ );	\
  exit ( 1 );  }


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


/*  20200829  apparently unused
void  print  ( const char * const * p )  {    //  -------------  argv :: print
  if  ( p )  {  printe ( "%s", *p );  p++;  }
  for  (  ;  p && *p;  p++  )  {  printe ( "  %s", *p );  }
  printe ( "\n" );  }
*/


class  opt  {    //  ---------------------------------------------  class  opt
  const char *  p  =  nullptr;
public:
  opt  ()  {}
  opt  ( const char * p )  :  p(p)  {}
  void  operator =   ( const char * s )  {  p  =  s;  }
  //  opt   operator ||  ( const opt & b )  const  {
  //    return  s == "0"  ?  opt{b.s}  :  opt{b.s}  ;  }
  bool  operator ==  ( const char * s )  const  {
    if  (  p == s  )  {  return  true;  }
    if  (  p == nullptr  ||  s == nullptr  )  {  return  false;  }
    return  strcmp ( p, s ) == 0;  }
  void  operator |=  ( const opt & b )  {  p  =  p  ?  p  :  b.p  ;  }
  int  n  ()  const  {  return  p  ?  1  :  0  ;  }
  const char * s  ()  const  {  return  p  ?  p  :  "0"  ;  }
};    //  end  struct  opt  --------------------------------  end  struct  opt


//  struct  str  ------------------------------------------------  struct  str

typedef  const char * const * Argv;
struct   mstr;                           //  mstr  =  mutable   string
typedef  const mstr  str;                //  str   =  immutable string
typedef  std :: vector < mstr >  vec;    //  vec   =  std :: vector < mstr >

struct  mstr  {

  const char *  s;
  int           n;

  mstr  ()  :  s ( nullptr ), n ( 0 )  {}

  mstr  ( const char * const src )  :
    s ( src ? strdup ( src ) : nullptr ),
    n ( src ? strlen ( src ) : 0 )  {}

  mstr  ( const char * const src, const int len )  :
    s ( src ? strndup ( src, len ) : nullptr ),
    n ( src ? strlen ( s ) : 0 )  {}

  mstr  ( mstr & a )  :
    s ( a.s ? strndup ( a.s, a.n ) : nullptr ),
    n ( a.s ? strlen ( s ) : 0 )  {}

  mstr  ( str & a )  :
    s ( a.s ? strndup ( a.s, a.n ) : nullptr ),
    n ( a.s ? strlen ( s ) : 0 )  {}

  mstr  ( str & a, str & b )  :
    s ( concat ( a, b ) ),
    n ( a.n + b.n )  {}

  ~mstr  ()  {
    if  ( s )  {  free ( (void*) s );  }  }

  mstr &  operator =  ( str & a )  {
    if  ( s )  {  free ( (void*) s );  s  =  nullptr;  n  =  0;  }
    s  =  a.s  ?  strndup ( a.s, a.n )  :  nullptr  ;
    n  =  a.s  ?  strlen ( s )  :  0  ;
    return  * this;  }

  bool  operator ==  ( const char * b )  const  {
    if  ( s == b )  {  return  true;  }
    if  ( s == nullptr  ||  b == nullptr )  {  return  false;  }
    return  strcmp ( s, b ) == 0;  }

  bool  operator !=  ( const char * b )  const  {
    return  not ( operator == ( b ) );  }

  bool  operator <  ( const char * b )  const  {
    if  ( s == b )  {  return  false;  }
    if  ( s == nullptr )  {  return  true;   }
    if  ( b == nullptr )  {  return  false;  }
    return  strcmp ( s, b ) < 0;  }

  operator const char *  ()  const  {  return  s;  }

  mstr  operator +  ( str & b )  const  {  return  mstr ( * this, b );  }

  mstr &  operator +=  ( str & b )  {
    * this  =  mstr ( * this, b );  return  * this;  }


  mstr  basename  ()  const  {    //  ------------------------  mstr  basename
    if  ( s == nullptr )  {  return  nullptr;  }
    const char *  found  =  strrchr ( s, '/' );
    if  ( found )  {  return  mstr ( found + 1 );  }
    return  * this;  }


  const char *  begin  ()  const  {    //  ----------------------  mstr  begin
    return  s;  }


  const char *  chr  ( const int c )  const  {    //  ---------  mstr  chr
    return  :: strchr ( s, c );  }


  static mstr  claim  ( const char * unowned )  {    //  --------  mstr  claim
    mstr  rv;
    rv.s  =  unowned;
    rv.n  =  rv.s  ?  strlen ( rv.s )  :  0  ;
    return  rv;  }


  vec  descend  ()  const  {    //  ---------------------------  mstr  descend
    vec  rv;
    const char *  p  =  s;
    while  (  p  &&  *p  )  {
      while  (  *p == '/'  )  {  p ++;  }
      while  (  *p  &&  *p != '/'  )  {  p ++;  }
      str  path  ( s, p - s );
      rv .push_back ( path );  }
    return  rv;  }


  const char *  end  ()  const  {    //  --------------------------  mstr  end
    return  s + n;  }


  mstr  head  ( const char * sep, int start = 0 )  const  {    //  -----  head
    if  ( s == nullptr )  {  return  nullptr;  }
    int  min  =  start < n  ?  start  :  n  ;
    const char *  found  =  strstr ( s + min, sep );
    if  ( found )  {  return  mstr ( s, found - s );  }
    return  * this;  }


  bool  is_subdir_of  ( str & parent )  const  {    //  --  mstr  is_subdir_of
    return  * this == parent  ||  startswith ( parent + "/" );  }


  int  spn  ( const char * accept, int start = 0 )  const  {    //  ------ spn
    if  ( s == nullptr )  {  return  0;  }
    int  min  =  start < n  ?  start  :  n  ;
    return  min + strspn ( s + min, accept );  }


  template < typename... T >    //  --------------------------  mstr  snprintf
  void  snprintf  ( const char * format, size_t lim, T... args )  {
    char    buf_s[lim];  :: snprintf ( buf_s, lim, format, args... );
    size_t  buf_n  =  strlen ( buf_s );
    char *  new_s  =  (char*) malloc ( n + buf_n + 1 );
    new_s[0]  =  '\0';
    strncat ( new_s, s, n );
    strncat ( new_s, buf_s, buf_n);
    if  ( s )  {  free ( (void*) s );  }
    s  =  new_s;
    n  =  strlen ( s );  }


  void  snprintf  ( const char * format, size_t lim )  {    //  ----  snprintf
    snprintf ( format, lim, nullptr );  }


  bool  startswith  ( str & expect )  const  {    //  ------  mstr  starstwith
    return  s  &&  strncmp ( s, expect.s, expect.n ) == 0;  }


  const char *  find  ( const char * needle )  const  {    //  ---  mstr  find
    return  s  ?  strstr ( s, needle )  :  nullptr  ;  }


  mstr  sub  ( int len )  const  {    //  -------------------------  mstr  sub
    if  ( len <= n )  {  return  s + len;  }
    return  nullptr;  }


  mstr  tail  ( const char * sep )  const  {    //  --------------  mstr  tail
    if  ( s == nullptr )  {  return  nullptr;  }
    const char *  found  =  strstr ( s, sep );
    if  ( found )  {  return  found + strlen ( sep );  }
    return  nullptr;  }


  vec  tokens  ()  const  {    //  -----------------------------  mstr  tokens
    vec  rv;
    const char *  p  =  s;
    while  (  p  &&  * p  )  {
      while  (  p  &&  * p == ' '  )  {  p ++;  }
      const char *  p2  =  p;
      while  (  p2  &&  * p2  &&  * p2 != ' '  )  {  p2 ++;  }
      if  ( p2 > p )  {
	mstr  token ( p, p2 - p );
	rv .push_back ( token );
	p  =  p2;  }  }
    return  rv;  }


private:


  const char *  concat  ( str & a, str & b )  {    //  -----------  mstr  cat
    char *  buf  =  (char*)  malloc ( a.n + b.n + 1 );
    buf[0]  =  '\0';
    strncat ( buf, a.s, a.n );
    strncat ( buf, b.s, b.n );
    return  buf;  }


};    //  end  struct  mstr


str  nullstr  =  {  nullptr,  0  };    //  --------------------------  nullstr


mstr  operator +  ( const char * a, str & b )  {    //  ----------------  op +
  return  mstr ( a, b );  }


//  end  struct  str  ---------------------------------------  end  strct  str




struct  Lib  {    //  -------------------------------------------  struct  Lib


  static void  assert_is_dir    //  ----------------------  Lib  assert_is_dir
  ( const char * const path, const char * const m )  {
    if  ( is_dir ( path ) )  {  return;  }
    printe ( "lxroot  %s  directory not found  %s\n", m, path  );
    exit ( 1 );  }


  static bool  eq  ( const char * a, const char * b )  {    //  -----  Lib  eq
    if  ( a == NULL  ||  b == NULL )  return  false;
    return  strcmp ( a, b ) == 0;  }


  static str  getcwd  ()  {    //  ------------------------------  Lib  getcwd
    return  str :: claim ( get_current_dir_name() );  }


  static str  home  ()  {    //  ----------------------------------  Lib  home
    return  getenv ( "HOME" );  }


  static bool  is_busybox ( str & path )  {    //  ----------  Lib  is_busybox
    return  is_link ( path )  &&  readlink ( path ) == "/bin/busybox";  }


  static bool  is_dir  ( const char * path )  {    //  ----------  Lib  is_dir
    struct stat  st;
    if  (  path  &&  stat ( path, & st ) == 0  &&  st .st_mode & S_IFDIR  )  {
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


  static bool  is_file  ( const char * path )  {    //  --------  Lib  is_file
    struct stat  st;
    if  (  path  &&  stat ( path, & st ) == 0  &&  st .st_mode & S_IFREG  )  {
      return  true;  }
    errno  =  ENOENT;
    return  false;  }


  static bool  is_link  ( const char * path )  {    //  --------  Lib  is_link
    struct stat  st;
    if  (  path
	   &&  lstat ( path, & st ) == 0
	   &&  S_ISLNK ( st .st_mode )  )  {  return  true;  }
    errno  =  ENOENT;
    return  false;  }


  static str  readlink  ( str path )  {    //  ----------------  Lib  readlink
    struct stat  st;
    if  (  lstat ( path, & st ) == 0  &&  st .st_mode & S_IFLNK  )  {
      ssize_t  lim  =  st .st_size + 2;
      char *   buf  =  (char*)  malloc ( lim );
      if  ( buf )  {
	memset ( buf, '\0', lim );
	ssize_t  len  =  :: readlink ( path, buf, lim );
	if  ( len == lim - 2 )  {
	  str  rv  =  buf;
	  free ( buf );
	  return  rv;  }  }
      printe ( "lxroot  readlink  failed  %s\n", path.s );
      exit ( 1 );  }
    return  nullptr;  }


  static str  realpath  ( const char * const path )  {    //  ------  realpath
    return  str :: claim ( :: realpath ( path, nullptr ) );  }


  static Argv  skip  ( Argv a, int n )  {    //  ------------------  Lib  skip
    while  (  a  &&  * a  &&  n-- > 0  )  {  a++;  }
    return  a;  }


};    //  end  struct  Lib  --------------------------------  end  struct  Lib




class  Tokens  {    //  ---------------------------------------  class  Tokens


  vec     t;
  size_t  n  =  0;


public:


  Tokens  ()  {}    //  ----------------------------------------  Tokens  ctor


  Tokens  ( const Argv & argv )  {  read_from ( argv );  }
  Tokens  ( const str  & path )  {  read_from ( path );  }


  operator  bool  ()  const  {    //  -----------------------  Tokens  op bool
    return  n < t.size();  }


  operator  str &  ()  const  {    //  -----------------------  Tokens  op str
    return  peek();  }


  void  bad_token  ( const char * m )  {    //  -----------  Tokens  bad_token
    printe ( "tokens  bad token  %s  %lu  %lu  >%s<\n",
	     m, n, t.size(), s()  );
    exit ( 1 );  }


  const char *  chr  ( const int c )  const  {    //  -----------  Tokens  chr
    return  peek() .chr ( c );  }


  bool  is  ( const char * expect )  {    //  --------------------  Tokens  is
    if  (  n < t.size()  &&  t.at(n) == expect  )  {
	//  printe ( "tokens  is    %s\n", t.at(n).s );
	n ++;  return  true;  }
    return  false;  }


  bool  more  ()  const  {    //  ------------------------------  Tokens  more
    return  n < t.size();  }


  str  next  ()  {    //  --------------------------------------  Tokens  next
    if  ( n < t.size() )  {
      //  printe ( "tokens  next  %s\n", t.at(n).s );
      return  t.at(n++);  }
    return  nullptr;  }


  str &  peek  ()  const  {    //  -----------------------------  Tokens  peek
    return  n < t.size()  ?  t.at(n)  :  nullstr  ;  }


  bool  peek  ( const char * expect )  const  {    //  ---------  Tokens  peek
    return  peek() == expect;  }


  void  print  ()  const  {    //  ----------------------------  Tokens  print
    for  ( auto & e : t )  {  printe ( ">%s<\n", e.s );  }  }


  void  read_from  ( const char * const * argv )  {    //  ---------  readfrom
    for  (  ;  * argv;  argv ++  )  {  t .push_back ( * argv );  }  }


  void  read_from  ( const char * path )  {    //  --------  Tokens  read_from
    FILE *  f  =  fopen ( path, "r" );
    if  ( f == nullptr )  {  die_pe ( "read_from  %s", path );  }
    mstr    s;
    auto  next         =  [&]  ()  {  s  =  token_read ( f );  };
    auto  skip_to_eol  =  [&]  ()  {
      while  ( s && s != "\n" )  {  next();  }  };
    for  ( next();  s;  )  {
      if       ( s == "\n" )  {  next();  }
      else if  ( s == "#"  )  {  skip_to_eol();       next();  }
      else                    {	 t .push_back ( s );  next();  }  }
    assert ( fclose ( f ) == 0 );  }


  const char *  s  ()  const  {    //  ----------------------------  Tokens  s
    if  ( n < t.size() )  {  return  t.at(n).s;  }
    return  nullptr;  }


  size_t  spn  ( const char * accept )  {    //  ----------------  Tokens  spn
    return  peek() .spn ( accept );  }


  mstr  tail  ( const char * sep )  const  {    //  Tokens  ------------  tail
    return  peek() .tail ( sep );  }


  str  token_read  ( FILE * f )  {    //  ----------------  Tokens  token_read

    int  c;
    auto  next  =  [&]  ()  {  c  =  fgetc ( f );  return  c != EOF;  };
    auto  undo  =  [&]  ()  {  ungetc ( c, f );  };

    do  {  next();  }  while  (  c == ' '  ||  c == '\t'  );
    if  (  c ==  EOF  )  {  return  nullptr;  }
    if  (  c == '\n'  )  {  return "\n";  }

    std :: string  buf  ( 1, c );
    while  ( next() )  {
      if  (  c == ' '  ||  c == '\n'  ||  c == '\t'  )  {  undo();  break; }
      buf .append ( 1, c );  }

    return  buf .length()  ?  buf .c_str()  :  nullptr  ;  }


};    //  end  class  Tokens  ----------------------------  end  class  Tokens




class  Vec  :  public std :: vector < mstr >  {    //    ---------  class  Vec

  //  Vec can cast to char**.  This is used with execve().


  const char * *  p  =  nullptr;
  std :: string   b;


public:


  Vec  ()  {}


  Vec  ( const char * const * p )  {
    for  (  ;  p  &&  * p;  p ++  )  {  push_back ( * p );  }  }


  ~Vec  ()  {
    if  ( p )  {  free ( p );  }  }


  operator  char * const *  ()  {    //  -----------------------  Vec  op cast
    //  convert Vec to an array of char*
    if  ( p )  {  free ( p );  }
    p  =  (const char**) malloc ( (size() + 1 ) * ( sizeof ( char * ) ) );
    for  (  size_t n = 0;  n < size();  n ++  )  {  p[n]  =  at(n).s;  }
    p[size()]  =  nullptr;
    //  note:  we cast away const to allow passwing to execv().
    return  (char**) p;  }


  void  consume  ( Tokens & t )  {    //  ----------------------  Vec  consume
    while  ( t )  {  push_back ( t .next() );  }  }


  const char *  s  ()  {    //  --------------------------------------  Vec  s
    b.clear();
    for  (  str & s  :  * this  )  {
      b.append ( s );  b.append ( "  " );  }
    if  ( b.size() > 1 )  {  b.resize ( b.size() - 2 );  }
    return  b.c_str();  }


};    //  end  class  Vec  ----------------------------------  end  class  Vec




class  Env  :  public std :: map < mstr, mstr >  {    //    ------  class  Env


  Vec  v;


public:


  operator  char * const *  ()  {    //  -----------------------  Env  op cast
    v .clear();
    for  (  const auto & e  :  * this  )  {
      v .push_back ( e.first + "=" + e.second );  }
    return  v;  }


  str  get  ( str & name )  {    //  -------------------------------  Env  get
    if  ( count ( name ) )  {  return  at ( name );  }
    return  nullstr;  }


};    //  end  class  Env  ----------------------------------  end  class  Env




struct  PBind  {    //  ---------------------------------------  struct  PBind


  //  20200824  idea  perhaps all binds should be recursive?
  //                  ( is there any downside to recursive binds? )

  //  20200829  question  does readonly propogate recursively?


  mstr  type, src, dst;
  opt   readonly;
  opt   recursive  =  "rec";    //  recursive is now the default


  PBind  ( str & type )  :  type(type)  {}

  PBind  ( str & type,  str & src, str & dst, opt readonly )
    :  type(type), src(src), dst(dst), readonly(readonly)  {}


  bool  src_contains  ( str & path )  const  {    //  -  PBind :: src_contains
    str &  s  =  src == "."  ?  dst  :  src;
    return  s .startswith ( path );  }


  void  print  ()  const  {    //  ---------------------------  PBind :: print
    printe ( "pbind  %s  %s  %s\n",  readonly.s(),  dst.s, src.s );  }


};    //  end  struct  PBind  ----------------------------  end  struct  PBind




struct  Profile  :  Lib  {    //  ---------------------------  struct  Profile


  mstr  cwd;
  mstr  new_cwd;
  mstr  name;         //  the name of the profile
  mstr  skel;         //  $HOME/.lxroot/$name
  opt   opt_net;      //  provide network interfaces
  opt   opt_pulse;    //  mount /run/user/$UID/pulse
  opt   opt_root;     //  set uid/gid to zero
  opt   opt_trace;    //  log syscalls to stderr
  opt   opt_write;    //  mount cwd as read-write (if possible)
  opt   opt_x11;      //  mount /tmp/.X11-unix
  Env   env;
  mstr  exec_path;
  Vec   exec_argv;

  bool  is_passthru  =  false;

  std :: vector < PBind >  binds;


  Profile &  operator =  ( const Profile * p )  {    //  ------  Profile  op =
    assert ( p == this );  return  * this;  }


  operator  bool  ()  const  {  return  name;  }    //  ----  Porfile  op bool


  void  print  ()  const  {    //  ---------------------------  Profile  print

    auto  max_dst_len  =  [this]  ()  {
      int  rv  =  0;
      for  (  const PBind & b  :  binds  )  {
	rv  =  std :: max ( rv, b .dst .n );  }
      return  rv;  };

    printe ( "profile\n" );
    printe ( "  cwd      %s\n", cwd.s )
    printe ( "  new_cwd  %s\n", new_cwd.s )
    printe ( "  name     %s\n", name.s );
    printe ( "  skel     %s\n", skel.s );
    printe ( "  opts     n%d  p%d  r%d  w%d  x%d\n",
	     opt_net.n(), opt_pulse.n(), opt_root.n(), opt_write.n(),
	     opt_x11.n() );
    printe ( "  binds    %ld\n", binds .size() );
    for  (  const PBind & b  :  binds  )  {  print ( b, max_dst_len());  }
    printe ( "  env      %ld\n", env .size() );
    printe ( "  exec     %s\n", exec_path.s );
    printe ( "  argv     %ld\n", exec_argv .size() );
    ;;;  }


  void  print  ( const PBind & b, const int dst_len )  const  {    //  -------
    //  "  bind     %-2s  %s  %s\n",
    mstr  s;  s .snprintf ( "  bind     %%-2s  %%-%ds  %%s\n", 80, dst_len );
    printe ( s.s, b.readonly.s(), b.dst.s, b.src.s );  }


  void  binds_append  ( const PBind & pb )  {    //  --  Profile  binds_append
    binds .push_back ( pb );  }


  void  binds_prepend  ( const PBind & pb )  {    //  ---------  binds_prepend
    binds .insert ( binds .begin(), pb );  }


  void  copy_from  ( const Profile & p )  {    //  -------  Profile  copy_from
    assert ( name == nullstr );
    name       =   p.name;
    exec_path  =   p.exec_path;
    opt_net    |=  p.opt_net;
    opt_pulse  |=  p.opt_pulse;
    opt_root   |=  p.opt_root;
    opt_write  |=  p.opt_write;
    opt_x11    |=  p.opt_x11;
    binds      =   p.binds;  }


  bool  has_bind_dst  ( str & dst )  {    //  ---------  Profile  has_bind_dst
    for  (  const PBind & b  :  binds  )  {
      if  ( b .dst == dst )  {  return  true;  }  }
    return  false;  }


  void  reset  ()  {    //  ----------------------------------  Profile  reset
    * this  =  Profile();  }


  void  trace  ()  {    //  ----------------------------------  Profile  trace
    if  ( opt_trace == "trace" )  {  printe ( "\n" );  print();  }  }



/*  20200905
  str  rootdir  ()  {    //  -------------------------------  Profile  rootdir
    assert ( p.name );
    return  home() + "/.lxroot/" + p.name;  }
*/


};    //  end  struct  Profile  ------------------------  end  struct  Profile




class  Profile_Parser  {    //  -----------------------  class  Profile_Parser


  Tokens &   t;
  Profile &  p;


  void  bind  ()  {    //  -----------------------------  Profile_Parser  bind
    if  ( t.is("bind") )  {
      PBind  b  ( "bind" );
      while  ( true )  {
	if       ( t.is("rec") )  {  b.recursive  =  "rec";  }
	else if  ( t.is("ro")  )  {  b.readonly   =  "ro";   }
	else if  ( t.is("rw")  )  {  b.readonly   =  "rw";   }
	else  {  break;  }  }
      if  (  t.peek("=")  ||  t.peek("@")  )  {
	;      b.src  =  t.next();  b.dst  =  t.next();  }
      else  {  b.dst  =  t.next();  b.src  =  t.next();  }
      p.binds_append ( b );  }
    else  {  t .bad_token ( "profile  read_bind" );  }  }


  void  opts  ()  {    //  -----------------------------  Profile_Parser  opts
    if  ( t.is("opts") )  {
      while  ( true )  {
	if       ( t.is("net")     )  {  p.opt_net    =  "net";    }
	else if  ( t.is("x11")     )  {  p.opt_x11    =  "x11";    }
	else if  ( t.is("pulse")   )  {  p.opt_pulse  =  "pulse";  }
	else  {  return;  }  }  }
    else  {  t .bad_token ( "profile  read_opts" );  }  }


public:


  Profile_Parser  ( Tokens & t, Profile & p )    //  ---  Profile_Parser  ctor
    :  t(t), p(p)  {}


  void  profile  ()  {    //  -----------------------  Profile_Parser  profile
    if  ( t.is("profile") )  {
      p.name  =  t.next();
      while  ( t )  {
	if       ( t.peek("bind" ) )  {  bind();  }
	else if  ( t.peek("opts" ) )  {  opts();  }
	else  {  break;  }  }  }  }


};    //  end  class  Profile_Parser  ------------  end  class  Profile_Parser




class  Profile_Loader  :  Lib  {    //  ---------------  class  Profile_Loader


  Tokens   tokens;
  Profile  profile;


  struct  Iterator  {    //  -----------------------  Profile_Loader  Iterator
    Profile_Loader *  p;
    Iterator ( Profile_Loader * p )  :  p(p)  {}
    bool  operator !=  ( const Iterator & end )  const  { return  p != end.p; }
    void  operator ++  ()  {
      p -> read_next_profile();
      p  =  p -> profile .name  ?  p  :  nullptr  ;  }
    const Profile &  operator *  ()  const  {  return  p -> profile;  }  };


  struct  Iterable  {    //  -----------------------  Profile_Loader  Iterable
    Profile_Loader *  pl;
    Iterable  ( Profile_Loader * pl )  :  pl(pl)  {}
    Iterator  begin  ()  {  return  Iterator ( pl      );  }
    Iterator  end    ()  {  return  Iterator ( nullptr );  }  };


  Iterable  profiles  ()  {    //  -----------------  Profile_Loader  profiles
    tokens .read_from ( home() + "/.config/lxroot/config" );
    read_next_profile();
    return  Iterable ( this );  }


  void  read_next_profile  ()  {    //  -------------------  read_next_profile
    profile .reset();
    Profile_Parser  parser  ( tokens, profile );
    parser .profile();  }


public:


  Profile  load_by_cwd  ( str & cwd )  {    //  -----------------  load_by_cwd
    for     (  const Profile & p   :  profiles()  )  {
      for   (  const PBind   & pb  :  p.binds    )  {
	if  ( cwd .is_subdir_of ( pb .src ) )  {  return  p;  }  }  }
    return  Profile();  }


  Profile  load_by_name  ( str & name )  {    //  --------------  load_by_name
    for  (  const Profile & p  :  profiles()  )  {
      if  ( p.name == name )  {  return  p;  }  }
    return  Profile();  }


};    //  end  class  Profile_Loader  ------------  end  class  Profile_Loader




class  Argv_Parser  :  Lib  {    //  ---------------------  class  Argv_Parser


  Tokens     t;
  Profile &  p;


  static bool  is_opt  ( str & s )  {    //  ------------  Argv_Parser  is_opt
    return  (  s .spn ( "-" ) == 1  &&
	       s .spn ( "anrx", 1 ) == s.n  );  }


  static bool  is_setenv  ( str & s )  {    //  ------  Argv_Parser  is_setenv
    const char *  var_name_allowed  =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz";
    const int  n  =  s .spn ( var_name_allowed );
    if  ( n == 0 )  {  return  false;  }
    return  s .spn ( "=", n ) > n;  }


  void  command  ()  {    //  --------------------------  Argv_Parser  command
    p.exec_path  =  t.peek();
    p.exec_argv .consume ( t );  }


  void  dashdash  ()  {    //  ------------------------  Argv_Parser  dashdash
    if  ( p.name )           {  return;  }
    if  ( p.binds .size() )  {  return;  }
    const Profile  p2  =  Profile_Loader() .load_by_cwd ( p.cwd );
    if  ( p2 .name )  {  p.copy_from ( p2 );  return;  }
    die2 ( "\n  No profile contains the current directory.\n"
	   "  (Try \"lxroot --help\" to see more options.)" );  }


  void  option  ()  {    //  ----------------------------  Argv_Parser  option
    if  ( is_opt (t.peek()) )  {
      if  ( t.chr('a') )  {  p.opt_pulse  =  "pulse";  }
      if  ( t.chr('g') )  {  p.opt_x11    =  "x11";    }
      if  ( t.chr('n') )  {  p.opt_net    =  "net";    }
      if  ( t.chr('r') )  {  p.opt_root   =  "root";   }
      //  ( t.chr('w') )  {  p.opt_write  =  "write";  }
      t.next();  }  }


  void  path  ()  {    //  --------------------------------  Argv_Parser  path
    if  ( is_dir ( t.peek() ) )  {
      str  src  =  realpath ( t.next() );
      p.binds_append ( PBind ( "bind", src, "/", "rw" ) );  }
    else  {  die2 ( "directory not found  %s", t.peek().s );  }  }


  void  profile  ()  {    //  --------------------------  Argv_Parser  profile
    const Profile  p2  =  Profile_Loader() .load_by_name ( t.tail("@") );
    t .next();
    if  ( p2 .name )  {  p.copy_from ( p2 );  }
    else  {  die2  ( "profile not found  %s", t.s() );  }  }


  void  setenv  ()  {    //  ----------------------------  Argv_Parser  setenv
    assert ( is_setenv(t) );
    str  head  =  t .peek() .head ( "=" );
    str  tail  =  t .next() .tail ( "=" );
    //  printe ( "read_setenv  %s  %s\n", head.s, tail.s )
    p.env [ head ]  =  tail;  }


  void  trace  ()  {    //  ------------------------------  Argv_Parser  trace
    p.opt_trace  =  "trace";  }


  void  version()  {    //  ----------------------------  Argv_Parser  version
    printe ( "lxroot  version  %s\n", LXROOT_VERSION );
    exit ( 0 );  }


public:


  Argv_Parser  ( const Argv & a, Profile & p )    //  -----  Argv_Parser  ctor
    :  t(skip(a,1)), p(p)  {}


  Profile *  parse  ()  {    //  -------------------------  Argv_Parser  parse

    //  lxroot  [-nrwx]  newroot   [--]  [n=v ...]  [command [arg ...]]
    //  lxroot  [-nrwx]  @profile  [--]  [n=v ...]  [command [arg ...]]
    //  lxroot  [-nrwx]  -- | n=v        [n=v ...]  [command [arg ...]]
    //  lxroot  [-nrwx]

    while  ( t )  {    //  note:  t.is() consumes a matching token
      if  ( t.is("--version") )  {  version();  continue;  }
      if  ( t.is("--help") )     {  usage();    exit(0);   }
      if  ( t.is("--trace") )    {  trace();    continue;  }
      if  ( is_opt(t) )          {  option();   continue;  }
      break;  }

    if       ( not t      )  {  dashdash();               }
    else if  ( t.is("--") )  {  dashdash();               }
    else if  ( t.spn("@") )  {  profile();   t.is("--");  }
    else                     {  path();      t.is("--");  }

    while  ( is_setenv(t) )  {  setenv();  }
    command();

    p.trace();
    return  & p;  }


};    //  end  class  Argv_Parser  ------------------  end  class  Argv_Parser




class  Profile_Finisher  :  Lib  {    //  -----------  class  Profile_Finisher


  Profile &  p;


  void  bind_implicit  ()  {    //  ---------  Profile_Finisher  bind_implicit

    if  (  p.has_bind_dst ( "/" ) )  {  return;  }
    if  (  p.name  )  {

      p.is_passthru  =  true;

      str  rootdir  =  home() + "/.lxroot/" + p.name;

      p.binds_prepend ( PBind ( "bind", rootdir, "/", "ro" ) );

      str  dirs   =  "  /etc  /opt  /usr  ";
      for  ( str & dir : dirs .tokens() )  {
	p.binds_append ( PBind ( "bind", dir, dir, "ro" ) );
	;;;  }  }  }


  void  dev_proc_sys  ()  {    //  -----------  Profile_Finisher  dev_proc_sys
    if  ( p.has_bind_dst ( "/" ) )  {
      if  ( not p.has_bind_dst ( "/dev" ) )  {
	p.binds_append ( PBind ( "bind", "/dev", "/dev", "rw" ) );  }
      if  ( not p.has_bind_dst ( "/sys" ) )  {
	p.binds_append ( PBind ( "bind", "/sys", "/sys", "rw" ) );  }
      if  ( not p.has_bind_dst ( "/proc" ) )  {
	p.binds_append ( PBind ( "proc", "proc", "/proc", "rw" ) );  }  }  }


  void  pbinds_expand  ()  {    //  ---------  Profile_Finisher  pbinds_expand
    for  (  PBind & pb  :  p.binds  )  {
      assert ( pb.dst .startswith ("/") );
      if  ( pb.src == "=" )  {  pb.src  =  pb.dst;  }
      if  ( pb.src == "@" )  {  pb.src  =  p.skel + pb.dst;  }  }  }


  void  new_cwd  ()  {    //  ---------------------  Profile_Finisher  new_cwd
    assert ( p.cwd );
    for  (  const PBind & pb  :  p.binds  )  {
      if  (  p.new_cwd == nullstr  &&
	     p.cwd .startswith ( pb .src )  )  {
	p.new_cwd  =   pb .dst == "/"  ?  ""  :  pb .dst  ;
	p.new_cwd  +=  p.cwd .tail ( pb .src );  }  }  }


  void  skel_set  ()  {    //  -------------------  Profile_Finisher  skel_set
    if  ( p.name )  {  p.skel  =  home() + "/.lxroot/" + p.name;  }  }


public:


  Profile_Finisher  ( Profile & p )  :  p(p)  {}


  Profile *  finish  ()  {    //  ------------------  Profile_Finisher  finish
    skel_set();
    pbinds_expand();
    bind_implicit();
    new_cwd();
    dev_proc_sys();
    p.trace();
    return  & p;  }


};    //  end  class  Profile_Finisher  --------  end  class  Profile_Finisher




class  Opendir  {    //  -------------------------------------  class  Opendir


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


  Opendir  ( str & path )  {  dirp  =  opendir ( path.s );  readdir();  }
  ~Opendir  ()  {  if  ( dirp )  {  closedir ( dirp );  }  }
  Iterator  begin  ()  {  return  Iterator ( this );  }
  Iterator  end    ()  {  return  nullptr;  }


  bool  operator  ==  ( const char * const expect )  const  {    //  --  op ==
    return  name()  &&  expect  &&  strcmp ( name(), expect ) == 0;  }


  ino_t  inode  ()  const  {    //  --------------------------  Opendir  inode
    return  entp  ?  entp -> d_ino  :  -1  ;  }


  bool  is_dir  ()  const  {    //  -------------------------  Opendir  is_dir
    switch  ( assert ( entp ) -> d_type )  {
      case  DT_DIR:  return  true;  break;
      case  DT_UNKNOWN:
	printe ( "opendir  is_dir  error  type is dt_unknown\n" );
	exit ( 1 );  break;  }
    return  false;  }


  const char *  name  ()  const  {    //  ---------------------  Opendir  name
    return  entp  ?  entp -> d_name  :  nullptr  ;  }


};    //  end  class  Opendir  ---------------------------  end  clas  Opendir




//  macro  trace  ----------------------------------------------  macro  trace
#define  trace( format, ... )  {			\
  if  ( trace_flag )  {			\
    fprintf ( stderr, "lxroot  trace  "  format "\n",	\
              ##__VA_ARGS__ );  }  }


//  20200905  Why do both try1 and try2 exists?  Can/should I combine them?


//  macro  try1  ------------------------------------------------  macro  try1
#define  try1( function, format, ... )  {		\
  trace ( format, ##__VA_ARGS__ );		\
  if  ( (function) ( __VA_ARGS__ ) == 0 )  {  return;  }	\
  else  {  die_pe ( format, ##__VA_ARGS__ );  }  }


//  macro  try2  ------------------------------------------------  macro  try2
#define  try2( function, format, ... )  {		\
  if  ( (function) ( __VA_ARGS__ ) == 0 )  {  return;  }	\
  else  {  die_pe ( format, ##__VA_ARGS__ );  }  }




struct  Syscall  {    //  -----------------------------------  struct  Syscall


  //  note  all Syscall methods call exit(1) on error.


  bool     trace_flag  =  false;
  pid_t    fork_pid    =  -2;
  pid_t    wstatus     =  0;


  Syscall  ( const Profile & p )  {    //  --------------------  Syscall  ctor
    trace_flag  =  p.opt_trace == "trace";  }


  void  bind  ( const char * const   source,    //  -----------  Syscall  bind
		const char * const   target,
		const unsigned long  flags,
		const bool           makedirs = false )  {

    if  ( makedirs )  {  Syscall :: makedirs ( target, 0700 );  }
    Lib :: assert_is_dir ( source, "bind" );
    Lib :: assert_is_dir ( target, "bind" );

    auto  bind  =  [=]  ( const unsigned long flags )  {
      char buf[5];               //  see  /usr/include/linux/mount.h
      buf[0]  =  flags & MS_REC      ?  'c'  :  '_'  ;    //  0x4000
      buf[1]  =  flags & MS_BIND     ?  'b'  :  '_'  ;    //  0x1000
      buf[2]  =  flags & MS_REMOUNT  ?  'm'  :  '_'  ;    //  0x0020
      buf[3]  =  flags & MS_RDONLY   ?  'o'  :  '_'  ;    //  0x0001
      buf[4]  =  '\0';
      trace ( "bind     0x%04lx(%s)  %-5s  %s", flags, buf, source, target );
      const int  rv  =  :: mount ( source, target, NULL, flags, NULL );
      if  ( rv == 0 )  {  return;  }
      //  20200829
      //  printe ( "bind    rv %d    is EINVAL %d\n", rv, errno == EINVAL );
      if  (  errno == EINVAL
	     &&  ( flags & MS_BIND )
	     &&  ( not ( flags & MS_REC ) )
	     )  {
	printe (  "\n"  "lxroot  error  bind  0x%04lx  %-5s  %s\n",
		  flags, source, target  );
	printe (  "  Note:  An unprivileged non-recursive bind mount() "
		  "has failed with EINVAL.\n"
		  "         If apporpriate, try a recursive bind mount() "
		  "instead, as follows:\n"
		  "           bind  rec  <dst>  <src>\n"
		  "         See \"man 2 mount\" for further details.\n" );
	exit ( 1 );  }
      die_pe ( "bind  0x%04lx  %-5s  %s", flags, source, target );  };

    const unsigned long  accept  =  MS_RDONLY  |  MS_REC;
    assert ( ( flags | accept ) == accept );

    bind ( MS_BIND | flags );
    if  ( flags & MS_RDONLY )  {
      bind ( MS_BIND | MS_RDONLY | MS_REMOUNT );  }  }


  void  chdir  ( const char * path )  {    //  ---------------  Syscall  chdir
    try1( :: chdir, "chdir    %s", path );  }


  void  chroot  ( const char * new_root )  {    //  ---------  Syscall  chroot
    try1( :: chroot, "chroot   %s", new_root );  }


  void  close  ( int fd )  {    //  --------------------------  Syscall  close
    try2 ( :: close, "close  %d", fd );  }


  void  execve  ( str &  pathname,    //  -------------------  Syscall  execve
		  Vec &  argv,
		  const char * const  envp[] )  {
    trace ( "execve   %s  %s\n", pathname.s, argv.s() );
    if  ( pathname .chr ( '/' ) )  {    //  path is specified, so use execve()
      :: execve ( pathname, argv, (char**) envp );  }
    else  {    //  only filename is specified, so use execvpe()
      char **  old  =  environ;
      environ       =  (char**) envp;
      :: execvpe ( pathname, argv, (char**) envp );
      environ       =  old;  }
    //  execve only returns on failure, so ...
    die_pe ( "execve  %s", pathname.s );  }


  void  fork  ()  {    //  ------------------------------------  Syscall  fork
    if  ( fork_pid != -2 )  {  die_pe ( "extra fork?" );  }
    if  ( ( fork_pid = :: fork() ) >= 0 )  {
      trace ( "fork     (fork returned %d)", fork_pid );
      return;  }
    die_pe ( "fork" );  }


  void  makedirs  ( str & path, const mode_t mode )  {    //  ------  makedirs
    if  ( Lib :: is_dir ( path ) )  {  return;  }
    for  ( str & s : str(path) .descend() )  {
      if  ( not Lib :: is_dir ( s ) )  {
	printe ( "lxroot  mkdir  %s\n", s.s );
	this -> mkdir ( s, mode );  }  }  }


  void  mkdir  ( const char *   path,    //  -----------------  Syscall  mkdir
		 const mode_t   mode )  {
    if  ( Lib :: is_dir ( path ) )  {  return;  }
    try1( :: mkdir, "mkdir    %s  %o", path, mode );  }


  void  mount  ( const char *   source,    //  ---------------  Syscall  mount
		 const char *   target,
		 const char *   filesystemtype )  {
    trace ( "mount    %s  %s  %s", source, target, filesystemtype );
    if  ( :: mount ( source, target, filesystemtype, 0, NULL ) == 0 )  {
      return;  }
    die_pe ( "mount  %s  %s  %s",  source, target, filesystemtype );  }


  void  open  ( int *          fd,    //  ---------------------  Syscall  open
		const char *   pathname,
		const int      flags )  {
    if  ( ( * fd = :: open ( pathname, flags ) ) >= 0 )  {  return;  }
    die_pe ( "open  %s  %d", pathname, flags );  }


  void  pivot  ( const char *   new_root,    //  -------------  Syscall  pivot
		 const char *   put_old )  {
    trace ( "pivot    %s  %s", new_root, put_old );
    if  ( syscall ( SYS_pivot_root, new_root, put_old ) == 0 )  {  return;  }
    die_pe ( "pivot  %s  %s", new_root, put_old );  }


  void  rmdir  ( const char * pathname )  {    //  -----------  Syscall  rmdir
    try1( :: rmdir, "rmdir    %s", pathname );  }


  void  umount2  ( const char *   target,    //  -----------  Syscall  umount2
		 int            flags )  {
    try1( :: umount2, "umount2  %s  0x%x", target, flags );  }


  void  unshare  ( const int flags )  {    //  -------------  Syscall  unshare
    try1( :: unshare, "unshare  0x%08x", flags );  }


  pid_t  wait  ()  {    //  -----------------------------------  Syscall  wait
    trace ( "wait     (parent calls wait)" );
    pid_t  pid  =  :: wait ( & wstatus );
    if  ( pid > 0 )  {
      trace ( "wait     wait returned  pid %d  status 0x%x",
	      pid, wstatus );
      return  pid;  }
    die_pe ( "wait" );  }


  void  write  ( int fd, const void * buf, ssize_t count )  {    //  --  write
    assert ( count >= 0 );
    if  ( :: write ( fd, buf, count ) == count )  {  return;  }
    die_pe ( "write  %d  %ld", fd, count );  }


};    //  end  struct  Syscall  ------------------------  end  struct  Syscall




class  Chroot_Preparer  :  Lib  {    //  -------------  class  Chroot_Preparer


  const Profile &  p;
  Syscall &        sys;


  void  skel_mkdir  ()  {  sys .makedirs ( p.skel, 0700 );  }


  void  skel_perpare_passthru  ()  {    //  -----------  skel_prepare_passthru

    if  ( not p.is_passthru )  {  return;  }

    auto  symlink  =  [&]  ( const char * s )  {
      if  ( is_link ( str("/") + s ) )  {
	str  text  =  str("usr/") + s;
	str  path  =  p.skel + "/" + s;
	if  ( is_link ( path ) )  {  return;  }
	printe ( "symlink  %s\n", path.s );
	assert ( :: symlink ( text, path ) == 0 );  }  };

    str  links  =  " bin lib lib32 lib64 libx32 sbin ";
    for  (  str & s  :  links .tokens()  )  {  symlink ( s );  }  }


public:


  Chroot_Preparer  ( const Profile & p, Syscall & s )  :  p(p), sys(s)  {}


  void  prepare  ()  {    //  ----------------------  Chroot_Preparer  prepare
    skel_mkdir();
    skel_perpare_passthru();
    ;;;  }


};    //  end  class  Chroot_Preparer  ----------  end  class  Chroot_Preparer




class  Launcher  :  Lib  {    //  ---------------------------  class  Launcher


  Profile &  p;
  Syscall &        sys;

  uid_t    uid         =  getuid();
  gid_t    gid         =  getgid();
  str      pivot       =  pivot_set();
  mstr     put_old;
  bool     trace_flag  =  false;


  str  pivot_set  ()  {    //  --------------------------  Launcher  pivot_set
    if  ( p.name )  {  return  home() + "/.lxroot/.pivot";  }
    return  p.binds[0] .src;  }


  void  do_unshare  ()  {    //  -----------------------  Launcher  do_unshare
    int  clone_flags  =  0;
    clone_flags  |=  p.binds.size()  ?  CLONE_NEWNS  :  0  ;
    clone_flags  |=  CLONE_NEWPID;
    clone_flags  |=  CLONE_NEWUSER;
    clone_flags  |=  p.opt_net == "net"  ?  0  :  CLONE_NEWNET  ;
    sys .unshare ( clone_flags );  }


  void  do_uid_map  ()  {    //  -----------------------  Launcher  do_uid_map

    //  see  https://lwn.net/Articles/532593/

    uid_t  un_uid  =  p.opt_root == "root"  ?  0  :  uid  ;
    gid_t  un_gid  =  p.opt_root == "root"  ?  0  :  gid  ;

    char  u_map[80];
    char  g_map[80];
    int   fd;

    snprintf ( u_map, sizeof u_map, "%u %u 1\n", un_uid, uid );
    snprintf ( g_map, sizeof g_map, "%u %u 1\n", un_gid, gid );

    trace ( "uid_map  %u %u 1  deny  %u %u 1",
	    un_uid,  uid,  un_gid,  gid );

    sys .open   (  & fd,  "/proc/self/uid_map",  O_RDWR    );
    sys .write  (  fd,    u_map,  strlen ( u_map )         );
    sys .close  (  fd                                      );
    sys .open   (  & fd,  "/proc/self/setgroups",  O_RDWR  );
    sys .write  (  fd,    "deny", 4                        );
    sys .close  (  fd                                      );
    sys .open   (  & fd,  "/proc/self/gid_map",  O_RDWR    );
    sys .write  (  fd,     g_map, strlen ( g_map )         );
    sys .close  (  fd                                      );  }


  void  bind_prepare  ()  {    //  -------------------  Launcher  bind_prepare
    sys .makedirs ( pivot, 0700 );  }


  void  bind_opt_pulse  ()  {    //  ---------------  Launcher  bind_opt_pulse
    if  ( p.opt_pulse == "pulse" )  {
      str  xdg_dir  =  getenv ( "XDG_RUNTIME_DIR" );
      if  ( xdg_dir )  {
	str  pulse_dir  =  xdg_dir + "/pulse";
	sys .bind ( pulse_dir, pivot + pulse_dir, 0, true );
	return;  }
      else  {
	warn ( "bind_opt_pulse()  XDG_RUNTIME_DIR not set" );  }  }  }


  void  bind_opt_x11  ()  {    //  -------------------  Launcher  bind_opt_x11
    if  ( p.opt_x11 == "x11"  )  {
      sys .bind ( "/tmp/.X11-unix", pivot + "/tmp/.X11-unix", 0, true );  }  }


  void  bind_opts  ()  {    //  -------------------------  Launcher  bind_opts
    bind_opt_pulse();
    bind_opt_x11();  }


  /*  20200907  already refactored (I think)
  void  bind_skel_prepare  ()  {    //  ---------  Launcher  bind_skel_prepare

    str  skel  =  home() + "/.lxroot/.skel";

    if  ( not is_dir ( skel ) )  {
      assert ( :: mkdir ( skel, 0755 ) == 0 );  }

    if  ( not is_empty_dir ( skel ) )  {  return;  }

    auto  mkdir  =  [&]  ( const char * s )  {
      if  ( is_dir ( str("/") + s ) )  {
	str  path  =  skel + "/" + s;
	printe ( "mkdir    %s\n", path.s );
	assert ( :: mkdir ( path, 0755 ) == 0 );  }  };

    auto  symlink  =  [&]  ( const char * s )  {
      if  ( is_link ( str("/") + s ) )  {
	str  text  =  str("usr/") + s;
	str  path  =  skel + "/" + s;
	printe ( "symlink  %s\n", path.s );
	assert ( :: symlink ( text, path ) == 0 );  }  };

    str  dirs   =  " boot dev etc home opt proc run srv sys usr tmp var ";
    str  links  =  " bin lib lib32 lib64 libx32 sbin ";

    for  (  str & s  :  dirs  .tokens()  )  {  mkdir   ( s );  }
    for  (  str & s  :  links .tokens()  )  {  symlink ( s );  }  }
  */


  /*  20200903  part of bind_ubuntu()
  bool  bind_ubuntu  ()  {    //  ---------------------  Launcher  bind_ubuntu

    if  ( has_bind_dst ( "/" ) )  {  return  false;  }

    bind_skel_prepare();

    str  skel   =  home() + "/.lxroot/.skel";
    str  bind   =  "  /boot  /etc  /opt  /usr  ";
    //  20200430
    //  str  check  =  bind  +  "  /dev  /proc  /sys  ";

    Bind(  st,  skel,  pivot,  MS_RDONLY  );

    for  ( str & dir  :  bind .tokens()  )  {
      Bind(  st,  dir,  pivot + dir,  MS_RDONLY  );  }

    / *  20200430
    str  src  =  home() + "/.lxroot/" + name;
    Bind(  st,  src + "/home",  pivot + "/home",  0       );
    Bind(  st,  src + "/tmp",   pivot + "/tmp",   0       );  * /
    bind_pbinds();

    Bind(  st,  "/dev",         pivot + "/dev",   MS_REC  );
    Bind(  st,  "/sys",         pivot + "/sys",   MS_REC  );

    bind_opts();

    return  true;  }
  */


  /*  20200423  keep bind_auto?
  void  bind_auto  ( str & src )  {    //  --------------  Launcher  bind_auto
    printe ( "bind_auto  %s\n", src.s );
    if  (  is_dir ( src )  &&  is_dir ( dst + src )  )  {
      Bind(  st,  src,  dst + src,  MS_BIND  );  }  }
  */


  /*  20200423
  void  bind_pulse  ()  {    //  -----------------------  Launcher  bind_pulse
    mstr  path;  path .snprintf ( "/run/user/%d/pulse", 80, uid );
    for  (  str & s  :  path .descend()  )  {
      printf ( "  %s\n", s.s );
      ;;;  }
    ;;;; }
  */


  /*  20200423
  void  bind_x11  ()  {    //  ---------------------------  Launcher  bind_x11
    printe ( "bind_opt_x11  %d\n", opt_x11 );
    if  ( opt_x11 )  {  bind_auto ( "/tmp/.X11-unix" );  }  }
  */


  /*  20200903  part of bind_ubuntu()
  void  do_bind_20200829  ()  {    //  -----------  Launcher  do_bind_20200829
    bind_prepare();
    if  ( bind_ubuntu() )  {  return;  }
    bind_pbinds();
    Bind(  st,  "/dev",  pivot + "/dev",  MS_REC  );
    Bind(  st,  "/sys",  pivot + "/sys",  MS_REC  );
    bind_opts();  }
  */


  /*  20200907  moved to Profile_Finisher :: expand()
  str  bind_src  ( str & src, str & dst )  {    //  ------  Launcher  bind_src
    assert ( dst .startswith ( "/" ) );
    if  ( src == "." )  {  return  home() + "/.lxroot/" + p.name + dst;  };
    if  ( src .startswith ( "/" ) )  {  return  src;  }
    return  home() + "/.lxroot/" + src;  }
  */


  void  bind_pbinds  ()  {    //  ---------------------  Launcher  bind_pbinds
    trace ( "binds_pbinds()  pivot  %s", pivot.s );
    for  (  const PBind & b  :  p.binds  )  {
      if  ( b.type == "bind" )  {
	str            dst    =  pivot == "/"  ?  b.dst  :  pivot + b.dst;
	unsigned long  flags  =  0;
	flags  |=  (  b.recursive == "rec"  ?  MS_REC  :  0          );
	flags  |=  (  b.readonly  == "rw"   ?  0       :  MS_RDONLY  );
	sys .bind ( b.src, dst, flags, true );  }
      else  {
	trace ( "bind_pbinds()  skipping  %s  %s",  b.type.s, b.dst.s );
	}  }  }


  void  do_bind  ()  {    //  -----------------------------  Launcher  do_bind
    bind_prepare();
    bind_pbinds();
    bind_opts();  }


  void  pivot_prepare  ( str & pivot )  {    //  --------------  pivot_preapre
    getpwent();    //  load getpwent()'s shared libraries *before* pivot !!
    endpwent();
    //  verify that pivot has at least one sub-direcotry (for put_old)
    for  (  const Opendir & e  :  Opendir ( pivot )  )  {
      if  (  e == "."  ||  e == ".."  )  {  continue;  }
      if  (  e.is_dir()  )  {
	put_old  =  str ( "/", e.name() );  return;  }  }
    printe ( "pivot_prepare  error  pivot contains no directories\n" );
    exit ( 1 );  }


  void  do_pivot  ()  {    //  ---------------------------  Launcher  do_pivot
    pivot_prepare ( pivot );
    sys .pivot  ( pivot,  pivot + put_old );
    sys .chdir  ( "/" );
    sys .chroot ( "/" );  }


  void  do_fork  ()  {    //  -----------------------------  Launcher  do_fork
    sys .fork();
    if  ( sys .fork_pid == 0 )  {  return;  }    //  child returns
    if  ( sys .fork_pid >  0 )  {    //  parent waits for child to exit
      sys .wait();
      if  ( WIFEXITED ( sys .wstatus ) )  {
	exit ( WEXITSTATUS ( sys .wstatus ) );  }
      printe ( "lxroot  warning  child exited abnormally\n" );
      exit ( 1 );  }  }


  void  exec_prepare_argv  ()  {    //  ---------  Launcher  exec_prepare_argv

    auto  try_shell  =  [this]  ( str & shell )  {
      if  ( p.exec_path or not shell )  {  return;  };
      if  ( is_file ( shell )  and  access ( shell.s, X_OK ) == 0 )  {
	p.exec_path  =  shell;
	p.exec_argv .push_back ( shell );
	if  ( shell == "/bin/bash" )  {
	  p.exec_argv .push_back ( "--norc" );  }  }  };

    try_shell ( p.env .get ( "SHELL" ) );
    try_shell ( "/bin/bash" );
    try_shell ( "/bin/sh" );  }


  void  exec_prepare_chdir  ()  {    //  -------  Launcher  exec_prepare_chdir
    if  (  p.new_cwd  )  {  sys .chdir ( p.new_cwd );  return;  }
    if  (  p.binds .size()  &&  p.env .count ( "HOME" )  )  {
      if  ( is_dir ( p.env [ "HOME" ] ) )  {
	sys .chdir ( p.env [ "HOME" ] );  }  }  }


  void  exec_prepare_env  ()  {    //  -----------  Launcher  exec_prepare_env

    mstr             s;
    const uid_t      uid    =  getuid();
    struct passwd *  pwent  =  nullptr;

    while  ( ( pwent = getpwent() ) )  {    //  parse /etc/passwd
      if ( pwent -> pw_uid == uid )  {
	p.env [ "HOME"    ]  =  pwent -> pw_dir;
	p.env [ "LOGNAME" ]  =  pwent -> pw_name;
	p.env [ "SHELL"   ]  =  pwent -> pw_shell;
	p.env [ "USER"    ]  =  pwent -> pw_name;   break;  }  }
    endpwent();

    if  ( s = getenv ( "TERM" ) )  {  p.env [ "TERM" ]  =  s;  }

    if  ( p.opt_pulse == "pulse" )  {    //  set XDG_RUNTIME_DIR
      str  xdg_dir  =  getenv ( "XDG_RUNTIME_DIR" );
      if  ( xdg_dir )  {  p.env [ "XDG_RUNTIME_DIR" ]  =  xdg_dir;  }  }

    if  ( p.opt_x11 == "x11"  && ( s = getenv ( "DISPLAY" ) ) )  {
      p.env [ "DISPLAY" ]  =  s;  }    //  set DISPLAY

    if  ( p.env.count ( "PATH" ) == 0 )  {
      p.env [ "PATH" ]  =
	(  "/usr/local/bin:"   "/usr/bin:"   "/bin:"
	   "/usr/local/sbin:"  "/usr/sbin:"  "/sbin"  );  }
    ;;;  }


  void  exec_prepare_prompt  ()  {    //  -----  Launcher  exec_prepare_prompt

    if  (  p.exec_path == "/bin/bash"  ||  is_busybox ( p.exec_path )  )  {

      str   term  =  getenv ( "TERM" );
      str   user  =  getenv ( "USER" );
      mstr  opts  =  nullptr;
      str   host  =  p.name  ?  "@" + p.name  :  "./" + pivot .basename()  ;

      if  (  p.opt_net      == "net"
	     ||  p.opt_root == "root"
	     ||  p.opt_x11  == "x11"  )  {
	opts  =  "-";
	if  ( p.opt_pulse == "pulse" )  {  opts  +=  "a";  }
	if  ( p.opt_x11   == "x11"   )  {  opts  +=  "g";  }
	if  ( p.opt_net   == "net"   )  {  opts  +=  "n";  }
	if  ( p.opt_root  == "root"  )  {  opts  +=  "r";  }  }

      mstr  ps1;

      if  ( term  ==  "xterm-256color" )  {    //  set terminal title
	ps1  +=  "\\[\\e]0;" + user + "  ";
	if  ( opts )  {  ps1  +=  opts  +  "  ";  }
	ps1  +=  host  +  "  \\W\\007\\]";  }

      //nst char *  bright_cyan  =  "\\[\\e[0;96m\\]";
      const char *  bright_red   =  "\\[\\e[0;91m\\]";
      const char *  cyan         =  "\\[\\e[0;36m\\]";
      //nst char *  red          =  "\\[\\e[0;31m\\]";
      const char *  normal       =  "\\[\\e[0;39m\\]";

      ps1  +=  str("\n") + cyan + user + "  ";
      if  ( opts )  {  ps1  +=  bright_red + opts + cyan + "  ";  }
      ps1  +=  host + "  \\W" + normal + "  ";
      p.env [ "PS1" ]  =  ps1;  }

    /*  20200626  todo  add a custom prompt for other shells  */

    ;;;  }


  void  exec_mount_proc  ()  {    //  -------------  Launcher  exec_mount_proc
    for  (  const PBind & pb  :  p.binds  )  {
      if  ( pb.type == "proc" )  {
	sys .makedirs ( "/proc", 0700 );
	sys .mount ( "proc", "/proc", "proc" );  break;  }  }  }


  void  do_exec  ()  {    //  -----------------------------  Launcher  do_exec

    //  We try to do as much as possible before calling fork().  Howewer...
    //  It seems /proc can only be mounted:
    //    1) after fork() (which sort of makes sense), and
    //    2) before unmounting put_old (which is mildly surprising?).

    //  Therefore, we mount() /proc and umount2() put_old here, in the
    //  child process, to ensure both calls finish before execve() is
    //  called.

    if  ( p.binds .size() )  {
      exec_mount_proc();
      sys .umount2 ( put_old, MNT_DETACH );  }

    exec_prepare_env();
    exec_prepare_chdir();     //  depends on env  ( HOME  )
    exec_prepare_argv();      //  depends on env  ( SHELL )
    exec_prepare_prompt();    //  depends on argv
    sys .execve ( p.exec_path, p.exec_argv, p.env );  }


  /*  20200907
  void  test_chromium  ()  {    //  -----------------  Launcher  test_chromium
    if  ( p.name == "chromium" )  {
      test_profile_print();
      p     =  Profile_Finisher ( p ) .finish();
      test_profile_print();
      test_exit();
      ;;;  }  }


  void  test_exit  ()  const  {    //  ------------------  Launcher  test_exit
    printe ( "\n"  "lxroot  launcher  test_exit()  exiting ...\n\n" );
    exit ( 2 );  }


  void  test_profile_print  ()  const  {    //  ----------  test_profile_print
    printe ( "\n" );  p.print();  }


  void  test_trace_activate  ()  {    //  -----  Launcher  test_trace_activate
    trace_flag  =  sys .trace_flag  =  true;  }
  */


public:


  Launcher  ( Profile & p, Syscall & s )    //  --------------  Launcher  ctor
    :  p(p), sys(s)  {  trace_flag  =  s.trace_flag;  }


  void  launch  ()  {    //  -------------------------------  Launcher  launch
    if  ( p.opt_trace == "trace" )  {  printe ( "\n" );  }
    pivot_set();
    do_unshare();
    do_uid_map();
    if  ( p.binds .size() )  {  do_bind();  do_pivot();  }
    do_fork();
    do_exec();  }


};    //  end  class  Launcher  ------------------------  end  class  Launcher




int  main  ( const int argc, const char * const argv[] )  {    //  -----  main

  Profile  p;
  p.cwd  =  Lib :: getcwd();
  p      =  Argv_Parser      ( argv, p ) .parse();
  p      =  Profile_Finisher ( p       ) .finish();

  Syscall  sys    ( p );
  Chroot_Preparer ( p, sys ) .prepare();
  Launcher        ( p, sys ) .launch();

  printe ( "lxroot  error  launch() returned unexpectedly\n" );
  return  1;  }




//    lxroot.c  -  a Linux rootless alternative to /usr/sbin/chroot
//
//    Copyright (c) 2020 Parke Bostrom, parke.nexus at gmail.com
//
//    This program is free software; you can redistribute it and/or
//    modify it under the terms of version 2 of the GNU General Public
//    License as published by the Free Software Foundation.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public
//    License along with this program; if not, write to the Free
//    Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
//    Boston, MA 02110-1301 USA.
