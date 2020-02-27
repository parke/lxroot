

//  Copyright (c) 2020 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv2 (see end of file) WITHOUT ANY WARRANTY.


#define  LXROOT_VERSION  "0.0.20200227.1354"


//  compile with:  gcc  -Wall  -Werror  lxroot.c  -o lxroot


#define   _GNU_SOURCE
#include  <assert.h>
#include  <errno.h>
#include  <sched.h>
#include  <limits.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <unistd.h>
#include  <sys/syscall.h>
#include  <sys/mount.h>
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <sys/wait.h>
#include  <fcntl.h>


#define  not  !


void  usage  ()  {    //  ---------------------------------------------  usage
  printf ( "\n" );
  printf ( "usage:  lxroot  [-nr]  /path/to/new_root  [dir]...  "
	   "[command [arg]...]\n" );
  printf ( "  options:\n" );
  printf ( "    -n           unshare network interfaces (CLONE_NEWNET)\n" );
  printf ( "    -r           map uid and gid to 0 (simulate root user)\n" );
  printf ( "    [dir]...     bind mount these directories inside new_root\n" );
  printf ( "    [command]    command to exec (defaults to /bin/sh)\n" );  }


//  lib  ----------------------------------------------------------------  lib


char *  concat  ( const char * a, const char * b )  {    //  ---------  concat
  if  (  a == NULL  ||  b == NULL  )  {  return  NULL;  }
  char *  rv  =  malloc ( strlen(a) + strlen(b) + 1 );    //  leak
  if  ( rv )  {  rv[0]  =  '\0';  strcat ( rv, a );  strcat ( rv, b );  }
  return  rv;  }


int  eq  ( const char * a, const char * b )  {    //  --------------------  eq
  if  ( a == NULL  ||  b == NULL )  return  0;
  return  strcmp ( a, b ) == 0;  }


int  is_dir  ( const char * path )  {    //  -------------------------  is_dir
  struct stat  st;
  if  (  stat ( path, & st ) == 0  &&  st .st_mode & S_IFDIR  )  {
    return  1;  }
  errno  =  ENOENT;
  return  0;  }


int  is_file  ( const char * path )  {    //  -------------------------  is_dir
  struct stat  st;
  if  (  stat ( path, & st ) == 0  &&  st .st_mode & S_IFREG  )  {
    return  1;  }
  errno  =  ENOENT;
  return  0;  }


int  is_option  ( const char * s )  {    //  ----------------------  is_option
  return  (  s
	     &&  strlen ( s ) < 4
	     &&  strspn ( s, "-"   ) == 1
	     &&  strspn ( s, "-nr" ) == strlen ( s )  );  }


int  list_contains  ( const char * const *  list,    //  ------  list_contains
		      const char *          s )  {
  for  (  ;  list && * list;  list ++  )  {
    if  ( eq ( * list, s ) )  {  return  1;  }  }
  return  0;  }


//  state  ------------------------------------------------------------  state


typedef struct  {    //  --------------------------------------  struct  State

  const char * const * const  environ;
  const int                   argc;
  const char * const * const  argv;
  const uid_t                 uid;
  const gid_t                 gid;

  int                         argn;
  const char *                arg;
  const char * *              new_env;

  pid_t  fork;       //  return value from fork()
  int    wstatus;    //  wait status

  const char *  new_root_rel;    //  may be a relative path
  const char *  new_root;        //  will be an absolute path

  char  opt_n, opt_r;
  char  bind_new_root, pivot, trace_flag, unshare;

  }  State;


State  state_init  ( const int           argc,    //  -------------  state_init
		     const char * const  argv[] )  {

  State  st  =  { (const char**) environ, argc, argv, getuid(), getgid() };
  st .argn  =  1;
  st .arg   =  argc && argv  ?  argv[1]  :  NULL  ;
  st .fork  =  -2;
  return  st;  }


void  arg_next  ( State * st )  {    //  ---------------------------  arg_next
  if  ( st -> argn >= st -> argc )  {  return;  }
  st -> argn  +=  1;
  st -> arg   =   st -> argv [ st -> argn ];  }


const char *  arg_consume  ( State * st )  {    //  -------------  arg_consume
  const char *  rv  =  st -> arg;
  arg_next ( st );
  return  rv;  }


const char *  arg_consume_dir  ( State * st )  {    //  -----  arg_consume_dir
  const char *  path  =  arg_consume ( st );
  assert ( path );
  assert ( is_dir ( path ) );
  return  path;  }


