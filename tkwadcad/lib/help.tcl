#
# Help package for tkwadcad
#
# Copyright (C) 1999-2001 by Udo Munk (munkudo@aol.com)
#
# Permission to use, copy, modify, and distribute this software
# and its documentation for any purpose and without fee is
# hereby granted, provided that the above copyright notice
# appears in all copies and that both that copyright notice and
# this permission notice appear in supporting documentation.
# The author and contibutors make no representations about the
# suitability of this software for any purpose. It is provided
# "as is" without express or implied warranty.
#

package provide WCHelp 1.3

namespace eval WCHelp {

  variable about
  variable helpkeys
  variable helpmouse

  namespace export About HelpKeys HelpMouse

  # We need a fixed sized font for the text displayed in the text widgets,
  # so that the output looks OK. Documentation says use "systemfixed" under
  # Windows, but on my system that still doesn't use a fixed sized font and
  # the output looks yuckie. Tried "ansifixed" and "oemfixed" too, that all
  # aren't fixed sized fonts, someone at MS has a different idea than me about
  # what a fixed sized font is...
  #if { [string compare $tcl_platform(platform) windows] == 0 } {
  #  font create mytext -family systemfixed -size 12
  #} else {
  #  font create mytext -family fixed -size 12
  #}

  # So use Courier font, that is a fixed size font on all(?) systems.
  font create mytext -family courier -size 12

  proc About {} {
    set f .about
    if [Dialog_Create $f "About" -borderwidth 10] {
      text $f.txt -width 65 -heigh 16 -borderwidth 2 -relief sunken \
	-font mytext
      button $f.ok -text OK -font widgets -command {set about(ok) 1}
      grid $f.txt -sticky news
      grid $f.ok -sticky ns -pady 5
      wm resizable $f 0 0
      Dialog_Show $f
    }

    $f.txt config -state normal
    $f.txt delete 1.0 end

    $f.txt insert end "\t\tTkWadCad Release 1.2\n"
    $f.txt insert end "\t\t====================\n\n"
    $f.txt insert end "Copyright (C) 1998-2001 by Udo Munk (munkudo@aol.com)\n\n"
    $f.txt insert end "Permission to use, copy, modify, and distribute this software\n"
    $f.txt insert end "and its documentation for any purpose and without fee is\n"
    $f.txt insert end "hereby granted, provided that the above copyright notice\n"
    $f.txt insert end "appears in all copies and that both that copyright notice and\n"
    $f.txt insert end "this permission notice appear in supporting documentation.\n"
    $f.txt insert end "The author and contibutors make no representations about the\n"
    $f.txt insert end "suitability of this software for any purpose. It is provided\n"
    $f.txt insert end "AS IS without express or implied warranty.\n\n"
    $f.txt insert end "id Software, Raven Software and Activision do not support this\n"
    $f.txt insert end "software.\n"
    $f.txt config -state disabled

    set about(ok) 0
    Dialog_Wait $f about(ok) $f.ok
    Dialog_Dismiss $f
  }

  proc HelpKeys {} {
    set f .helpkey
    if [Dialog_Create $f "Help on Keys" -borderwidth 10] {
      text $f.txt -width 76 -heigh 25 -borderwidth 2 -relief sunken \
	-font mytext
      scrollbar $f.s -command {.helpkey.txt yview} -orient vertical
      button $f.ok -text OK -font widgets -command {set helpkeys(ok) 1}
      grid $f.txt $f.s -sticky news
      grid $f.ok -sticky ns -pady 5
      wm resizable $f 0 0
      Dialog_Show $f
    }

    $f.txt config -state normal
    $f.txt delete 1.0 end

    $f.txt insert end "Key accelarators only work, if the pointer is set on the map drawing canvas.\n\n"
    $f.txt insert end "Key    Function                        Key    Function\n"
    $f.txt insert end "============================================================================\n"
    $f.txt insert end "q      Quit                            Up     Scroll up\n"
    $f.txt insert end "F1     This help                       Down   Scroll down\n"
    $f.txt insert end "Enter  Edit object properties          Left   Scroll left\n"
    $f.txt insert end "DEL    Delete object                   Right  Scroll right\n"
    $f.txt insert end "CTRL-C Copy object properties          CTRL-P Paste object properties\n\n"
    $f.txt insert end "g, G   Dec./Inc. grid step             +, -   Zoom in/out\n\n"
    $f.txt insert end "CTRL-T Insert thing                    <, >   Rotate thing left, right\n"
    $f.txt insert end "CTRL-V Insert vertex\n"
    $f.txt insert end "CTRL-L Insert line                     CTRL-S Split line\n"
    $f.txt insert end "CTRL-F Flip line                       CTRL-D Flip sidedefs\n\n"
    $f.txt insert end "CTRL-M Make Wad                        CTRL-B Build Wad\n"
    $f.txt insert end "CTRL-R Run Wad\n\n"
    $f.txt insert end "v      Show/hide vertices              t      Show/hide things\n"
    $f.txt insert end "a      Show/hide line arrowheads\n"
    $f.txt config -state disabled

    set helpkeys(ok) 0
    Dialog_Wait $f helpkeys(ok) $f.ok
    Dialog_Dismiss $f
  }

  proc HelpMouse {} {
    set f .helpmouse
    if [Dialog_Create $f "Help on Mouse" -borderwidth 10] {
      text $f.txt -width 70 -heigh 20 -borderwidth 2 -relief sunken \
	-font mytext
      scrollbar $f.s -command {.helpmouse.txt yview} -orient vertical
      button $f.ok -text OK -font widgets -command {set helpmouse(ok) 1}
      grid $f.txt $f.s -sticky news
      grid $f.ok -sticky ns -pady 5
      wm resizable $f 0 0
      Dialog_Show $f
    }

    $f.txt config -state normal
    $f.txt delete 1.0 end

    $f.txt insert end "To move an object place the pointer over it, press and hold button 1.\n"
    $f.txt insert end "Then drag it to the new location and drop it by releasing button 1.\n\n"
    $f.txt insert end "To edit an objects properties double click on it with button 1.\n\n"
    $f.txt insert end "To delete an object click on it with button 3.\n\n"
    $f.txt insert end "To split a line and insert a new vertex move the pointer over the line\n"
    $f.txt insert end "at the location where to split it. Then click with button 2.\n\n"
    $f.txt insert end "To scroll the map without using the scroll bars place the pointer on\n"
    $f.txt insert end "an empty spot on the map. Then press button 2 and move the pointer\n"
    $f.txt insert end "to scroll the map.\n"
    $f.txt config -state disabled

    set helpmouse(ok) 0
    Dialog_Wait $f helpmouse(ok) $f.ok
    Dialog_Dismiss $f
  }

}
