/*
 * Phoenix-RTOS
 *
 * Rotating Rectangle
 *
 * Example of user application
 *
 * Copyright 2021 Phoenix Systems
 * Author: Damian Loewnau
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */


/*					a
*	(x0,y0)	.................
*			|				|
*			|		.		|
*		b	|	(xc,yc)		|
*			|...............|
*/

#include <stdio.h>
#include <stdlib.h>
#include <graph.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <termios.h>

#include <sys/threads.h>

typedef struct {
	unsigned int xc;
	unsigned int yc;
	unsigned int a;
	unsigned int b;
	unsigned int color;
} rectangle_t;

typedef struct {
	rectangle_t rec;
	rectangle_t toRemove;
	unsigned int rotSpeed;
	unsigned int maxxc;
	unsigned int maxyc;
	unsigned int minxc;
	unsigned int minyc;
	char stop;
} rotrectangle_t;

/* Enables/disables canon mode and echo */
static int rotrectangle_switchmode(int canon)
{
	struct termios state;

	tcgetattr(STDIN_FILENO, &state);
	if (canon) {
		state.c_lflag |= ICANON;
		state.c_lflag |= ECHO;
	} else {
		state.c_lflag &= ~ICANON;
		state.c_lflag &= ~ECHO;
		state.c_cc[VMIN] = 1;
	}
	tcsetattr(STDIN_FILENO, TCSANOW, &state);
	
	return 0;
}


static void rotrectangle_getkey(void *arg)
{
	char ch;
	rotrectangle_t *self = (rotrectangle_t *)arg;
	while ((self->stop == 0) && (ch = getchar())) {
		if (ch == 27) {					/*ESC code from arrow keys*/
			getchar();
			ch = getchar();
		}
		switch (ch) {
			case 67:
				if (self->rec.xc < (self->maxxc-2))
					self->rec.xc+=3;
				break;
			case 68:
				if (self->rec.xc > (self->minxc+2))
					self->rec.xc-=3;
				break;
			case 65:
				if (self->rec.yc > (self->minyc+2))
					self->rec.yc-=3;
				break;
			case 66:
				if (self->rec.yc < (self->maxyc-2))
					self->rec.yc+=3;
				break;
			case 43:
				if (self->rotSpeed < 5)
					self->rotSpeed++;
				break;
			case 45:
				if (self->rotSpeed > 1)
					self->rotSpeed--;
				break;
			case 'r':
				self->rec.color = 4;
				break;
			case 'g':
				self->rec.color = 2;
				break;
			case 'b':
				self->rec.color = 1;
				break;
			case 'q':
				self->stop = 1;
				break;
			default:
				break;
		}
	}
}

static int rotrectangle_print(rectangle_t *self, graph_t *graphp, unsigned int angle)
{
	double dx, dy, angleRad, xcDouble, ycDouble, aDouble, bDouble;
	unsigned int x0, y0, x, y;		/* (x,y) is start point for next rectangle lines */

	xcDouble = (double)self->xc;
	ycDouble = (double)self->yc;
	aDouble = (double)self->a;
	bDouble = (double)self->b;

	angleRad = ((double)(angle)*3.141592)/180.0;
	x0 = (unsigned int)(xcDouble - ( (sin(M_PI/2-angleRad-atan(bDouble/aDouble)))*sqrt(pow(aDouble, 2.0)+pow(bDouble, 2.0)) )/2.0);
	y0 = (unsigned int)(ycDouble - ( (cos(M_PI/2-angleRad-atan(bDouble/aDouble)))*sqrt(pow(aDouble, 2.0)+pow(bDouble, 2.0)) )/2.0);

	x = x0;
	y = y0;
	dx = (self->a)*cos(angleRad); 
	dy = (self->a)*sin(angleRad);
	graph_line(graphp, x, y, (int)dx, (int)dy, 1, self->color, GRAPH_QUEUE_HIGH);

	x = x0+dx;
	y = y0+dy;
	dx = (self->b)*cos(angleRad+(0.5*3.141592));
	dy = (self->b)*sin(angleRad+(0.5*3.141592));
	graph_line(graphp, x, y, (int)dx, (int)dy, 1, self->color, GRAPH_QUEUE_HIGH);

	x = x0;
	y = y0;
	dx = (self->b)*cos(angleRad+(0.5*3.141592)); 
	dy = (self->b)*sin(angleRad+(0.5*3.141592));
	graph_line(graphp, x, y, (int)dx, (int)dy, 1, self->color, GRAPH_QUEUE_HIGH);

	x = x0+dx;
	y = y0+dy;
	dx = (self->a)*cos(angleRad); 
	dy = (self->a)*sin(angleRad);
	graph_line(graphp, x, y, (int)dx, (int)dy, 1, self->color, GRAPH_QUEUE_HIGH);

	return 0;
}

