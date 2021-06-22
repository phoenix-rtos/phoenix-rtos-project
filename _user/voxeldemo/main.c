/*
 * Phoenix-RTOS
 *
 * Self Contained Voxel Space Demo
 *
 * Example of user application
 *
 * Copyright 2021 Phoenix Systems
 * Author: Gerard Swiderski
 *
 * This file is part of Phoenix-RTOS.
 *
 * %LICENSE%
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <graph.h>


volatile unsigned int flagQuit;


typedef struct _camera_t {
	float x, y, h;
	float angle;
	int horiz;
	int dist;
} camera_t;


typedef struct _voxel_t {
	int *bufBack;
	uint8_t *mapHeight;
	uint8_t *mapColor;
	uint32_t *pixels, *palette;
	unsigned int width;
	unsigned int height;
	camera_t cam;
} voxel_t;


static inline int clamp(int x)
{
	return x < 0 ? 0 : x > 0xff ? 0xff : x;
}


static int interpolate(uint32_t *tab, uint32_t a, uint32_t b, int n)
{
	unsigned int j;

	for (j = 0; j < n; j++) {
		*tab = ((j * (((uint8_t)(b >> 16))) + (n - j) * (((uint8_t)(a >> 16)))) / n) << 16;
		*tab |= ((j * (((uint8_t)(b >> 8))) + (n - j) * (((uint8_t)(a >> 8)))) / n) << 8;
		*tab |= (j * (((uint8_t)(b))) + (n - j) * (((uint8_t)(a)))) / n;
		tab++;
	}

	return n;
}


static void voxel_free(voxel_t *v)
{
	free(v->palette);
	free(v->pixels);
	free(v->bufBack);
	free(v->mapColor);
	free(v->mapHeight);
}


static int voxel_init(voxel_t *v, graph_t *g)
{
	do {
		if (g->width < 800 || g->height < 600)
			return -1;

		if ((v->mapHeight = malloc(1024 * 1024)) == NULL)
			break;

		if ((v->mapColor = malloc(1024 * 1024)) == NULL)
			break;

		if ((v->bufBack = malloc(g->width * sizeof(int))) == NULL)
			break;

		if ((v->pixels = malloc(g->width * g->height * g->depth)) == NULL)
			break;

		if ((v->palette = malloc(256 * g->depth)) == NULL)
			break;

		v->width = g->width;
		v->height = g->height;

		return EOK;
	} while (0);

	voxel_free(v);

	return -1;
}


static void voxel_drawLine(uint32_t *pixel, int width, int x, int ht, int hb, uint32_t rgb)
{
	if (ht < 0)
		ht = 0;

	if (ht >= hb)
		return;

	pixel += ht * width + x;
	while (ht < hb) {
		*pixel = rgb;
		pixel += width;
		ht++;
	}
}


static void voxel_drawView(voxel_t *v)
{
	int hs = 0, x = 0, y = 0, i;
	int scale_height = 120;

	float z = 1.0f;
	float dz = 1.0f;
	float cosa = cosf(v->cam.angle);
	float sina = sinf(v->cam.angle);

	for (i = 0; i < v->width; i++)
		v->bufBack[i] = v->height;

	for (z = 1.0, dz = 1.0; z < v->cam.dist;) {
		float cosz = cosa * z, sinz = sina * z;

		float ax = -cosz - sinz, ay = sinz - cosz;
		float bx = cosz - sinz, by = -sinz - cosz;

		float dx = (bx - ax) / v->width;
		float dy = (by - ay) / v->width;

		ax += v->cam.x, ay += v->cam.y;

		float invz = 1.0f / z * scale_height;

		for (i = 0; i < v->width; i++) {
			x = ((int)ax) & (1024 - 1);
			y = ((int)ay) & (1024 - 1);

			hs = (v->cam.h - v->mapHeight[x + y * 1024]) * invz + v->cam.horiz;

			voxel_drawLine(v->pixels, v->width, i, hs, v->bufBack[i], v->palette[v->mapColor[x + y * 1024]]);

			if (hs < v->bufBack[i])
				v->bufBack[i] = hs;

			ax += dx;
			ay += dy;
		}

		dz += 0.0005;
		z += dz;
	}
}


static void voxel_drawSky(voxel_t *v, int post)
{
	int i, j, alpha;
	uint32_t col, *pixels = v->pixels;
	uint8_t r0, r1, g0, g1, b0, b1;

	if (post)
		col = 0x6c9eff;
	else
		col = 0x009eff;

	for (j = 1080 / 2; j > 0; j--) {
		alpha = j < 255 && post ? j : 0xff;

		for (i = 0; i < v->width; i++) {
			r0 = ((unsigned)*pixels >> 16) & 0xff;
			g0 = ((unsigned)*pixels >> 8) & 0xff;
			b0 = ((unsigned)*pixels) & 0xff;

			r1 = ((unsigned)col >> 16) & 0xff;
			g1 = ((unsigned)col >> 8) & 0xff;
			b1 = ((unsigned)col) & 0xff;

			r0 = ((r0) + ((((r1) - (r0)) * (alpha)) >> 8));
			g0 = ((g0) + ((((g1) - (g0)) * (alpha)) >> 8));
			b0 = ((b0) + ((((b1) - (b0)) * (alpha)) >> 8));

			*pixels = (((unsigned)r0 << 16) & 0xff0000);
			*pixels += (((unsigned)g0 << 8) & 0x00ff00);
			*pixels += (((unsigned)b0 << 0) & 0x0000ff);

			pixels++;
		}
	}
}


static void voxel_genLand(voxel_t *v)
{
	unsigned int tmp;
	int p, i, j, k, k2, p2;
	int a, b, c, d;

	/* Start from a plasma clouds fractal */
	v->mapHeight[0] = 128;
	for (p = 1024; p > 1; p = p2) {
		p2 = p >> 1;
		k = p * 8 + 20;
		k2 = k >> 1;
		for (i = 0; i < 1024; i += p) {
			for (j = 0; j < 1024; j += p) {
				a = v->mapHeight[(i << 10) + j];
				b = v->mapHeight[(((i + p) & 0x3ff) << 10) + j];
				c = v->mapHeight[(i << 10) + ((j + p) & 0x3ff)];
				d = v->mapHeight[(((i + p) & 0x3ff) << 10) + ((j + p) & 0x3ff)];

				v->mapHeight[(i << 10) + ((j + p2) & 0x3ff)] =
					clamp(((a + c) >> 1) + (rand() % k - k2));
				v->mapHeight[(((i + p2) & 0x3ff) << 10) + ((j + p2) & 0x3ff)] =
					clamp(((a + b + c + d) >> 2) + (rand() % k - k2));
				v->mapHeight[(((i + p2) & 0x3ff) << 10) + j] =
					clamp(((a + b) >> 1) + (rand() % k - k2));
			}
		}
	}

	/* clamp unwanted noise level */
	for (i = 0; i < 1024 * 1024; i += 1024) {
		for (j = 0; j < 1024; j++) {
			if (v->mapHeight[i + j] < 50)
				v->mapHeight[i + j] = 50;

			v->mapHeight[i + j] -= 50;
		}
	}

	/* Initialize color map */
	for (i = 0; i < 1024 * 1024; i += 1024)
		for (j = 0; j < 1024; j++)
			v->mapColor[i + j] = v->mapHeight[i + j];

	/* clamp water level */
	for (i = 0; i < 1024 * 1024; i += 1024) {
		for (j = 0; j < 1024; j++) {
			if (v->mapHeight[i + j] < 100)
				v->mapHeight[i + j] = 100;

			v->mapHeight[i + j] -= 100;
		}
	}
	/* Smooth height map */
	for (k = 0; k < 20; k++) {
		for (i = 0; i < 1024 * 1024; i += 1024)
			for (j = 0; j < 1024; j++) {
				tmp = v->mapHeight[((i + 1024) & 0xffc00) + j];
				tmp += v->mapHeight[i + ((j + 1) & 0x3ff)];
				tmp += v->mapHeight[((i - 1024) & 0xffc00) + j];
				tmp += v->mapHeight[i + ((j - 1) & 0x3ff)];
				v->mapHeight[i + j] = tmp >> 2;
			}
	}

	/* Smooth color map */
	for (k = 0; k < 1; k++) {
		for (i = 0; i < 1024 * 1024; i += 1024)
			for (j = 0; j < 1024; j++) {
				tmp = v->mapColor[((i + 1024) & 0xffc00) + j];
				tmp += v->mapColor[i + ((j + 1) & 0x3ff)];
				tmp += v->mapColor[((i - 1024) & 0xffc00) + j];
				tmp += v->mapColor[i + ((j - 1) & 0x3ff)];
				v->mapColor[i + j] = tmp >> 2;
			}
	}
}


