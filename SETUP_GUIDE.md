# Stylus Hardware Anchor (SHA)
## Setup & Testing Guide
### Zero to Hero — Windows | macOS | Linux

For the canonical doc index, see `README.md` (Start Here). For copy/paste `cast` commands, see `docs/CAST_CHEATSHEET.md`.

---

## 📋 Introduction

Welcome to the Stylus Hardware Anchor (SHA) deployment guide. This document walks you through deploying and testing hardware verification on the Arbitrum Sepolia testnet (prototype deployment).

### What You'll Learn:

- How to set up your development environment on Windows, macOS, or Linux
- How to flash firmware to your ESP32-S3 hardware
- How to extract your unique hardware identity
- How to authorize your hardware on the Arbitrum blockchain
- How to run verification benchmarks and tests

### ⚠️ Important Notice

This is a **research prototype**, not production infrastructure. The current deployed contract at `0xD661a1aB8CEFaaCd78F4B968670C3bC438415615` has known limitations:

- ✅ **Batch verification** (primary interface) works correctly
- ⚠️ **Single verification** has counter synchronization issues under investigation
- 📊 **Batch verification** is the recommended integration pattern for DePIN use cases

See README.md for full technical details.

---

## 🛠️ What You Need

### Hardware Requirements

- ESP32-S3 development board (any variant with USB support)
- USB-C or Micro-USB cable (depending on your board)
- Computer running Windows, macOS, or Linux

### Software Prerequisites

- Internet connection for downloading tools and testnet ETH
- MetaMask wallet with Arbitrum Sepolia configured
- Terminal/Command Prompt access

### Important Contract Information

The contract address is deployment-specific and should be provided via your `.env`:

- `RPC_URL`
- `CONTRACT_ADDRESS`

---

## Step 1: Get Testnet ETH ⛽

Before you can authorize your hardware on-chain, you need Arbitrum Sepolia ETH for gas fees. This is free testnet currency.

### Recommended Faucets

1. **Google Cloud Faucet**
   - URL: https://cloud.google.com/application/web3/faucet/ethereum/sepolia
   - Requirements: Google account

2. **Sepolia PoW Faucet**
   - URL: https://sepolia-faucet.pk910.de/
   - Requirements: None (browser-based mining)

3. **Chainlink Faucet**
   - URL: https://faucets.chain.link/arbitrum-sepolia
   - Requirements: GitHub or Twitter account

> 💡 **Pro Tip:** Get Sepolia ETH first, then bridge it to Arbitrum Sepolia using the official Arbitrum bridge at bridge.arbitrum.io if needed.

---

## Step 2: Environment Setup 🖥️

Choose your operating system and follow the corresponding instructions:

### 🪟 Windows Setup

#### A. Install Python 3.x

