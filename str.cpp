

//  str.cpp  -  Parke's string library.
//  Copyright (c) 2021 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.

//  version  20211019




struct  mFrag;     //  -------------------------------  declare  struct  mFrag
struct  mStr;      //  --------------------------------  declare  struct  mStr
class   oStr;      //  ---------------------------------  declare  class  oStr
class   Concat;    //  -------------------------------  declare  class  Concat

typedef  const char *  mstr;    //  ---------------------------  typedef  mstr
typedef  const mstr    str;     //  ----------------------------  typedef  str
typedef  const mFrag   Frag;    //  xxfr  ---------------------  typedef  Frag
typedef  const mStr    Str;     //  xxst  ----------------------  typedef  Str




struct  mFrag  {    //  xxmf  ---------------------------------  struct  mFrag


  mstr  s  =  nullptr;
  int   n  =  0;


  mFrag  ()  {}    //  ------------------------------------------  mFrag  ctor
  mFrag  ( str s        )  :  s(s),  n(s?strlen(s):0)  {}
  mFrag  ( str s, int n )  :  s(s),  n(n)              {}
  mFrag  ( str a, str b )  :  s(a),  n( b - a + 1 )    {}


  explicit operator bool  ()  const  {  return  s;  }    //  --------  op bool


  bool  operator ==  ( Frag & o )  const  {    //  -------------  mFrag  op ==
    return  n == o.n  &&  memcmp ( s, o.s, n ) == 0;  }


  Concat  operator +  ( Frag & o )  const;    //  ---------------  mFrag  op +


  char  c  ()  const  {  return  n > 0  ?  * s  :  '\0'  ;  }    //  ------  c
  void  next  ()  {  if  ( n > 0 )  {  s ++;  n --;  }  }    //  -------  next


  mFrag  capture_until  ( const char accept )  {    //  -------  capture_until
    return  Frag ( s, find(accept).s - s );  }


  mFrag &  find  ( const char accept )  {    //  ----------------  mFrag  find
    while  (  c()  &&  c() not_eq accept )  {  next();  }  return  * this;  }


  mFrag &  find_skip  ( const char accept )  {    //  ------  mFrag  find_skip
    return  find ( accept ) .skip ( accept );  }


  mFrag &  skip  ( const char accept )  {    //  ----------------  mFrag  skip
    while  (  c()  &&  c() == accept  )  {  next();  }  return  * this;  }


/*  20211019  disabled due to creation of str.cpp.
              global_opt_trace is undeclared.
              it seems this trace was unused anyway.
  void  trace  ( const char * const message )  const  {    // --  mFrag  trace
    if  ( global_opt_trace == o_trace )  {
      printe ( "%s  >", message );
      fwrite ( s, n, 1, stderr );
      printe ( "<\n" );  }  }
*/


};    //  end  struct  mFrag  ----------------------------  end  struct  mFrag




struct  mStr  {    //  xxms  -----------------------------------  struct  mStr

  //  mStr is a convenience wrapper around a mstr (i.e. 'const char *').
  //  s points to either (a) nullptr or (b) a null-terminated string.
  //  an mStr does not own s.  (But see also derived class oStr.)
  //  an mStr makes no guarantee about the lifetime of *s.


  mstr  s  =  nullptr;   //  a pointer to the string.


  mStr  ()  {}    //  --------------------------------------------  mStr  ctor
  mStr  ( str s )  :  s(s)  {}    //  ----------------------------  mStr  ctor


  operator bool  ()  const  {  return  s;  }    //  --------  mStr  cast  bool
  operator Frag  ()  const  {  return  s;  }    //  --------  mStr  cast  Frag


  bool  operator ==  ( Str & o )  const  {    //  ---------------  mStr  op ==
    if  ( s == nullptr  &&  o.s == nullptr )  {  return  true;   }
    if  ( s == nullptr  ||  o.s == nullptr )  {  return  false;  }
    return  strcmp ( s, o.s ) == 0;  }


  mStr  operator ||  ( const mStr & o )  const  {    //  --------  mStr  op ||
    return  s  ?  * this  :  o  ;  }


