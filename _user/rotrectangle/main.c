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


/*                 a
*     (x0,y0) ...............
*           |               |
*           |      .        |
*         b |   (xc,yc)     |
*           |...............|
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
	volatile unsigned int xc;
	volatile unsigned int yc;
	unsigned int a;
	unsigned int b;
	unsigned int color;
} rectangle_t;


typedef struct {
	handle_t lock;
	rectangle_t rec;
	rectangle_t prev;
	volatile unsigned int speed;
	unsigned int maxxc;
	unsigned int maxyc;
	unsigned int minxc;
	unsigned int minyc;
	char stop;
} rotrectangle_t;


/* Enables/disables canon mode and echo */
static void rotrectangle_switchmode(int canon)
{
	struct termios state;

	tcgetattr(STDIN_FILENO, &state);
	if (canon) {
		state.c_lflag |= ICANON;
		state.c_lflag |= ECHO;
	}
	else {
		state.c_lflag &= ~ICANON;
		state.c_lflag &= ~ECHO;
		state.c_cc[VMIN] = 1;
	}
	tcsetattr(STDIN_FILENO, TCSANOW, &state);
}


static void rotrectangle_getkey(void *arg)
{
	char ch;
	rotrectangle_t *self = (rotrectangle_t *)arg;
	while ((self->stop == 0) && (ch = getchar())) {
		if (ch == 27) { /*ESC code from arrow keys*/
			getchar();
			ch = getchar();
		}
		mutexLock(self->lock);
		switch (ch) {
			case 67:
				if (self->rec.xc < (self->maxxc - 4))
					self->rec.xc += 5;
				break;
			case 68:
				if (self->rec.xc > (self->minxc + 4))
					self->rec.xc -= 5;
				break;
			case 65:
				if (self->rec.yc > (self->minyc + 4))
					self->rec.yc -= 5;
				break;
			case 66:
				if (self->rec.yc < (self->maxyc - 4))
					self->rec.yc += 5;
				break;
			case 43:
				if (self->speed < 5)
					self->speed++;
				break;
			case 45:
				if (self->speed > 1)
					self->speed--;
				break;
			case 'q':
				self->stop = 1;
				break;
			default:
				break;
		}
		mutexUnlock(self->lock);
	}

	endthread();
}


static void rotrectangle_print(const rectangle_t *self, graph_t *graphp, unsigned int angle)
{
	double dx, dy, alfa, xc, yc, a, b;
	unsigned int x0, y0, x, y; /* (x,y) is start point for next rectangle lines */

	xc = (double)self->xc;
	yc = (double)self->yc;
	a = (double)self->a;
	b = (double)self->b;

	alfa = ((double)angle * M_PI) / 180.0;
	x0 = (unsigned int)(xc - ((sin(M_PI / 2 - alfa - atan(b / a))) * sqrt(pow(a, 2.0) + pow(b, 2.0))) / 2.0);
	y0 = (unsigned int)(yc - ((cos(M_PI / 2 - alfa - atan(b / a))) * sqrt(pow(a, 2.0) + pow(b, 2.0))) / 2.0);

	x = x0;
	y = y0;
	dx = a * cos(alfa);
	dy = a * sin(alfa);
	graph_line(graphp, x, y, (int)dx, (int)dy, 1, self->color, GRAPH_QUEUE_HIGH);

	x = x0 + dx;
	y = y0 + dy;
	dx = b * cos(alfa + (0.5 * M_PI));
	dy = b * sin(alfa + (0.5 * M_PI));
	graph_line(graphp, x, y, (int)dx, (int)dy, 1, self->color, GRAPH_QUEUE_HIGH);

	x = x0;
	y = y0;
	dx = b * cos(alfa + (0.5 * M_PI));
	dy = b * sin(alfa + (0.5 * M_PI));
	graph_line(graphp, x, y, (int)dx, (int)dy, 1, self->color, GRAPH_QUEUE_HIGH);

	x = x0 + dx;
	y = y0 + dy;
	dx = a * cos(alfa);
	dy = a * sin(alfa);
	graph_line(graphp, x, y, (int)dx, (int)dy, 1, self->color, GRAPH_QUEUE_HIGH);
}


static void rotrectangle_save(rotrectangle_t *self)
{
	self->prev.xc = self->rec.xc;
	self->prev.yc = self->rec.yc;
	self->prev.a = self->rec.a;
	self->prev.b = self->rec.b;
	self->prev.color = 0;
}


static void rotrectangle_rotating(rotrectangle_t *self, graph_t *graphp)
{
	unsigned int alfa;
	while (self->stop == 0) {
		for (alfa = 0; alfa < 360; alfa += self->speed) {
			if (self->stop != 0)
				break;
			mutexLock(self->lock);
			rotrectangle_print(&(self->rec), graphp, alfa);
			/* filling rectangle functions are commented out, because of problem with screen refreshing, it's demo without it */
			/* graph_fill(graphp, self->rec.xc, self->rec.yc, self->rec.color, GRAPH_FILL_FLOOD, GRAPH_QUEUE_HIGH); */
			rotrectangle_save(self);
			mutexUnlock(self->lock);
			usleep(10000); /* min value */
			rotrectangle_print(&(self->prev), graphp, alfa);
			/* graph_fill(graphp, self->rec.xc, self->rec.yc, 0, GRAPH_FILL_FLOOD, GRAPH_QUEUE_HIGH); */
		}
	}
}


static void rotrectangle_startpanel(void)
{
	printf("*** Rotating rectangle program ***\n");
	printf("  User interface:\n");
	printf("    )up arrow - move up\n");
	printf("    )down arrow - move down\n");
	printf("    )right arrow - move right\n");
	printf("    )left arrow - move left\n");
	printf("    )'+' - increase rotation speed\n");
	printf("    )'-' - decrease rotation speed\n");
	printf("    )'q' - exit\n");
	printf("  Press enter to start: \n");
	getchar();
}


int main(void)
{
	int ret;
	char *stack;

	graph_adapter_t adapter = GRAPH_ANY;
	graph_mode_t mode = GRAPH_DEFMODE;
	graph_freq_t freq = GRAPH_DEFFREQ;
	graph_t graph;

	rotrectangle_t rotrectangle;

	static const unsigned int stacksz = 1024;

	rotrectangle.speed = 3;
	rotrectangle.stop = 0;
	rotrectangle.rec.a = 180;
	rotrectangle.rec.b = 90;
	rotrectangle.rec.color = 60000;

	rotrectangle_startpanel();

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

	rotrectangle.rec.xc = graph.width / 2;
	rotrectangle.rec.yc = graph.height / 2;
	rotrectangle.minyc = rotrectangle.minxc = (unsigned int)(sqrt(pow((double)rotrectangle.rec.a, 2.0) + pow((double)rotrectangle.rec.b, 2.0)) / 2) + 2;
	rotrectangle.maxyc = graph.height - rotrectangle.minyc;
	rotrectangle.maxxc = graph.width - rotrectangle.minxc;

	rotrectangle_switchmode(0);

	if ((stack = (char *)malloc(stacksz)) == NULL)
		return 1;

	mutexCreate(&rotrectangle.lock);

	beginthread(rotrectangle_getkey, 4, stack, stacksz, (void *)&rotrectangle);

	rotrectangle_rotating(&rotrectangle, &graph);

	rotrectangle_switchmode(1);

	graph_close(&graph);
	graph_done();

	free(stack);

	return 0;
}
