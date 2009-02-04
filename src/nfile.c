/*
 * See Licensing and Copyright notice in naev.h
 */

/**
 * @file nfile.c
 *
 * @brief Handles read/write abstractions to the users directory.
 *
 * @todo add support for windows and mac os.
 */


#include "nfile.h"

#include "ncompat.h"

#include <stdio.h> 
#include <string.h>
#include <stdarg.h>
#if HAS_POSIX
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h> 
#include <errno.h>
#endif /* HAS_POSIX */

#include "naev.h"
#include "log.h"



#define BLOCK_SIZE      128*1024 /**< 128 kilobytes. */



static char naev_base[PATH_MAX] = "\0"; /**< Stores naev's base path. */
/**
 * @brief Gets naev's base path (for saves and such).
 *
 *    @return The base path to naev.
 */
char* nfile_basePath (void)
{
   char *home;

   if (naev_base[0] == '\0') {
#if HAS_UNIX
      home = getenv("HOME");
      snprintf(naev_base,PATH_MAX,"%s/.naev/",home);
#else /* HAS_UNIX */
#error "Feature needs implementation on this Operating System for NAEV to work."
#endif /* HAS_UNIX */
   }
   
   return naev_base;
}


/**
 * @brief Creates a directory if it doesn't exist.
 *
 * Uses relative paths to basePath.
 *
 *    @param path Path to create directory if it doesn't exist.
 *    @return 0 on success.
 */
int nfile_dirMakeExist( const char* path, ... )
{
   char file[PATH_MAX];
   va_list ap;

   if (path == NULL)
      return -1;
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(file, PATH_MAX, path, ap);
      va_end(ap);
   }

#if HAS_POSIX
   struct stat buf;

   stat(file,&buf);
   if (!S_ISDIR(buf.st_mode))
      if (mkdir(file, S_IRWXU | S_IRWXG | S_IRWXO) < 0) {
         WARN("Dir '%s' does not exist and unable to create", file);
         return -1;
      }
#else /* HAS_POSIX */
#error "Feature needs implementation on this Operating System for NAEV to work."
#endif /* HAS_POSIX */

   return 0;
}


/**
 * @brief Checks to see if a file exists.
 *
 *    @param path printf formatted string pointing to the file to check for existance.
 *    @return 1 if file exists, 0 if it doesn't or -1 on error.
 */
int nfile_fileExists( const char* path, ... )
{
   char file[PATH_MAX];
   va_list ap;

   if (path == NULL)
      return -1;
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(file, PATH_MAX, path, ap);
      va_end(ap);
   }
#if HAS_POSIX
   struct stat buf;

   if (stat(file,&buf)==0) /* stat worked, file must exist */
      return 1;

#else /* HAS_POSIX */
   FILE *f;

   /* Try to open the file, C89 compliant, but not as precise as stat. */
   f = fopen(file, 'r');
   if (f != NULL) {
      fclose(f);
      return 1;
   }
#endif /* HAS_POSIX */
   return 0;
}


/**
 * @brief Lists all the visible files in a directory.
 *
 * Should also sort by last modified but that's up to the OS in question.
 * Paths are relative to base directory.
 *
 *    @param[out] nfiles Returns how many files there are.
 *    @param path Directory to read.
 */
char** nfile_readDir( int* nfiles, const char* path, ... )
{
   char file[PATH_MAX], base[PATH_MAX];
   char **files;
   va_list ap;

   if (path == NULL) {
      *nfiles = 0;
      return NULL;
   }
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(base, PATH_MAX, path, ap);
      va_end(ap);
   }

