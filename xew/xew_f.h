#ifndef FD_xew_mainf_h_
#define FD_xew_mainf_h_
/* Header file generated with fdesign. */

/**** Callback routines ****/

extern void wb_cb(FL_OBJECT *, long);
extern void export_cb(FL_OBJECT *, long);
extern void snd_cho_cb(FL_OBJECT *, long);
extern void null_cb(FL_OBJECT *, long);
extern void quit_cb(FL_OBJECT *, long);
extern void pal_ch_cb(FL_OBJECT *, long);
extern void bg_cb(FL_OBJECT *, long);


/**** Forms and Objects ****/

extern FL_FORM *xew_mainf;

extern FL_OBJECT
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



/**** Creation Routine ****/

extern void create_the_forms(void);

#endif /* FD_xew_mainf_h_ */
