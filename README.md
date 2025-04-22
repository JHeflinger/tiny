[TINY BUILDER](https://github.com/JHeflinger/tiny) - A tiny builder for MinGW C projects
=======================================================

## INFO

Tiny is a small project manager for C projects. If you're using gcc as your compiler, and you're tired of CMake, tired of Bazel, tired of premake, and all those complex mega builders, then you're in the right place! Tiny is super simple, super easy to set up, and super fast!
> NOTE: Tiny is not a commercial product, and is just my personal build tool. If you try to break it, it may just break! Use at your own risk!

## STATUS

Tiny is currently experimental! It supports incremental builds in C and can be used for all simple projects. Builds for Linux and Windows are currently supported. There are no plans to port for Mac as of now, but if you'd like to submit a PR to do so, feel free!

## REQUIREMENTS

All Tiny requires is gcc installed and ready to use on your path. Beyond that, you're good to go!

## BUILDING

There is no official builder for Tiny, it's all in one singular C file! Compile that into an 
executable however you wish! With gcc, it's as simple as `gcc tiny.c`

## HOW TO USE

Tiny works right out of the box, so you can just run it in your project's working directory! By default, it will look for a `src/` folder in your working directory, and use all the `.c` and `.h` files to compile your project, and it will look for a `main.c` file in that directory to use as the entrypoint. However, if you'd like to customize this, keep reading to learn about `.tinyconf`!!

To configure Tiny to be a bit more accustomed to your use, make a `.tinyconf` file in your working directory. In `.tinyconf`, there are several options you can use to configure your project:

| Precursor | Value | Description |
| --------- | ----- | ----------- |
| PROJECT | <path_to_directory | This is where your project files that you will be working with should be located! All your `.c` and `.h` files should be here. |
| MAIN | <file_name> | This is what file you have the main function in. Note that this is just the filename, not the actual path to it! |
| INCLUDE | <path_to_directory> | This is where you can define directories of files you want included in your available headers of your project. If you have third party vendors, define those here! |
| LINK | <link_argument> | This is where you can define what you want to link into your program, such as OpenGL or pthreads |
| LIB | <path_to_directory> | If you have libraries that you want to link that aren't on your path, enter the path to those libraries here! |
| SOURCE | <path_to_directory_or_file> | Here you can define outside sources to include into your project. If you have third party vendors with `.c` files, include those here! You can provide the path to a singular file, or a directory of sources, whichever works best for you! |

Additionally, you can also preface each configuration line with an operating system to use it exclusively on that operating system build! Currently only `WINDOWS` and `LINUX` are supported,
as there are only builds for those respective operating systems available. Some example usage of this feature may look like the following:

```
WINDOWS INCLUDE vendor/windows/lib/include
LINUX INCLUDE vendor/linux/lib/include
```

Some overall example content of what a `.tinyconf` can look like can look like the following:

```
PROJECT src
MAIN main.c
INCLUDE vendor/raylib/include
INCLUDE vendor/EasyC/include
LINUX LINK :linux_amdx64_libraylib.a
LINK m
LINK pthread
LINK GL
LINK GLU
LINUX LIB vendor/raylib/lib
SOURCE vendor/EasyC/include
```
There are also some various flags you can add to customize your build process!

| Flag | Description |
| ---- | ----------- |
| -p | optimizes your build with -O3 and defines a PROD definition in your code |
| -v | outputs version |
| -a | audits project directory for vulnerabilities |

## OUTPUT

Tiny compiles your executable into a build/program.exe file. So once your build completes, run it from there. Happy building!
