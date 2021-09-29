This is a small command-line tool that creates a spining wheel GIF animation
from any given picture.
-------------------------------------------------------------------------

USAGE:

   HeLeCheLun  [-h] [--version] [-a <18>] [-d <40>] [-w <512>] filepaths


Where:

   -d <40>,  --delay <40>
     Interframe delay in milliseconds

   -a <18>,  --angle-step <18>
     Interframe angle step

   -w <512>,  --max-width <512>
     Maximum width size

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.

   filepaths (accepted multiple times) <Input file path(s)>
     (required) Input image file(s)


Source Code is here: https://github.com/Wohlstand/wheel-maker-tool/

-----------------------------------------------------------------------

How to build it from the source? Easily by the simple CMake way:

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=MinSizeRel ..
make -j 5

and then, in the build directory you will get the "HeLeCheLun" executable
