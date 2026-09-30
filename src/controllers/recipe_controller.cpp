#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <Arduino.h>
#include <string.h>
#include "config/logging.h"

#include "recipe_controller.h"

#define COMM_TIMEOUT_MS 1000u   /* ACK-Timeout [ms] */
#define COMM_MAX_TX_RETRIES 5u  /* max. retries after timeout */

#define SERVICE_ID_RECIPE           0x00u
#define SERVICE_ID_RECIPE_FINISHED  0x01u
#define SERVICE_ID_ACK              0xFFu

/* send and receive callback functions */
static void OnDataSent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status);
static void OnDataRecv(const esp_now_recv_info_t *rx_info, const uint8_t *data, int len);

RecipeController *_instance;

void RecipeController::init(Preferences* preferences)
{
  prefs = preferences;

  LOG_BLE("Initializing RecipeController...\n");
  WiFi.mode(WIFI_STA);
  WiFi.STA.begin();
  LOG_BLE("  - WiFi initialized!\n");

  if (esp_now_init() != ESP_OK) { return; }
  LOG_BLE("  - esp_now initialized!\n");

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  LOG_BLE("  - esp_now callbacks registered!\n");

  memset(&peerInfo, 0u, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, peerAddress, sizeof(peerInfo.peer_addr));
  esp_now_add_peer(&peerInfo);
  LOG_BLE("  - esp_now peer added!\n");

  // read MAC address for debugging purposes
  // #TODO: implement a function for auto-detecting the peer MAC address
  //        so that it doest not have to be hardcoded in the source code
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("  - MAC:%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("  - Failed to read MAC address");
  }

  eCommStatus = RecipeCommStatus::IDLE;
  u8TxLen = 0u;
  u8TxRetries = 0u;
  u32TimeOld = 0u;
  u8RxLen = 0u;
  
  target_weight_mg = prefs->getInt("recipe_weightmg", 0);
  target_time_ms = prefs->getInt("recipe_time_ms", 0);
  LOG_BLE("  - extraction recipe read:\n    - weight: %u, time: %u\n", target_weight_mg, target_time_ms);

  _instance = this;
  
  LOG_BLE("RecipeController initialized successfully!\n");
}

bool RecipeController::send_recipe(Recipe * psRecipe, bool finished)
{
  if (psRecipe == nullptr) { return false; }

  /* copy data to tx buffer */
  if (finished) {
    abyTxBuffer[0u] = (uint8_t)SERVICE_ID_RECIPE_FINISHED;
  } else {
    abyTxBuffer[0u] = (uint8_t)SERVICE_ID_RECIPE;
  }
  memcpy(&abyTxBuffer[1u], psRecipe, sizeof(Recipe));
  u8TxLen = sizeof(Recipe) + 1u;
  
  /* set tx state */
  u8TxRetries = 0u;
  eCommStatus = RecipeCommStatus::SEND;
  return true;
}

bool RecipeController::is_recipe_available(void)
{
  if (u8RxLen > 0u)
  {
    if (abyRxBuffer[0u] == SERVICE_ID_RECIPE)
    { return true; }
  }
  return false;
}

bool RecipeController::receive_recipe(Recipe * psRecipe, bool * finished) 
{
if ((psRecipe == nullptr) || (finished == nullptr)) { return false; }

  if (!is_recipe_available()) { return false; }
  memcpy(psRecipe, &abyRxBuffer[1u], sizeof(Recipe));
  *finished = (abyRxBuffer[0u] == SERVICE_ID_RECIPE_FINISHED);

  /* send ACK back, #TODO: senden kann an dieser stelle im callback noch fehlschalgen, hier müsste eigentlich nochmal überprüft werden */
  uint8_t ack = SERVICE_ID_ACK;
  esp_now_send((uint8_t *)peerAddress, &ack, 1u);

  /* clear rx buffer */
  u8RxLen = 0u;
  return true;
}

RecipeStatus RecipeController::cycle(void)
{
  switch (eCommStatus)
  {
    case RecipeCommStatus::IDLE:
    {
      return RecipeStatus::OK;
    }

    case RecipeCommStatus::SEND:
    {
      esp_err_t eRes = esp_now_send((uint8_t *)peerAddress, abyTxBuffer, u8TxLen);
      if (eRes == ESP_OK)
      {
        u32TimeOld = millis();
        eCommStatus = RecipeCommStatus::ACK_WAIT;
        return RecipeStatus::BUSY;
      }
      else
      {
        LOG_BLE("esp_now_send failed with error code 0x%x\n", (int)eRes);
        eCommStatus = RecipeCommStatus::ERROR;
        return RecipeStatus::ERROR;
      }
    }

    case RecipeCommStatus::ACK_WAIT:
    {
      /* check for incoming ACK */
      if (u8RxLen > 0u)
      {
        if (abyRxBuffer[0u] == SERVICE_ID_ACK)
        {
          /* consume ACK */
          u8RxLen = 0u;
          eCommStatus = RecipeCommStatus::IDLE;
          u8TxRetries = 0u;
          u32TimeOld = 0u;
          return RecipeStatus::OK;
        }
        else
        {
          /* ignore other incoming packets while waiting */
          u8RxLen = 0u;
        }
      }

      /* timeout */
      if ((u32TimeOld != 0u) && ((millis() - u32TimeOld) > COMM_TIMEOUT_MS))
      {
        u8TxRetries++;
        if (u8TxRetries > COMM_MAX_TX_RETRIES)
        {
          eCommStatus = RecipeCommStatus::ERROR;
          return RecipeStatus::ERROR;
        }

        /* retry */
        eCommStatus = RecipeCommStatus::SEND;
        return RecipeStatus::BUSY;
      }

      return RecipeStatus::BUSY;
    }

    case RecipeCommStatus::ERROR:
    default:
    {
      /* clear state and buffers */
      u8TxLen = 0u;
      u8TxRetries = 0u;
      u32TimeOld = 0u;
      u8RxLen = 0u;
      eCommStatus = RecipeCommStatus::IDLE;
      return RecipeStatus::ERROR;
    }
  }

  return RecipeStatus::ERROR;
}

void RecipeController::save_target_recipe(void)
{
    if (prefs) {
        prefs->putInt("recipe_weightmg", target_weight_mg);
        prefs->putInt("recipe_time_ms", target_time_ms);
    }
}

static void OnDataSent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{
  (void)tx_info;

  if (status != ESP_NOW_SEND_SUCCESS)
  {
    /* mark error so CycleRecipe can handle retries */
    _instance->eCommStatus = RecipeController::RecipeCommStatus::ERROR;
  }
}

static void OnDataRecv(const esp_now_recv_info_t *rx_info, const uint8_t *data, int len)
{
  (void)rx_info;
  
  if ((len > 0) && (len <= (int)sizeof(_instance->abyRxBuffer))) {
    memcpy(_instance->abyRxBuffer, data, len);
    _instance->u8RxLen = len;
  }
}