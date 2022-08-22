

//  str.cpp  -  Parke's string library.
//  Copyright (c) 2022 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.


//  version  20220822


;                 typedef  const int     cint;    //  --------  typedef  cint
;                 typedef  const char *  mstr;    //  --------  typedef  mstr
;                 typedef  const mstr    str;     //  --------  typedef   str
struct  mStr;     typedef  const mStr    Str;     //  --------  typedef   Str
struct  mCat2;    typedef  const mCat2   Cat2;    //  --------  typedef  Cat2
struct  mCat3;    typedef  const mCat3   Cat3;    //  --------  typedef  Cat3
struct  mCat4;    typedef  const mCat4   Cat4;    //  --------  typedef  Cat4




struct  mStr  {    //  xxms  -----------------------------------  struct  mStr


  int   n  =  0;          //  for non-empty strings, length *including* '\0'
  mstr  s  =  nullptr;    //  a pointer to the string


  mStr  ()  {}
  mStr  ( str s )          :  n(ctor_len(s)),  s(s)  {}
  mStr  ( str s, cint n )  :  n(n),            s(s)  {}


  bool  operator !=  ( Str & o )  const  {  return  not operator == (o);  }
  Str   operator ||  ( Str & o )  const  {  return  n  ?  * this  :  o  ;  }
  Cat2  operator +  ( Str & o )  const;    //  ----------  declare  mStr  op +
  explicit operator bool  ()  const  {  return  n;  }    //  -----  cast  bool
  static int  max  ( cint a, cint b )  {  return  a>b ? a : b ;  }    //   max


  bool  operator ==  ( Str & o )  const  {    //  ---------------  mStr  op ==
    if  ( s == nullptr  &&  o.s == nullptr )  {  return  true;   }
    if  ( s == nullptr  ||  o.s == nullptr )  {  return  false;  }
    return  len() == o.len()  &&  ( memcmp ( s, o.s, len() ) == 0 );  }


  Str  basename  ()  const  {    //  -------------------------  mStr  basename
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
      if     ( b[0] == '\0'        )  {  return  Str ( a, b-a );  }
      mstr  c  =  b + 1;
      while  ( c[0] == '/'         )  {  c++;  }
      if     ( c[0] == '\0'        )  {  return  Str ( a, b-a );  }
      a  =  c;  }  }


  Str  capture_until  ( const char accept )  const  {    //  --  capture_until
    return  Str ( s, find(accept).s - s );  }


  const void *  chr  ( cint c )  const  {    //  ------------------  mStr  chr
    return  s  ?  :: memchr ( s, c, n )  :  nullptr  ;  }


  constexpr static int  ctor_len  ( mstr p )  {    //  -------  mStr  ctor_len
    if  (  p == nullptr  ||  * p == '\0'  )  {  return  0;  }
    int  j  =  1;  while  ( p && * p )  {  j++;  p++;  };  return  j;  }


  Str  env_name  ()  const  {    //  ------------------------  mStr  env_name
    return  head ( "=" );  }


  Str  find  ( const char accept )  const  {    //  --------------  mStr  find
    //  20220820  are find() and tail() redundant?
    int  j  =  0;  for  (  ;  j < n && s[j] not_eq accept;  j ++  );
    return  Str ( s+j, n-j );  }


  Str  head  ( str sep, int start = 0 )  const  {    //  ---------  mStr  head
    if  ( s == nullptr )  {  return  nullptr;  }
    mstr  p      =  s;
    while  (  * p  &&  start-- > 0  )  {  p++;  }
    str   found  =  :: strstr ( p, sep );
    if  ( found )  {  return  Str ( p, found - p );  }
    return  nullptr;  }


  bool  is_inside  ( Str path )  const  {    //  ------------  mStr  is_inside

    //  return true iff path is an ancestor of s, or is equal to s.

    mstr  descendant  =  s;
    mstr  ancestor    =  path.s;
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


  int  len  ()  const  {    //  -----------------------------------  mStr  len
    //  the length not including the (optional) terminating '\0'
    return  n - ( n && s[n-1] == '\0' ? 1 : 0 );  }


  Str  skip_all  ( char c )  const  {    //  -----------------  mStr  skip_all
    int   j  =  n;  mstr  p  =  s;
    while  (  j  &&  * p == c )  {  j --;  p ++;  };  return  Str(p,j);  }


  int  spn  ( Str accept )  const  {    //  -----------------------  mStr  spn
    //  danger  spn() assumes (safely?) that s and accept are null-terminated.
    return  s  ?  :: strspn ( s, accept.s )  :  0  ;  }


  bool  starts_with  ( Str o )  const  {    //  -----------  mStr  starts_with
    return  len() >= o.len()  &&  memcmp ( s, o.s, o.len() ) == 0;  }


  Str  tail  ( str sep )  const  {    //  ------------------------  mStr  tail
    Str  found  =  :: strstr ( s, sep );
    return  found.s  ?  found.s + :: strlen(sep)  :  nullptr  ;  }


  Str  trunk  ( )  const  {    //  ------------------------------  mStr  trunk
    return  skip_all('/') .capture_until('/');  }


};    //  end  struct  mStr  ------------------------------  end  struct  mStr




