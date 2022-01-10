# Lxroot - a software virtualization tool


### About Lxroot

`lxroot` is a lightweight alternative to `chroot`, Docker, and other software virtualization tools.

`lxroot` allows a non-root user to quickly and easily create a "chroot-style" virtual software environment (via Linux namespaces), and then run one or more programs (a "guest userland") inside that environment.


### Videos

-  Lxroot presentation at PackagingCon 2021:  [**abstract**](https://pretalx.com/packagingcon-2021/talk/PMPUSW/)  |  [**slides**](https://pretalx.com/media/packagingcon-2021/submissions/PMPUSW/resources/20211110_Lxroot_7ILURuB.pdf)  |  [**video**](https://www.youtube.com/watch?v=1rw7ww0k_mk)


### Lxroot and `vland`

While you can run `lxroot` directly, I recommend learning `vland` first.

[`vland`](https://github.com/parke/vland) is a virtual userland manager.  `vland` is implemented as a convenience wrapper around `lxroot`.  `vland` can automatically download, install, and configure a guest userland for use with `lxroot`.  Both `vland` and `lxroot` operate without root access.


###  Supported guest Linux distributions

The below table summarizes the Linux distributions that I have used as guests with `vland` and `lxroot`.  
Other Linux distributions may also work inside `lxroot`.

|  Distro  |  Can install root filesystem?  |  Can install packages?  |  Can build packages?  |
|  :--     |  :-:                           |  :-:                    |  :-:                  |
|  Alpine  |  yes                           |  yes                    |  yes                  |
|  Arch    |  yes                           |  yes                    |  yes                  |
|  Ubuntu  |  work in progress              |  work in progress       |  unknown              |
|  Void    |  yes                           |  probably yes?          |  unknown              |


### Learn more

*  `vland`
   *  [use cases](https://github.com/parke/lxroot/wiki/use_cases)
   *  [tutorial](https://github.com/parke/vland/wiki/tutorial) - installation and usage
   *  [man page](https://github.com/parke/vland/wiki/man_page)
   *  [wiki](https://github.com/parke/lxroot/wiki)
* `lxroot`
   *  [tutorial](https://github.com/parke/lxroot/wiki/tutorial) - installation and usage
   *  [man page](https://github.com/parke/lxroot/wiki/man_page)
   *  [limitations](https://github.com/parke/lxroot/wiki/limitations)
