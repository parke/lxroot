

//  unit.cpp  -  Lxroot's C++ unit tests.
//  Copyright (c) 2021 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.

//  version  20211030


#define   LXROOT_MAIN_SKIP  1
#include  "lxroot.cpp"


struct  Unit  {    //  -----------------------------------------  struct  Unit


  void  t_basename  ( Str s, Str expect )  {    //  --------  Unit  t_basename
    if  ( s.basename() == expect )  {  return;  }
    oStr  actual  =  s.basename();
    printe ( "basename_test  failed\n" );
    printe ( "  s       %s\n", s.s      );
    printe ( "  expect  %s\n", expect.s );
    printe ( "  actual  %s\n", actual.s );
    abort();  }


  void  t_basename_all  ()  {    //  -------------------  Unit  t_basename_all

    t_basename ( "",          ""    );
    t_basename ( "/",         "/"   );
    t_basename ( "//",        "/"   );
    t_basename ( "///",       "/"   );
    t_basename ( "a",         "a"   );
    t_basename ( "/b",        "b"   );
    t_basename ( "c/",        "c"   );
    t_basename ( "/d/",       "d"   );
    t_basename ( "e/f",       "f"   );
    t_basename ( "/g/h",      "h"   );
    t_basename ( "abc",       "abc" );
    t_basename ( "/def",      "def" );
    t_basename ( "ghi/",      "ghi" );
    t_basename ( "/jkl/",     "jkl" );
    t_basename ( "mno/pqr",   "pqr" );
    t_basename ( "/stu/vwx",  "vwx" );
    t_basename ( "./xyz",     "xyz" );

    return;  }


  void  t_oStr  ()  {    //  -----------------------------------  Unit  t_oStr

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
