/* This is copyrighted software but you may use/distribute it     *
 * in accordance with the conditions set out in the file COPYING  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <ctype.h>
#include "defs.h"

struct termios saved_attributes;

int
mresetterm ()
{

  if (isatty (STDIN_FILENO))
    {
      tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
    }

  return 0;
}

int
msetterm ()
{
  struct termios tattr;

  if (isatty (STDIN_FILENO))
    {

      /* Save terminal attributes */
      tcgetattr (STDIN_FILENO, &saved_attributes);

      /* Set the terminal modes. */
      tcgetattr (STDIN_FILENO, &tattr);
      tattr.c_lflag &= ~(ICANON | ECHO);	/* Clear ICANON and ECHO. */

      tattr.c_cc[VMIN] = 1;
      tattr.c_cc[VTIME] = 0;

/*  tcsetattr(STDIN_FILENO,TCSAFLUSH,&tattr); */

      tcsetattr (STDIN_FILENO, TCSANOW, &tattr);
    }
  return 0;
}

int 
readc (FILE * in)
{
  int retval;
  if (isatty (fileno (in)))
    {
      fflush (stdout);
      msetterm ();
      retval = fgetc (in);
      mresetterm ();
      fflush (stdout);
    }
  else
    {
      retval = fgetc (in);
    }
  return retval;
}

#ifdef USE_TERMCAP

int 
tailmatch (char *cntrl, int cntrlen, char *inseq, int len)
{
  int result;
  /* should never happen */
  if (len > cntrlen || cntrl == NULL)
    {
      result = 2;
    }
  /* normal case */
  else
    {
      /* if there is a difference between strings bomb */
      if (strncmp (cntrl, inseq, len))
	{
	  result = 2;
	}
      else
	{
	  /* everything matches */
	  if (len == cntrlen)
	    {
	      result = 0;
	    }
	  /* everything might match some time later */
	  else
	    {
	      result = 1;
	    }
	}
    }
  return result;
}

/* readl() receives buf which it should fill with no
 * more that len characters. The calling function will throw
 * away anything after a control character, but it is a good
 * idea to terminate the buffer with \n\0 anyway.
 */