static void voxel_genPalette(voxel_t *v)
{
	unsigned int i, idx = 0;
	uint8_t step[] = { 50, 50, 50, 50, 10, 10, 10, 15 };
	uint32_t colors[] = {
		0x3c789c, 0x4884a8, /* water */
		0x3d7028, 0x846c4c, /* grass */
		0xc6b3a3, 0xb8ada2, /* sand 1 */
		0x846c4c, 0x8c7454, /* sand 2 */
		0xbcc8d8,           /* frost-snow */
	};

	for (i = 0; i < sizeof(step); i++)
		idx += interpolate(&v->palette[idx], colors[i], colors[i + 1], step[i]);
}


static void signalHandler(int signo)
{
	flagQuit = 1;
}


static int voxel_demo(graph_t *g)
{
	voxel_t v = {
		.cam = { .h = 300, .horiz = 200, .dist = 300 },
	};

	if (voxel_init(&v, g) < 0)
		return -1;

	signal(SIGINT, signalHandler);
	signal(SIGQUIT, signalHandler);
	signal(SIGTERM, signalHandler);

	voxel_genLand(&v);
	voxel_genPalette(&v);

	for (flagQuit = 0; flagQuit == 0;) {
		memset(v.pixels, 0xff, g->width * g->height * g->depth);

		voxel_drawView(&v);
		voxel_drawSky(&v, 1);

		v.cam.x += 0.8f;
		v.cam.y += 0.008f;
		v.cam.angle += 0.008f;

		/* FIXME: flip double buffer to screen, this shall be done in graph_commit() */
		memcpy(g->data, v.pixels, g->width * g->height * g->depth);

		graph_commit(g);
	}

	signal(SIGTERM, SIG_DFL);
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);

	voxel_free(&v);

	return 0;
}


int main(void)
{
	int ret;
	graph_t g;

	if ((ret = graph_init()) < 0) {
		fprintf(stderr, "failed to initialize library\n");
		return ret;
	}

	if ((ret = graph_open(&g, GRAPH_ANY, 0x2000)) < 0) {
		fprintf(stderr, "failed to initialize graphics adapter\n");
		graph_done();
		return ret;
	}

	do {
		if ((ret = graph_mode(&g, GRAPH_DEFMODE, GRAPH_DEFFREQ)) < 0) {
			fprintf(stderr, "failed to set graphics mode\n");
			break;
		}

		if (g.depth != 4) {
			fprintf(stderr, "Demo requires 32-bit video resolution\n");
			break;
		}

		if ((ret = voxel_demo(&g)) < 0) {
			fprintf(stderr, "Something's gone teribly wrong\n");
			break;
		}
	} while (0);

	graph_close(&g);
	graph_done();

	return ret;
}
