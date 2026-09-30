#pragma once
#include <lvgl.h>
#include "../../controllers/recipe_controller.h"

class UIManager;
enum class UIState;

// Controlls recipe communication, including sending and receiving recipes, handling timeouts, and updating the UI accordingly

class GrindSyncUIController {
  public:
    explicit GrindSyncUIController(UIManager* manager);

    void build_controls();
    void register_events();

    void on_state_changed(UIState new_state);
    void update(UIState current_state);

  private:
    void handle_grind_size_adjust_accept();

    static GrindSyncUIController* instance_;
    UIManager* ui_manager_;
    RecipeStatus recipe_status_;
    Recipe recipe_feedback_;
    uint32_t last_recipe_send_time_ms_;
};