/* Map drawing module.
   Paolo Bonzini, August 2008.

   This source code is released for free distribution under the terms
   of the GNU General Public License.  */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>

#include "toolkit.h"
#include "field.h"
#include "robotserver.h"

static cairo_t *map_context;
static cairo_t *cr;

/*
 * transforms degrees in to radians
 */
double
degtorad(int degrees)
{
	double radiants;
	radiants = degrees * M_PI/180;
	return radiants;
}

double _getDistance(double x1, double y1, double x2, double y2)
{
	double x, y;
	x = x2 - x1;
	y = y2 - y1;
	return sqrt(x*x + y*y);
}

void
shot_animation(cairo_t *cr, double size, double direction, struct cannon *can,
	       double origin_x, double origin_y)
{
	double dist;
	int time = can->timeToReload-RELOAD_RATIO/2;/*reduce the reload time by half of it so it draws the
										explosion and the flash for half the reload time*/
	
	if(time <= 0)return;/*if the gun is loaded don't paint anything*/
	
	cairo_save (cr);
	cairo_pattern_t *pat;
	cairo_translate (cr, can->x, can->y);

#if 0
	/* flash of the shot, should be translate to the robot's position.  */
	cairo_scale(cr, size, size);
	cairo_rotate(cr, direction);
	
	pat = cairo_pattern_create_linear (128, 64,
                                     128, 128.0);
	cairo_pattern_add_color_stop_rgba (pat, 0, 1, 0, 0, time/(RELOAD_RATIO/2.0));
	cairo_pattern_add_color_stop_rgba (pat, 1, 1, 1, 0, time/(RELOAD_RATIO/2.0));
	cairo_set_source (cr, pat);
	
	cairo_move_to (cr, 32, 50);
	cairo_arc_negative (cr, 32, 32, 18, 90 * (M_PI/180.0), 0);
	cairo_arc_negative (cr, 68, 32, 18, M_PI, 90 * (M_PI/180.0));
	cairo_arc_negative (cr, 50, 50, 18, M_PI, 0);
	cairo_fill (cr);
	cairo_pattern_destroy (pat);
#endif
	
	/* explosion*/
	cairo_arc (cr, 0, 0, 40, 0, 2*M_PI);
	pat = cairo_pattern_create_radial (0, 0, 10, 0, 0, 40);
	cairo_pattern_add_color_stop_rgba (pat, 0, 1, 0, 0, time/(RELOAD_RATIO/2.0));
	cairo_pattern_add_color_stop_rgba (pat, 0.3, 1, 0.5, 0, time/(RELOAD_RATIO/2.0));
	cairo_pattern_add_color_stop_rgba (pat, 0.6, 1, 0.2, 0, time/(RELOAD_RATIO/2.0));
	cairo_set_source (cr, pat);
	
	cairo_fill (cr);
	cairo_pattern_destroy (pat);
	
	cairo_restore(cr);

	/* draw track from the robot to the target */
	cairo_save(cr);
	cairo_translate(cr, origin_x, origin_y);
	cairo_rotate(cr, direction);
	dist = _getDistance(origin_x, origin_y, can->x, can->y);
	cairo_move_to(cr, dist, -5);
	cairo_line_to(cr, 0, 0);
	cairo_line_to(cr, dist, 5);
	cairo_set_source_rgba(cr, 1, 0.5, 0, (double)time / RELOAD_RATIO);
	cairo_fill_preserve(cr);
	cairo_set_source_rgba(cr, 1, 0, 0, (double)time / RELOAD_RATIO);
	cairo_set_line_width(cr, 1);
	cairo_stroke(cr);
	cairo_restore(cr);
}

/*
 *draws the cannon with the right orientation
 */
void
draw_cannon(cairo_t *cr, double direction, int reload)
{
	double x1=-5, y1=51,
		x2=-5, y2=19,
		x3=5, y3=19,
		x4=5, y4=51;
		
	cairo_save(cr);
	cairo_rotate(cr, direction);
	cairo_set_source_rgba (cr, 0, 0, 0, (double)(RELOAD_RATIO - reload) / RELOAD_RATIO);
	cairo_move_to (cr, x1, y1);
	cairo_line_to (cr, x2, y2);
	cairo_line_to (cr, x3, y3);
	cairo_line_to (cr, x4, y4);
	cairo_close_path(cr);
	
	cairo_fill(cr);
	cairo_restore(cr);
}

/*
 *draws the radar inside the robot with the right orientation
 */
