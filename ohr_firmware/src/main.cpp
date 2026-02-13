/**
 * anchor OHR - Prototype Firmware
 * Deterministic Identity + Replay-Safe Receipt
 * Ethereum-compatible Keccak-256 (0x01 padding)
 */

#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_secure_boot.h"
#include "esp_flash_encrypt.h"
#include "esp_chip_info.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

#include "sha3.h"

static const char *TAG = "anchor_OHR";

// ============================================================================
// PROTOCOL CONSTANTS (FROZEN)
// ============================================================================
#define anchor_HWI_DOMAIN "anchor_OHR_V1"
#define anchor_RCT_DOMAIN "anchor_RCT_V1"
#define anchor_HWI_DOMAIN_LEN 13
#define anchor_RCT_DOMAIN_LEN 13

// ============================================================================
// ETHEREUM KECCAK-256
// ============================================================================
static void anchor_keccak256(const uint8_t *input, size_t len, uint8_t *output) {
    keccak_256(input, len, output);
}

// ============================================================================
// CHIP ID
// ============================================================================
static esp_err_t anchor_get_chip_id(uint8_t chip_id[16]) {
    uint8_t mac[6];
    esp_err_t err = esp_read_mac(mac, ESP_MAC_WIFI_STA);
    if (err != ESP_OK) return err;

    memcpy(chip_id, mac, 6);
    memset(chip_id + 6, 0x00, 10);

    ESP_LOGI(TAG, "✓ Using base MAC for identity");
    return ESP_OK;
}

// ============================================================================
// SECURITY FINGERPRINT (PROTOTYPE)
// ============================================================================
static esp_err_t anchor_get_security_state_fingerprint(uint8_t digest[32]) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    uint8_t temp[32] = {0};
    temp[0] = chip_info.model;
    temp[1] = chip_info.cores;
    temp[2] = chip_info.revision;

    anchor_keccak256(temp, 32, digest);
    return ESP_OK;
}

// ============================================================================
// HARDWARE IDENTITY
// ============================================================================
static esp_err_t anchor_derive_hardware_identity(uint8_t hardware_identity[32]) {

    uint8_t material[128] = {0};
    size_t offset = 0;
    esp_err_t err;

    memcpy(material + offset, anchor_HWI_DOMAIN, anchor_HWI_DOMAIN_LEN);
    offset += anchor_HWI_DOMAIN_LEN;

    uint8_t chip_id[16];
    err = anchor_get_chip_id(chip_id);
    if (err != ESP_OK) return err;

    memcpy(material + offset, chip_id, 16);
    offset += 16;

    bool sb = esp_secure_boot_enabled();
    bool fe = esp_flash_encryption_enabled();

    material[offset++] = sb ? 0x01 : 0x00;
    material[offset++] = fe ? 0x01 : 0x00;

    uint8_t fingerprint[32];
    err = anchor_get_security_state_fingerprint(fingerprint);
    if (err != ESP_OK) return err;

    memcpy(material + offset, fingerprint, 32);
    offset += 32;

    anchor_keccak256(material, offset, hardware_identity);

    memset(material, 0, sizeof(material));
    memset(chip_id, 0, sizeof(chip_id));
    memset(fingerprint, 0, sizeof(fingerprint));

    return ESP_OK;
}

// ============================================================================
// FIRMWARE HASH
// ============================================================================
static esp_err_t anchor_get_firmware_hash(uint8_t firmware_hash[32]) {
    const esp_app_desc_t *app_desc = esp_ota_get_app_description();
    anchor_keccak256(app_desc->app_elf_sha256, 32, firmware_hash);
    return ESP_OK;
}

// ============================================================================
// COUNTER (REPLAY PROTECTION)
// ============================================================================
static esp_err_t anchor_increment_counter(uint64_t *new_counter) {

    nvs_handle_t h;
    esp_err_t err = nvs_open("anchor", NVS_READWRITE, &h);
    if (err != ESP_OK) return err;

    uint64_t val = 0;
    err = nvs_get_u64(h, "counter", &val);

    if (err == ESP_ERR_NVS_NOT_FOUND) {
        val = 0;
    } else if (err != ESP_OK) {
        nvs_close(h);
        return err;
    }

    val++;

    err = nvs_set_u64(h, "counter", val);
    if (err != ESP_OK) {
        nvs_close(h);
        return err;
    }

    err = nvs_commit(h);
    nvs_close(h);
    if (err != ESP_OK) return err;

    *new_counter = val;
    return ESP_OK;
}

// ============================================================================
// RECEIPT GENERATION (117-BYTE PREIMAGE)
// ============================================================================
static esp_err_t anchor_generate_receipt(
    const uint8_t exec_hash[32],
    uint8_t digest[32],
    uint64_t *counter_out
) {

    esp_err_t err;

    uint8_t hw_id[32];
    err = anchor_derive_hardware_identity(hw_id);
    if (err != ESP_OK) return err;

    uint8_t fw_hash[32];
    err = anchor_get_firmware_hash(fw_hash);
    if (err != ESP_OK) return err;

    uint64_t counter;
    err = anchor_increment_counter(&counter);
    if (err != ESP_OK) return err;

    uint8_t material[256] = {0};
    size_t off = 0;

    memcpy(material + off, anchor_RCT_DOMAIN, anchor_RCT_DOMAIN_LEN);
    off += anchor_RCT_DOMAIN_LEN;

    memcpy(material + off, hw_id, 32);
    off += 32;

    memcpy(material + off, fw_hash, 32);
    off += 32;

    memcpy(material + off, exec_hash, 32);
    off += 32;

    uint64_t be = __builtin_bswap64(counter);
    memcpy(material + off, &be, 8);
    off += 8;

    anchor_keccak256(material, off, digest);

    *counter_out = counter;

    memset(material, 0, sizeof(material));
    memset(hw_id, 0, sizeof(hw_id));
    memset(fw_hash, 0, sizeof(fw_hash));

    return ESP_OK;
}

// ============================================================================
// JSON OUTPUT
// ============================================================================
static void print_receipt_json(
    const uint8_t receipt_digest[32],
    const uint8_t hardware_identity[32],
    uint64_t counter
) {
    printf("{\n");
    printf("  \"receipt_digest\": \"0x");
    for(int i=0;i<32;i++) printf("%02x", receipt_digest[i]);
    printf("\",\n");

    printf("  \"hardware_identity\": \"0x");
    for(int i=0;i<32;i++) printf("%02x", hardware_identity[i]);
    printf("\",\n");

    printf("  \"counter\": %llu\n", (unsigned long long)counter);
    printf("}\n");
}

// ============================================================================
// MAIN
// ============================================================================
void setup() {

    ESP_LOGI(TAG, "anchor OHR Prototype Starting");

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    uint8_t hw_id[32];
    if (anchor_derive_hardware_identity(hw_id) != ESP_OK) {
        ESP_LOGE(TAG, "Hardware identity failed");
        return;
    }

    printf("Hardware Identity: 0x");
    for(int i=0;i<32;i++) printf("%02x", hw_id[i]);
    printf("\n");

    uint8_t exec_hash[32] = {
        0xDE,0xAD,0xBE,0xEF,0xCA,0xFE,0xBA,0xBE,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,1
    };

    uint8_t receipt[32];
    uint64_t counter;

    if (anchor_generate_receipt(exec_hash, receipt, &counter) != ESP_OK) {
        ESP_LOGE(TAG, "Receipt generation failed");
        return;
    }

    print_receipt_json(receipt, hw_id, counter);
}

void loop() {}