static int rotrectangle_save(rotrectangle_t *self)
{
	self->toRemove.xc = self->rec.xc;
	self->toRemove.yc = self->rec.yc;
	self->toRemove.a = self->rec.a;
	self->toRemove.b = self->rec.b;
	self->toRemove.color = 0;

	return 0;
}

static int rotrectangle_rotating(rotrectangle_t *self, graph_t *graphp)
{
	unsigned int alfa, ret;
	ret = 0;
	while (self->stop == 0) {
		for (alfa=0; alfa<360; alfa+=self->rotSpeed) {
			if (self->stop != 0)
				break;
			ret = rotrectangle_print(&(self->rec), graphp, alfa);
			// graph_fill(graphp, self->rec.xc, self->rec.yc, self->rec.color, GRAPH_FILL_FLOOD, GRAPH_QUEUE_HIGH);
			ret = rotrectangle_save(self);
			usleep(10000);		/* min value */
			ret = rotrectangle_print(&(self->toRemove), graphp, alfa);
			// graph_fill(graphp, self->rec.xc, self->rec.yc, 0, GRAPH_FILL_FLOOD, GRAPH_QUEUE_HIGH);
		}
	}
	
	return ret;
}

static int rotrectangle_startpanel(rotrectangle_t *self)
{
	printf("*** Rotating rectangle program ***\n");
	printf("  User interface:\n");
	printf("    )up arrow - move up\n");
	printf("    )down arrow - move down\n");
	printf("    )right arrow - move right\n");
	printf("    )left arrow - move left\n");
	printf("    )'+' - increase rotation speed\n");
	printf("    )'-' - decrease rotation speed\n");
	printf("    )'r' - turn red\n");
	printf("    )'g' - turn green\n");
	printf("    )'b' - turn blue\n");
	printf("    )'q' - exit\n");
	printf("  Press any key to start: \n");
	getchar();

	return 0;
}

int main(void)
{
	int ret;
	char stack[1024] __attribute__ ((aligned(8)));

	graph_adapter_t adapter = GRAPH_ANY;
	graph_mode_t mode = GRAPH_DEFMODE;
	graph_freq_t freq = GRAPH_DEFFREQ;
	graph_t graph;

	rotrectangle_t rotrectangle;

	rotrectangle.rotSpeed = 3;
	rotrectangle.stop = 0;
	rotrectangle.rec.xc = 100;
	rotrectangle.rec.yc = 100;
	rotrectangle.rec.a = 60;
	rotrectangle.rec.b = 30;
	rotrectangle.rec.color = 4;


	ret = rotrectangle_startpanel(&rotrectangle);

	rotrectangle_switchmode(0);

	if ((ret = graph_init()) < 0) {
		fprintf(stderr, "test_graph: failed to initialize library\n");
		return ret;
	}
	if ((ret = graph_open(&graph, adapter, 0x2000)) < 0) {
		fprintf(stderr, "test_graph: failed to initialize graphics adapter\n");
		graph_done();
		return ret;
	}
	if ((ret = graph_mode(&graph, mode, freq)) < 0) {
		fprintf(stderr, "test_graph: failed to set graphics mode\n");
		return ret;
	}

	if ((ret = graph_rect(&graph, 0, 0, graph.width, graph.height, 0, GRAPH_QUEUE_HIGH)) < 0) {
		fprintf(stderr, "test_graph: failed to clear screen\n");
		return ret;
	}

	rotrectangle.minyc = rotrectangle.minxc = (unsigned int)(sqrt(pow((double) rotrectangle.rec.a, 2.0)+pow((double) rotrectangle.rec.b, 2.0))/2)+2;
	rotrectangle.maxyc = graph.height-rotrectangle.minyc;
	rotrectangle.maxxc = graph.width-rotrectangle.minxc;

	ret = beginthread(rotrectangle_getkey, 4, stack, sizeof(stack), &rotrectangle);

	ret = rotrectangle_rotating(&rotrectangle, &graph);

	graph_close(&graph);

	return ret;
}
