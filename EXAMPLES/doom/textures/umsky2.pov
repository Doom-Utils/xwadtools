// vi:set tabstop=4:
//
// This sky texture is derived from 's_cloud4.pov' included in povray3.
// Modified to render a sky texture for Doom by Udo Munk.
//
// This texture doesn't match seamlessly and looks pretty bad as is in
// Doom. This should not be used as is, it should be modified to render
// with a width of 1024 pixels and then used as a 360 degrees sky.

global_settings {
	assumed_gamma 1.0
}

#include "doom.inc"

camera {
	location <0, 1, -100>
	up y
	right x*1.33
	direction z
	look_at <0, 20, 0>
	angle 57
}

light_source { <100, 100, -50> doom4 }

#declare P_Cloud =
pigment {
	bozo
	color_map {
		[0.0, 0.1	color red 0.85 green 0.85 blue 0.85
					color red 0.55 green 0.60 blue 0.65]
		[0.1, 0.5	color red 0.55 green 0.60 blue 0.65
					color rgb <0.184, 0.184, 0.309>]
		[0.5, 1.001	color rgb <0.184, 0.184, 0.309>
					color rgb <0.1, 0.1, 0.2>]
	}
	turbulence 0.65
	octaves 6
	omega 0.707
	lambda 2
	scale <6, 4, 6>
}

#declare S_Cloud =
sky_sphere {
	pigment {
		gradient y
		pigment_map {
			[0.00 rgb <0.184, 0.184, 0.309>]
			[0.15 P_Cloud scale <0.05, 0.40, 0.05> rotate z*30]
			[0.45 P_Cloud scale <0.25, 0.15, 0.50>]
		}
	}
}

sky_sphere { S_Cloud }
