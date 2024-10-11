/* This is copyrighted software but you may use/distribute it     *
 * in accordance with the conditions set out in the file COPYING  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "defs.h"

int 
pathinit ()
{
  int i;

  for (i = 0; i < DRIVES; i++)
    {
      drivetab[i].path = NULL;
      drivetab[i].plen = 0;
    }

  return 0;
}

/* receives unix path and drive - returns 0 if ok */

int 
map (int drive, char *str)
{
  int retval = 0;

/* before we reallocate we might as well zap the old stuff */

  if (drivetab[drive].plen)
    {
      free (drivetab[drive].path);
      drivetab[drive].path = NULL;
      drivetab[drive].plen = 0;

#ifdef DEBUG
      fprintf (stderr, "In map() : Cleaning up before reallocating \n");
#endif

    }

/* have some margin for cds */

  drivetab[drive].plen = CEM (strlen (str), 16) + 32;
  drivetab[drive].path = (char *) malloc (drivetab[drive].plen * sizeof (char));

  if (drivetab[drive].path != NULL)
    {
      strcpy (drivetab[drive].path, str);
      drivetab[drive].slide = strlen (str);
      drivetab[drive].valid = 1;
      if (drivetab[drive].path[drivetab[drive].slide - 1] != '/')
	{
	  drivetab[drive].path[drivetab[drive].slide] = '/';
	  drivetab[drive].path[drivetab[drive].slide + 1] = '\0';
	  drivetab[drive].slide++;

#ifdef DEBUG
	  fprintf (stderr, "In map() : Appended a trailing / \n");
#endif

	}
/* if nothing else has been allocated, then make this the default drive */
      if (drivecur == (-1))
	{
	  drivecur = drive;
	  if (chdir (str))
	    retval = 1;
	}
    }
  else
    {
      drivetab[drive].valid = 0;
      drivetab[drive].plen = 0;
#ifdef DEBUG
      fprintf (stderr, "Memory allocation failed in map()\n");
#endif
      retval = 1;
    }

  return retval;
}

/* receives drive  -  returns 0 if ok */

int 
zap (int drive)
{
  drivetab[drive % DRIVES].valid = 0;

  if (drivetab[drive % DRIVES].plen)
    {
      free (drivetab[drive % DRIVES].path);
      drivetab[drive % DRIVES].plen = 0;
#ifdef DEBUG
      fprintf (stderr, "Zapped RAM in drive %c:\n", 'A' + drive);
#endif
    }

  return 0;
}

/* receives unix path and returns 0 if ok */

int 
setlo (int drive, char *str)
{
  int retval = 0;
  char *tptr;

#ifdef DEBUG
  fprintf (stderr, "Setlo received <%s>\n", str);
#endif

  if (drivetab[drive].valid)
    {
      if ((!(strncmp (drivetab[drive].path, str, drivetab[drive].slide))) ||
	  (
	(!strncmp (drivetab[drive].path, str, drivetab[drive].slide - 1)) &&
	    (strlen (str) < drivetab[drive].slide) &&
	    (drivetab[drive].slide - 1)
	  ))
	{
	  if (strlen (str) > drivetab[drive].plen)
	    {
	      if ((tptr = realloc (drivetab[drive].path, (CEM (strlen (str), 16) + 16) * sizeof (char))) == NULL)
		{
		  return 1;
#ifdef DEBUG
		  fprintf (stderr, "realloc failed in setlo()\n");
#endif
		}
	      else
		{
		  drivetab[drive].plen = strlen (str) + strlen (str) % 16;
		  drivetab[drive].path = tptr;
#ifdef DEBUG
		  fprintf (stderr, "realloc ok in setlo()\n");
#endif
		}
	    }
	  strcpy (drivetab[drive].path, str);
	  if (drivetab[drive].path[strlen (str) - 1] != '/')
	    {
#ifdef DEBUG
	      fprintf (stderr, "In setlo() : last char is <%c>\n", drivetab[drive].path[strlen (str) - 1]);
	      fprintf (stderr, "In setlo() : appending trailing / to path %s\n", drivetab[drive].path);
#endif
	      strcat (drivetab[drive].path, "/");
	    }
	}
      else
	{
	  retval = 1;
#ifdef DEBUG
	  fprintf (stderr, "In setlo() : paths did not match\n");
#endif
	}
    }
  else
    {
      retval = 1;

#ifdef DEBUG
      fprintf (stderr, "In setlo() : attempting to use invalid drive\n");
#endif

    }

#ifdef DEBUG
  fprintf (stderr, "Setlo returned <%s>\n", str);
#endif

  return retval;
}

/* construct a unix path from drivetab path */

