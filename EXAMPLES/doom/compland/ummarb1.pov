// vi:set tabstop=4:
//
// Copyright (C) 1999 by Udo Munk (munkudo@aol.com)
//
// Feel free to use, share and modify, but leave my Copyright intact.
//
// Scene file for povray3, renders some sort of marble texture for Doom.

global_settings {
	assumed_gamma 1.0
}

#include "doom.inc"
#include "pinkmarb.map"

camera {
	location <0, 0, -20>
    look_at <0, 0, 0>
}

light_source { <-50, 50, -1000> color doom88 }
background { color doom195 }

plane { z, 10
		texture {
			pigment {
				granite
				color_map { M_PinkMarb }
				scale 1
			}
			finish { specular 0.44 roughness 0.25 }
		}
}
