/*
 * "$Id: osx.c,v 1.13 2005/01/11 21:20:17 mike Exp $"
 *
 *   MacOS X package gateway for the ESP Package Manager (EPM).
 *
 *   Copyright 2002-2004 by Easy Software Products.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 * Contents:
 *
 *   make_osx() - Make a MacOS X software distribution package.
 */

/*
 * Include necessary headers...
 */

#include "epm.h"


/*
 * Local globals...
 */
 
static const char * const pm_paths[] =	/* Paths to PackageMaker program */
		{
		  "/Developer/Applications/PackageMaker.app",
		  "/Developer/Applications/Utilities/PackageMaker.app",
		  NULL
		};


/*
 * Local functions...
 */

static int	make_subpackage(const char *prodname, const char *directory,
		                dist_t *dist, const char *subpackage);


/*
 * 'make_osx()' - Make a Red Hat software distribution package.
 */

int					/* O - 0 = success, 1 = fail */
make_osx(const char     *prodname,	/* I - Product short name */
         const char     *directory,	/* I - Directory for distribution files */
         const char     *platname,	/* I - Platform name */
         dist_t         *dist,		/* I - Distribution information */
	 struct utsname *platform)	/* I - Platform information */
{
  int		i;			/* Looping var */
  FILE		*fp;			/* Spec file */
  char		filename[1024];		/* Destination filename */
  char		current[1024];		/* Current directory */


  REF(platname);
  REF(platform);

 /*
  * Create the main package and subpackages (if any)...
  */

  if (make_subpackage(prodname, directory, dist, NULL))
    return (1);

  for (i = 0; i < dist->num_subpackages; i ++)
    if (make_subpackage(prodname, directory, dist, dist->subpackages[i]))
      return (1);

