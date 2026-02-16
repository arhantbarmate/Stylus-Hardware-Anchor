# Stylus Hardware Anchor  Gas Benchmarks

Network: Arbitrum Sepolia (chain-id 421614)
Contract: 0xCb360Be81EC122Bf146ba7212B3C9E4b943a1180
Date: 2026-02-16
Compiler Target: wasm32-unknown-unknown
Build Profile: release

## Evidence linkage

This benchmark is intended to be used alongside the Sepolia evidence bundle for this deployment:

- Commit: 146691c397419d6e5926a61d59590aa051a43ebb
- WASM sha256: f6e7c720630a6fa2e3fd3b42804bed53f1c497ea3e0034f56e26defa57551732
- Deploy tx: 0x540078d91a32502c1e1970221eee090b32ee46607f552a6cfec09b3d5c9aba7d
- Activate tx: 0xe4d28c930fb1a4fc4130f8fdaa37df1c109b740f90048f4ebc345ef6acd05259

## Benchmark: authorizeNode(bytes32)

### Conditions

- Caller: contract owner
- Input: fresh hardware IDs per transaction
- Pattern: cold  warm storage over repeated mapping writes
- Tx type: EIP-1559 (type 2)
- Network: Arbitrum Sepolia

### Transactions (last 5)

| Tx # | Block | Gas Used | Status |
| ---: | ----: | -------: | :----- |
| 1 | 242893851 | 74,927 | Success |
| 2 | 242893811 | 74,942 | Success |
| 3 | 242893772 | 74,953 | Success |
| 4 | 242893732 | 74,970 | Success |
| 5 | 242893701 | 74,971 | Success |

### Statistics

- Average gas: 74,952
- Minimum: 74,927
- Maximum: 74,971
- Variance: 44 gas
- Stability: extremely stable execution profile

### Observations

- Gas usage is consistent across calls.
- No unexpected spikes.
- No reverts.
- Storage write dominates cost (authorized node mapping update).
- Owner check has negligible overhead.
- Custom errors do not materially change gas in the success path.

### Interpretation

For a Stylus WASM contract performing:

- owner authentication
- a mapping write (`authorized_nodes`)
- an error-safe return path

~75k gas is normal and healthy.

A Solidity implementation can sometimes be lower for equivalent storage patterns; Stylus includes additional WASM dispatch/host call overhead. For a prototype phase this delta is acceptable.

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
