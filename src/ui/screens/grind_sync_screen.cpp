#include "grind_sync_screen.h"
#include <Arduino.h>
#include "../ui_helpers.h"

void GrindSyncScreen::create() {
  screen = lv_obj_create(lv_scr_act());
  lv_obj_set_size(screen, LV_PCT(100), LV_PCT(80));
  lv_obj_align(screen, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_TRANSP, 0); // Keep transparent
  lv_obj_set_style_border_width(screen, 0, 0);
  lv_obj_set_style_pad_all(screen, 0, 0);
  lv_obj_add_flag(screen, LV_OBJ_FLAG_CLICKABLE); // Make the parent screen container clickable

  // Use flex layout for centering
  lv_obj_set_layout(screen, LV_LAYOUT_FLEX);
  lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(screen, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_pad_gap(screen, 15, 0);

  // Satus lable
  status_label = lv_label_create(screen);
  lv_label_set_text(status_label, "Sending\nRecipe...");
  lv_obj_set_style_text_font(status_label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(status_label, lv_color_hex(THEME_COLOR_SECONDARY), 0);
  lv_obj_set_style_text_align(status_label, LV_TEXT_ALIGN_CENTER, 0);

  // Recipe label
  recipe_label = lv_label_create(screen);
  lv_label_set_text(recipe_label, "Weight: 0.0 g\nTime: 0.0 s");
  lv_obj_set_style_text_font(recipe_label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(recipe_label, lv_color_hex(THEME_COLOR_TEXT_PRIMARY), 0);
  lv_obj_set_style_text_align(recipe_label, LV_TEXT_ALIGN_CENTER, 0);

  // Adjust grind size label
  adjust_grind_size_label = lv_label_create(screen);
  lv_label_set_text(adjust_grind_size_label, "Sug. Adj.: 0.0 °");
  lv_obj_set_style_text_font(adjust_grind_size_label, &lv_font_montserrat_32, 0);
  lv_obj_set_style_text_color(adjust_grind_size_label, lv_color_hex(THEME_COLOR_TEXT_PRIMARY), 0);
  lv_obj_set_style_text_align(adjust_grind_size_label, LV_TEXT_ALIGN_CENTER, 0);

  // Accept button for grind size adjustment, styled like the grind button.
  // Created on the active screen (not the flex container) so it can sit at the
  // bottom right, paired with the grind button at the bottom left.
  adjust_grind_size_accept_button = lv_btn_create(lv_scr_act());
  lv_obj_set_size(adjust_grind_size_accept_button, 100, 100);
  lv_obj_align(adjust_grind_size_accept_button, LV_ALIGN_BOTTOM_MID, 60, -10);
  lv_obj_set_style_radius(adjust_grind_size_accept_button, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(adjust_grind_size_accept_button, lv_color_hex(THEME_COLOR_SUCCESS), 0);
  lv_obj_set_style_border_width(adjust_grind_size_accept_button, 0, 0);
  lv_obj_set_style_shadow_width(adjust_grind_size_accept_button, 0, 0);

  lv_obj_t* accept_icon = lv_img_create(adjust_grind_size_accept_button);
  lv_img_set_src(accept_icon, LV_SYMBOL_OK);
  lv_obj_center(accept_icon);
  lv_obj_set_style_text_font(accept_icon, &lv_font_montserrat_32, 0);

  lv_obj_add_flag(adjust_grind_size_accept_button, LV_OBJ_FLAG_HIDDEN);
}

void GrindSyncScreen::show() {
  if (screen) {
    lv_obj_clear_flag(screen, LV_OBJ_FLAG_HIDDEN);
    visible = true;
  }
}

void GrindSyncScreen::hide() {
  if (screen) {
    lv_obj_add_flag(screen, LV_OBJ_FLAG_HIDDEN);
    hide_adjust_grind_size();
    visible = false;
  }
}

void GrindSyncScreen::update_recipe(float weight, float time) {
  if (screen) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Weight: %.1f g\nTime: %.1f s", weight, time);
    lv_label_set_text(recipe_label, buffer);
  }
}

void GrindSyncScreen::hide_recipe() {
  if (recipe_label) {
    lv_obj_add_flag(recipe_label, LV_OBJ_FLAG_HIDDEN);
  }
}

void GrindSyncScreen::show_recipe() {
  if (recipe_label) {
    lv_obj_clear_flag(recipe_label, LV_OBJ_FLAG_HIDDEN);
  }
}

void GrindSyncScreen::update_status(const char* status) {
  if (screen) {
    lv_label_set_text(status_label, status);
  }
}

void GrindSyncScreen::update_adjust_grind_size(float rotation) {
  if (screen) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "Sug. Adj.: %.1f °", rotation);
    lv_label_set_text(adjust_grind_size_label, buffer);
  }
}

void GrindSyncScreen::hide_adjust_grind_size() {
  if (adjust_grind_size_label) {
    lv_obj_add_flag(adjust_grind_size_label, LV_OBJ_FLAG_HIDDEN);
  }
  if (adjust_grind_size_accept_button) {
    lv_obj_add_flag(adjust_grind_size_accept_button, LV_OBJ_FLAG_HIDDEN);
  }
}

void GrindSyncScreen::show_adjust_grind_size() {
  if (adjust_grind_size_label) {
    lv_obj_clear_flag(adjust_grind_size_label, LV_OBJ_FLAG_HIDDEN);
  }
  if (adjust_grind_size_accept_button) {
    lv_obj_clear_flag(adjust_grind_size_accept_button, LV_OBJ_FLAG_HIDDEN);
  }
}