int 
fromlo (char *str, int len)
{
  int retval = 0;
  int drive;
  int i = 0;
  int j;
  struct stat fst;

  char *tptr;

  drive = drivecur;

#ifdef DEBUG
  fprintf (stderr, "Fromlo receives <%s>\n", str);
#endif

/* a x: is the only difference between unix an los */

  if (str[1] == ':')
    {
      drive = toupper (str[0]) - 'A';
      if (drivetab[drive].valid)
	{
	  if (str[2] == '\\')
	    {
	      if ((drivetab[drive].slide + strlen (str)) < len)
		{
		  if (drivetab[drive].slide == 1)
		    {
		      for (i = 0; i < strlen (str); i++)
			{
			  str[i] = str[i + 2];
			}
		      retval = 0;
		    }
		  else
		    {
		      j = drivetab[drive].slide - 3;
		      for (i = strlen (str); i >= 0; i--)
			{
			  str[i + j] = str[i];
			}
		      strncpy (str, drivetab[drive].path, j + 2);
		      retval = 0;
		    }
		}
	      else
		{
		  retval = 2;
#ifdef DEBUG
		  fprintf (stderr, "In fromlo : buffer passed too small (%d bytes)\n", len);
#endif
		}
	    }


	  else
	    {
	      if ((strlen (drivetab[drive].path) + strlen (str)) < len)
		{
		  if (drivetab[drive].path[1] == '\0')
		    {
		      for (i = 0; i < strlen (str); i++)
			{
			  str[i + 1] = str[i + 2];
			}
		      str[0] = '/';
		    }
		  else
		    {
		      j = strlen (drivetab[drive].path) - 2;
		      for (i = strlen (str); i >= 0; i--)
			{
			  str[i + j] = str[i];
			}
		      strncpy (str, drivetab[drive].path, strlen (drivetab[drive].path));
		    }
		}
	      else
		{
		  retval = 1;
#ifdef DEBUG
		  fprintf (stderr, "In fromlo : buffer passed too small [%d bytes]\n", len);
#endif
		}
	    }
	}
      else
	{
	  retval = 1;
#ifdef DEBUG
	  fprintf (stderr, "In fromlo : path not valid\n");
#endif
	}
    }
  else
    {
      if (str[0] == '\\')
	{
	  if (drivetab[drivecur].valid)
	    {
	      if ((strlen (str) + drivetab[drivecur].slide) >= len)
		{
#ifdef DEBUG
		  fprintf (stderr, "In fromlo : buffer passed from calling function too small\n");
#endif
		  retval = 1;
		}
	      else
		{
#ifdef DEBUG
		  fprintf (stderr, "In fromlo : about to move things back\n");
#endif
		  for (i = strlen (str); i >= 0; i--)
		    {
		      str[drivetab[drivecur].slide - 1 + i] = str[i];
		    }
#ifdef DEBUG
		  fprintf (stderr, "In fromlo : about to copy over\n");
#endif
		  strncpy (str, drivetab[drivecur].path, drivetab[drivecur].slide - 1);
		}
	    }
	  else
	    {
	      retval = 1;
	    }
	}
    }

/* clean up slashes */
  i = 0;
  while (!((isspace (str[i])) || (iscntrl (str[i]))))
    {
      if (str[i] == '\\')
	str[i] = '/';
      i++;
    }
  str[i] = '\0';


  if (!retval && (drivetab[drive].slide > 1))
    {
      i = strlen (str);
      j = stat (str, &fst);
#ifdef DEBUG
      fprintf (stderr, "In fromlo : <%s> canditate for a check\n", str);
#endif

      if ((!(fst.st_mode & S_IFDIR)) || j)
	{
	  while (i && str[i] != '/')
	    i--;
#ifdef DEBUG
	  fprintf (stderr, "In fromlo : <%s> is not a dir -- it has to be part of one\n", str);
#endif
	}
#ifdef DEBUG
      else
	fprintf (stderr, "In fromlo : <%s> is a dir\n", str);
#endif
      if (i)
	{
#ifdef DEBUG
	  fprintf (stderr, "In fromlo() : we are not in the root directory\n");
#endif
	  j = CEM (strlen (str) + strlen (drivetab[drive].path), 16);
#ifdef DEBUG
	  fprintf (stderr, "In fromlo() : we are allocating %d bytes\n", j);
#endif
	  tptr = (char *) malloc (j * sizeof (char));
	  if (tptr != NULL)
	    {
	      strncpy (tptr, str, i);
	      tptr[i] = '\0';
#ifdef DEBUG
	      fprintf (stderr, "In fromlo() : going to change to <%s>\n", tptr);
#endif
	      if (chdir (tptr))
		{
#ifdef DEBUG
		  fprintf (stderr, "In fromlo() : chdir <%s> failed\n", tptr);
#endif
		  retval = 1;
		}
	      else
		{
		  if (NULL != getcwd (tptr, j))
		    {
		      if (strncmp (tptr, drivetab[drive].path, (drivetab[drive].slide - 1)) ||
			  (tptr[drivetab[drive].slide - 1] != '\0' && tptr[drivetab[drive].slide - 1] != '/')
			)
			{
			  retval = 1;
#ifdef DEBUG
			  fprintf (stderr, "Request failed since <%s> does not match <%s> in first %d locations\n", tptr, drivetab[drive].path, drivetab[drive].slide - 1);
#endif
			}
#ifdef DEBUG
		      else
			{
			  fprintf (stderr, "In fromlo() : Request ok <%s> matches <%s> in first %d places\n", tptr, drivetab[drive].path, drivetab[drive].slide - 1);
			}
#endif
		    }
		  else
		    {
		      retval = 1;
#ifdef DEBUG
		      perror ("In fromlo() : could not aquire cwd");
#endif
		    }
		  chdir (drivetab[drivecur].path);
		}
	      free (tptr);
	    }
	}
    }

#ifdef DEBUG
  fprintf (stderr, "Fromlo() returns %s\n", str);
#endif

  return retval;
}

