

//  Copyright (c) 2020 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv2 (see end of file) WITHOUT ANY WARRANTY.


#define  LXROOT_VERSION  "0.0.20200201.1755"


//  compile with:  gcc  -Wall  -Werror  lxroot.c  -o lxroot


#define   _GNU_SOURCE
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
  printf ( "usage:  lxroot  [-r]  /path/to/newroot  [dir]...  "
	   "[command [arg]...]\n" );
  printf ( "  options:\n" );
  printf ( "    -r           map uid and gid to 0 (simulated root user)\n" );
  printf ( "    [dir]...     bind mount these directories inside nweroot\n" );
  printf ( "    [command]    command to exec (defaults to /bin/sh)\n" );  }


typedef struct  {    //  -------------------------------------  struct  Config
  char           valid;
  char **  args;
  const char *   path;
  const char *   new_root;    //  realpath of path
  long           uid, gid, un_uid, un_gid;  }  Config;


//  wrapper functions  ------------------------------------  wrapper functions


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


//  macro  die  --------------------------------------------------  macro  die
//  see  https://stackoverflow.com/q/5588855   regarding ##__VA_ARGS__
//  see  https://stackoverflow.com/a/11172679  regarding ##__VA_ARGS__
#define  die( format, ... )						\
  fprintf ( stderr, "lxroot  error  " format "  ", ##__VA_ARGS__ );	\
  perror(NULL);								\
  exit ( 1 );


//  macro  try  ---------------------------------------------------  macro  try
#define  try( function, format, ... )				\
  if  ( function ( __VA_ARGS__ ) == 0 )  {  return;  }		\
  else  {  die ( format, ##__VA_ARGS__ );  }


//  note  the below capitalized functions call exit(1) on error


void  Bind  ( const char * source,    //  ------------------------------  Bind
	      const char * target,
	      unsigned long mountflags )  {
  if  ( mount ( source, target, NULL, mountflags, NULL ) == 0 )  {  return;  }
  die ( "bind  %s  %s  %ld", source, target, mountflags );  }


void  Chdir  ( const char * path )  {    //  --------------------------  Chdir
  try ( chdir, "chdir  %s", path );  }


void  Chroot  ( const char * new_root )  {    //  --------------------  Chroot
  try ( chroot, "chroot  %s", new_root );  }


void  Close  ( int fd )  {    //  -------------------------------------  Close
  try ( close, "close  %d", fd );  }


void  Execve  ( const char * pathname,    //  ------------------------  Execve
		char * const argv[],
		char * const envp[]  )  {
  execve ( pathname, argv, envp );
  //  execve only returns on failure, so ...
  die ( "execve  %s", pathname );  }


void  Fork  ( pid_t * pid )  {    //  ----------------------------------  Fork
  if  ( ( * pid = fork() ) >= 0 )  {  return;  }
  die ( "fork" );  }


void  Mkdir  ( const char * path, mode_t mode )  {    //  -------------  Mkdir
  if  ( is_dir ( path ) )  {  return;  }
  try ( mkdir, "mkdir  %s  %o", path, mode );  }


void  Mount  ( const char * source,    //  ----------------------------  Mount
	       const char * target,
	       const char * filesystemtype )  {
  if  ( mount ( source, target, filesystemtype, 0, NULL ) == 0 )  {  return;  }
  die ( "mount  %s  %s  %s",  source, target, filesystemtype );  }


void  Open  ( int * fd, const char * pathname, int flags )  {    //
  if  ( ( * fd = open ( pathname, flags ) ) >= 0 )  {  return;  }
  die ( "open  %s  %d", pathname, flags );  }


void  Pivot  ( const char * new_root,    //  --------------------------  Pivot
	       const char * put_old )  {
  if  ( syscall ( SYS_pivot_root, new_root, put_old ) == 0 )  {  return;  }
  die ( "pivot  %s  %s", new_root, put_old );  }


void  Rmdir  ( const char * pathname )  {    //  ----------------------  Rmdir
  try ( rmdir, "rmdir  %s", pathname );  }


void  Umount2  ( const char * target, int flags )  {    //  ---------  Umount2
  try ( umount2, "umount2  %s  %d", target, flags );  }


void  Unshare  ( int flags )  {    //  ------------------------------  Unshare
  try ( unshare, "unshare  %d", flags );  }


pid_t  Wait  ( int * wstatus )  {    //  -------------------------------  Wait
  pid_t  pid  =  wait ( wstatus );  if  ( pid > 0 )  {  return  pid;  }
  die ( "wait" );  }


void  Write  ( int fd, const void * buf, size_t count )  {    //  -----  Write
  if  ( write ( fd, buf, count ) == count )  {  return;  }
  die ( "write  %d  %ld", fd, count );  }


//  lxroot  ----------------------------------------------------------  lxroot


void  config_parse    //  --------------------------------------  config_parse
( Config * config,  const int argc, char * argv[] )  {

  Config  c  =  { 0 };

  c.valid   =  1;    //  assume valid until proven otherwise
  c.args    =  & argv[1];
  c.un_uid  =  c.uid  =  getuid();    //  default
  c.un_gid  =  c.gid  =  getgid();    //  default

  if  ( argc == 1 )  {  c.valid  =  0;  }

  if  ( eq ( * c.args, "-r" ) )  {
    c.un_uid  =  c.un_gid  =  0;
    c.args ++;  }

  if  ( * c.args )  {
    c.new_root  =  realpath ( * c.args, NULL );    //  leak
    c.args ++; }

  * config  =  c;  }


void  config_print  ( Config * c )  {    //  -------------------  config_print
  printf(  "lxroot version    %s\n",        LXROOT_VERSION         );
  printf(  "config_valid      %d\n",        c -> valid             );
  printf(  "path              %s\n",        c -> path              );
  printf(  "new_root          %s\n",        c -> new_root          );
  printf(  "uid               %ld  %ld\n",  c -> un_uid, c -> uid  );
  printf(  "gid               %ld  %ld\n",  c -> un_gid, c -> gid  );  }


void  Uid_map  ( Config * c )  {    //  -----------------------------  Uid_map

  char  u_map[80];
  char  g_map[80];
  int   fd;

  snprintf ( u_map, sizeof u_map, "%ld %ld 1\n", c -> un_uid, c -> uid );
  snprintf ( g_map, sizeof g_map, "%ld %ld 1\n", c -> un_gid, c -> gid );

  Open   (  & fd,  "/proc/self/uid_map",  O_RDWR    );
  Write  (  fd,  u_map,  strlen ( u_map )           );
  Close  (  fd                                      );
  Open   (  & fd,  "/proc/self/setgroups",  O_RDWR  );
  Write  (  fd, "deny", 4                           );
  Close  (  fd                                      );
  Open   (  & fd,  "/proc/self/gid_map",  O_RDWR    );
  Write  (  fd, g_map, strlen ( g_map )             );
  Close  (  fd                                      );  }


void  Bind_dirs  ( Config * c )  {    //  -------------------------  Bind_dirs

  for  (  ;  * c->args;  c->args ++ )  {

    const char *  source       =  * c->args;
    char *        target       =  concat ( c->new_root, source );    //  leak
    char *        real_source  =  realpath ( source, NULL );         //  leak
    char *        real_target  =  realpath ( target, NULL );         //  leak

    if  ( not is_dir ( real_source ) )  {  return;  }
    if  ( not is_dir ( real_target ) )  {  die ( "bind_dir  %s", target );  }

    //  printf ( "\n"  "bind_dir\n" );
    //  printf ( "  source  %s\n", real_source );
    //  printf ( "  target  %s\n", real_target );

    Bind ( real_source, real_target, MS_BIND );  }  }


void  args_print ( const char * const * args )  {    //  ---------  args_print
  printf ( "args\n" );
  for  (  ;  * args;  args++  )  {
    printf ( "  %s\n", * args );  }  }


void  Child_exec  ( Config * config )  {    //  ------------------  Child_exec
  //  args_print ( config -> args );
  char * const    bin_sh[]  =  {  "/bin/sh",  NULL  };
  char * const *  argv      =  config -> args;
  argv  =  ( * argv )  ?  argv  :  bin_sh  ;
  //  args_print ( config -> args );
  Execve ( argv[0], argv, environ );  }


void  Child  ( Config * config )  {    //  ----------------------------  Child

  const char *  new_root      =  config -> new_root;
  const char *  lxroot_pivot  =  concat ( new_root, "/tmp/lxroot_pivot" );

  Chdir       (  new_root                          );
  Bind        (  ".",  ".",  MS_BIND               );
  Chdir       (  new_root                          );
  Bind        (  "/dev", "dev", MS_BIND | MS_REC   );
  Bind        (  "/sys", "sys", MS_BIND | MS_REC   );
  Bind_dirs   (  config                            );
  Mkdir       (  lxroot_pivot, 0700                );
  Pivot       (  new_root,  lxroot_pivot           );
  Chdir       (  "/"                               );
  Chroot      (  "/"                               );
  Mount       (  "proc",  "/proc",  "proc"         );
  Umount2     (  "/tmp/lxroot_pivot",  MNT_DETACH  );
  Rmdir       (  "/tmp/lxroot_pivot"               );
  Child_exec  (  config                            );  }


void  lxroot  ( Config * config )  {    //  --------------------------  lxroot

  if  ( ! is_dir ( config -> new_root ) )  {  die ( "%s", config -> path );  }

  int  clone_flags  =  0;
  //  clone_flags  |=  CLONE_NEWNET;    //  intentionally disabled
  clone_flags  |=  CLONE_NEWNS;
  clone_flags  |=  CLONE_NEWPID;
  clone_flags  |=  CLONE_NEWUSER;

  pid_t  pid;
  int    wstatus;

  Unshare  (  clone_flags  );
  Uid_map  (  config       );
  Fork     (  & pid        );

  if  ( pid == 0 )  {
    Child ( config );
    fprintf ( stderr, "lxroot  warning  unexpected return from Child()\n" ); }

  if  ( pid > 0 )  {
    Wait (  & wstatus );
    if  ( WIFEXITED ( wstatus ) )  {  exit ( WEXITSTATUS ( wstatus ) );  }
    fprintf ( stderr, "lxroot  warning  child exited abnormally\n" ); }

  exit ( 1 );  }


int  main  ( const int argc, char * argv[] )  {    //  -----------------  main
  Config  config  =  { 0 };
  config_parse ( & config, argc, argv );
  if  ( config .valid == 0 )  {  usage();  }
  printf ( "\n" );  config_print ( & config );  printf ( "\n" );
  if  ( config .valid )  {  lxroot ( & config );  return  0;  }
  return  1;  }




//    lxroot.c  -  a rootless Linux alternative to /usr/sbin/chroot
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
