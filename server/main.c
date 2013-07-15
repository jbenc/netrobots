/* Main program.
   Paolo Bonzini, August 2008.

   This source code is released for free distribution under the terms
   of the GNU General Public License.  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "drawing.h"
#include "field.h"
#include "net_defines.h"

int
main (int argc, char **argv)
{
  unsigned int i = 0, start_ticks;
  int phase = 0;

  /* initialize SDL and create as OpenGL-texture source */
  cairo_t *cairo_context;
  srandom(time(NULL) + getpid());
  server_init(argc, argv);
  init_cairo();

  start_ticks = SDL_GetTicks ();
	
  /* enter event-loop */
  for (;;) {
	SDL_Event event;
	i++;
	draw_sdl ();
	event.type = -1;
	SDL_PollEvent (&event);

	/* check for user hitting close-window widget */
	if (event.type == SDL_QUIT)
		break;

	/* Call functions here to parse event and render on cairo_context...  */
	switch (phase) {
	case 0:
		/* connecting */
		if (server_process_connections(&event))
			phase++;
		break;
	case 1:
		/* game in progress */
		if (server_cycle(&event)) {
			complete_ranking();
			phase++;
		}
		break;
	case 2:
		/* display ranking */
		server_finished_cycle(&event);
		break;
	}
    }
	

  printf ("%.2f fps\n", (i * 1000.0) / (SDL_GetTicks () - start_ticks));

  /* clear resources before exit */
  destroy_cairo();

  return 0;
}
