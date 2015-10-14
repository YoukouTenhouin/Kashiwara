#ifndef __IZUMO_MISC_MACRO_STREQU_H__
#define __IZUMO_MISC_MACRO_STREQU_H__

#define IZM_STREQU1(a, b) ((a)[0] == (b)[0])
#define IZM_STREQU2(a, b) ((a)[0] == (b)[0] && IZM_STREQU1(a+1, b+1))
#define IZM_STREQU3(a, b) ((a)[0] == (b)[0] && IZM_STREQU2(a+1, b+1))
#define IZM_STREQU4(a, b) ((a)[0] == (b)[0] && IZM_STREQU3(a+1, b+1))
#define IZM_STREQU5(a, b) ((a)[0] == (b)[0] && IZM_STREQU4(a+1, b+1))
#define IZM_STREQU6(a, b) ((a)[0] == (b)[0] && IZM_STREQU5(a+1, b+1))
#define IZM_STREQU7(a, b) ((a)[0] == (b)[0] && IZM_STREQU6(a+1, b+1))
#define IZM_STREQU8(a, b) ((a)[0] == (b)[0] && IZM_STREQU7(a+1, b+1))
#define IZM_STREQU9(a, b) ((a)[0] == (b)[0] && IZM_STREQU8(a+1, b+1))

#endif	/* __IZUMO_MISC_MACRO_STREQU_H__ */
