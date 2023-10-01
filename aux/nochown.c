

//  nochown.c  -  Ignore syscalls to chown().  Useful with some containers.
//  Copyright (c) 2023 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.


//  lxroot / aux / nochown.c  version  20231001


#include  <seccomp.h>    //  seccomp
#include  <stdio.h>      //  fprintf
#include  <stdlib.h>     //  exit
#include  <unistd.h>     //  chown, execvp


static void  usage  ()  {    //  --------------------------------------  usage
  fprintf ( stderr, "\n"  "usage:  nochown  command  [arg ...]\n" );  }


static scmp_filter_ctx  ctx;    //  ----------------------  static global  ctx


static int  die  ( char * s )  {    //  ---------------------------------  die
  fprintf ( stderr, "nochown  fatal error  %s\n", s );
  exit ( 1 );  }


static int  rule  ( const int syscall )  {    //  ----------------------  rule
  return  seccomp_rule_add ( ctx, SCMP_ACT_ERRNO(0), syscall, 0 );  }


void  main  ( const int argc, char ** argv )  {    //  -----------------  main

  argc < 2  &&  (  usage(),  exit(0),  0  );

  (  ctx  =  seccomp_init ( SCMP_ACT_ALLOW )  )  ||  die ( "seccomp_init" );

  rule (SCMP_SYS( chown    ))  &&  die ( "rule" );
  rule (SCMP_SYS( fchown   ))  &&  die ( "rule" );
  rule (SCMP_SYS( lchown   ))  &&  die ( "rule" );
  rule (SCMP_SYS( fchownat ))  &&  die ( "rule" );

  seccomp_load ( ctx )         &&  die ( "seccomp_load" );
  seccomp_release ( ctx );
  chown ( "/no_file", 0, 0 )   &&  die ( "sanity failed" );

  argc < 2  &&  die ( "argc < 2" );
  execvp ( argv[1], argv + 1 );
  die ( "execvp" );  }


//  nochown.c  -  Ignore syscalls to chown().  Useful with some containers.
//
//  Copyright (c) 2023 Parke Bostrom, parke.nexus at gmail.com
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