int 
readl (char *buf, int len)
{
  int retval = 0;

  int offset = 0;
  int base = 0;
  int dispindex = 0;

  static previous = 0;

  char quit = 0;
  char tbuf[2 * INBUF];
  char dispbuf[INBUF];

  int k1strl = 0;
  int k3strl = 0;
  int kustrl = 0;
  int krstrl = 0;
  int kdstrl = 0;
  int klstrl = 0;

  int rval;
  int nomatch;

#ifdef DEBUG
  fprintf (stderr, "Starting readl\n");
#endif

  if (isatty (STDIN_FILENO))
    {

      if (k1str != NULL)
	k1strl = strlen (k1str);
      if (k3str != NULL)
	k3strl = strlen (k3str);
      if (kustr != NULL)
	kustrl = strlen (kustr);
      if (krstr != NULL)
	krstrl = strlen (krstr);
      if (klstr != NULL)
	klstrl = strlen (klstr);
      if (kdstr != NULL)
	kdstrl = strlen (kdstr);

      fflush (stdout);
      msetterm ();

      while (!quit)
	{
	  read (STDIN_FILENO, &tbuf[offset + base], 1);

#ifdef DEBUG
	  fprintf (stderr, "Base=<%d>,offset=<%d>,char=<%c>\n", base, offset, tbuf[base + offset]);
#endif

	  offset++;

	  nomatch = 1;
	  while (nomatch && offset)
	    {

	      if ((rval = tailmatch (k1str, k1strl, tbuf + base, offset)) == 0)
		{
#ifdef DEBUG
		  fprintf (stderr, "Found [F1] with offset=<%d>\n", offset);
#endif
		  offset = 0;
		  nomatch = 0;
		  if (dispindex < previous && (buf[dispindex] != '\n'))
		    {
		      putchar (buf[dispindex]);
		      dispbuf[dispindex] = buf[dispindex];
		      dispindex++;
		    }
		}
	      else
		{
		  if (rval == 1)
		    nomatch = 0;
		  if ((rval = tailmatch (k3str, k3strl, tbuf + base, offset)) == 0)
		    {
#ifdef DEBUG
		      fprintf (stderr, "Found [F3] with offset=<%d>\n", offset);
#endif
		      offset = 0;
		      nomatch = 0;
		      while (dispindex)
			{
			  putchar ('\b');
			  putchar (' ');
			  putchar ('\b');
			  dispindex--;
			}
		      while (buf[dispindex] != '\0' && buf[dispindex] != '\n')
			{
			  putchar (buf[dispindex]);
			  dispbuf[dispindex] = buf[dispindex];
			  dispindex++;
			}
		    }
		  else
		    {
		      if (rval == 1)
			nomatch = 0;
		      if ((rval = tailmatch (kustr, kustrl, tbuf + base, offset)) == 0)
			{
#ifdef DEBUG
			  fprintf (stderr, "Found [UP] with offset=<%d>\n", offset);
#endif
			  offset = 0;
			  nomatch = 0;
			  while (dispindex)
			    {
			      putchar ('\b');
			      putchar (' ');
			      putchar ('\b');
			      dispindex--;
			    }
			  while (buf[dispindex] != '\0' && buf[dispindex] != '\n')
			    {
			      putchar (buf[dispindex]);
			      dispbuf[dispindex] = buf[dispindex];
			      dispindex++;
			    }
			}
		      else
			{
			  if (rval == 1)
			    nomatch = 0;
			  if ((rval = tailmatch (krstr, krstrl, tbuf + base, offset)) == 0)
			    {
#ifdef DEBUG
			      fprintf (stderr, "Found [RIGHT] with offset=<%d>\n", offset);
#endif
			      offset = 0;
			      nomatch = 0;
			      if (dispindex < previous && (buf[dispindex] != '\n'))
				{
				  putchar (buf[dispindex]);
				  dispbuf[dispindex] = buf[dispindex];
				  dispindex++;
				}
			    }
			  else
			    {
			      if (rval == 1)
				nomatch = 0;
			      if ((rval = tailmatch (klstr, klstrl, tbuf + base, offset)) == 0)
				{
#ifdef DEBUG
				  fprintf (stderr, "Found [LEFT] with offset=<%d>\n", offset);
#endif
				  offset = 0;
				  nomatch = 0;
				  if (dispindex > 0)
				    {
				      putchar ('\b');
				      putchar (' ');
				      putchar ('\b');
				      dispindex--;
				    }
				}
			      else
				{
				  if (rval == 1)
				    nomatch = 0;
				  if ((rval = tailmatch (kdstr, kdstrl, tbuf + base, offset)) == 0)
				    {
#ifdef DEBUG
				      fprintf (stderr, "Found [DOWN] with offset=<%d>\n", offset);
#endif
				      offset = 0;
				      nomatch = 0;
				    }
				  else
				    {
				      if (rval == 1)
					nomatch = 0;
				    }
				}
			    }
			}
		    }
		}

	      if (nomatch)
		{
#ifdef DEBUG
		  fprintf (stderr, "Found [%c] with offset=<%d>\n", tbuf[base], offset);
#endif
		  switch (tbuf[base])
		    {
		    case '\004':
		      quit = 1;
		      retval = 1;
		      break;
		    case 27:
		      putchar ('\\');
		      putchar ('\n');
		      dispindex = 0;
		      break;
		    case '\r':
		    case '\n':
		      quit = 1;
		      putchar ('\n');
		      dispbuf[dispindex++] = '\n';
		      dispbuf[dispindex] = '\0';
		      break;
		    case 12:	/* cntrl L */
		      buf[0] = '\n';
		      buf[1] = '\0';
		      putchar ('\\');
		      putchar ('\n');
		      lcls (0, 0, tokenstream, stdin, stdout);
		      dispindex = 0;
		      quit = 1;
		      break;
		    case 8:
		    case 127:
		      if (dispindex > 0)
			{
			  putchar ('\b');
			  putchar (' ');
			  putchar ('\b');
			  dispindex--;
			}
		      break;
		    case '\t':
		      if (dispindex < (len - 2))
			{
			  putchar (' ');
			  dispbuf[dispindex] = tbuf[base];
			  dispindex++;
			}
		      else
			{
			  putchar ('\a');
			}
		      break;
		    default:
		      if (dispindex < (len - 2))
			{
			  putchar (tbuf[base]);
			  dispbuf[dispindex] = tbuf[base];
			  dispindex++;
			}
		      else
			{
			  putchar ('\a');
			}
		      break;
		    }

		  base++;
		  offset--;
		  if (!offset)
		    {
		      base = 0;
		    }
		}
	    }

	  fflush (stdout);
	}
      previous = dispindex;
      strncpy (buf, dispbuf, dispindex);

      buf[dispindex + 1] = '\n';
      buf[dispindex + 2] = '\0';

      mresetterm ();
    }
  else
    {
      if (fgets (buf, len, stdin) == NULL)
	{
	  retval = 1;
	}
    }

#ifdef DEBUG
  fprintf (stderr, "Got command <%s>", buf);
#endif

  return retval;
}

#else

int 
readl (char *buf, int len)
{
  static oldc = 0;
  int retval = 0;
  int count = 0;
  int quit = 0;
  char old;


  if (isatty (STDIN_FILENO))
    {
      fflush (stdout);
      msetterm ();

      while (!quit)
	{
	  old = buf[count];
	  read (STDIN_FILENO, &buf[count], 1);

	  if (count < (len - 2))
	    {
	      if (isalnum (buf[count])
		  || ispunct (buf[count])
		  || buf[count] == ' ')
		{
		  putchar (buf[count]);
		}
	      else if (buf[count] == '\t')
		{
		  putchar (' ');
		}
	      else if (buf[count] == '\004')
		{
		  quit = 1;
		  retval = 1;
		}
	      else if (buf[count] == 27)
		{
		  putchar ('\\');
		  putchar ('\n');
		  count = (-1);
		}
	      else if (buf[count] == 12)
		{
		  lcls (0, 0, tokenstream, stdin, stdout);
		  buf[0] = '\n';
		  buf[1] = '\0';
		  quit = 1;
		}
	      else if (buf[count] == 8 || buf[count] == 127)
		{
		  if (count > 0)
		    {
		      putchar ('\b');
		      putchar (' ');
		      putchar ('\b');
		      count--;
		    }
		  count--;
		}
	      else if (buf[count] == '\n' || buf[count] == '\r')
		{
		  buf[count + 1] = '\0';
		  quit = 1;
		  putchar ('\n');
		}
	      else if (buf[count] == prevchar)
		{
		  if (oldc > count)
		    {
		      buf[count] = old;
		      putchar (old);
		    }
		  else
		    {
		      putchar ('\a');
		      count--;
		    }
		}
	      else
		{
		  putchar ('*');
		}
	      count++;
	    }
	  else
	    {
	      putchar ('\a');
	    }
	  fflush (stdout);
	}

      mresetterm ();
    }
  else
    {
      if (fgets (buf, len, stdin) == NULL)
	{
	  retval = 1;
	}
    }

  oldc = count - 1;

  return retval;
}

#endif
