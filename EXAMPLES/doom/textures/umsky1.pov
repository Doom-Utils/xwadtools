// vi:set tabstop=4:
//
// This sky texture is derived from 'skysph2.pov' included in povray3, by
// Dieter Bayer. Modified to render a sky texture for Doom by Udo Munk.
//
// The TrueColor image looks very good, the Doom palette quantised image
// looks less good, not enough shades of blue in the Doom palette. So
// blue skies probably are not a great idea.

global_settings {
	assumed_gamma 1.0
}

#include "doom.inc"

camera {
	location <0, 2.5, -4>
	right <4/3, 0, 0>
	up <0, 1, 0>
	direction <0, 0, 1>
	look_at <0, 4, 0>
}

background { color doom195 }

// Dummy object, out of view, without the sky won't render
sphere { -2*y, 1
	pigment { color rgb <1, 1, 1> }
}

sky_sphere {
	pigment {
		gradient y
		color_map { [0.75 color doom193] [1.00 color doom200] }
		scale 2
		translate <-1, -1, -1>
	}
	pigment {
		bozo
		turbulence 0.5
		octaves 7
		omega .49876
		lambda 2.5432
		color_map {
			[ 0.0 color rgbf<.75, .75, .75, 0.1> ]
			[ 0.4 color rgbf<.9, .9, .9, .9> ]
			[ 0.7 color rgbf<1, 1, 1, 1> ]
		}
		scale 6/10
		scale <1, 0.3, 0.3>
	}
	pigment {
		bozo
		turbulence 0.6
		octaves 8
		omega .5125
		lambda 2.56578
		color_map {
			[ 0.0 color rgbf<.375, .375, .375, 0.2> ]
			[ 0.4 color rgbf<.45, .45, .45, .9> ]
			[ 0.6 color rgbf<0.5, 0.5, 0.5, 1> ]
		}
		scale 6/10
		scale <1, 0.3, 0.3>
	}
}
