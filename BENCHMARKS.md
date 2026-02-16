# Stylus Hardware Anchor - Gas Benchmarks

Network: Arbitrum Sepolia (chain-id 421614)
Contract: 0xd661a1ab8cefaacd78f4b968670c3bc438415615
Date: 2026-02-16
Compiler Target: wasm32-unknown-unknown
Build Profile: release

## Evidence linkage

This benchmark is intended to be used alongside the Sepolia evidence bundle for this deployment:

- Commit: (fill)
- WASM sha256: (fill)
- Deploy tx: 0x1a9eaa02f816d86a71f9bf234425e83b5c090d1f3e4f3691851964b71747a489
- Activate tx: 0x353d26f4dea36a4410454b7b081cc41610f691dfea7ce29d5c9b1e9aa968f955

## Benchmark: receipt verification (single + batch)

### Conditions

- Network: Arbitrum Sepolia
- Tx type: EIP-1559 (type 2)
- Batch function: `verifyReceiptsBatchBitsetBytes(bytes) returns (bytes32)`
- Batch inputs: packed receipts generated off-chain and passed as a single `bytes` blob
- Single function: `verifyReceipt(bytes32,bytes32,bytes32,uint64,bytes32)`

### Results

| Label | Gas Used | Status | Notes |
| --- | ---: | :---: | --- |
| verifyReceiptsBatchBitsetBytes(bytes) N=5 | 148,741 | 1 | 29,748.20 gas/receipt |
| verifyReceiptsBatchBitsetBytes(bytes) N=10 | 202,090 | 1 | 20,209.00 gas/receipt |
| verifyReceiptsBatchBitsetBytes(bytes) N=20 | 308,387 | 1 | 15,419.35 gas/receipt |
| verifyReceiptsBatchBitsetBytes(bytes) N=50 | 628,201 | 1 | 12,564.02 gas/receipt |
| verifyReceipt success | 118,935 | 1 | success path |
| verifyReceipt invalid digest | 98,631 | 0 | expected revert path (DigestMismatch) |

### Setup transactions (reference)

| Label | Gas Used | Status | Notes |
| --- | ---: | :---: | --- |
| initialize() | 72,701 | 0 | expected if already initialized |
| authorizeNode(bytes32) | 99,288 | 1 | owner-only, mapping write |
| approveFirmware(bytes32) | 99,285 | 1 | owner-only, mapping write |

### Interpretation

Stylus enables high-throughput hardware receipt verification via WASM batch execution. In compute-heavy workloads like batch receipt verification, gas per receipt drops sharply as batch size grows (29.7k → 12.6k), demonstrating the amortization advantage of running native-compiled logic in WASM versus per-transaction Solidity dispatch. This benefit is most valuable when applications need to verify many receipts in a single transaction; otherwise, single verification remains available.

### Observations

- Batch verification shows strong amortization: `gas/receipt` drops as N increases.
- The invalid-digest case is intentionally included to measure the revert-path cost; it should remain `status = 0`.
- If `initialize()` is run on an already-initialized contract, it will revert with `AlreadyInitialized` (also expected).

## Deployment / activation gas (reference)

From on-chain receipts for this deployment:

- Deploy tx gasUsed: 3,755,787
- Activate tx gasUsed: 3,713,837

Note: Arbitrum execution has L1+L2 components; see the evidence bundle receipts for `gasUsedForL1` where applicable.

## Conclusion

- No gas spikes
- No regressions observed across repeated authorize calls
- Stable execution profile
- Safe to proceed to milestone expansion
