# Stylus Hardware Anchor (SHA)

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Arbitrum](https://img.shields.io/badge/Arbitrum-Stylus-blue.svg)
![Rust](https://img.shields.io/badge/Rust-WASM-orange.svg)
![ESP32](https://img.shields.io/badge/Hardware--Bound-ESP32--S3-green.svg)
![Sepolia](https://img.shields.io/badge/Deployed-Arbitrum%20Sepolia-blue.svg)

> **Hardware truth as a primitive. Silicon identity on-chain. No TEE. No secure element. Just a $5 chip.**

---

## The Problem Nobody Has Fixed

DePIN networks assume their nodes are real hardware. But nobody proves it.

Right now, anyone can spin up a hundred virtual machines, register a hundred identities, and farm rewards — and the chain has no idea. Software identities are trivially faked. Serial numbers can be cloned. Private keys can be copied. ZKPs prove computation correctness, not that the source data is real.

**The gap is at the silicon level.**

---

## What SHA Does

Stylus Hardware Anchor (SHA) is a reusable hardware identity verification primitive for Arbitrum Stylus smart contracts. It cryptographically binds the immutable eFuse identifiers inside an ESP32-S3 microcontroller to on-chain execution logic.

eFuses are burned into the chip during fabrication. They are permanent. They cannot be changed. They cannot be emulated. A virtual machine has no eFuse.

SHA turns that physical fact into an on-chain guarantee.

---

## Why This Is Only Possible on Stylus

Running receipt verification in Solidity is prohibitively expensive. The Keccak reconstruction, counter enforcement, and batch validation logic requires computation that would make per-receipt gas costs unviable at scale.

Stylus changes the economics:

| Batch Size | Total Gas | Gas Per Receipt |
|:---:|---:|---:|
| N=5 | 148,741 | 29,748 |
| N=10 | 202,090 | 20,209 |
| N=20 | 308,387 | 15,419 |
| N=50 | 628,201 | **12,564** |

Gas per receipt drops sharply as batch size grows — native-compiled WASM amortization at work. This is impossible to replicate in Solidity at these costs.

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│              LAYER 1 — SILICON (ESP32-S3)               │
│                                                         │
│  Manufacturer-burned eFuse identifiers                  │
│  Base MAC (6 bytes) + Chip Model (1 byte)               │
│  → Keccak-256 (Ethereum-compatible, 0x01 padding)       │
│  → 32-byte Hardware Identity (HW_ID)                    │
└─────────────────────┬───────────────────────────────────┘
                      │ HW_ID + Firmware Hash + Counter
                      ▼
┌─────────────────────────────────────────────────────────┐
│            LAYER 2 — MIDDLEWARE (Python SDK)            │
│                                                         │
│  Receipt generation and signing                         │
│  Device authorization flows                             │
│  On-chain submission helpers                            │
└─────────────────────┬───────────────────────────────────┘
                      │ Packed receipt blob (bytes)
                      ▼
┌─────────────────────────────────────────────────────────┐
│         LAYER 3 — VERIFICATION (Stylus / Rust / WASM)  │
│                                                         │
│  1. Identity Check:    authorized_nodes[hw_id] == true  │
│  2. Firmware Gate:     approved_firmware[fw_hash] == true│
│  3. Replay Protection: counter > counters[hw_id]        │
│  4. Digest Verify:     Keccak-256 reconstruction        │
│                                                         │
│  → ReplayDetected()   if counter already used           │
│  → DigestMismatch()   if receipt tampered               │
│  → Status 1           if all four checks pass           │
└─────────────────────────────────────────────────────────┘
```

### Receipt Format (117 bytes)

| Field | Size | Value |
|-------|------|-------|
| Protocol ID | 13 bytes | `"anchor_RCT_V1"` |
| Hardware ID | 32 bytes | Keccak-256(eFuse data) |
| Firmware Hash | 32 bytes | Keccak-256(firmware binary) |
| Execution Hash | 32 bytes | Keccak-256(computation result) |
| Counter | 8 bytes | Monotonic uint64 (Big-Endian) |
| **Total** | **117 bytes** | **→ Keccak-256 → 32-byte digest** |

---

## Four On-Chain Guarantees

**Device-Bound Identity** — 1 physical chip = 1 on-chain identity, derived from manufacturer-burned eFuse registers that cannot be changed or cloned.

**Replay Protection** — Monotonic counter enforcement via `ReplayDetected()` custom error. Each receipt can be processed exactly once. The counter state is stored per hardware ID on-chain.

**Firmware Governance** — Execution is restricted to firmware hashes explicitly approved by the contract owner. Unauthorized firmware versions are rejected at the contract level.

**Deterministic Verification** — Ethereum-compatible Keccak-256 (0x01 padding) across all three layers — ESP32 firmware, Python middleware, and Stylus contract — with 10,000+ cross-validated test vectors confirming cryptographic parity.

---

## Live Evidence

| Artifact | Link |
|----------|------|
| Stylus Contract | [`0xD661a1aB8CEFaaCd78F4B968670C3bC438415615`](https://sepolia.arbiscan.io/address/0xD661a1aB8CEFaaCd78F4B968670C3bC438415615) |
| On-chain Activity | [89+ verified transactions on Arbiscan](https://sepolia.arbiscan.io/address/0xD661a1aB8CEFaaCd78F4B968670C3bC438415615) |
| Deploy TX | `0x1a9eaa02f816d86a71f9bf234425e83b5c090d1f3e4f3691851964b71747a489` |
| Activate TX | `0x353d26f4dea36a4410454b7b081cc41610f691dfea7ce29d5c9b1e9aa968f955` |
| WASM sha256 | `4c00997c2bb00e8b786f2ea9d4e3eb87600bf6995bf4e3dd4debf6c473a5bd26` |
| Gas Benchmarks | See [`BENCHMARKS.md`](BENCHMARKS.md) |
| v1.0.0 Release | [View Release](https://github.com/arhantbarmate/stylus-hardware-anchor/releases/tag/v1.0.0) |

---

## Quick Start

### Prerequisites

- Rust + `cargo-stylus` (v0.6.3)
- Python 3.10+
- Foundry (`cast`)
- An Arbitrum Sepolia RPC URL

### Setup

```bash
git clone https://github.com/arhantbarmate/stylus-hardware-anchor
cd stylus-hardware-anchor
cp .env.example .env
# Edit .env with your RPC_URL, CONTRACT_ADDRESS, PRIVATE_KEY, HW_ID, FW_HASH

python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

### Run Gas Benchmarks

```bash
# First run — setup + benchmark
python scripts/run_gas_benchmarks.py --setup --batch-fn bitset --sizes "1,5,10,20"

# Subsequent runs — benchmark only (state already initialized)
python scripts/run_gas_benchmarks.py --batch-fn bitset --sizes "1,5,10,20,50"
```

> **Note:** Always use `--setup` on the first run or after contract redeployment. The setup phase runs `initialize()`, `authorizeNode()`, and `approveFirmware()` to prepare contract state. Without it, all verification calls will revert.

### Key Cast Commands

```bash
# Read contract state
cast call $CONTRACT "isNodeAuthorized(bytes32)(bool)" $HW_ID --rpc-url $RPC_URL
cast call $CONTRACT "isFirmwareApproved(bytes32)(bool)" $FW_HASH --rpc-url $RPC_URL
cast call $CONTRACT "getCounter(bytes32)(uint64)" $HW_ID --rpc-url $RPC_URL

# Admin setup (owner only)
cast send $CONTRACT "authorizeNode(bytes32)" $HW_ID --rpc-url $RPC_URL --private-key $PK
cast send $CONTRACT "approveFirmware(bytes32)" $FW_HASH --rpc-url $RPC_URL --private-key $PK
```

> **Important:** Stylus SDK 0.6.x exports camelCase ABI names. Always use `authorizeNode` not `authorize_node`, `verifyReceipt` not `verify_receipt`. Snake_case will produce a different selector and revert with `0x`.

---

## Known Limitations

**Single-call `verifyReceipt` counter synchronization** — The single-call verification path has a counter synchronization issue with the batch path. Batch verification is the primary and recommended interface. This is under investigation and will be addressed in v0.2.

**`ReplayDetected()` after batch runs** — This is correct behavior, not a bug. The monotonic counter advances with each batch, so single-call verification using an already-consumed counter value correctly reverts. This demonstrates SHA's core replay protection working as designed.

**Research prototype** — The current deployed contract at `0xD661a1aB8CEFaaCd78F4B968670C3bC438415615` has not undergone a professional third-party audit. Treat it as a testnet research prototype. Mainnet deployment is deferred to Phase 2.

---

## Technical Challenges Overcome

Building cross-layer cryptographic parity between C firmware, Python middleware, and Rust/WASM contracts required solving several non-obvious problems:

**Keccak-256 vs NIST SHA-3** — Standard ESP32 libraries provide SHA-3 (0x06 padding). Ethereum uses Keccak-256 (0x01 padding). The digests are different. SHA implements a custom Keccak-256 on the ESP32 with Ethereum-compatible padding, validated against 10,000+ cross-layer test vectors.

**Stylus SDK 0.6.x camelCase ABI export** — The SDK exports external method names in camelCase, not snake_case. Calling `authorize_node` computes a different 4-byte selector than `authorizeNode`. Documented in `CAST_CHEATSHEET.md` and fixed in all scripts.

**`ruint` dependency conflict** — Stylus SDK pulled `ruint v1.17.2` requiring unstable `edition2024`. Fixed via `Cargo.toml` git patch forcing `v1.12.3`.

**Unaligned memory access on ESP32** — Direct `uint64_t*` casting in the Keccak implementation caused undefined behavior on strict architectures. Replaced with byte-wise XOR throughout.

See [`DEBUGGING_POSTMORTEM.md`](docs/DEBUGGING_POSTMORTEM.md) and [`TECHNICAL_CHALLENGES.md`](docs/TECHNICAL_CHALLENGES.md) for full details.

---

## Roadmap

### Phase 1 — Current Scope ($49,000 — Under Review)
*Arbitrum Sepolia testnet only — 6 months*

**Milestone 1 — Security Hardening ($24,000)**
- Coverage-guided fuzz testing (≥1M execution cycles via `cargo-fuzz`)
- Input size enforcement and DoS mitigation
- Threat model and attack surface documentation
- Reproducible benchmark scripts

**Milestone 2 — Developer SDK ($25,000)**
- Python SDK (`anchor-verifier`) published to PyPI
- Rust crate (`stylus-hardware-primitives`) published to crates.io
- 3 reference integration templates deployable on Sepolia
- Minimal developer dashboard with gas analytics

### Phase 2 — Future Scope (Separate Grant)
- Professional third-party security audit
- Mainnet deployment
- Hardware reference design
- Ecosystem integrations (DePIN networks on Arbitrum)

---

## Documentation Index

| Document | Contents |
|----------|----------|
| [`SETUP_GUIDE.md`](SETUP_GUIDE.md) | Environment setup and dependencies |
| [`BENCHMARKS.md`](BENCHMARKS.md) | Gas benchmarks with reproducible scripts |
| [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) | Detailed system design |
| [`docs/DEPLOYMENT_GUIDE.md`](docs/DEPLOYMENT_GUIDE.md) | Contract deployment walkthrough |
| [`docs/CAST_CHEATSHEET.md`](docs/CAST_CHEATSHEET.md) | All cast commands with correct ABI names |
| [`docs/DEBUGGING_POSTMORTEM.md`](docs/DEBUGGING_POSTMORTEM.md) | ABI export and revert resolution |
| [`docs/TECHNICAL_CHALLENGES.md`](docs/TECHNICAL_CHALLENGES.md) | Engineering challenges and solutions |
| [`docs/MILESTONE_1.md`](docs/MILESTONE_1.md) | Prototype validation evidence |

---

## Grant Status

- Arbitrum Foundation Stylus Sprint grant: $49,000 — **under review**
- vlayer ecosystem grant: $10,000 — **under review**
- No funding received to date. Pre-revenue infrastructure project.

---

## License

MIT — See [LICENSE](LICENSE)

---

*Built by [Orthonode Infrastructure Labs](https://github.com/arhantbarmate) — hardware-rooted verification for Arbitrum.*
