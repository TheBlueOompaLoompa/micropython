#include "py/runtime.h"
#include "hardware/spi.h"

static mp_obj_t graphics_init(mp_obj_t a_obj, mp_obj_t b_obj) {
    return true;
}
static MP_DEFINE_CONST_FUN_OBJ_2(graphics_add_ints_obj, graphics_init);

static const mp_rom_map_elem_t graphics_module_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_graphics) },
    { MP_ROM_QSTR(MP_QSTR_add_ints), MP_ROM_PTR(&graphics_add_ints_obj) },
};
static MP_DEFINE_CONST_DICT(graphics_module_globals, graphics_module_globals_table);

const mp_obj_module_t graphics_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&graphics_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_graphics, graphics_module);