  char  operator *  ()  const  {    //  --------------------------  mStr  op *
    return  s  ?  *s  :  '\0';  }


  char  operator []  ( int index )  const  {    //  -------------  mStr  op []
    if  ( s == nullptr )  {  return  0;  }
    return  skip ( index ) .s[0];  }


  mStr  operator ++  ( int )  {    //  --------------------------  mStr  op ++
    return  s && *s  ?  s++  :  s  ;  }


  Concat  operator +  ( Frag o )  const;    //  ---------  declare  mStr  op +
  Concat  operator +  ( str  o )  const;    //  ---------  declare  mStr  op +
  Concat  operator +  ( Str  o )  const;    //  ---------  declare  mStr  op +


  Frag  basename  ()  const  {    //  -------------------------  mStr  basename
    if  ( s == nullptr )  {  return  nullptr;  }
    //    /foo/bar
    //     a  bc
    //      a is the first character of    this basename
    //      b is the first character after this basename  ( '/' or '\0' )
    //      c is the first non-slash after b
    //    return the last basename
    mstr a  =  s;
    while  ( a[0] == '/' && a[1] )  {  a++;  }
    for  (;;)  {
      mstr  b  =  a + ( a[0] ? 1 : 0 );
      while  ( b[0] && b[0] != '/' )  {  b++;  }
      if     ( b[0] == '\0'        )  {	 return  Frag ( a, b-1 );  }
      mstr  c  =  b + 1;
      while  ( c[0] == '/'         )  {  c++;  }
      if     ( c[0] == '\0'        )  {  return  Frag ( a, b-1 );  }
      a  =  c;  }  }


  static void  basename_test  ( Str s, Str expect );    //  ------------------


  Frag  capture_until  ( char c )  const  {    //  ------  mStr  capture_until
    mStr  p  =  s;  while  (  p && * p  &&  * p != c  )  {  p ++;  }
    return  Frag ( s, p.s - s );  }


  const void *  chr  ( const int c )  const  {    //  -------------  mStr  chr
    return  s  ?  :: strchr ( s, c )  :  nullptr  ;  }


  Frag  env_name  ()  const  {    //  ------------------------  mStr  env_name
    return  head ( "=" );  }


  Frag  head  ( str sep, int start = 0 )  const  {    //  --------  mStr  head
    if  ( s == nullptr )  {  return  Frag();  }
    mstr  p      =  s;
    while  (  * p  &&  start-- > 0  )  {  p++;  }
    str   found  =  :: strstr ( p, sep );
    if  ( found )  {  return  Frag ( p, found - p );  }
    return  Frag();  }


