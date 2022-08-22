

//  unit.cpp  -  Lxroot's C++ unit tests.
//  Copyright (c) 2021 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.

//  version  20220822


#define   LXROOT_MAIN_SKIP  1
#include  "lxroot.cpp"


struct  Unit  {    //  -----------------------------------------  struct  Unit


  void  t_basename  ( Str input, Str expect, cint line )  {    //  -----------
    Str  actual  =  input.basename();
    if  ( actual == expect )  {  return;  }
    printe ( "\n"  "basename_test  failed  line %d\n", line );
    printe ( "  input   %d  '%s'", input.n,  input.s  );
    printe ( "  expect  %d  '%s'", expect.n, expect.s );
    printe ( "  actual  %d  '%s'", actual.n, actual.s );
    abort();  }


  void  t_Str  ()  {    //  -------------------------------------  Unit  t_Str

    printe ("");

    Str  null   =  nullptr;       assert  ( null.n  == 0 );
    ;                             assert  ( null.s  == nullptr );

    Str  empty  =  "";            assert  ( empty.n == 0 );
    ;                             assert  ( empty.s not_eq nullptr );

    Str  a      =  "a";           assert  ( a.n  == 2 );
    Str  a1     =  { "a", 1 };    assert  ( a1.n == 1 );
    Str  a2     =  { "a", 2 };    assert  ( a2.n == 2 );

    Str  trace  =  "--trace";     assert  ( trace.starts_with ("--") );
    ;                             assert  ( trace.starts_with ("-") );
    ;                             assert  ( trace == "--trace" );

    return;  }


  void  t_Cat  ()  {    //  -------------------------------------------  t_Cat

    Str  a  =  "a";
    Str  b  =  "b";
    Str  c  =  "c";
    Str  d  =  "d";

    Cat2  c2  =  a + b;

    assert  ( c2.a == "a" );
    assert  ( c2.b == "b" );

    assert  ( (&c2.a)[0] == "a" );
    assert  ( (&c2.a)[1] == "b" );

    Cat3  c3  =  a + b + c;

    assert  ( c3.a == "a" );
    assert  ( c3.b == "b" );
    assert  ( c3.c == "c" );

    assert  ( (&c3.a)[0] == "a" );
    assert  ( (&c3.a)[1] == "b" );
    assert  ( (&c3.a)[2] == "c" );

    Cat3  c3a  =  "a" + b + "c";

    assert  ( c3a.a == "a" );
    assert  ( c3a.b == "b" );
    assert  ( c3a.c == "c" );

    assert  ( (&c3a.a)[0] == "a" );
    assert  ( (&c3a.a)[1] == "b" );
    assert  ( (&c3a.a)[2] == "c" );

    assert  ( oStr (  a  +  b  ) == "ab" );
    assert  ( oStr (  a  + "b" ) == "ab" );
    assert  ( oStr ( "a" +  b  ) == "ab" );

    assert  ( oStr (  a  +  b  +  c  ) == "abc" );
    assert  ( oStr (  a  + "b" + "c" ) == "abc" );
    assert  ( oStr ( "a" +  b  +  c  ) == "abc" );

    assert  ( oStr (  a  +  b  +  c  +  d  ) == "abcd" );
    assert  ( oStr (  a  + "b" + "c" + "d" ) == "abcd" );
    assert  ( oStr ( "a" +  b  +  c  +  d  ) == "abcd" );

    return;  }


  void  t_basename_all  ()  {    //  -------------------  Unit  t_basename_all

    t_basename ( "",          "",     __LINE__  );
    t_basename ( "/",         "/",    __LINE__  );
    t_basename ( "//",        "/",    __LINE__  );
    t_basename ( "///",       "/",    __LINE__  );
    t_basename ( "a",         "a",    __LINE__  );
    t_basename ( "/b",        "b",    __LINE__  );
    t_basename ( "c/",        "c",    __LINE__  );
    t_basename ( "/d/",       "d",    __LINE__  );
    t_basename ( "e/f",       "f",    __LINE__  );
    t_basename ( "/g/h",      "h",    __LINE__  );
    t_basename ( "abc",       "abc",  __LINE__  );
    t_basename ( "/def",      "def",  __LINE__  );
    t_basename ( "ghi/",      "ghi",  __LINE__  );
    t_basename ( "/jkl/",     "jkl",  __LINE__  );
    t_basename ( "mno/pqr",   "pqr",  __LINE__  );
    t_basename ( "/stu/vwx",  "vwx",  __LINE__  );
    t_basename ( "./xyz",     "xyz",  __LINE__  );

    return;  }


  void  t_oStr  ()  {    //  -----------------------------------  Unit  t_oStr

    Str   a   =  "a";                 //  printe ( "a   '%s'", a.s  );
    Str   a1  =  {"a", 1};
    Str   a2  =  {"a", 2};            //  printe ( "a   '%s'", a.s  );

    assert ( a  == "a" );
    assert ( a1 == "a" );
    assert ( a2 == "a" );
    assert ( a  == a1  );
    assert ( a  == a2  );

    Str   b   =  "b";                 //  printe ( "b   '%s'", b.s  );

    assert  ( a not_eq b );

    oStr  ab  =  a + b;               //  printe ( "ab  '%s'", ab.s );

    assert  ( ab == "ab" );           //
    assert  ( ab == oStr(a+b)  );     //

    Str  c    =  "c";                 //
    Str  d    =  "d";                 //
    oStr  cd  =  c + "=" + d;         //

    assert  ( cd == "c=d" );          //

    oStr  ef  =  cd + "ef" + "gh";    //

    assert  ( ef == "c=defgh" );      //
    assert  ( cd == "c=d" );          //

    return;  }


  static void  t_Argv  ()  {    //  ----------------------------  Unit  t_Argv

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

    return;  }


  void  t_st_to_ms  ()  {    //  ---------------------------  Unit  t_st_to_ms

    const auto  st_to_ms  =  Syscall :: st_to_ms;

    assert(    st_to_ms ( ST_RDONLY      )  ==  MS_RDONLY         );
    assert(    st_to_ms ( ST_NOSUID      )  ==  MS_NOSUID         );
    assert(    st_to_ms ( ST_NODEV       )  ==  MS_NODEV          );
    assert(    st_to_ms ( ST_NOEXEC      )  ==  MS_NOEXEC         );
    assert(    st_to_ms ( ST_SYNCHRONOUS )  ==  MS_SYNCHRONOUS    );
    assert(    st_to_ms ( ST_MANDLOCK    )  ==  MS_MANDLOCK       );
    assert(    st_to_ms ( ST_NOATIME     )  ==  MS_NOATIME        );
    assert(    st_to_ms ( ST_NODIRATIME  )  ==  MS_NODIRATIME     );
    assert(    st_to_ms ( ST_RELATIME    )  ==  MS_RELATIME       );

    return;  }


  void  main  ()  {    //  -------------------------------------  Unit  main
    t_Str();
    t_Cat();
    t_basename_all();
    t_oStr();
    t_Argv();
    t_st_to_ms();
    printf ( "unit.cpp  done  all tests passed\n" );  }


};    //  end  struct  Unit  ------------------------------  end  struct  Unit




int  main  ()  {    //  --------------------------------------------  ::  main
  Unit() .main();
  return  0;  }




//  unit.cpp  -  Lxroot's C++ unit tests.
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
