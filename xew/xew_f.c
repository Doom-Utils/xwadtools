/* Form definition file generated with fdesign. */

#include "forms.h"
#include <stdlib.h>
#include "xew_f.h"

FL_FORM *xew_mainf;

FL_OBJECT
        *wad_brow,
        *img_pxm,
        *snd_cho,
        *img_cho,
        *pal_type_cho,
        *exp_img_fmt_cho,
        *rev_but,
        *mesg_brow,
        *wheel_pxm,
        *img_txt;

void create_form_xew_mainf(void)
{
  FL_OBJECT *obj;

  if (xew_mainf)
     return;

  xew_mainf = fl_bgn_form(FL_NO_BOX,600,540);
  obj = fl_add_box(FL_UP_BOX,0,0,600,540,"");
    fl_set_object_color(obj,FL_COL1,FL_BOTTOM_BCOL);
  wad_brow = obj = fl_add_browser(FL_HOLD_BROWSER,10,30,230,345,"WAD Entries:");
    fl_set_object_color(obj,FL_TOP_BCOL,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,wb_cb,0);
  img_pxm = obj = fl_add_pixmap(FL_NORMAL_PIXMAP,260,30,320,200,"Image:");
    fl_set_object_boxtype(obj,FL_FLAT_BOX);
    fl_set_object_color(obj,FL_YELLOW,FL_BLACK);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_button(FL_TOUCH_BUTTON,260,240,240,40,"Export this WAD entry to file");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,export_cb,0);
  snd_cho = obj = fl_add_choice(FL_NORMAL_CHOICE,410,350,90,20,"Play Sound Effects");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,snd_cho_cb,0);
  img_cho = obj = fl_add_choice(FL_NORMAL_CHOICE,410,310,90,20,"View Images");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,null_cb,0);
  obj = fl_add_text(FL_NORMAL_TEXT,290,370,210,30,"MUS Entries Played Externally");
    fl_set_object_boxtype(obj,FL_NO_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_button(FL_NORMAL_BUTTON,510,240,70,40,"Quit");
    fl_set_object_lsize(obj,FL_LARGE_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,quit_cb,0);
  pal_type_cho = obj = fl_add_choice(FL_NORMAL_CHOICE,410,330,90,20,"Palette Type");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,pal_ch_cb,0);
  exp_img_fmt_cho = obj = fl_add_choice(FL_NORMAL_CHOICE,410,290,90,20,"Image Export Format Is");
    fl_set_object_boxtype(obj,FL_UP_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,null_cb,0);
  rev_but = obj = fl_add_lightbutton(FL_PUSH_BUTTON,500,350,90,20,"Reversed");
    fl_set_object_color(obj,FL_COL1,FL_RED);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,null_cb,0);
  mesg_brow = obj = fl_add_browser(FL_NORMAL_BROWSER,10,400,580,130,"Messages:");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  wheel_pxm = obj = fl_add_pixmap(FL_NORMAL_PIXMAP,340,460,40,40,"");
    fl_set_object_lcol(obj,FL_COL1);
  img_txt = obj = fl_add_text(FL_NORMAL_TEXT,310,10,160,20,"");
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLDITALIC_STYLE);
  obj = fl_add_button(FL_TOUCH_BUTTON,480,10,100,20,"Background...");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,bg_cb,0);
  fl_end_form();

}
/*---------------------------------------*/

void create_the_forms(void)
{
  create_form_xew_mainf();
}

