#include <Arduino.h>
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_mac.h"

// Tiny Keccak-256 implementation for ESP32
// Based on https://github.com/coruus/keccak-tiny
extern "C" {
  void keccak256(const uint8_t* input, size_t len, uint8_t output[32]);
}

// Keccak-256 state permutation (compact implementation)
#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))

static void keccakf(uint64_t st[25]) {
  static const uint64_t keccakf_rndc[24] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
  };
  static const unsigned keccakf_rotc[24] = {
    1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14,
    27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44
  };
  static const unsigned keccakf_piln[24] = {
    10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4,
    15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1
  };

  for (int round = 0; round < 24; round++) {
    uint64_t bc[5], t;
    
    // Theta
    for (int i = 0; i < 5; i++)
      bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];
    
    for (int i = 0; i < 5; i++) {
      t = bc[(i + 4) % 5] ^ ROTL64(bc[(i + 1) % 5], 1);
      for (int j = 0; j < 25; j += 5)
        st[j + i] ^= t;
    }
    
    // Rho Pi
    t = st[1];
    for (int i = 0; i < 24; i++) {
      int j = keccakf_piln[i];
      bc[0] = st[j];
      st[j] = ROTL64(t, keccakf_rotc[i]);
      t = bc[0];
    }
    
    // Chi
    for (int j = 0; j < 25; j += 5) {
      for (int i = 0; i < 5; i++)
        bc[i] = st[j + i];
      for (int i = 0; i < 5; i++)
        st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
    }
    
    // Iota
    st[0] ^= keccakf_rndc[round];
  }
}

void keccak256(const uint8_t* input, size_t len, uint8_t output[32]) {
  uint64_t st[25] = {0};
  const size_t rate = 136; // 1088 bits / 8
  size_t offset = 0;
  
  // Absorb
  while (len >= rate) {
    for (size_t i = 0; i < rate / 8; i++) {
      st[i] ^= ((uint64_t*)input)[i];
    }
    keccakf(st);
    input += rate;
    len -= rate;
  }
  
  // Pad
  uint8_t temp[200] = {0};
  memcpy(temp, input, len);
  temp[len] = 0x01;
  temp[rate - 1] |= 0x80;
  
  for (size_t i = 0; i < rate / 8; i++) {
    st[i] ^= ((uint64_t*)temp)[i];
  }
  keccakf(st);
  
  // Squeeze
  memcpy(output, st, 32);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  // 1. Read Base MAC (eFuse-backed)
  uint8_t base_mac[6];
  esp_read_mac(base_mac, ESP_MAC_WIFI_STA);

  // 2. Read chip info
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  // 3. Build identity material (16 bytes)
  uint8_t material[16] = {0};
  memcpy(material, base_mac, 6);
  material[6] = chip_info.model;
  material[7] = chip_info.revision;
  // Remaining 8 bytes stay zero-padded

  // 4. Hash with REAL Keccak-256 (Ethereum-compatible)
  uint8_t hw_id[32];
  keccak256(material, sizeof(material), hw_id);

  // ---- OUTPUT ----
  Serial.println("\n╔════════════════════════════════════════════╗");
  Serial.println("║   anchor ORTHONODE HARDWARE IDENTITY        ║");
  Serial.println("╚════════════════════════════════════════════╝");
  Serial.println();

  Serial.print("Base MAC (eFuse):     ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", base_mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  Serial.printf("Chip Model:           ESP32-S3 (Model %d)\n", chip_info.model);
  Serial.printf("Chip Revision:        %d\n", chip_info.revision);
  Serial.println();

  Serial.println("Hardware Identity (Keccak-256):");
  Serial.print("0x");
  for (int i = 0; i < 32; i++) {
    Serial.printf("%02x", hw_id[i]);
  }
  Serial.println();
  Serial.println();
  
  Serial.println("✅ This identity is ready for on-chain authorization");
  Serial.println("   Copy the hex string above and use authorize_node()");
  Serial.println();
}

void loop() {
  // Nothing - identity extraction is one-time
}