void
draw_radar(cairo_t *cr, double direction)
{
	double x1=0, y1=22,
		x2=20, y2=-10,
		x3=-20, y3=-10;
	
	cairo_save(cr);
	cairo_rotate(cr, direction);
	cairo_set_source_rgba(cr, 1, 0, 0, 0.8);
	cairo_move_to(cr, x1, y1);
	cairo_line_to(cr, x2, y2);
	cairo_line_to(cr, x3, y3);
	cairo_close_path(cr);
	
	cairo_fill(cr);
	cairo_restore(cr);
}

/*
 *draws a robot with a given parameters
 */
void
_draw_robot(cairo_t *cr, double x, double y, int degree, int cannon_degree,
	    int radar_degree, float *color, int reload, double size)
{
	double x1=-70, y1=-30,
		y2=10,
		x4=70,
		x5=0, y5=-100;
	double px2=-70, py2=100,
		px3=70, py3=100,
		px4=70, py4=10;
	
	cairo_save(cr);
	cairo_translate(cr, x, y);
	cairo_scale(cr, size, size);
	cairo_save(cr);
	cairo_rotate(cr, degtorad(90+degree));

	cairo_set_source_rgba (cr, color[0], color[1], color[2], 0.6);
	cairo_set_line_width (cr, 2);
	cairo_move_to (cr,x1,y1);   
	cairo_line_to (cr,x1,y2);
	cairo_curve_to (cr, px2, py2, px3, py3, px4, py4);
	cairo_line_to (cr, x4, y1);
	cairo_line_to (cr, x5, y5);
	cairo_close_path (cr);
	cairo_set_fill_rule (cr, CAIRO_FILL_RULE_EVEN_ODD);
	cairo_move_to (cr, 30, 0);
	cairo_arc_negative (cr, 0, 0, 30, 0, -2*M_PI);

	cairo_fill_preserve (cr);
	cairo_set_source_rgba (cr, 0.1, 0.7, 0.5, 0.5);	
	
	cairo_stroke (cr);
	cairo_restore(cr); /* pop rotate */
	draw_cannon(cr, degtorad(270+cannon_degree), reload);
	draw_radar(cr, degtorad(270+radar_degree));
	cairo_restore(cr); /* pop translate/scale */
}

/*
 *draws a robot with a given size, using the various parameters(orientation, position,..)
 *from the robot struct
 */
void
draw_robot(cairo_t *cr, struct robot *myRobot, double size)
{
	_draw_robot(cr, myRobot->x, myRobot->y, myRobot->degree, myRobot->cannon_degree,
		    myRobot->radar_degree, myRobot->color, 0, size);
	shot_animation(cr, size, degtorad(myRobot->cannon_degree), &myRobot->cannon[0],
		       myRobot->x, myRobot->y);
	shot_animation(cr, size, degtorad(myRobot->cannon_degree), &myRobot->cannon[1],
		       myRobot->x, myRobot->y);
}

#define min(a, b)	((a) < (b) ? (a) : (b))

/*
 *draws the name and a health bar for every robot (maybe in the future also the reloading animation
 */
void
draw_stats(cairo_t *cr, struct robot **all)
{
	int i;
	int space = 25;
	cairo_pattern_t *pat;
	cairo_text_extents_t ext;

	cairo_save(cr);
	cairo_translate(cr, WIN_HEIGHT, 0);

	cairo_set_source_rgb(cr, 0.9, 0.9, 0.9);
	cairo_rectangle(cr, 0, 0, 140, WIN_HEIGHT);
	cairo_fill(cr);

	cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(cr, 13.0);
	pat = cairo_pattern_create_linear(100, 0, 0, 0);
	cairo_pattern_add_color_stop_rgba(pat, 1, 1, 0, 0, 1);
	cairo_pattern_add_color_stop_rgba(pat, 0, 0, 1, 0, 1);

	for(i = 0; i < max_robots; i++){
		_draw_robot(cr, 15, 16 + i * space, 0, 0, 0, all[i]->color,
			    min(all[i]->cannon[0].timeToReload, all[i]->cannon[1].timeToReload),
			    0.15);

		/* rectangle around life bar */
		cairo_rectangle(cr, 34, 6 + i * space, 102, 22);
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_set_line_width(cr, 1);
		cairo_stroke(cr);

		/* line with gradient from red to green */
		cairo_save(cr);
		cairo_translate(cr, 35, 17 + i * space);
		cairo_move_to(cr, 0, 0);
		cairo_line_to(cr, 100 - all[i]->damage, 0);
		cairo_set_line_width(cr, 20);
		cairo_set_source(cr, pat);
		cairo_stroke(cr);
		cairo_restore(cr);

		/* display the name of the robot */
		cairo_text_extents(cr, all[i]->name, &ext);
		cairo_move_to(cr, 85 - ext.width / 2, 21 + i * space);
		cairo_show_text(cr, all[i]->name);

		/* grey out robot if killed */
		if (all[i]->damage >= 100) {
			cairo_rectangle(cr, 0, 4 + i * space, 140, space);
			cairo_set_source_rgba(cr, 0.9, 0.9, 0.9, 0.5);
			cairo_fill(cr);
		}
	}
	cairo_pattern_destroy(pat);
	cairo_restore(cr);
}