struct  mCat2  {    //  ---------------------------------------  struct  mCat2
  Str  a, b;    //  used for operator + and string concatenation
  mCat2  ( Str & a, Str & b )  :  a(a), b(b)  {}
  Cat3  operator +  ( Str & c )  const;  };

struct  mCat3  {    //  ---------------------------------------  struct  mCat3
  Str  a, b, c;    //  used for operator + and string concatenation
  mCat3  ( Cat2 & o, Str & c )  :  a(o.a), b(o.b), c(c)  {}
  Cat4  operator +  ( Str & d )  const;  };

struct  mCat4  {    //  ---------------------------------------  struct  mCat4
  Str  a, b, c, d;    //  used for operator + and string concatenation
  mCat4  ( Cat3 & o, Str & d )  :  a(o.a), b(o.b), c(o.c), d(d)  {}  };


Cat2  operator +  ( str a, Str & b )           {  return  Cat2(a,b);      }
Cat2  mStr  :: operator +  ( Str & b )  const  {  return  Cat2(*this,b);  }
Cat3  mCat2 :: operator +  ( Str & c )  const  {  return  Cat3(*this,c);  }
Cat4  mCat3 :: operator +  ( Str & d )  const  {  return  Cat4(*this,d);  }




struct  oStr  :  mStr   {    //  xxos  -------------------------  struct  oStr


  oStr  ()  {}    //  --------------------------------------------  oStr  ctor
  oStr  ( str    o )  {  append(o);    }    //  ------------------  oStr  ctor
  oStr  ( Str &  o )  {  append(o);    }    //  ------------------  oStr  ctor
  oStr  ( Cat2 & o )  {  append(o);    }    //  ------------------  oStr  ctor
  oStr  ( Cat3 & o )  {  append(o);    }    //  ------------------  oStr  ctor
  oStr  ( Cat4 & o )  {  append(o);    }    //  ------------------  oStr  ctor
  oStr  (       oStr && o )  {  s=o.s; n=o.n; o.s=nullptr; o.n=0; }    // ctor
  oStr  ( const oStr &  o )  :  mStr()  {  append(o);  }    //  --  oStr  ctor


  ~oStr  ()  {    //  --------------------------------------------  oStr  dtor
    if  ( n )  {  memset ( (char*) s, 'c', n - 1 );  }
    free ( (char*) s );  }


