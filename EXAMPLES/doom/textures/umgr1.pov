// vi:set tabstop=4:
//
// Copyright (C) 1999 by Udo Munk (munkudo@aol.com)
//
// Feel free to use, share and modify, but leave my Copyright intact.
//
// Scene file for povray3, renders some sort of granite texture for Doom.

global_settings {
	assumed_gamma 1.0
}

#include "doom.inc"

camera {
	location <0, 0, -125>
	look_at <0, 0, 0>
}

plane { z, 10
	texture {
		pigment { granite color_map { [0 doom3] [1 doom4] } }
		finish { ambient 1 reflection 0 diffuse 0 }
	}
}
