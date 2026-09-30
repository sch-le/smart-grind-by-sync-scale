#pragma once
#include <cstdint>
#include <esp_now.h>
#include <Preferences.h>

enum class RecipeStatus {
  ERROR,
  OK,
  BUSY
};

struct Recipe {
  uint32_t weight; /* coffee weight [mg] */
  uint32_t time;   /* extraction time [ms] */
};

class RecipeController {
  public:

    enum class RecipeCommStatus {
      IDLE = 0,
      SEND,
      ACK_WAIT,
      ERROR,
    };

    /* send-buffer */
    uint8_t abyTxBuffer[sizeof(Recipe) + 1u];
    uint8_t u8TxLen = 0u;
    uint8_t u8TxRetries = 0u;

    /* receive-buffer */
    uint8_t abyRxBuffer[sizeof(Recipe) + 1u];
    uint8_t u8RxLen = 0u;

    RecipeCommStatus eCommStatus = RecipeCommStatus::IDLE;

    /**
     * @brief Initialize recipe communiation
     * @param preferences Pointer to preferences to save the extraction recipe
     */
    void init(Preferences* preferences);

    /**
     * @brief Send recipe to peer device
     * @param psRecipe Pointer to STRUCT_RECIPE containing the recipe to be sent
     * @param finished Flag indicating if the extraction is completed (true) or not (false)
     * @return true if the recipe was sent successfully, false otherwise
     */
    bool send_recipe(Recipe * psRecipe, bool finished);
    
    /**
     * @brief Check if recipe is available from peer device
     * @return true if recipe is available, false otherwise
     */
    bool is_recipe_available(void);
    
    /**
     * @brief Receive recipe from peer device
     * @param psRecipe Pointer to Recipe where the received recipe will be stored
     * @param finished Flag indicating if the extraction is completed (true) or not (false)
     * @return true if recipe was received successfully, false otherwise
     */
    bool receive_recipe(Recipe * psRecipe, bool * finished);
    
    /**
     * @brief Cycle recipe communication state machine
     * @return ENUM_RECIPE_STATUS indicating the current status of the recipe communication
     */
    RecipeStatus cycle(void);

    uint32_t get_target_weight_mg (void) { return target_weight_mg; }
    void set_target_weight_mg (uint32_t value){ target_weight_mg = value; }
    uint32_t get_target_time_ms (void) { return target_time_ms; }
    void set_target_time_ms (uint32_t value) { target_time_ms = value; }
    void save_target_recipe(void);


  private:

    const uint8_t peerAddress[6] = {0xE4, 0xB0, 0x63, 0xAE, 0x43, 0x68};
    esp_now_peer_info_t peerInfo;
    uint32_t u32TimeOld = 0u;
    Preferences* prefs = nullptr;
    uint32_t target_weight_mg = 0u;
    uint32_t target_time_ms = 0u;
};