void draw_results(cairo_t *cr)
{
	int i;
	int yoffset, x, y, xwidth, font_size;
	char text[32];
	cairo_text_extents_t ext;

	assert(dead_robots == max_robots);

	cairo_save(cr);
	cairo_translate(cr, 20, 20);
	cairo_rectangle(cr, 0, 0, WIN_HEIGHT - 40, WIN_HEIGHT - 40);
	cairo_set_source_rgba(cr, 0.7, 0.7, 0.6, 0.5);
	cairo_fill(cr);

	yoffset = 120;
	for (i = 0; i < 5 && i < max_robots; i++) {
		cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
				       CAIRO_FONT_WEIGHT_BOLD);
		cairo_set_source_rgb(cr, 0, 0.3, 0);
		if (i == 0)
			font_size = 42;
		else
			font_size = 30;
		cairo_set_font_size(cr, font_size);
		snprintf(text, sizeof(text), "%d.", i + 1);
		cairo_text_extents(cr, ranking[i]->name, &ext);
		xwidth = ext.width;
		cairo_text_extents(cr, text, &ext);
		xwidth += ext.width + 3 * font_size;
		x = (WIN_HEIGHT - 40 - xwidth) / 2;
		y = yoffset + i * 60;
		cairo_move_to(cr, x, y);
		cairo_show_text(cr, text);
		_draw_robot(cr, x + ext.width + font_size, y - font_size * 0.4, 0, 0, 0,
			    ranking[i]->color, 0, font_size / 150.0);
		cairo_move_to(cr, x + 3 * font_size, y);
		cairo_show_text(cr, ranking[i]->name);

		cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_ITALIC,
				       CAIRO_FONT_WEIGHT_NORMAL);
		if (i == 0) {
			font_size = 21;
		} else
			font_size = 15;
		cairo_set_font_size(cr, font_size);

		snprintf(text, sizeof(text), "time: %ld:%02ld", ranking[i]->life_length.tv_sec / 60,
			 ranking[i]->life_length.tv_sec % 60);
		cairo_text_extents(cr, text, &ext);
		cairo_move_to(cr, (WIN_HEIGHT - 40) / 2 - ext.width - 20,
			      yoffset + font_size + 5 + i * 60);
		cairo_show_text(cr, text);

		snprintf(text, sizeof(text), "score: %d", ranking[i]->score);
		cairo_move_to(cr, (WIN_HEIGHT - 40) / 2 + 20,
			      yoffset + font_size + 5 + i * 60);
		cairo_show_text(cr, text);

		if (i == 0)
			yoffset += 20;
	}
	cairo_restore(cr);
}

void
draw (cairo_t *cr)
{
	int i;
	cairo_save (cr);
  
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_paint(cr);
	cairo_scale(cr, WIN_HEIGHT/1000.0, WIN_HEIGHT/1000.0);
	for(i = 0; i < max_robots; i++){
		draw_robot(cr, all_robots[i], 0.4);
	}
	cairo_restore(cr);	
	draw_stats(cr, all_robots);
/* cairo_save (cairo_context);
  cairo_set_operator (cairo_context, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_surface (cairo_context, cairo_get_target (map_context), 0, 0);
  cairo_paint (cairo_context);
  cairo_restore (cairo_context);*/
}

void
update_display(int finished)
{	
  draw(cr);
  if (finished)
	draw_results(cr);
}

void
init_cairo(int *argc, char ***argv)
{
	cr = init_toolkit(argc, argv);
}

void
destroy_cairo ()
{
	free_toolkit(cr);
}

event_t process_cairo(void)
{
	return process_toolkit(&cr);
}
