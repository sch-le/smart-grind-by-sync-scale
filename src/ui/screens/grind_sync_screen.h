#pragma once
#include <lvgl.h>
#include "../../config/constants.h"

class GrindSyncScreen {
  private:
    lv_obj_t* screen;
    lv_obj_t* status_label;
    lv_obj_t* recipe_label;
    lv_obj_t* adjust_grind_size_label;
    lv_obj_t* adjust_grind_size_accept_button;
    bool visible;
  
  public:
    void create();
    void show();
    void hide();
    bool is_visible() const { return visible; }
    lv_obj_t* get_screen() const { return screen; }
    void update_recipe(float weight, float time);
    void hide_recipe();
    void show_recipe();
    void update_adjust_grind_size(float rotation);
    void hide_adjust_grind_size();
    void show_adjust_grind_size();
    lv_obj_t* get_adjust_grind_size_accept_button() const { return adjust_grind_size_accept_button; }
    
    void update_status(const char* status);
};