1. Download Python from [python.org/downloads](https://python.org/downloads)
2. During installation, check **"Add Python to PATH"**
3. Verify installation by opening Command Prompt and typing:

```bash
python --version
```

#### B. Install PlatformIO Core

Open Command Prompt as Administrator and run:

```bash
pip install platformio
```

#### C. Install USB Drivers

Most ESP32-S3 boards use CP210x or CH340 USB-to-serial chips. Download and install:

- **CP210x Driver:** https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
- **CH340 Driver:** https://sparks.gogo.co.nz/ch340.html

#### D. Setup Python Virtual Environment

Create a dedicated project folder and virtual environment:

```bash
mkdir stylus-hardware-anchor
cd stylus-hardware-anchor
python -m venv venv
venv\Scripts\activate
```

Clone the repository and install dependencies:

```bash
git clone https://github.com/arhantbarmate/stylus-hardware-anchor .
pip install -r requirements.txt
pip install -r requirements-dev.txt
```

> 💡 **Important:** Always activate the virtual environment before running scripts:
> ```bash
> venv\Scripts\activate  # Windows
> source venv/bin/activate  # macOS/Linux
> ```

---

### 🍎 macOS Setup

#### A. Install Homebrew (if not installed)

Open Terminal and run:

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

#### B. Install Python & PlatformIO

```bash
brew install python3 platformio
```

#### C. Install USB Drivers (if needed)

For CH340-based boards, you may need:

```bash
brew install --cask wch-ch34x-usb-serial-driver
```

#### D. Setup Python Virtual Environment

```bash
mkdir stylus-hardware-anchor && cd stylus-hardware-anchor
python3 -m venv venv
source venv/bin/activate
```

Clone the repository and install dependencies:

```bash
git clone https://github.com/arhantbarmate/stylus-hardware-anchor .
pip install -r requirements.txt
pip install -r requirements-dev.txt
```

> 💡 **Important:** Always activate the virtual environment before running scripts:
> ```bash
> source venv/bin/activate  # macOS/Linux
> ```

---

### 🐧 Linux Setup

#### A. Install Dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y python3 python3-pip python3-venv git
pip3 install platformio
```

#### B. Add User to dialout Group

This gives you permission to access USB serial ports without sudo:

```bash
sudo usermod -a -G dialout $USER
```

> ⚠️ **Important:** Log out and log back in for this change to take effect.

#### C. Setup Python Virtual Environment

```bash
mkdir stylus-hardware-anchor && cd stylus-hardware-anchor
python3 -m venv venv
source venv/bin/activate
```

Clone the repository and install dependencies:

```bash
git clone https://github.com/arhantbarmate/stylus-hardware-anchor .
pip install -r requirements.txt
pip install -r requirements-dev.txt
```

> 💡 **Important:** Always activate the virtual environment before running scripts:
> ```bash
> source venv/bin/activate  # Linux
> ```

---

## Step 4: Environment Configuration �

### Set Up Environment Variables

Create a `.env` file with your configuration:

```bash
cp .env.example .env
```

Edit `.env` with your settings:

```bash
# RPC Configuration
RPC_URL=https://arb-sepolia.g.alchemy.com/v2/YOUR_API_KEY

# Contract Address (Arbitrum Sepolia)
CONTRACT_ADDRESS=0xD661a1aB8CEFaaCd78F4B968670C3bC438415615

# Your Private Key (testnet only!)
PRIVATE_KEY=your_private_key_here

# Hardware Identity (from ESP32-S3)
HW_ID=0x52fdfc072182654f163f5f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f

# Firmware Hash
FW_HASH=0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef

# Chain ID
CHAIN_ID=421614
```

> ⚠️ **Security Warning:** Never commit your `.env` file to version control. Use a separate testnet wallet only.

### 📂 Repository Structure:

- `stylus_anchor/` — Stylus smart contract source (Rust)
- `scripts/` — Python authorization and benchmarking tools
- `docs/` — Technical documentation and architecture
- `README.md` — Project overview and quick start
- `BENCHMARKS.md` — Gas benchmark results and methodology

---

## Step 4: Connect Your ESP32-S3 🔌

1. Connect your ESP32-S3 to your computer via USB
2. Identify the serial port:

### Windows

Open Device Manager → Ports (COM & LPT)

Look for **COM3**, **COM4**, etc.

### macOS

Open Terminal and run:

```bash
ls /dev/cu.*
```

Look for `/dev/cu.usbserial-XXXX` or `/dev/cu.usbmodemXXXX` (where XXXX are device-specific characters)

### Linux

Open Terminal and run:

```bash
ls /dev/ttyUSB* /dev/ttyACM*
```

Look for `/dev/ttyUSB0` or `/dev/ttyACM0`

---

## Step 5: Flash the Firmware 📲

Navigate to the firmware directory and build/flash:

### Build & Upload

```bash
cd ohr_firmware
pio run --target upload
```

⏱️ **Expected Time:** 2-5 minutes for first-time build

### What Happens During Flashing

- PlatformIO downloads ESP32-S3 toolchain (first time only)
- Compiles the firmware from source
- Uploads binary to your ESP32-S3 via USB
- Device automatically reboots with new firmware

### ✅ Success Indicator:

You should see output ending with:

```
SUCCESS
```

---

## Step 6: Extract Your Hardware ID 🔑

Your ESP32-S3's unique identity is generated from its eFuse MAC address. Let's extract it:

### Open Serial Monitor

```bash
pio device monitor --baud 115200
```

### What You'll See

The device will output diagnostic information. Look for a line like this:

```
Hardware Identity: 0x52fdfc072182654f163f5f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f
```

> 📋 **Important:** Copy this entire 64-character hexadecimal string. You will need it for authorization.

### Understanding the Hardware ID

This identity is:

- **Unique:** Derived from factory-burned eFuse MAC address
- **Permanent:** Cannot be changed or cloned
- **Deterministic:** Same device always produces the same ID

> 💡 **Pro Tip:** Press `Ctrl+C` (`Cmd+C` on macOS) to exit the serial monitor.

---

## Step 7: Configure Authorization 🔐

Before authorizing your hardware on-chain, you need to set up your wallet credentials:

### Export Your Private Key

1. Open MetaMask
2. Click the three dots → Account Details → Show Private Key
3. Enter your password and copy the key

> ⚠️ **Security Warning:** Never share your private key. This is for testnet only. Use a separate wallet for testing, not your main wallet with real funds.

### Set Environment Variable

**Windows (Command Prompt):**

```bash
set PRIVATE_KEY=abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890
```

**macOS / Linux:**

```bash
export PRIVATE_KEY="abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890"
```

> ⚠️ **Important:** Remove the `0x` prefix from your private key before setting the variable.

---

## Step 8: Authorize Your Hardware On-Chain ⛓️

Now for the moment of truth! Let's register your hardware on the Arbitrum Sepolia blockchain:

### Run Authorization Script

Navigate to the scripts directory:

```bash
cd ../scripts
```

Run the authorization script with your Hardware ID:

```bash
python authorize_hardware.py --hw-id 0x52fdfc072182654f163f5f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f
```

*Replace `YOUR_COPIED_HARDWARE_ID` with the 64-character hex string you copied from the serial monitor.*

### What Happens Next

1. Script connects to Arbitrum Sepolia RPC
2. Creates and signs authorization transaction
3. Submits transaction to the blockchain
4. Waits for transaction confirmation

### Expected Output

```
Transaction sent: 0x1a9eaa02f816d86a71f9bf234425e83b5c090d1f3e4f3691851964b71747a489
Waiting for confirmation...
✅ Hardware Authorized!
Transaction Hash: 0x1a9eaa02f816d86a71f9bf234425e83b5c090d1f3e4f3691851964b71747a489
```

### 🎉 Success!

Your hardware is now authorized on the Arbitrum blockchain!

---

## Step 9: Verify On-Chain 🔍

Let's verify that your hardware authorization was recorded on the blockchain:

### View on Arbiscan

1. Copy your transaction hash from the output
2. Visit [sepolia.arbiscan.io](https://sepolia.arbiscan.io)
3. Paste your transaction hash in the search bar
4. You should see your authorization transaction with status **Success**

### Alternative: View Contract State

You can also view the contract directly:

1. Visit: https://sepolia.arbiscan.io/address/$CONTRACT_ADDRESS
2. Click **"Read Contract"**
3. Query `isNodeAuthorized` with your hardware ID

---

## 🔧 Troubleshooting Guide

### Port Not Detected

**Problem:** ESP32-S3 not showing up in port list

**Solutions:**

- Try a different USB cable (some cables are power-only)
- Install CH340 or CP210x drivers (see Step 2)
- Try a different USB port
- On Linux, ensure you're in the dialout group and logged out/in

---

### Flash Failed / Permission Denied

**Problem:** Upload fails with permission error

**Solutions:**

- **Windows:** Close any serial monitor programs, run as Administrator
- **macOS:** Grant Terminal full disk access in System Preferences → Security
- **Linux:** Verify dialout group membership, use `sudo pio run --target upload` as last resort

---

### Private Key / Web3 Errors

**Problem:** "Invalid private key" or connection errors

**Solutions:**

- Ensure private key has NO `0x` prefix
- Verify environment variable is set: `echo $PRIVATE_KEY` (Unix) or `echo %PRIVATE_KEY%` (Windows)
- Check your internet connection
- Verify you have Arbitrum Sepolia ETH in your wallet

---

### Verification Hash Mismatch

**Problem:** "Digest Mismatch" when verifying receipts

**If you see a mismatch:**

- Ensure domain tag is **13 bytes** (`"anchor_RCT_V1"`) and receipt material is **117 bytes**.
- Keccak-256 must use **Ethereum-compatible padding (0x01)** everywhere (firmware, Python, Stylus).
- Baseline cryptographic parity has been achieved across the stack; report persistent mismatches as bugs.

---

## Step 7: Testing & Benchmarks 🧪

### Prerequisites

Ensure your virtual environment is activated:

```bash
# Windows
venv\Scripts\activate

# macOS/Linux  
source venv/bin/activate
```

### Run Verification Benchmarks

Test the contract verification functionality:

```bash
# Run benchmarks with automatic setup
python scripts/run_gas_benchmarks.py --setup --batch-fn bitset --sizes "1,5,10,20"
```

**Expected Results:**
- ✅ Batch verification: All tests pass (status 1)
- ⚠️ Single verification: May show counter synchronization issues (known limitation)
- 📊 Gas usage: ~12.5k-29.7k gas per receipt (amortized with batch size)

### Test Individual Scripts

Verify individual components:

```bash
# Test hardware authorization
python scripts/authorize_hardware.py --hw-id $HW_ID

# Test receipt verification
python scripts/anchor_verifier.py

# Test receipt generation
python scripts/generate_test_receipt.py
```

### Understanding Test Results

- **Batch verification**: Primary interface, works correctly
- **Single verification**: Known counter synchronization issue under investigation  
- **🔄 Replay Protection**: `verifyReceipt` reverts with `ReplayDetected()` after batch runs — this is correct behavior. The monotonic counter enforces that each receipt can only be processed once, demonstrating SHA's core security feature.
- **Gas benchmarks**: See `BENCHMARKS.md` for detailed results

> 💡 **Note**: Always use `--setup` on first run or after contract redeployment to initialize contract state.

---

## ✅ Success Checklist

You have successfully completed the SHA deployment if:

- ☐ **Environment Setup Complete**
  - All required tools installed (Python, PlatformIO, drivers)

- ☐ **Hardware Connected**
  - ESP32-S3 detected and serial port identified

- ☐ **Firmware Flashed Successfully**
  - Upload completed without errors

- ☐ **Hardware ID Extracted**
  - 64-character hexadecimal identity obtained from serial monitor

- ☐ **Authorization Transaction Confirmed**
  - Transaction hash received and verified on Arbiscan Sepolia

- ☐ **On-Chain Verification**
  - Contract state shows your hardware as authorized

- ☐ **Testing Complete**
  - Benchmarks run successfully with `--setup` flag
  - Batch verification tests pass (status 1)
  - Individual scripts function correctly

### 🎉 Congratulations!

You've successfully deployed hardware verification on Arbitrum!

---

## 🚀 Next Steps & Advanced Topics

### Understanding the Architecture

For a deep dive into how SHA works, read the architecture documentation:

https://github.com/arhantbarmate/stylus-hardware-anchor/blob/main/docs/ARCHITECTURE.md

### Watch the Demo

See SHA in action with our video demonstration:

https://www.loom.com/share/6867ea17f56a458dae9375b29b640882

### Explore the Source Code

Browse the full repository:

- **Smart Contract:** `contracts/src/lib.rs` — Rust/Stylus implementation
- **Firmware:** `ohr_firmware/src/` — ESP32-S3 identity generation
- **Scripts:** `scripts/` — Python authorization tools

### Milestone 2 Preview

The next milestone will introduce:

- Keccak-256 alignment between firmware and verification
- Full cryptographic proof generation and validation
- Enhanced security features and anti-tampering mechanisms
- Gas optimization for future mainnet deployment

---

## 📞 Support & Resources

### Official Links

- **Website:** https://orthonode.xyz
- **GitHub:** https://github.com/arhantbarmate/stylus-hardware-anchor
- **Contract (Sepolia):** `$CONTRACT_ADDRESS` (from your local `.env`)
- **Twitter/X:** [@OrthonodeSys](https://twitter.com/OrthonodeSys)

### Community & Feedback

Found a bug? Have a suggestion? We'd love to hear from you:

- Open an issue on GitHub
- Reach out on Twitter
- Email: infrastructure@orthonode.xyz

### Acknowledgments

This project has been submitted for Arbitrum ecosystem grant consideration. No official endorsement or funding has been confirmed at this time.

---

## 📄 Document Information

**Orthonode Infrastructure Labs**  
Building Hardware Trust Infrastructure for Decentralized Networks

© 2026 Orthonode Infrastructure Labs. All Rights Reserved.

*Document Version: 1.0 | Last Updated: February 2026*