int  eq_consume  ( State * st, const char * expect )  {    //  ---  eq_consume
  if  ( eq ( st -> arg, expect ) )  {  arg_next(st);  return  1;  }
  return  0;  }


//  system call wrappers  ------------------------------  system call wrappers


//  macro  die  --------------------------------------------------  macro  die
//  see  https://stackoverflow.com/q/5588855   regarding ##__VA_ARGS__
//  see  https://stackoverflow.com/a/11172679  regarding ##__VA_ARGS__
#define  die( format, ... )  {						\
  fprintf ( stderr, "lxroot  error  " format "  ", ##__VA_ARGS__ );	\
  perror(NULL);								\
  exit ( 1 );  }


//  macro  die2  ------------------------------------------------  macro  die2
#define  die2( format, ... )  {						\
  fprintf ( stderr, "lxroot  error  " format "\n", ##__VA_ARGS__ );	\
  exit ( 1 );  }


//  macro  trace  ----------------------------------------------  macro  trace
#define  trace( format, ... )  {			\
  if  ( st && st -> trace_flag )  {			\
    fprintf ( stderr, format "\n", ##__VA_ARGS__ );  }  }


//  macro  try  --------------------------------------------------  macro  try
#define  try( function, st, format, ... )  {		\
  trace ( format, ##__VA_ARGS__ );		\
  if  ( function ( __VA_ARGS__ ) == 0 )  {  return;  }	\
  else  {  die ( format, ##__VA_ARGS__ );  }  }


//  macro  try2  ------------------------------------------------  macro  try2
#define  try2( function, format, ... )  {		\
  if  ( function ( __VA_ARGS__ ) == 0 )  {  return;  }	\
  else  {  die ( format, ##__VA_ARGS__ );  }  }


//  note  the below capitalized functions call exit(1) on error


void  Bind  ( const State *  st,    //  --------------------------------  Bind
	      const char *   source,
	      const char *   target,
	      unsigned long  mountflags )  {
  trace ( "bind     %s  %s  0x%lx", source, target, mountflags );
  if  ( mount ( source, target, NULL, mountflags, NULL ) == 0 )  {  return;  }
  die ( "bind  %s  %s  %ld", source, target, mountflags );  }


void  Chdir  ( const State * st, const char * path )  {    //  --------  Chdir
  try ( chdir, st, "chdir    %s", path );  }


void  Chroot  ( const State * st, const char * new_root )  {    //  --  Chroot
  try ( chroot, st, "chroot   %s", new_root );  }


void  Close  ( int fd )  {    //  -------------------------------------  Close
  try2 ( close, "close  %d", fd );  }


void  Execve  ( const State *       st,    //  -----------------------  Execve
		const char *        pathname,
		const char * const  argv[],
		const char * const  envp[] )  {

  if  ( st -> trace_flag )  {
    fprintf ( stderr, "execve " );
    for  ( const char * const * p = argv;  * p;  p++  )  {
      fprintf  ( stderr, "  %s", * p );  }
    fprintf ( stderr, "\n" );  }

  execve ( pathname, (char**) argv, (char**) envp );
  //  execve only returns on failure, so ...
  die ( "execve  %s", pathname );  }


void  Fork  ( State * st )  {    //  -----------------------------------  Fork
  if  ( st -> fork != -2 )  {  die ( "extra fork?" );  }
  if  ( ( st -> fork = fork() ) >= 0 )  {
    trace ( "fork     (fork returned %d)", st -> fork );
    return;  }
  die ( "fork" );  }


void  Mkdir  ( const State *  st,    //  ------------------------------  Mkdir
	       const char *   path,
	       const mode_t   mode )  {
  if  ( is_dir ( path ) )  {  return;  }
  try ( mkdir, st, "mkdir    %s  %o", path, mode );  }


void  Mount  ( const State *  st,    //  ------------------------------  Mount
	       const char *   source,
	       const char *   target,
	       const char *   filesystemtype )  {
  trace ( "mount    %s  %s  %s", source, target, filesystemtype );
  if  ( mount ( source, target, filesystemtype, 0, NULL ) == 0 )  {  return;  }
  die ( "mount  %s  %s  %s",  source, target, filesystemtype );  }


void  Open  ( int *          fd,
	      const char *   pathname,
	      const int      flags )  {
  if  ( ( * fd = open ( pathname, flags ) ) >= 0 )  {  return;  }
  die ( "open  %s  %d", pathname, flags );  }


void  Pivot  ( const State *  st,    //  ------------------------------  Pivot
	       const char *   new_root,
	       const char *   put_old )  {
  trace ( "pivot    %s  %s", new_root, put_old );
  if  ( syscall ( SYS_pivot_root, new_root, put_old ) == 0 )  {  return;  }
  die ( "pivot  %s  %s", new_root, put_old );  }


void  Rmdir  ( const State * st, const char * pathname )  {    //  ----  Rmdir
  try ( rmdir, st, "rmdir    %s", pathname );  }


void  Umount2  ( const State *  st,    //  --------------------------  Umount2
		 const char *   target,
		 int            flags )  {
  try ( umount2, st, "umount2  %s  0x%x", target, flags );  }


void  Unshare  ( const State * st, const int flags )  {    //  ------  Unshare
  try ( unshare, st, "unshare  0x%08x", flags );  }


pid_t  Wait  ( State * st )  {    //  ----------------------------------  Wait
  trace ( "wait     (parent will now call wait)" );
  pid_t  pid  =  wait ( & st -> wstatus );
  if  ( pid > 0 )  {
    trace ( "wait     wait returned  pid %d  status 0x%x",
		  pid, st -> wstatus );
    return  pid;  }
  die ( "wait" );  }


void  Write  ( int fd, const void * buf, size_t count )  {    //  -----  Write
  if  ( write ( fd, buf, count ) == count )  {  return;  }
  die ( "write  %d  %ld", fd, count );  }


//  doers  ------------------------------------------------------------  doers


void  Uid_map ( State * st )  {    //  ------------------------------  Uid_map

  uid_t  un_uid  =  st -> opt_r  ?  0  :  st -> uid  ;
  uid_t  un_gid  =  st -> opt_r  ?  0  :  st -> gid  ;

  char  u_map[80];
  char  g_map[80];
  int   fd;

  snprintf ( u_map, sizeof u_map, "%u %u 1\n", un_uid, st -> uid );
  snprintf ( g_map, sizeof g_map, "%u %u 1\n", un_gid, st -> gid );

  trace ( "uid_map  %u %u 1  %u %u 1",
		un_uid,  st->uid,  un_gid,  st->gid );

  Open   (  & fd,  "/proc/self/uid_map",  O_RDWR    );
  Write  (  fd,  u_map,  strlen ( u_map )           );
  Close  (  fd                                      );
  Open   (  & fd,  "/proc/self/setgroups",  O_RDWR  );
  Write  (  fd, "deny", 4                           );
  Close  (  fd                                      );
  Open   (  & fd,  "/proc/self/gid_map",  O_RDWR    );
  Write  (  fd, g_map, strlen ( g_map )             );
  Close  (  fd                                      );  }


void  do_unshare  ( State * st )  {    //  -----------------------  do_unshare

  assert ( st -> unshare == 0 );
  if  ( st -> new_root == NULL )  {  die ( "new_root is NULL" );  }

  int  clone_flags  =  0;
  clone_flags  |=  CLONE_NEWNS;
  clone_flags  |=  CLONE_NEWPID;
  clone_flags  |=  CLONE_NEWUSER;
  clone_flags  |=  st -> opt_n  ?  CLONE_NEWNET  :  0  ;

  Unshare ( st, clone_flags );
  Uid_map ( st );

  st -> unshare  =  1;  }


void  do_bind_new_root  ( State * st )  {    //  -----------  do_bind_new_root

  if  ( st -> unshare == 0 )  {  do_unshare ( st );  }

  Chdir  (  st,  st -> new_root                     );
  Bind   (  st,  ".",  ".",  MS_BIND                );
  Chdir  (  st,  st -> new_root                     );
  Bind   (  st,  "/dev",  "dev",  MS_BIND | MS_REC  );
  Bind   (  st,  "/sys",  "sys",  MS_BIND | MS_REC  );
  st -> bind_new_root  =  1;  }


void  do_pivot  ( State * st )  {    //  ---------------------------  do_pivot

  assert ( st -> pivot == 0 );
  if  ( st -> bind_new_root == 0 )  {  do_bind_new_root ( st );  }

  const char *  new_root      =  st -> new_root;
  const char *  lxroot_pivot  =  concat ( new_root, "/tmp/lxroot_pivot" );

  Mkdir    (  st,  lxroot_pivot, 0700       );
  Pivot    (  st,  new_root,  lxroot_pivot  );
  Chdir    (  st,  "/"                      );
  Chroot   (  st,  "/"                      );

  st -> pivot  =  1;  }


void  do_fork  ( State * st )  {    //  -----------------------------  do_fork

  assert  ( st -> fork == -2 );
  if  ( st -> pivot == 0 )  {  do_pivot ( st );  }

  Fork ( st );
  if  ( st -> fork == 0 )  {  return;  }    //  child returns
  if  ( st -> fork >  0 )  {    //  parent waits for child to exit
    Wait ( st );
    if  ( WIFEXITED ( st -> wstatus ) )  {
      exit ( WEXITSTATUS ( st -> wstatus ) );  }
    fprintf ( stderr, "lxroot  warning  child exited abnormally\n" );
    exit ( 1 );  }  }


void  do_exec  ( State * st )  {    //  -----------------------------  do_exec

  if  ( st -> fork == -2 )  {  do_fork ( st );  }

  //  We try to do as much as possible before calling fork().  Howewer...
  //  It seems /proc can only be mounted:
  //    1) after fork() (which sort of makes sense), and
  //    2) before unmounting put_old (which is mildly surprising?).

  Mount    (  st,  "proc",  "/proc",  "proc"         );
  Umount2  (  st,  "/tmp/lxroot_pivot",  MNT_DETACH  );
  Rmdir    (  st,  "/tmp/lxroot_pivot"               );

  const char * const          bin_sh[]  =  {  "/bin/sh",  NULL  };
  const char * const * const  argv      =  st -> argn < st -> argc  ?
    & st -> argv [ st -> argn ]  :  bin_sh  ;

  Execve ( st, argv[0], argv, st -> environ );
  exit ( 1 );  }    //  Execve only returns on error


//  arg handlers  ----------------------------------------------  arg handlers


int  is_command  ( const char * s )  {    //  --------------------  is_command
  static const char * const commands[]  =
    { "chdir", "pivot", "trace", "untrace", NULL };
  return  list_contains ( commands, s );  }


void  arg_chdir  ( State * st )  {    //  -------------------------  arg_chdir
  Chdir ( st, arg_consume_dir ( st ) );  }


void  arg_command  ( State * st )  {    //  ---------------------  arg_command
  if       ( eq_consume ( st, "chdir"   ) )  {  arg_chdir ( st );        }
  else if  ( eq_consume ( st, "pivot"   ) )  {  do_pivot  ( st );        }
  else if  ( eq_consume ( st, "trace"   ) )  {  st -> trace_flag  =  1;  }
  else if  ( eq_consume ( st, "untrace" ) )  {  st -> trace_flag  =  0;  }
  else  die ( "arg_command  bad command  %s", (st -> arg) );  }


void  arg_dir  ( State * st )  {    //  -----------------------------  arg_dir
  assert ( is_dir ( st -> arg ) );

  if  ( st -> new_root == NULL )  {
    st -> new_root_rel  =  arg_consume(st);
    st -> new_root      =  realpath ( st -> new_root_rel, NULL );    //  leak
    trace ( "new_root  =  %s", st -> new_root );
    assert ( is_dir ( st -> new_root_rel ) );
    assert ( is_dir ( st -> new_root     ) );
    do_bind_new_root ( st );
    return;  }

  assert ( st -> new_root );
  const char *  source_rel  =  arg_consume_dir ( st );
  const char *  source      =  realpath ( source_rel, NULL );        //  leak
  const char *  target      =  concat ( st -> new_root, source );    //  leak
  Bind ( st, source_rel, target, MS_BIND );  }


void  arg_file  ( State * st )  {    //  ---------------------------  arg_file
  assert ( is_file ( st -> arg ) );
  printf ( "arg_file  %s  unimplemented  exiting...\n", st -> arg );
  exit ( 3 );  }


void  arg_option ( State * st )  {    //  ------------------------  arg_option
  assert ( is_option ( st -> arg ) );
  trace ( "arg_option  %s", st -> arg );
  if  ( strchr ( st -> arg, 'n' ) )  {  st -> opt_n  =  1;  }
  if  ( strchr ( st -> arg, 'r' ) )  {  st -> opt_r  =  1;  }
  arg_next ( st );  }


void  arg_process  ( State * st )  {    //  ---------------------  arg_process
  const char *  s  =  st -> arg;
  if       ( is_option  ( s ) )  {  arg_option   ( st );  }
  else if  ( is_command ( s ) )  {  arg_command  ( st );  }
  else if  ( is_dir     ( s ) )  {  arg_dir      ( st );  }
  else if  ( is_file    ( s ) )  {  do_exec      ( st );  }
  else                           {  do_exec      ( st );  }  }


int  main  ( const int argc, const char * argv[]  )  {    //  ----------  main
  State  st  =  state_init ( argc, argv );
  while  ( st .argn < st .argc )  {  arg_process ( & st );  }
  do_exec ( & st );    //  note  do_exec() will only return on error
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