/* constuct a los path from unix path */

/* untested */

int 
tolo (int drive, char *str, int len)
{
  int retval = 1;
  int i;

#ifdef DEBUG
  fprintf (stderr, "Tolo() received <%s>\n", str);
#endif

  if (drivetab[drive].valid)
    {
      if ((!strncmp (drivetab[drive].path, str, drivetab[drive].slide)) ||
	  (!strncmp (drivetab[drive].path, str, drivetab[drive].slide - 1) &&
	   (strlen (drivetab[drive].path) >= (strlen (str))) &&
	   (drivetab[drive].slide - 1)))
	{
	  if (drivetab[drive].slide > 2)
	    {
#ifdef DEBUG
	      fprintf (stderr, "In the flaky bit\n");
#endif
	      str[0] = 'A' + drive;
	      str[1] = ':';
	      str[2] = '/';
	      if (drivetab[drive].slide < strlen (str))
		{
		  for (i = drivetab[drive].slide; str[i] != '\0'; i++)
		    {
		      str[3 + i - drivetab[drive].slide] = str[i];
		    }
		  str[3 + i - drivetab[drive].slide] = '\0';
		}
	      else
		{
		  str[3] = '\0';
		}
	    }
	  else
	    {
	      for (i = strlen (str); i >= 0; i--)
		{
		  str[i + 2] = str[i];
		}
	      str[0] = 'A' + drive;
	      str[1] = ':';
	      str[2] = '/';
	    }
	  retval = 0;
	}
#ifdef DEBUG
      else
	{
	  fprintf (stderr, "In tolo() : input string %s did not match drive base\n", str);
	}
#endif
    }

  /* remember to convert / to \ down here */

  for (i = 0; i < strlen (str); i++)
    {
      if (str[i] == '/')
	{
	  str[i] = '\\';
	}
      else
	{
	  str[i] = toupper (str[i]);
	}
    }

#ifdef DEBUG
  fprintf (stderr, "Tolo() returned <%s>\n", str);
#endif

  return retval;
}

/* free up the ram */

int 
pathexit ()
{
  int i;
  for (i = 0; i < DRIVES; i++)
    {
      zap (i);
    }
  return 0;
}

/* diagnostics */

int 
pathdump (FILE * out)
{
  int i;
  for (i = 0; i < 26; i++)
    {
      if (drivetab[i].valid)
	{
	  fprintf (out, "%c:=%s\n", i + 'A', drivetab[i].path);
	}
    }
  return 0;
}


#ifdef OODEBUG
int 
main ()
{
  char t[70], r[70], s[70];
  pathinit ();
  while (t[0] != 'q')
    {

      scanf ("%s", t);
      if (fromlo (t, 70))
	{
	  fprintf (stderr, "In main() : Conversion by fromlo() failed \n");
	}
      getcwd (r, 70);
      if (chdir (t))
	{
	  fprintf (stderr, "In main() : chdir() failed with request %s\n", t);
	}
      getcwd (s, 70);
      if (setlo (2, s))
	{
	  fprintf (stderr, "In main() : Setlo failed with s=%s\n", s);
	  chdir (r);
	}
      pathdump (stderr);
      if (tolo (2, s, 70))
	{
	  fprintf (stderr, "In main() : tolo() failed\n");
	}
      printf ("%s>", s);
    }
  pathdump (stderr);
  return 0;
}
#endif
