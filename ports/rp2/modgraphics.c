#include <stdint.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "ST7735_TFT.h"
#include "font.h"

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 128

#define XY2I(x, y) y * SCREEN_WIDTH + x

uint16_t screen_buf[SCREEN_WIDTH*SCREEN_HEIGHT];

void write_pixel(uint16_t c, uint16_t x, uint16_t y) {
	uint16_t i = XY2I(x, y);
	screen_buf[i] = c;
}

void update_region(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
	st7735_set_region(y, y+h-1, x, x+w-1);
	for(uint16_t sx = x; sx < x+w; sx++) {
		for(uint16_t sy = y; sy < y+h; sy++) {
			st7735_fill_send(screen_buf[XY2I(sx, sy)]);
		}
	}
	st7735_fill_finish();
}

mp_obj_t graphics_init(void) {
	st7735_init();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(graphics_init_obj, graphics_init);

static mp_obj_t graphics_start(void) {
	st7735_fill_start();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(graphics_start_obj, graphics_start);

static mp_obj_t graphics_finish(void) {
	st7735_fill_finish();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(graphics_finish_obj, graphics_finish);

static mp_obj_t graphics_fill(mp_obj_t color) {
	uint16_t c = mp_obj_get_int(color);

	st7735_fill_start();
	for(uint16_t i = 0; i < SCREEN_WIDTH*SCREEN_HEIGHT; i++) {
		st7735_fill_send(c);
		screen_buf[i] = c;
	}
	st7735_fill_finish();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(graphics_fill_obj, graphics_fill);

static mp_obj_t graphics_fill_rect(mp_obj_t color, mp_obj_t xy, mp_obj_t wh) {
	uint16_t c = mp_obj_get_int(color);

	int sx = 0;
	int sy = 0;
	int sw = 0;
	int sh = 0;

	size_t len = 0;
	mp_obj_t *tuple;
	mp_obj_tuple_get(xy, &len, &tuple); 
	assert(len == 2);

	sx = mp_obj_get_int(tuple[0]);
	sy = mp_obj_get_int(tuple[1]);

	mp_obj_tuple_get(wh, &len, &tuple); 
	assert(len == 2);
	
	sw = mp_obj_get_int(tuple[0]);
	sh = mp_obj_get_int(tuple[1]);

	for(uint16_t x = sx; x < sx+sw; x++) {
		for(uint16_t y = sy; y < sy+sh; y++) {
			write_pixel(c, x, y);
		}
	}
	update_region(sx, sy, sw, sh);

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(graphics_fill_rect_obj, graphics_fill_rect);

static mp_obj_t graphics_fill_circle(mp_obj_t color, mp_obj_t center, mp_obj_t radius) {
	uint16_t c = mp_obj_get_int(color);


	size_t len = 0;
	mp_obj_t *tuple;
	mp_obj_tuple_get(center, &len, &tuple); 
	assert(len == 2);

	int sr = mp_obj_get_int(radius);
	int sx = mp_obj_get_int(tuple[0]) - sr;
	int sy = mp_obj_get_int(tuple[1]) - sr;
	int sw = sr*2;
	int sh = sr*2;

	for(int i = 0; i < sw*sh; i++) {
		int x = floor((float)i / (float)sw) + sx;
		int y = i % sw + sy;
		if(sqrt(pow(sx - x + sr, 2) + pow(sy - y + sr, 2)) <= sr) {
			write_pixel(c, x, y);
		}
	}
	update_region(sx, sy, sw, sh);

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(graphics_fill_circle_obj, graphics_fill_circle);

static mp_obj_t graphics_pixel(mp_obj_t color, mp_obj_t x, mp_obj_t y) {
	uint16_t c = mp_obj_get_int(color);
	uint16_t x_int = mp_obj_get_int(x);
	uint16_t y_int = mp_obj_get_int(y);
	st7735_set_region(y_int, y_int, x_int, x_int);
	st7735_fill_send(c);
	screen_buf[XY2I(x_int, y_int)] = c;
	st7735_fill_finish();
	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(graphics_pixel_obj, graphics_pixel);

static mp_obj_t graphics_color(mp_obj_t r, mp_obj_t g, mp_obj_t b) {
	uint16_t r_int = MAX(MIN(mp_obj_get_float(r), 255.0f), 0.0f)/255.0f*31;
	uint16_t g_int = MAX(MIN(mp_obj_get_float(g), 255.0f), 0.0f)/255.0f*63;
	uint16_t b_int = MAX(MIN(mp_obj_get_float(b), 255.0f), 0.0f)/255.0f*31;
	uint16_t c = (r_int << 11) | (g_int << 5) | b_int;
	return mp_obj_new_int(c);
}
static MP_DEFINE_CONST_FUN_OBJ_3(graphics_color_obj, graphics_color);

static mp_obj_t graphics_text(mp_obj_t text_obj, mp_obj_t color, mp_obj_t xys) {
	size_t len = 0;
	const char* text = mp_obj_str_get_data(text_obj, &len);
	uint16_t c = mp_obj_get_int(color);

	size_t tlen = 0;
	mp_obj_t *tuple;
	mp_obj_tuple_get(xys, &tlen, &tuple); 
	assert(tlen == 3);
	uint16_t sx = mp_obj_get_int(tuple[0]);
	uint16_t sy = mp_obj_get_int(tuple[1]);
	uint8_t s = mp_obj_get_int(tuple[2]);
	uint16_t size_mult = 8 * MIN(MAX(s, 1), 3);
	uint16_t w = len * size_mult;
	uint16_t h = size_mult;


	for(uint16_t i = 0; i < len; len++) {
		uint64_t ch = font[(uint8_t)text[i]];
		for(uint16_t y = 0; y < h; y++) {
			for(uint16_t x = 0; x < size_mult; x++) {
				switch (s) {
					default:
						if(ch >> (8*y + x) % 2 != 0)
							write_pixel(c, sx+x+i*8, sy+y);
						break;
					case 2:
						break;
					case 3:
						break;
				}
			}
		}
	}

	update_region(sx, sy, w, h);

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_3(graphics_text_obj, graphics_text);

static const mp_rom_map_elem_t graphics_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_graphics) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&graphics_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_start), MP_ROM_PTR(&graphics_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_finish), MP_ROM_PTR(&graphics_finish_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill), MP_ROM_PTR(&graphics_fill_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&graphics_pixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill_rect), MP_ROM_PTR(&graphics_fill_rect_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill_circle), MP_ROM_PTR(&graphics_fill_circle_obj) },
    { MP_ROM_QSTR(MP_QSTR_color), MP_ROM_PTR(&graphics_color_obj) },
    { MP_ROM_QSTR(MP_QSTR_text), MP_ROM_PTR(&graphics_text_obj) },
};
static MP_DEFINE_CONST_DICT(graphics_module_globals, graphics_module_globals_table);

const mp_obj_module_t graphics_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&graphics_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_graphics, graphics_module);