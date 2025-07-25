# mms-cpp-floodfill

Write a Micromouse maze-solving algorithm in C++ using Flood-Fill algorithm.

For use with [mackorone/mms](https://github.com/mackorone/mms), a Micromouse simulator.

## Setup

1. Clone this repository
1. [Download the Micromouse simulator](https://github.com/mackorone/mms#download)
1. Run the simulator and click the "+" button to configure a new algorithm
1. Enter the config for your algorithm (name, directory, build command, and run command)
1. For the Run command, you can choose between the two 'Main' files. Simply write the file name of the one you want in the Build COmmand dialog box. 
1. Click the "Run" button

## Examples

Windows:

![](https://github.com/mackorone/mms-cpp/blob/master/config-windows.png)

Linux (Ubuntu) and MacOS:

![](https://github.com/mackorone/mms-cpp/blob/master/config-linux.png)


## Notes

- If you're using Windows, you may need to download and install [MinGW](http://mingw.org/wiki/Getting_Started)
- Communication with the simulator is done via stdin/stdout, use stderr to print output
- Descriptions of all available API methods can be found at [mackorone/mms#mouse-api](https://github.com/mackorone/mms#mouse-api)
- The code is a BFS flood-fill algorithm
- Credits to [Mackarone](https://github.com/mackorone) for the API files and .gitignore 
