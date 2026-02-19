# Main Branch Changes — ZKP Integration Merge Plan

> This document specifies exactly what changes in `main` at each phase merge.
> Nothing merges to main until the phase is tested and working on Sepolia.

---

## Phase 1 → Main (No merge — docs/scaffold only)

Phase 1 does not merge to main. It exists only in `feat/zkp-vlayer-integration`.
The branch is publicly visible and referenced in the vlayer grant application.

---

## Phase 2 → Main (after Sepolia validation)

### Files ADDED to main

```
zkp/
  circuits/
    execution_proof.nr            ← Noir ZK circuit
  contracts/
    IZkVerifier.rs                ← verifier interface trait
    sha_v2_interface.rs           ← SHA v2 ABI spec
  prover/
    generate_proof.py             ← vlayer prover wrapper
  scripts/
    prove_and_submit.py           ← end-to-end submit script
  tests/
    test_zk_verify.rs             ← ZK integration tests

docs/zkp/
  ARCHITECTURE.md
  CIRCUIT_SPEC.md
  ZK_ROADMAP.md
  INTEGRATION.md
  BENCHMARKS_ZK.md               ← gas benchmarks for ZK path

PROGRESS.md
```

### Files MODIFIED in main

#### `contracts/src/lib.rs`
**Change:** Add SHA v2 functions. ALL v1 code preserved, no modifications.

```rust
// ADDITIONS ONLY — existing functions untouched:

// New storage fields (appended to end of storage struct):
zk_verifier: StorageAddress,
zk_mode_enabled: StorageBool,
zk_verify_count: StorageU256,

// New error variants:
ZkVerifierNotSet,
ZkProofInvalid,

// New admin functions:
pub fn set_zk_verifier(&mut self, verifier: Address) -> Result<(), Error>
pub fn set_zk_mode(&mut self, enabled: bool) -> Result<(), Error>

// New verification function:
pub fn verify_receipt_with_zk(
    &mut self,
    hw_id: FixedBytes<32>,
    fw_hash: FixedBytes<32>,
    exec_hash: FixedBytes<32>,
    counter: u64,
    claimed_digest: FixedBytes<32>,
    zk_proof: Bytes,
) -> Result<(), Error>

// New view functions:
pub fn get_zk_verifier(&self) -> Address
pub fn is_zk_mode_enabled(&self) -> bool
pub fn get_zk_verify_count(&self) -> U256
```

#### `README.md`
**Change:** Add ZKP section after existing Architecture Overview.

```markdown
## ZKP Integration (via vlayer)

SHA v2 extends hardware identity with ZK execution correctness proofs.
See [`feat/zkp-vlayer-integration`](https://github.com/arhantbarmate/stylus-hardware-anchor/tree/feat/zkp-vlayer-integration)
and [`docs/zkp/`](docs/zkp/) for full specification.

| Layer | Guarantee |
|-------|-----------|
| Silicon Identity | eFuse → allowlist |
| Firmware Governance | Approved hash gating |
| Replay Protection | Monotonic counter |
| **ZK Execution Proof** | **vlayer circuit verifier** |
```

#### `BENCHMARKS.md`
**Change:** Add ZK path benchmark rows.

```markdown
## Benchmark: ZK-extended receipt verification

| Label | Gas Used | Status | Notes |
|-------|----------|--------|-------|
| verifyReceiptWithZk (single) | TBD | 1 | v1 + ZK verifier |
| verifyReceipts BatchZk N=50  | TBD | 1 | Phase 3 target |
```

#### `ROADMAP.md`
**Change:** Add vlayer ZK section to Phase 2 scope (existing SHA grant Phase 2 or new vlayer grant scope).

---

## Phase 3 → Main (after batch aggregation validated)

### Additional changes:
- `contracts/src/lib.rs` — add `verify_receipts_batch_with_zk()`
- `zkp/circuits/batch_execution_proof.nr` — aggregation circuit
- `BENCHMARKS.md` — batch ZK rows

---

## Phase 4 → Main (future scope)

- Recursive circuit
- Asymptotic gas benchmarks
- DePIN reference integration

---

## Deployment Addresses (to be filled in Phase 2)

| Contract | Network | Address |
|----------|---------|---------|
| SHA v1 | Arbitrum Sepolia | `0xD661a1aB8CEFaaCd78F4B968670C3bC438415615` |
| SHA v2 | Arbitrum Sepolia | TBD (Phase 2 deploy) |
| vlayer Verifier | Arbitrum Sepolia | TBD (from vlayer) |