  if (dist->num_subpackages)
  {
   /*
    * Create a meta package for the whole shebang...
    */

    if (Verbosity)
      puts("Creating MacOS X metapackage...");

    getcwd(current, sizeof(current));

   /*
    * Copy the resources for the license, readme, and welcome (description)
    * stuff...
    */

    if (Verbosity)
      puts("Copying temporary resource files...");

    snprintf(filename, sizeof(filename), "%s/MetaPackage", directory);
    make_directory(filename, 0777, 0, 0);

    snprintf(filename, sizeof(filename), "%s/MetaResources", directory);
    make_directory(filename, 0777, 0, 0);

    snprintf(filename, sizeof(filename), "%s/MetaResources/License.txt",
             directory);
    copy_file(filename, dist->license, 0644, 0, 0);

    snprintf(filename, sizeof(filename), "%s/MetaResources/ReadMe.txt",
             directory);
    copy_file(filename, dist->readme, 0644, 0, 0);

    snprintf(filename, sizeof(filename), "%s/MetaResources/Welcome.txt",
             directory);
    if ((fp = fopen(filename, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create welcome file \"%s\" - %s\n",
              filename, strerror(errno));
      return (1);
    }

    fprintf(fp, "%s version %s\n", dist->product, dist->version);
    fprintf(fp, "Copyright %s\n", dist->copyright);

    for (i = 0; i < dist->num_descriptions; i ++)
      if (!dist->descriptions[i].subpackage)
        fprintf(fp, "%s\n", dist->descriptions[i].description);

    fclose(fp);

    snprintf(filename, sizeof(filename), "%s/%s-metadesc.plist", directory,
             prodname);
    if ((fp = fopen(filename, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create description file \"%s\" - %s\n",
              filename, strerror(errno));
      return (1);
    }

    fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", fp);
    fputs("<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n", fp);
    fputs("<plist version=\"1.0\">\n", fp);
    fputs("<dict>\n", fp);
    fputs("        <key>IFPkgDescriptionDeleteWarning</key>\n", fp);
    fputs("        <string></string>\n", fp);
    fputs("        <key>IFPkgDescriptionDescription</key>\n", fp);
    fputs("        <string>", fp);
    for (i = 0; i < dist->num_descriptions; i ++)
      if (!dist->descriptions[i].subpackage)
        fprintf(fp, "%s\n", dist->descriptions[i].description);
    fputs("</string>\n", fp);
    fputs("        <key>IFPkgDescriptionTitle</key>\n", fp);
    fprintf(fp, "        <string>%s</string>\n", dist->product);
    fputs("        <key>IFPkgDescriptionVersion</key>\n", fp);
    fprintf(fp, "        <string>%s</string>\n", dist->version);
    fputs("</dict>\n", fp);
    fputs("</plist>\n", fp);

    fclose(fp);

   /*
    * Do the info file for the packager...
    */

    snprintf(filename, sizeof(filename), "%s/%s-metainfo.plist", directory,
             prodname);
    if ((fp = fopen(filename, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create package information file \"%s\" - %s\n",
              filename, strerror(errno));
      return (1);
    }

    fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", fp);
    fputs("<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n", fp);
    fputs("<plist version=\"1.0\">\n", fp);
    fputs("<dict>\n", fp);
    fputs("        <key>IFPkgFormatVersion</key>\n", fp);
    fputs("        <real>0.1</real>\n", fp);
    fputs("        <key>IFPkgFlagAuthorizationAction</key>\n", fp);
    fputs("        <string>RootAuthorization</string>\n", fp);
    fputs("        <key>IFPkgFlagComponentDirectory</key>\n", fp);
    fputs("        <string>../Packages</string>\n", fp);
    fputs("        <key>IFPkgFlagPackageList</key>\n", fp);
    fputs("        <array>\n", fp);
    fputs("                <dict>\n", fp);
    fputs("                        <key>IFPkgFlagPackageLocation</key>\n", fp);
    fprintf(fp, "                        <string>%s.pkg</string>\n", prodname);
    fputs("                        <key>IFPkgFlagPackageSelection</key>\n", fp);
    fputs("                        <string>selected</string>\n", fp);
    fputs("                </dict>\n", fp);
    for (i = 0; i < dist->num_subpackages; i ++)
    {
      fputs("                <dict>\n", fp);
      fputs("                        <key>IFPkgFlagPackageLocation</key>\n", fp);
      fprintf(fp, "                        <string>%s-%s.pkg</string>\n",
              prodname, dist->subpackages[i]);
      fputs("                </dict>\n", fp);
    }
    fputs("        </array>\n", fp);
    fputs("        <key>CFBundleName</key>\n", fp);
    fprintf(fp, "        <string>%s</string>\n", dist->product);
    fputs("        <key>CFBundleGetInfoString</key>\n", fp);
    fprintf(fp, "        <string>%s %s</string>\n", dist->product, dist->version);
    fputs("        <key>CFBundleShortVersionString</key>\n", fp);
    fprintf(fp, "        <string>%s</string>\n", dist->version);
    fputs("</dict>\n", fp);
    fputs("</plist>\n", fp);

    fclose(fp);

   /*
    * Build the distribution...
    */

    if (Verbosity)
      puts("Building OSX package...");

    if (directory[0] == '/')
      strlcpy(filename, directory, sizeof(filename));
    else
      snprintf(filename, sizeof(filename), "%s/%s", current, directory);

    for (i = 0; pm_paths[i]; i ++)
      if (!access(pm_paths[i], 0))
	break;

    if (pm_paths[i])
      run_command(NULL, "%s/Contents/MacOS/PackageMaker -build "
			"-p %s/%s.mpkg "
		        "-f %s/MetaPackage "
			"-r %s/MetaResources "
			"-d %s/%s-metadesc.plist "
			"-i %s/%s-metainfo.plist",
		  pm_paths[i],
		  filename, prodname,
		  filename,
		  filename,
		  filename, prodname,
		  filename, prodname);
    else
    {
      fputs("epm: Unable to find \"PackageMaker\" program!\n", stderr);
      return (1);
    }

    snprintf(filename, sizeof(filename), "%s/%s.mpkg", directory, prodname);
    if (access(filename, 0))
      return (1);

   /*
    * Remove temporary files...
    */

    if (!KeepFiles)
    {
      if (Verbosity)
	puts("Removing temporary distribution files...");

      run_command(NULL, "/bin/rm -rf %s/MetaPackage", directory);

      run_command(NULL, "/bin/rm -rf %s/MetaResources", directory);

      snprintf(filename, sizeof(filename), "%s/%s-metadesc.plist", directory,
               prodname);
      unlink(filename);

      snprintf(filename, sizeof(filename), "%s/%s-metainfo.plist", directory,
               prodname);
      unlink(filename);
    }
  }

  return (0);
}


/*
 * 'make_subpackage()' - Make a OSX subpackage.
 */

static int
make_subpackage(const char *prodname,	/* I - Product short name */
                const char *directory,	/* I - Directory for distribution files */
		dist_t     *dist,	/* I - Distribution  information */
                const char *subpackage)	/* I - Subpackage */
{
  int		i;			/* Looping var */
  FILE		*fp;			/* Spec file */
  char		prodfull[1024],		/* Full product name */
		filename[1024],		/* Destination filename */
		pkgdir[1024],		/* Package directory */
		pkgname[1024];		/* Package name */
  file_t	*file;			/* Current distribution file */
  command_t	*c;			/* Current command */
  struct passwd	*pwd;			/* Pointer to user record */
  struct group	*grp;			/* Pointer to group record */
  char		current[1024];		/* Current directory */
  const char	*option;		/* Init script option */


  if (subpackage)
    snprintf(prodfull, sizeof(prodfull), "%s-%s", prodname, subpackage);
  else
    strlcpy(prodfull, prodname, sizeof(prodfull));

  if (Verbosity)
    printf("Creating %s MacOS X package...\n", prodfull);

  getcwd(current, sizeof(current));

 /*
  * Copy the resources for the license, readme, and welcome (description)
  * stuff...
  */

  if (Verbosity)
    puts("Copying temporary resource files...");

  snprintf(filename, sizeof(filename), "%s/%s/Resources", directory, prodfull);
  make_directory(filename, 0777, 0, 0);

  snprintf(filename, sizeof(filename), "%s/%s/Resources/License.txt",
           directory, prodfull);
  copy_file(filename, dist->license, 0644, 0, 0);

  snprintf(filename, sizeof(filename), "%s/%s/Resources/ReadMe.txt",
           directory, prodfull);
  copy_file(filename, dist->readme, 0644, 0, 0);

  snprintf(filename, sizeof(filename), "%s/%s/Resources/Welcome.txt",
           directory, prodfull);
  if ((fp = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create welcome file \"%s\" - %s\n",
            filename, strerror(errno));
    return (1);
  }

  fprintf(fp, "%s version %s\n", dist->product, dist->version);
  fprintf(fp, "Copyright %s\n", dist->copyright);

  for (i = 0; i < dist->num_descriptions; i ++)
    if (dist->descriptions[i].subpackage == subpackage)
      fprintf(fp, "%s\n", dist->descriptions[i].description);

  fclose(fp);

  snprintf(filename, sizeof(filename), "%s/%s-desc.plist", directory, prodfull);
  if ((fp = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create description file \"%s\" - %s\n",
            filename, strerror(errno));
    return (1);
  }

  fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", fp);
  fputs("<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n", fp);
  fputs("<plist version=\"1.0\">\n", fp);
  fputs("<dict>\n", fp);
  fputs("        <key>IFPkgDescriptionDeleteWarning</key>\n", fp);
  fputs("        <string></string>\n", fp);
  fputs("        <key>IFPkgDescriptionDescription</key>\n", fp);
  fputs("        <string>", fp);
  for (i = 0; i < dist->num_descriptions; i ++)
    if (dist->descriptions[i].subpackage == subpackage)
      fprintf(fp, "%s\n", dist->descriptions[i].description);
  fputs("</string>\n", fp);
  fputs("        <key>IFPkgDescriptionTitle</key>\n", fp);
  fprintf(fp, "        <string>%s</string>\n", dist->product);
  fputs("        <key>IFPkgDescriptionVersion</key>\n", fp);
  fprintf(fp, "        <string>%s</string>\n", dist->version);
  fputs("</dict>\n", fp);
  fputs("</plist>\n", fp);

  fclose(fp);

 /*
  * Do the info file for the packager...
  */

  snprintf(filename, sizeof(filename), "%s/%s-info.plist", directory, prodfull);
  if ((fp = fopen(filename, "w")) == NULL)
  {
    fprintf(stderr, "epm: Unable to create package information file \"%s\" - %s\n",
            filename, strerror(errno));
    return (1);
  }

  fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", fp);
  fputs("<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n", fp);
  fputs("<plist version=\"1.0\">\n", fp);
  fputs("<dict>\n", fp);
  fputs("        <key>IFPkgFormatVersion</key>\n", fp);
  fputs("        <real>0.1</real>\n", fp);
  fputs("        <key>IFPkgFlagAuthorizationAction</key>\n", fp);
  fputs("        <string>RootAuthorization</string>\n", fp);
  if (dist->num_subpackages && !subpackage)
  {
    fputs("        <key>IFPkgFlagIsRequired</key>\n", fp);
    fputs("        <true/>\n", fp);
  }
  fputs("        <key>CFBundleName</key>\n", fp);
  fprintf(fp, "        <string>%s</string>\n", dist->product);
  fputs("        <key>CFBundleGetInfoString</key>\n", fp);
  fprintf(fp, "        <string>%s %s</string>\n", dist->product, dist->version);
  fputs("        <key>CFBundleShortVersionString</key>\n", fp);
  fprintf(fp, "        <string>%s</string>\n", dist->version);
  fputs("</dict>\n", fp);
  fputs("</plist>\n", fp);

  fclose(fp);

 /*
  * Do pre/post install commands...
  */

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->type == COMMAND_PRE_INSTALL && c->subpackage == subpackage)
      break;

  if (i)
  {
    snprintf(filename, sizeof(filename), "%s/%s/Resources/preflight",
             directory, prodfull);
    if ((fp = fopen(filename, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create preinstall script \"%s\" - %s\n",
              filename, strerror(errno));
      return (1);
    }

    fputs("#!/bin/sh\n", fp);

    for (; i > 0; i --, c ++)
      if (c->type == COMMAND_PRE_INSTALL && c->subpackage == subpackage)
	fprintf(fp, "%s\n", c->command);

    fclose(fp);
    chmod(filename, 0755);
  }

  for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
    if (c->type == COMMAND_POST_INSTALL && c->subpackage == subpackage)
      break;

  if (!i)
  {
    for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
      if (tolower(file->type) == 'i' && file->subpackage == subpackage)
        break;
  }

  if (i)
  {
    snprintf(filename, sizeof(filename), "%s/%s/Resources/postflight",
             directory, prodfull);
    if ((fp = fopen(filename, "w")) == NULL)
    {
      fprintf(stderr, "epm: Unable to create postinstall script \"%s\" - %s\n",
              filename, strerror(errno));
      return (1);
    }

    fputs("#!/bin/sh\n", fp);

    for (i = dist->num_commands, c = dist->commands; i > 0; i --, c ++)
      if (c->type == COMMAND_POST_INSTALL && c->subpackage == subpackage)
	fprintf(fp, "%s\n", c->command);

    for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
      if (tolower(file->type) == 'i' && file->subpackage == subpackage)
        qprintf(fp, "/Library/StartupItems/%s/%s start\n",
	        file->dst, file->dst);

    fclose(fp);
    chmod(filename, 0755);
  }

 /*
  * Copy the files over...
  */

  if (Verbosity)
    puts("Copying temporary distribution files...");

  for (i = dist->num_files, file = dist->files; i > 0; i --, file ++)
  {
   /*
    * Only add files in this subpackage...
    */

    if (file->subpackage != subpackage)
      continue;

   /*
    * Find the username and groupname IDs...
    */

    pwd = getpwnam(file->user);
    grp = getgrnam(file->group);

    endpwent();
    endgrent();

   /*
    * Copy the file or make the directory or make the symlink as needed...
    */

    switch (tolower(file->type))
    {
      case 'c' :
      case 'f' :
          if (strncmp(file->dst, "/etc/", 5) == 0)
            snprintf(filename, sizeof(filename), "%s/%s/Package/private%s",
	             directory, prodfull, file->dst);
          else
            snprintf(filename, sizeof(filename), "%s/%s/Package%s",
	             directory, prodfull, file->dst);

	  if (Verbosity > 1)
	    printf("%s -> %s...\n", file->src, filename);

	  if (copy_file(filename, file->src, file->mode, pwd ? pwd->pw_uid : 0,
			grp ? grp->gr_gid : 0))
	    return (1);
          break;
      case 'i' :
          snprintf(filename, sizeof(filename),
	           "%s/%s/Package/Library/StartupItems/%s/%s",
	           directory, prodfull, file->dst, file->dst);

	  if (Verbosity > 1)
	    printf("%s -> %s...\n", file->src, filename);

	  if (copy_file(filename, file->src, file->mode, pwd ? pwd->pw_uid : 0,
			grp ? grp->gr_gid : 0))
	    return (1);

          snprintf(filename, sizeof(filename),
	           "%s/%s/Package/Library/StartupItems/%s/StartupParameters.plist",
	           directory, prodfull, file->dst);
	  if ((fp = fopen(filename, "w")) == NULL)
	  {
	    fprintf(stderr, "epm: Unable to create init data file \"%s\" - %s\n",
        	    filename, strerror(errno));
	    return (1);
	  }

          fputs("{\n", fp);
          fprintf(fp, "  Description = \"%s\";\n", dist->product);
	  qprintf(fp, "  Provides = (%s);\n",
	          get_option(file, "provides", file->dst));
          if ((option = get_option(file, "requires", NULL)) != NULL)
	    qprintf(fp, "  Requires = (%s);\n", option);
          if ((option = get_option(file, "uses", NULL)) != NULL)
	    qprintf(fp, "  Uses = (%s);\n", option);
          if ((option = get_option(file, "order", NULL)) != NULL)
	    qprintf(fp, "  OrderPreference = \"%s\";\n", option);
	  fputs("}\n", fp);

	  fclose(fp);

          snprintf(filename, sizeof(filename),
	           "%s/%s/Package/Library/StartupItems/%s/Resources/English.lproj",
	           directory, prodfull, file->dst);
          make_directory(filename, 0777, 0, 0);

          snprintf(filename, sizeof(filename),
	           "%s/%s/Package/Library/StartupItems/%s/Resources/English.lproj/Localizable.strings",
	           directory, prodfull, file->dst);
	  if ((fp = fopen(filename, "w")) == NULL)
	  {
	    fprintf(stderr, "epm: Unable to create init strings file \"%s\" - %s\n",
        	    filename, strerror(errno));
	    return (1);
	  }

	  fputs("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", fp);
	  fputs("<!DOCTYPE plist SYSTEM \"file://localhost/System/Library/DTDs/PropertyList.dtd\">\n", fp);
	  fputs("<plist version=\"0.9\">\n", fp);
	  fputs("<dict>\n", fp);
	  fprintf(fp, "        <key>Starting %s</key>\n", dist->product);
	  fprintf(fp, "        <string>Starting %s</string>\n", dist->product);
	  fputs("</dict>\n", fp);
	  fputs("</plist>\n", fp);

	  fclose(fp);
          break;
      case 'd' :
          if (strncmp(file->dst, "/etc/", 5) == 0 ||
	      strcmp(file->dst, "/etc") == 0)
            snprintf(filename, sizeof(filename), "%s/%s/Package/private%s",
	             directory, prodfull, file->dst);
          else
            snprintf(filename, sizeof(filename), "%s/%s/Package%s",
	             directory, prodfull, file->dst);

	  if (Verbosity > 1)
	    printf("Directory %s...\n", filename);

          make_directory(filename, file->mode, pwd ? pwd->pw_uid : 0,
			 grp ? grp->gr_gid : 0);
          break;
      case 'l' :
          if (strncmp(file->dst, "/etc/", 5) == 0)
            snprintf(filename, sizeof(filename), "%s/%s/Package/private%s",
	             directory, prodfull, file->dst);
          else
            snprintf(filename, sizeof(filename), "%s/%s/Package%s",
	             directory, prodfull, file->dst);

	  if (Verbosity > 1)
	    printf("%s -> %s...\n", file->src, filename);

          make_link(filename, file->src);
          break;
    }
  }

 /*
  * Build the distribution...
  */

  if (Verbosity)
    puts("Building OSX package...");

  if (directory[0] == '/')
    strlcpy(filename, directory, sizeof(filename));
  else
    snprintf(filename, sizeof(filename), "%s/%s", current, directory);

  if (dist->num_subpackages)
  {
   /*
    * Place the individual packages in the "Packages" subdirectory;
    * a metapackage will handle the whole shebang...
    */

    snprintf(pkgdir, sizeof(pkgdir), "%s/Packages", filename);
    make_directory(pkgdir, 0777, 0, 0);

    snprintf(pkgname, sizeof(pkgname), "%s/Packages/%s.pkg", filename,
             prodfull);
  }
  else
  {
   /*
    * The package stands alone - just put it in the output directory...
    */

    snprintf(pkgname, sizeof(pkgname), "%s/%s.pkg", filename, prodfull);
  }

  for (i = 0; pm_paths[i]; i ++)
    if (!access(pm_paths[i], 0))
      break;

  if (pm_paths[i])
    run_command(NULL, "%s/Contents/MacOS/PackageMaker -build "
		      "-p %s "
		      "-f %s/%s/Package "
		      "-r %s/%s/Resources "
		      "-d %s/%s-desc.plist "
		      "-i %s/%s-info.plist",
		pm_paths[i],
		pkgname,
		filename, prodfull,
		filename, prodfull,
		filename, prodfull,
		filename, prodfull);
  else
  {
    fputs("epm: Unable to find \"PackageMaker\" program!\n", stderr);
    return (1);
  }

 /*
  * Verify that the package was created...
  */

  if (access(pkgname, 0))
    return (1);

 /*
  * Remove temporary files...
  */

  if (!KeepFiles)
  {
    if (Verbosity)
      puts("Removing temporary distribution files...");

    run_command(NULL, "/bin/rm -rf %s/%s", directory, prodfull);

    snprintf(filename, sizeof(filename), "%s/%s-desc.plist", directory,
             prodfull);
    unlink(filename);

    snprintf(filename, sizeof(filename), "%s/%s-info.plist", directory,
             prodfull);
    unlink(filename);
  }

  return (0);
}


/*
 * End of "$Id: osx.c,v 1.13 2005/01/11 21:20:17 mike Exp $".
 */
