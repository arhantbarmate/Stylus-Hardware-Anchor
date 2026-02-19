# SHA × vlayer — ZKP Integration Branch

**Branch:** `feat/zkp-vlayer-integration`  
**vlayer Grant Applied:** 2026-02-19  
**SHA Sepolia Deployment:** [`0xD661a1aB8CEFaaCd78F4B968670C3bC438415615`](https://sepolia.arbiscan.io/address/0xD661a1aB8CEFaaCd78F4B968670C3bC438415615)  
**Current Phase:** Phase 1 — Architecture & Interface Design ✅

---

## What This Branch Is

SHA (Stylus Hardware Anchor) binds immutable ESP32-S3 silicon identity to Arbitrum Stylus contracts. It already provides:

| Layer | Guarantee | Status |
|-------|-----------|--------|
| Silicon Identity | eFuse → Keccak → on-chain allowlist | ✅ Live on Sepolia |
| Firmware Governance | Approved firmware hash gating | ✅ Live on Sepolia |
| Replay Protection | Monotonic counter enforcement | ✅ Live on Sepolia |
| **ZK Execution Proof** | **vlayer circuit + Stylus verifier** | 🔄 This branch |

This branch adds **Layer 4**: cryptographic proof that the computation *inside* the device was correct — not just that the device exists.

---

## The Security Gap We Are Closing

**SHA today proves:**
> "This physical device running approved firmware submitted this receipt."

**SHA + vlayer proves:**
> "This physical device running approved firmware **correctly executed this computation** and submitted this receipt."

These are materially different guarantees. The second is required for DePIN oracle networks, hardware-backed compute markets, and any application where execution correctness — not just device authenticity — is the security assumption.

---

## Architecture

```
┌─────────────────────────────────────────────────────────┐
│                    Application Layer                     │
│          (DePIN Sensors, Oracles, HW Custody)           │
└──────────────────────────┬──────────────────────────────┘
                           │
┌──────────────────────────┴──────────────────────────────┐
│                  Stylus Contract (SHA v2)                │
│                                                          │
│  Stage 1: Hardware Identity  ← verify eFuse allowlist   │
│  Stage 2: Firmware Check     ← verify fw_hash approved  │
│  Stage 3: Counter Enforce    ← monotonic replay guard   │
│  Stage 4: ZK Proof Verify    ← vlayer verifier  ◄─ NEW  │
└──────────────────────────┬──────────────────────────────┘
                           │
             ┌─────────────┴─────────────┐
             │                           │
┌────────────┴──────────┐   ┌────────────┴──────────────┐
│   Hardware (ESP32-S3)  │   │    vlayer Prover           │
│                        │   │    (off-chain)             │
│  eFuse → HW_ID         │   │                            │
│  Keccak receipt        │   │  execution_data →          │
│  Sends exec_data       │──▶│  zk_proof (Groth16/PLONK)  │
└────────────────────────┘   └───────────────────────────┘
```

---

## Phase Roadmap

### Phase 1 — Scaffold & Architecture ✅ CURRENT
**Deliverables:** Branch, directory structure, Rust interfaces, circuit spec, docs  
**Goal:** Demonstrate architectural readiness to vlayer grant reviewers  

### Phase 2 — Circuit + SHA v2 Contract ⏳ NEXT
**Deliverables:** Noir execution circuit, SHA v2 Stylus contract with `verify_receipt_with_zk()`, end-to-end testnet flow  
**Goal:** Working ZK verification path on Arbitrum Sepolia  

### Phase 3 — Batch Proof Aggregation ⏳ PENDING
**Deliverables:** Aggregate N proofs → 1 on-chain verify, gas benchmarks  
**Goal:** DePIN-scale throughput  

### Phase 4 — Recursive ZK ⏳ FUTURE
**Deliverables:** Recursive aggregation of 1000+ receipts into 1 proof  
**Goal:** Asymptotic gas reduction for large device networks  

---

## Repository Structure (This Branch)

```
zkp/
├── circuits/               # Noir ZK circuits (Phase 2)
│   └── execution_proof.nr  # Execution correctness circuit
├── contracts/
│   ├── IZkVerifier.rs      # Verifier interface trait ✅
│   └── sha_v2_interface.rs # SHA v2 contract interface ✅
├── prover/                 # Off-chain prover scripts (Phase 2)
├── scripts/
│   └── prove_and_submit.py # End-to-end prove → submit ✅ (scaffold)
└── tests/                  # ZK integration tests (Phase 2)

docs/zkp/
├── ARCHITECTURE.md         # Full ZK architecture spec ✅
├── CIRCUIT_SPEC.md         # Circuit design document ✅
├── INTEGRATION.md          # Step-by-step integration guide ✅
└── ZK_ROADMAP.md           # Detailed phase roadmap ✅
```

---

## Backward Compatibility Guarantee

`verify_receipt()` (SHA v1) is **never modified**.  
`verify_receipt_with_zk()` is **additive only**.  

A feature flag `zk_mode_enabled` allows the owner to:
- Run in SHA-only mode (current behavior)
- Enable ZK-required mode when circuits are hardened

No existing integrations break.

---

## Current Progress Snapshot

| Artifact | Status |
|----------|--------|
| Sepolia deployment (SHA v1) | ✅ Live |
| Gas benchmarks (12.5k–29.7k/receipt) | ✅ Published |
| ≥10,000 test vectors validated | ✅ Complete |
| ZKP branch scaffolded | ✅ This PR |
| IZkVerifier interface | ✅ Defined |
| SHA v2 contract interface | ✅ Defined |
| Noir execution circuit | ⏳ Phase 2 |
| vlayer prover integration | ⏳ Phase 2 |
| Batch aggregation | ⏳ Phase 3 |

---

## Links

- [Main README](../../README.md)
- [Architecture Doc](ARCHITECTURE.md)
- [Circuit Spec](CIRCUIT_SPEC.md)
- [ZK Roadmap](ZK_ROADMAP.md)
- [Integration Guide](INTEGRATION.md)
- [Live Contract on Sepolia](https://sepolia.arbiscan.io/address/0xD661a1aB8CEFaaCd78F4B968670C3bC438415615)
