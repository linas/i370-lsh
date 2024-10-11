


/* This is copyrighted software but you may use/distribute it     *
 * in accordance with the conditions set out in the file COPYING  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <utmp.h>
#include <sys/stat.h>
#include "defs.h"

/* print the prompt */

int 
printprompt (int drive, char *promtstr, FILE * stream)
{
  int j, i, esc;
  struct tm *tstr;
  time_t tt;
  char day[4];
  char *tmp;
  int retval = 0;
  FILE *fnull;
  struct stat fs;

  i = 0;
  esc = 0;

  while (promtstr[i] != '\0')
    {

#ifdef DEBUG
      fprintf (stderr, "printprompt : looping with string[%d]=%c\n", i, promtstr[i]);
#endif

      if (esc)
	{
	  esc = 0;
	  switch (promtstr[i])
	    {
	    case 'W':
	    case 'w':
	      if (!stat (UTMP_FILE, &fs))
		{
		  if (wtmpy < fs.st_mtime)
		    {
		      fputc ('*', stream);
		      wtmpy = fs.st_mtime;
		    }
		}
	      break;
	    case 'A':
	    case 'a':
	      fputc ('\a', stream);
	      break;
	    case 'R':
	      if (nomap)
		j = drivetab[drive].slide;
	      else
		j = 0;
	      fprintf (stream, "%s", drivetab[drive].path + j);
	      break;
	    case 'r':
	      fprintf (stream, "%s", drivetab[drive].path + drivetab[drive].slide);
	      break;
	    case '{':
	      j = i + 1;
	      while (!(promtstr[i] == '}' || iscntrl (promtstr[i])))
		i++;
	      if (promtstr[i] == '\0')
		promtstr[i + 1] = '\0';
	      promtstr[i] = '\0';
#ifdef DEBUG
	      fprintf (stderr, "Going to run <%s>\n", promtstr + j);
#endif
	      lex (promtstr + j, 0);
	      running = 0;
	      fnull = fopen ("/dev/null", "r");
	      stmts (0, tokencount, tokenstream, fnull, stream);
	      fclose (fnull);
	      running = 1;
	      promtstr[i] = '}';

	      break;
	    case '[':
	      j = i + 1;
	      while (!(promtstr[i] == ']' || iscntrl (promtstr[i])))
		i++;
	      if (promtstr[i] == '\0')
		promtstr[i + 1] = '\0';
	      promtstr[i] = '\0';
#ifdef DEBUG
	      fprintf (stderr, "Looking for value of <%s>\n", promtstr + j);
#endif
	      fprintf (stream, "%s", getenv (promtstr + j));
	      promtstr[i] = ']';
	      break;
	    case 'V':
	    case 'v':
	      fprintf (stream, LVERSION);
	      break;
	    case 'T':
	    case 't':
	      time (&tt);
	      tstr = localtime (&tt);
	      fprintf (stream, "%02d:%02d:%02d.00", tstr->tm_hour, tstr->tm_min, tstr->tm_sec);
	      break;
	    case 'D':
	    case 'd':
	      time (&tt);
	      tstr = localtime (&tt);
	      switch (tstr->tm_wday)
		{
		case 0:
		  strcpy (day, "Sun");
		  break;
		case 1:
		  strcpy (day, "Mon");
		  break;
		case 2:
		  strcpy (day, "Tue");
		  break;
		case 3:
		  strcpy (day, "Wed");
		  break;
		case 4:
		  strcpy (day, "Thu");
		  break;
		case 5:
		  strcpy (day, "Fri");
		  break;
		case 6:
		  strcpy (day, "Sat");
		  break;
		}
	      if (tstr->tm_year < 100)
		fprintf (stream, "%s %02d-%02d-19%02d", day, (tstr->tm_mon) + 1, tstr->tm_mday, tstr->tm_year);
	      else
		{
		  fprintf (stream, "%s %02d-%02d-20%02d", day, (tstr->tm_mon) + 1, tstr->tm_mday, tstr->tm_year);
		}
	      break;
	    case 'P':
	    case 'p':
	      tmp = (char *) malloc (CEM ((strlen (drivetab[drive].path) + 4), 16) * sizeof (char));
	      if (tmp == NULL)
		{
		  retval = 1;
#ifdef DEBUG
		  fprintf (stderr, "In printprompt() : malloc failed\n");
#endif
		}
	      else
		{
		  if (getcwd (tmp, (strlen (drivetab[drive].path) + 4) * sizeof (char)) != NULL)
		    {
		      if (!tolo (drive, tmp, 0))
			{
			  fprintf (stream, "%s", tmp);
			  /* was a bit too keen here - even los does not do */
			  /* if(tmp[strlen(tmp)-1]!='\\')fputc('\\',stream); */
			}
		    }
		  free (tmp);
		}
	      break;
	    case 'm':
	      if ((mistr != NULL) && (!stat (mistr, &fs)) && (maily < fs.st_mtime))
		{
		  fputc ('*', stream);
		  maily = fs.st_mtime;
		}
	      break;
/* I am sorry, but I simply could not resist */
	    case 'M':
	      switch ((doodle + i) % 4)
		{
		case 0:
		  fputc ('-', stream);
		  break;
		case 1:
		  if (i % 3)
		    fputc ('/', stream);
		  else
		    fputc ('\\', stream);
		  break;
		case 2:
		  fputc ('|', stream);
		  break;
		default:
		  if (i % 3)
		    fputc ('\\', stream);
		  else
		    fputc ('/', stream);
		  break;
		}
	      break;
	    case 'N':
	      fputc ('A' + drive, stream);
	      break;
	    case 'n':
	      fputc ('a' + drive, stream);
	      break;
	    case 'E':
	    case 'e':
	      fputc (27, stream);
	      break;
	    case '_':
	      fputc ('\n', stream);
	      break;
	    case 'B':
	    case 'b':
	      fputc ('|', stream);
	      break;
	    case 'L':
	    case 'l':
	      fputc ('<', stream);
	      break;
	    case 'C':
	    case 'c':
	      fputc (';', stream);
	      break;
	    case '$':
	      fputc ('$', stream);
	      break;
	    case 'Q':
	    case 'q':
	      fputc ('=', stream);
	      break;
	    case 'G':
	    case 'g':
	      fputc ('>', stream);
	      break;
	    case 'I':
	    case 'i':
	      fprintf (stream, "%d", getpid ());
	      break;
	    case 'X':
#ifdef USE_TERMCAP
	      if (tsstr != NULL)
		{
		  fputs (tsstr, stream);
		  /*fputs(hostr,stream); */
		}
#endif
	      break;
	    case 'x':
#ifdef USE_TERMCAP
	      if (fsstr != NULL)
		{
		  fputs (fsstr, stream);
		}
#endif
	      break;
	    case 'S':
#ifdef USE_TERMCAP
#ifdef DEBUG
	      fprintf (stream, "+");
#endif
	      if (sestr != NULL)
		{
#ifdef DEBUG
		  fprintf (stream, "+");
#endif
		  fputs (sestr, stream);
		}
#else
#endif
	      break;
	    case 's':
#ifdef USE_TERMCAP
#ifdef DEBUG
	      fprintf (stream, "+");
#endif
	      if (sostr != NULL)
		{
#ifdef DEBUG
		  fprintf (stream, "+");
#endif
		  fputs (sostr, stream);
		}
#else
#endif
	      break;
	    case 'U':
	    case 'u':
	      fprintf (stream, "%s", getlogin ());
	      break;
	    case 'H':
	    case 'h':
	      fputc ('\b', stream);
	      break;
	    case 'O':
	    case 'o':
	      fprintf (stream, "%d", quitstat);
	      break;
	    case 'Z':
	    case 'z':
	      fprintf (stream, "%d", doodle);
	      break;
	    default:
	      if (promtstr[i] != '\0')
		fputc ((int) promtstr[i], stream);
	      else
		fputc ((int) '$', stream);
	      break;
	    }
	}
      else
	{
	  if (promtstr[i] == '$')
	    esc = 1;
	  else
	    fputc ((int) promtstr[i], stream);
	}
      i++;
    }
  doodle++;
  return retval;
}
