//
// "$Id: gui-common.h,v 1.1.1.1.2.2 2009/09/30 12:59:35 bsavelev Exp $"
//
//   ESP Software Wizard common header file for the ESP Package Manager (EPM).
//
//   Copyright 1999-2006 by Easy Software Products.
//   Copyright 2009-2010 by Dr.Web.
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2, or (at your option)
//   any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//

//
// Include necessary headers...
//

#include "epmstring.h"
#include "epm.h"
#include <stdio.h>
#include <stdlib.h>
#include <FL/Fl_Help_View.H>
#include <libintl.h>
#include <locale.h>


//
// Distribution structures...
//

struct gui_depend_t			//// Dependencies
{
  int	type;				// Type of dependency
  char	product[64];			// Name of product or file
  int	vernumber[2];			// Version number(s)
};

struct gui_dist_t			//// Distributions
{
  int		type;			// Package type
  char		product[64];		// Product name
  char		name[256];		// Product long name
  char		version[32];		// Product version
  int		vernumber;		// Version number
  char		fulver[256];		// Full version
  int		num_depends;		// Number of dependencies
  gui_depend_t	*depends;		// Dependencies
  int		rootsize,		// Size of root partition files in kbytes
		usrsize;		// Size of /usr partition files in kbytes
  char		*filename;		// Name of package file
};

struct gui_intype_t			//// Installation types
{
  char		name[80];		// Type name
  char		label[1024];		// Type label
  int		num_products;		// Number of products to install (0 = select)
  int		products[200];		// Products to install
  int		size;			// Size of products in kbytes
};


//
// Define a C API function type for comparisons...
//

extern "C" {
typedef int (*compare_func_t)(const void *, const void *);
}


//
// Globals...
//

#ifdef _DEFINE_GLOBALS_
#  define VAR
#else
#  define VAR	extern
#endif // _DEFINE_GLOBALS_

VAR int			NumDists;	// Number of distributions in directory
VAR gui_dist_t		*Dists;		// Distributions in directory
VAR int			NumInstalled;	// Number of distributions installed
VAR gui_dist_t		*Installed;	// Distributions installed
VAR int			NumInstTypes;	// Number of installation types
VAR gui_intype_t	InstTypes[8];	// Installation types


//
// Prototypes...
//

void		gui_add_depend(gui_dist_t *d, int type, const char *name,
		               int lowver, int hiver);
gui_dist_t	*gui_add_dist(int *num_d, gui_dist_t **d);
gui_dist_t	*gui_find_dist(const char *name, int num_d, gui_dist_t *d);
void		gui_get_installed(void);
void		gui_load_file(Fl_Help_View *hv, const char *filename);
int		gui_sort_dists(const gui_dist_t *d0, const gui_dist_t *d1);
char*		findMypath(const char* argv);
void		set_gettext(char *argv[]);


//
// License names
//

#define LIC_EN "license"
#define LIC_RU "license.ru"

//
// Panes...
//

#define PANE_WELCOME	0
#define PANE_TYPE	1
#define PANE_SELECT	2
#define PANE_CONFIRM	3
#define PANE_LICENSE	4
#define PANE_INSTALL	5
#define PANE_POSTIN	6


//
// End of "$Id: gui-common.h,v 1.1.1.1.2.2 2009/09/30 12:59:35 bsavelev Exp $".
//
