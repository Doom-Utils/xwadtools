Activision and Raven are releasing this code for people to learn from and play
with. The code retains its original copyright and can not be used for profit, 
any work released using this code must contain credit for it.

Issues:
this was set up for a watcom makefile.  it should not be difficult to convert
for VC, or whatever.

UM sayz: now it is setup for a GNU makefile, because GNU make is available
         for all and any platforms, while watcom make, VC or whatever are not.

This is not supported software.  But i might be able to a tech question here
and there.

james monroe
jmonroe@mail.ravensoft.com


Another note by UM:
This compiler is very limited with the number of variables, scripts, etc.
it will compile into a BEHAVIOR lump. IMHO it is not a good idea to raise
this limits for extended Doom engines, because then some Hexen user can
compile scripts with stuff out of bounds without a compiler warning. This
will crash Hexen engines, because the fixed sized arrays in the code have
the same dimensions than here. So the right thing to do is to derive a new
compiler from this and name it xacc, or whatever, and leave this one alone
as it is.
