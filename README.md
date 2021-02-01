# FlightCraft3D_ver2

This is a game based off of Simon Hasur's game named FlightCraft3D, 
which is described as "a little and 'minimalist' 3D Flight simulator". 

For information on the original game, see https://sourceforge.net/projects/stargl3d/, 
where you can download FlightCraft3D. The a.out file in the project is the 
compiled version of the original FlightCraft3D.

What I hope to do with FlightCraft3D is use it as a learning tool; 
specifically I want to use it to learn SDL2 and OpenGL. FlightCraft3D 
was created using SDL and OpenGL, but older versions. The first thing I 
did was change it to use newer versions of these libraries. 

## Changes Made to the Code
Other changes implemented so far include:
* Removed code that was commented out
* Removed unused global variables
* Removed unused local variables
* Formatted the code
* Broke the code up from one massive 5000+ file (main.c) into several modules
* Updated makefile to take into account the new modules
* Broke up massive functions into several, more manageable, functions
* Removed unnecessary code that did basically nothing (loops that had no code 
inside of them, for example)
* Added several optimizations to the code
* Changed the keys that control the plane's up and down movement
* Added many comments indicating how variables are being used

## Changes Made to the Airplane Model
* Plane no longer has surfaces that do not join
* Added missing facet (on left side of plane)
* Removed unused facets
* Plane is now symmetrical (left-side was made a mirror image of the right side)

## Notes
To compile, open a terminal in the OpenGL directory and type make.
This will generate an executable named fc3D.

## Playing the Game
To start the game, in a terminal in the OpenGL directory, type ./fc3D.

On startup the plane is on its side and is falling. I usually wait until the 
plane is pointing down and then press the up-arrow key continually until the 
plane is horizontal. You can then add speed to the plane by pressing the 9 key. 
You can make the plane go left or right by pressing the left or right arrow 
keys, respectively.

You can change the viewpoint by pressing the o key, multiple times if necessary.
Doing so gives you 4 views, and eventually takes you back to the first view.
* The first view, on startup, is an external view.
* The second view is looking at the top of the plane, from directly above it.
* The third view is looking at the right-side of the plane.
* The fourth view is from a point within the cockpit. However the viewing angle is wrong, 
and you will have to use the c or v keys to rotate the viewpoint until it is 
facing forward. 

In the cockpit, you can also use the x key or Shift-X key to decrease or increase, 
respectively, the x position of the eyepoint in the cockpit.
You can use the y or Shift-Y key to decrase or increase, respectively, the y 
position of the eyepoint in the cockpit.
You can also use the z or Shift-z key to increase or decrease the z position 
of the eyepoint.

However, the z modification contains a bug, when pressing z it should decrease 
the z position of the eyepoint, and instead it is increased. Similarly pressing 
Shift-z should increase the z position of the eyepoint, but instead it is decreased.