#if HAS_POSIX
   int i,j,k, n;
   DIR *d;
   struct dirent *dir;
   char *name;
   int mfiles;
   struct stat sb;
   time_t *tt, *ft;
   char **tfiles;

   (*nfiles) = 0;
   mfiles = 128;
   tfiles = malloc(sizeof(char*)*mfiles);
   tt = malloc(sizeof(time_t)*mfiles);

   d = opendir(base);
   if (d == NULL)
      return NULL;

   /* Get the file list */
   while ((dir = readdir(d)) != NULL) {
      name = dir->d_name;

      /* Skip hidden directories */
      if (name[0] == '.')
         continue;

      /* Stat the file */
      snprintf( file, PATH_MAX, "%s/%s", base, name );
      if (stat(file, &sb) == -1)
         continue; /* Unable to stat */

      /* Enough memory? */
      if ((*nfiles)+1 > mfiles) {
         mfiles += 128;
         tfiles = realloc( tfiles, sizeof(char*) * mfiles );
         tt = realloc( tt, sizeof(time_t) * mfiles );
      }

      /* Write the information */
      tfiles[(*nfiles)] = strdup(name);
      tt[(*nfiles)] = sb.st_mtime;
      (*nfiles)++;
   }

   closedir(d);

   /* Sort by last changed date */
   if ((*nfiles) > 0) {

      /* Need to allocate some stuff */
      files = malloc( sizeof(char*) * (*nfiles) );
      ft = malloc( sizeof(time_t) * (*nfiles) );

      /* Fill the list */
      for (i=0; i<(*nfiles); i++) {
         n = -1;

         /* Get next lowest */
         for (j=0; j<(*nfiles); j++) {

            /* Is lower? */
            if ((n == -1) || (tt[j] > tt[n])) {

               /* Check if it's already there */
               for (k=0; k<i; k++)
                  if (strcmp(files[k],tfiles[j])==0)
                     break;

               /* New lowest */
               if (k>=i)
                  n = j;
            }
         }

         files[i] = tfiles[n];
         ft[i] = tt[n];
      }
      free(ft);
   }
   else
      files = NULL;

   /* Free temporary stuff */
   free(tfiles);
   free(tt);

#else /* HAS_POSIX */
#error "Feature needs implementation on this Operating System for NAEV to work."
#endif /* HAS_POSIX */

   /* found nothing */
   if ((*nfiles) == 0) {
      free(files);
      files = NULL;
   }

   return files;
}


/**
 * @brief Tries to read a file.
 *
 *    @param filesize Stores the size of the file.
 *    @param path Path of the file.
 *    @return The file data.
 */
char* nfile_readFile( int* filesize, const char* path, ... )
{
   char base[PATH_MAX];
   char *buf;
   FILE *file;
   int len, pos;
   va_list ap;

   if (path == NULL) {
      *filesize = 0;
      return NULL;
   }
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(base, PATH_MAX, path, ap);
      va_end(ap);
   }

   /* Open file. */
   file = fopen( base, "r" );
   if (file == NULL)
      return NULL;

   /* Get file size. */
   len = fseek( file, 0, SEEK_END );
   if (len == -1) {
      fclose(file);
      return NULL;
   }
   len = ftell(file);
   fseek( file, 0, SEEK_SET );

   /* Allocate buffer. */
   buf = malloc( len );
   if (buf == NULL) {
      WARN("Out of Memory!");
      fclose(file);
      return NULL;
   }

   /* Read the file. */
   pos = fread( buf, len, 1, file );
   if (pos != 1)
      WARN("Error occurred while reading '%s'.", base);

   *filesize = len;
   return buf;
}


/**
 * @brief Tries to create the file if it doesn't exist.
 *
 *    @param path Path of the file to create.
 */
int nfile_touch( const char* path, ... )
{
   char file[PATH_MAX];
   va_list ap;
   FILE *f;

   if (path == NULL)
      return -1;
   else { /* get the message */
      va_start(ap, path);
      vsnprintf(file, PATH_MAX, path, ap);
      va_end(ap);
   }

   /* Try to open the file, C89 compliant, but not as precise as stat. */
   f = fopen(file, "a+");
   if (f == NULL) {
      WARN("Unable to touch file '%s': %s", file, strerror(errno));
      return -1;
   }

   fclose(f);
   return 0;
}

