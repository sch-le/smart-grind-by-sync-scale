#include "grind_sync_controller.h"

#include <Arduino.h>
#include <cstdio>
#include <cstring>

#include "../ui_manager.h"
#include "../../controllers/recipe_controller.h"
#include "../../config/constants.h"

GrindSyncUIController* GrindSyncUIController::instance_ = nullptr;

GrindSyncUIController::GrindSyncUIController(UIManager* manager)
    : ui_manager_(manager) {
    instance_ = this;
}

void GrindSyncUIController::build_controls() {
  if (!ui_manager_) {
    return;
  }

  /* #TODO: nothing to do?*/

}

void GrindSyncUIController::register_events() {
  if (!ui_manager_) {
    return;
  }

  auto accept_btn = ui_manager_->grind_sync_screen.get_adjust_grind_size_accept_button();
  if (accept_btn) {
    lv_obj_add_event_cb(accept_btn, [](lv_event_t* e) {
      if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
      static_cast<GrindSyncUIController*>(lv_event_get_user_data(e))->handle_grind_size_adjust_accept();
    }, LV_EVENT_CLICKED, this);
  }

}

void GrindSyncUIController::update(UIState current_state)
{
  RecipeController* recipe_controller_ = ui_manager_->get_hardware_manager()->get_recipe_controller();
  recipe_status_ = recipe_controller_->cycle();

  switch (current_state)
  {
    case UIState::RECIPE_SENDING:
    {
      if (recipe_status_ == RecipeStatus::OK)
      {
        ui_manager_->switch_to_state(UIState::RECIPE_RECEIVING);
        last_recipe_send_time_ms_ = millis();
      }
      else if (recipe_status_ == RecipeStatus::ERROR)
      {
        ui_manager_->switch_to_state(UIState::RECIPE_TIMEOUT);
      }
      break;
    }
    case UIState::RECIPE_RECEIVING:
    { 
      if (recipe_controller_->is_recipe_available())
      {
        bool finished = false;
        if (recipe_controller_->receive_recipe(&recipe_feedback_, &finished))
        {
          if (finished)
          {
            ui_manager_->switch_to_state(UIState::ADJUSTING_GRIND_SIZE);
          }
          ui_manager_->grind_sync_screen.update_recipe((float)recipe_feedback_.weight / 1000.0f, (float)recipe_feedback_.time / 1000.0f);
        }
        else
        {
          ui_manager_->switch_to_state(UIState::RECIPE_TIMEOUT);
        }
      }

      if ((millis() - last_recipe_send_time_ms_) > RECIPE_TIMEOUT_MS)
      {
        ui_manager_->switch_to_state(UIState::RECIPE_TIMEOUT);
      }

      break;
    }
    case UIState::RECIPE_TIMEOUT:
    {
      break;
    }
    case UIState::ADJUSTING_GRIND_SIZE:
    {
      break;
    }
    default: { break; }
  }
}

void GrindSyncUIController::on_state_changed(UIState new_state) {
  if (!ui_manager_) {
    return;
  }

  RecipeController* recipe_controller_ = ui_manager_->get_hardware_manager()->get_recipe_controller();

  // Hide all elements initially, then show the relevant ones based on the new state
  ui_manager_->grind_sync_screen.hide_recipe();
  ui_manager_->grind_sync_screen.hide_adjust_grind_size();

  switch (new_state) {
    case UIState::RECIPE_SENDING:
    {
      Recipe recipe = { recipe_controller_->get_target_weight_mg(), recipe_controller_->get_target_time_ms() };

      // display target recipe on UI and send to coffee machine
      ui_manager_->grind_sync_screen.update_status("Sending\nRecipe...");
      ui_manager_->grind_sync_screen.update_recipe((float)recipe.weight / 1000.0f, (float)recipe.time / 1000.0f);
      ui_manager_->grind_sync_screen.show_recipe();
      recipe_controller_->send_recipe(&recipe, true);

      break;
    }
    case UIState::RECIPE_RECEIVING:
    {
      // receive recipe from coffee machine
      ui_manager_->grind_sync_screen.update_status("Receiving\nFeedback...");
      ui_manager_->grind_sync_screen.show_recipe();
      break;
    }
    case UIState::RECIPE_TIMEOUT:
    {
      ui_manager_->grind_sync_screen.update_status("Recipe\nTimeout");
      break;
    }
    case UIState::ADJUSTING_GRIND_SIZE:
    {
      ui_manager_->grind_sync_screen.update_status("Adjusting\nGrind Size...");
      ui_manager_->grind_sync_screen.update_recipe((float)recipe_feedback_.weight / 1000.0f, (float)recipe_feedback_.time / 1000.0f);
      ui_manager_->grind_sync_screen.show_recipe();
      ui_manager_->grind_sync_screen.show_adjust_grind_size();
      // #TODO: Grind size adjustment logic, evtl. weiterer Zustand?
      break;
    }
    default: { break; }
  };
}

void GrindSyncUIController::handle_grind_size_adjust_accept() {
  if (!ui_manager_ || !ui_manager_->state_machine) {
    return;
  }

  /* #TODO: Grind size adjustment accept logic */
  if (ui_manager_->state_machine->is_state(UIState::ADJUSTING_GRIND_SIZE))
  {
    LOG_BLE("[%lums GRIND_SIZE_ADJUST_ACCEPT] Grind size adjustment accepted\n", millis());
    // #TODO: Implement Logic to adjust grind size based of recipe feedback
  }
}