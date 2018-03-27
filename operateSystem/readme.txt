
1. how to run the fat system

1) Use make command to generate 5 binary files:

diskinfo
disklist
diskget
diskput
disktest (Because the test.img doesn't supply subdirectory in root directory entry, disktest binary
is used to insert a subdirectoy to test the commands (disklist, diskget, diskput) which have relationship with subdirectory function.)

2. Test the functions in root directory

2) ./diskinfo test.img (Show the fat system superblock info)
3) ./disklist test.img / (Show the files and directories in the root dir. There are 3 files in the root dir: mkfile.cc, foo.txt, disk.img.gz)
4) ./diskget test.img /mkfile.cc a.cc (Get the mkfile.cc in root dir in fat system to local system. After that command, you can use editor such as vim to read the a.cc file)
5) ./diskput test.img a.cc /b.cc (Put the local a.cc to fat filesystem into the root directory)
6) ./disklist test.img / (Then you can see that b.cc has already been in the fat system. Ofcouse you can get it from fat filesystem to local. Please try it use the diskget command!)


3. Test the functions about sub-directory

7) ./disktest test.img (Use my custom command `disktest` to insert a sub-directory named `subdirectory` in the root dir of the fat filesystem.)
8) ./disklist test.img / (Show the files in root directory, you will find a new directory named `subdirectoy` in the standard output.)
9) ./disklist test.img /subdirectory (Show the files in this sub-directory you will find 2 fake files I mocked. They don't have file blocks and are just used to test `disklist` command.)
10) ./diskput test.img a.cc /subdirectory/c.cc (Put a.cc to sub-directory)
11) ./disklist test.img /subdirectory (Show the subdirectory you will find that c.cc has already been there)
12) ./diskget test.img /subdirectory/c.cc d.cc (With this command you can get the c.cc from /subdirectory in fat filesystem to local system.)


