Save the penguin
================
An unfinished game for the ludum dare #46 jam (theme: keep it alive).

Concept:
 The penguin is stranded on a patch of ice, and there are tentacle monsters in
 the water, so it's afraid to swim. Help it return to its nest by floating
 nearby pieces of ice into its path for it to hop onto, before the tentacle
 monsters get it.

The game was unfortunately not completed, but here it is at it stands a few
minutes after the deadline.

Run it and press F1 for instructions. You can play around with the physics
and let the penguin jump between patches of ice, but there's no goal yet, and
the tentacles won't really harm you.

Credits
-------
Code: John Tsiombikas
Additional code: eleni-Maria Stea, and Zisis Sialveras.
Procedural graphics: John Tsiombikas
Penguin 3D model from: https://www.turbosquid.com/3d-models/3ds-penguin/653190
Environment map from: https://learnopengl.com/Advanced-OpenGL/Cubemaps

License
-------
Copyright (C) 2020 John Tsiombikas <nuclear@member.fsf.org>

This program is free software, feel free to use, modify, and/or redistribute
under the terms of the GNU General Public License v3, or at your option any
later version published by the Free Software Foundation. See COPYING for details

Build instructions
------------------
If you're building from git, grab the data with:
   `svn co svn://mutantstargoat.com/datadirs/ld46_keepalive data`.

First install all dependencies:
   - FreeGLUT
   - GLEW
   - Assimp

Then just type `make` to build.