  bool  is_inside  ( Str path )  const  {    //  ------------  mStr  is_inside

    //  return true iff path is an ancestor of s, or equal to s.

    mStr  descendant  =  s;
    mStr  ancestor    =  path;
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


  bool  is_same_path_as  ( Str path )  const  {    //  ------  is_same_path_as
    return  is_inside(path)  &&  path.is_inside(*this);  }


  int  n  ()  const  {    //  ---------------------------------------  mStr  n
    return  s  ?  strlen ( s )  :  0  ;  }


  Str  skip  ( int n )  const  {    //  --------------------------  mStr  skip
    mstr  p  =  s;
    while  ( n-- > 0 )  {
      if  (  p  &&  * p )  {  p++; }
      else  {  return  nullptr;  }  }
    return  p;  }


  Str  skip_all  ( char c )  const  {    //  -----------------  mStr  skip_all
    mStr  p  =  s;
    while  (  p  &&  * p == c )  {  p ++;  };  return  p;  }


  int  spn  ( Str accept )  const  {    //  -----------------------  mStr  spn
    return  s  ?  :: strspn ( s, accept.s )  :  0  ;  }


  bool  startswith  ( Frag o )  const  {    //  ------------  mStr  starstwith
    return  s  &&  strncmp ( s, o.s, o.n ) == 0;  }


  Str  tail  ( Str sep )  const  {    //  ------------------------  mStr  tail
    Str  found  =  :: strstr ( s, sep.s );
    return  found  ?  found.s + sep.n()  :  nullptr  ;  }


  Frag  trunk  ( )  const  {    //  ------------------------------  mStr  trunk
    return  skip_all('/') .capture_until('/');  }


  static void  unit_test  ()  {    //  ----------------------  mStr  unit_test

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


};    //  end  struct  mStr  ------------------------------  end  struct  mStr




mStr  leak  ( Frag o )  {    //  ---------------------  global  function  leak
  //  lxroot quickly exec()s on success or exit()s on failure.
  //  therefore, a few convenient and minor memory leaks are acceptable.
  char *  rv  =  (char*) malloc  ( o.n + 1 );    //  intentional leak.
  if  ( rv )  {
    memcpy ( rv, o.s, o.n );
    rv [ o.n ]  =  '\0';  }
  return  rv;  }




struct  Concat_2  {    //  xxco  ---------------------------  struct  Concat_2
  mFrag s[3];
  Concat_2  ( Frag a, Frag b = 0, Frag c = 0 )  :  s{a,b,c}  {}
};    //  end  struct  Concat_2  ----------------------  end  struct  Concat_2




Concat_2  s  ( Frag a, Frag b = 0, Frag c = 0 )  {    //  xxs  ----  global  s
  return  Concat_2 ( a, b, c );  }


Concat_2  s  ( Str a, Str b = 0, Str c = 0 )  {    //  xxs  -------  global  s
  return  Concat_2 ( a, b, c );  }





struct  oStr  :  mStr   {    //  xxos  -------------------------  struct  oStr


  int  n  =  0;


  oStr  ()  {}    //  --------------------------------------------  oStr  ctor

  oStr  ( Frag  o )             :  mStr()  {  * this  +=  o;  }
  oStr  ( str   o )             :  mStr()  {  * this  +=  o;  }
  oStr  ( Str & o )             :  mStr()  {  * this  +=  o;  }
  oStr  ( const Concat_2 & o )  :  mStr()  {  * this  =   o;  }

  oStr  (       oStr && o )  :  mStr()  {  s=o.s; n=o.n; o.s=nullptr; o.n=0; }
  oStr  ( const oStr &  o )  :  mStr()  {  * this  +=  o;  }


  ~oStr  ()  {    //  --------------------------------------------  oStr  dtor
    free ( (char*) s );  }


  void  operator =  ( Frag   o )        {  n=0;  * this += o;   }    //  -----
  void  operator =  ( str    o )        {  * this  =  Frag(o);  }    //  -----
  void  operator =  ( Str    o )        {  * this  =  Frag(o);  }    //  -----
  void  operator =  ( const oStr & o )  {  * this  =  Frag(o);  }    //  -----


  void  operator =  ( const Concat_2 & t )  {    //  -------------  oStr  op =
    oStr & r  =  * this;  r = t.s[0];  r += t.s[1];  r += t.s[2];  }


  oStr &  operator +=  ( Frag o )  {    //  ---------------------  oStr  op +=
    char *  p  =  (char*) assert ( realloc ( (char*) s, n + o.n + 1 ) );
    memcpy ( p+n, o.s, o.n );
    n     +=  o.n;
    p[n]  =   '\0';
    s     =   p;
    return  * this;  }


  static oStr  claim  ( Str s )  {    //  -----------------------  oStr  claim
    //  return an oStr that owns s.  s must be an unowned, malloc()ed string.
    oStr  rv;    rv.s=s.s;    rv.n=s.n();    return rv;  }


  static void  unit_test  ();    //  ------------------------  oStr  unit_test


};    //  end  struct  oStr  ------------------------------  end  struct  oStr




struct  Concat  {    //  xxco  --------------------------------  class  Concat

  oStr  s;

  Concat  ( Frag o )  :  s(o)  {}    //  -----------------------  Concat  ctor

  operator Frag          ()  const  {  return  s;  }    //  Concat  cast  Frag
  operator Str           ()  const  {  return  s;  }    //  Concat  cast  Str
  operator oStr          ()  const  {  return  s;  }    //  Concat  cast  oStr
  bool  operator ==  ( Str o )  const  {  return  s == o.s;  }    //  --------
  Concat &  operator +   ( Frag o )  {  s  +=  o;  return  * this;  }    //  -
  Concat &  operator +=  ( Frag o )  {  s  +=  o;  return  * this;  }    //  -


};    //  end  struct  Concat


Concat  mFrag :: operator +  ( Frag & o )  const  {    //  ------  mFrag  op +
  //  written as three statements to (1) avoid infinite recursion, and
  //  (2) allow named return value optimization.
  Concat rv (*this);    rv += o;    return rv;  }


Concat  mStr :: operator +  ( Frag o )  const  {    //  ----------  mStr  op +
    return  ((Frag)*this) + o;  }


Concat  mStr :: operator +  ( str o )  const  {    //  -----------  mStr  op +
    return  ((Frag)*this) + o;  }


Concat  mStr :: operator +  ( Str  o )  const  {    //  ----------  mStr  op +
    return  ((Frag)*this) + o;  }


Concat  operator +  ( str a, Frag b )  {    //  --------------------  ::  op +
  return  Frag(a) + b;  }


Concat  operator +  ( str a, Str b )  {    //  ---------------------  ::  op +
  return  Frag(a) + b;  }


void  mStr :: basename_test  ( Str s, Str expect )  {    //  --  basename_test
    if  ( s.basename() == expect )  {  return;  }
    oStr  actual  =  s.basename();
    printe ( "basename_test  failed\n" );
    printe ( "  s       %s\n", s.s      );
    printe ( "  expect  %s\n", expect.s );
    printe ( "  actual  %s\n", actual.s );
    abort();  }


void  oStr :: unit_test  ()  {    //  -----------------------  oStr  unit_test
    Str   a   =  "a";
    Str   b   =  "b";
    oStr  ab  =  a + b;
    assert  ( ab == "ab" );
    assert  ( ab == a+b  );
    Frag  c   =  "c";
    Frag  d   =  "d";
    oStr  cd  =  c + "=" + d;
    assert  ( cd == "c=d" );
    oStr  ef  =  cd + "ef" + "gh";
    assert  ( ef == "c=defgh" );
    assert  ( cd == "c=d" );
    ;;;  }


//  end  struct  Concat  --------------------------------  end  struct  Concat




struct  Argv  {    //  xxar  -----------------------------------  struct  Argv


  str *  p  =  nullptr;


  Argv  ()  {}    //  --------------------------------------------  Argv  ctor
  Argv  ( str * const p )  :  p(p)  {}


  Argv &  operator |=  ( const Argv & o )  {    //  -------------  Argv  op |=
    if  ( p == nullptr )  {  p  =  o.p;  }  return  * this;  }


  bool  operator ==  ( const Argv & o )  const  {    //  --------  Argv  op ==
    return  p == o.p;  }


  explicit  operator bool  ()  const  {    //  ----------------  Argv  op bool
    return  p;  }


  Str  operator *  ()  const  {    //  ---------------------------  Argv  op *
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


  Str  operator []  ( int n )  const  {    //  ------------------  Argv  op []
    return  * ( ( * this ) + n );  }


  oStr  concat  ( Frag sep = "  " )  const  {    //  -----------  Argv  concat
    Argv  o  ( * this );
    oStr  rv;
    if ( *(o.p) )  {  rv  +=  * o;    o++;  }
    for  (  ;  *(o.p)  ;  o++  )  {  rv  +=  sep;    rv  +=  * o;  }
    return  rv;  }


  Str  env_get  ( Str k )  const  {    //  --------------------  Argv  env_get
    for  (  Argv o (*this)  ;  *(o.p)  ;  o++  )  {
      if  ( (*o).env_name() == k )  {  return  (*o);  }  }
    return  nullptr;  }


  void  print  ( Str s )  const  {    //  -----------------------  Argv  print
    for  (  Argv o (*this)  ;  o  &&  o.p[0]  ;  o++  )  {
      printe  ( "%s%s\n", s.s, o[0].s );  }  }


  static void  unit_test  ()  {    //  ----------------------  Argv  unit_test

    str  p[]  =  { "a=1", "b=2", "c=3", nullptr };
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


//  str.cpp  -  Parke's string library.
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