  void  operator =  ( str    o )        {  n=0;  append(o);  }    //  --  op =
  void  operator =  ( Str &  o )        {  n=0;  append(o);  }    //  --  op =
  void  operator =  ( Cat2 & o )        {  n=0;  append(o);  }    //  --  op =
  void  operator =  ( Cat3 & o )        {  n=0;  append(o);  }    //  --  op =
  void  operator =  ( Cat4 & o )        {  n=0;  append(o);  }    //  --  op =
  void  operator =  ( const oStr & o )  {  n=0;  append(o);  }    //  --  op =

  void  operator +=  ( Str &  o )  {  append(o);  }    //  ------------  op +=
  void  operator +=  ( Cat2 & o )  {  append(o);  }    //  ------------  op +=
  void  operator +=  ( Cat3 & o )  {  append(o);  }    //  ------------  op +=
  void  operator +=  ( Cat4 & o )  {  append(o);  }    //  ------------  op +=


  void  append  ( Str & o )  {    //  --------------------------  oStr  append
    n         =  max(1,n) + o.len();
    char * p  =  realloc(n);
    memcpy ( p + n - 1 - o.len(), o.s, o.len() );  }


  void  append  ( char n, Str * p )  {    //  ------------------  oStr  append
    while  ( n -- > 0 )  {  append ( * p ++ );  }  }


  void  append  ( Cat2 & o )  {  append ( 2, & o.a );  }    //  ------  append
  void  append  ( Cat3 & o )  {  append ( 3, & o.a );  }    //  ------  append
  void  append  ( Cat4 & o )  {  append ( 4, & o.a );  }    //  ------  append


  static oStr  claim  ( Str o )  {    //  -----------------------  oStr  claim
    //  return an oStr that owns s.  s must be an unowned, malloc()ed string.
    oStr  rv;    rv.s=o.s;    rv.n=o.n;    return  rv;  }


  Str  leak  ()  {    //  ----------------------------------------  oStr  leak
    Str  rv  =  * this;  n=0;  s=nullptr;  return  rv;  }


  char *  realloc  ( cint j )  {    //  -----------------------  oStr  realloc
    char *  p  =  (char*) :: realloc ( (char*) s, j );
    if  ( p )  {  if(j){p[j-1]='\0';};  s=p;  return  p;  }
    fprintf ( stderr, "oStr  realloc failed\n" );  abort();  }


};    //  end  struct  oStr  ------------------------------  end  struct  oStr




struct  Argv  {    //  xxar  -----------------------------------  struct  Argv


  mstr *  p  =  nullptr;    //  nullptr terminated mstr[]


  Argv  ()  {}    //  --------------------------------------------  Argv  ctor
  Argv  ( mstr *  p )  :  p( p )          {}
  Argv  ( char ** p )  :  p( (mstr*) p )  {}    //  unsafe const kludge
  Argv  ( str *   p )  :  p( (mstr*) p )  {}    //  unsafe const kludge


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


  oStr  concat  ( Str sep = "  " )  const  {    //  -----------  Argv  concat
    Argv  o  ( * this );
    oStr  rv;
    if ( *(o.p) )  {  rv .append(*o);  o++;  }
    for  (  ;  *(o.p)  ;  o++  )  {  rv.append(sep);  rv.append(*o);  }
    return  rv;  }


  Str  env_get  ( Str k )  const  {    //  --------------------  Argv  env_get
    for  (  Argv o (*this)  ;  *(o.p)  ;  o++  )  {
      if  ( (*o).env_name() == k )  {  return  (*o);  }  }
    return  nullptr;  }


  int  len  ()  const  {    //  -----------------------------------  Argv  len
    int  n  =  0;  while  ( (*this)[n].s )  {  n ++;  }
    return  n;  }


  void  print  ( Str s )  const  {    //  -----------------------  Argv  print
    for  (  Argv o (*this)  ;  o  &&  o.p[0]  ;  o++  )  {
      fprintf  ( stderr, "%s%s\n", s.s, o[0].s );  }  }


};    //  end  struct  Argv  ------------------------------  end  struct  Argv




//  str.cpp  -  Parke's string library.
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
