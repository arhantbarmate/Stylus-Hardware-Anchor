# Cast Cheatsheet (StylusHardwareAnchor)

This project uses a **Stylus (Rust → WASM)** contract, but it still exposes a standard **EVM ABI**. Tooling like `cast` must call the **exported ABI names**, which for Stylus SDK 0.6.x are **camelCase**.

## Prerequisites

- A repo root `.env` file (copy from `.env.example`)
- A local private key file (example: `stylus_anchor/stylus_hardware_anchor/deployer.key`) **must not be committed**

The `.env` file should define:

- `RPC_URL`
- `CONTRACT_ADDRESS`

## Recommended session setup (WSL)

```bash
export RPC="${RPC_URL:-https://sepolia-rollup.arbitrum.io/rpc}"
export CONTRACT="${CONTRACT_ADDRESS}"

# If you use a key file:
export PK="0x$(cat stylus_anchor/stylus_hardware_anchor/deployer.key | tr -d '\r\n' | sed 's/^0x//')"
```

Sanity:

```bash
echo $RPC
echo $CONTRACT
cast wallet address --private-key "$PK"
cast call $CONTRACT "getOwner()(address)" --rpc-url $RPC
```

## Read-only calls

```bash
cast call $CONTRACT "getOwner()(address)" --rpc-url $RPC

cast call $CONTRACT "isNodeAuthorized(bytes32)(bool)" 0x<32-byte> --rpc-url $RPC
cast call $CONTRACT "isFirmwareApproved(bytes32)(bool)" 0x<32-byte> --rpc-url $RPC
cast call $CONTRACT "getCounter(bytes32)(uint64)" 0x<32-byte> --rpc-url $RPC
```

## Owner/admin transactions

```bash
cast send $CONTRACT "initialize()" --rpc-url $RPC --private-key "$PK" -v

cast send $CONTRACT "authorizeNode(bytes32)" 0x<32-byte> --rpc-url $RPC --private-key "$PK" -v
cast send $CONTRACT "revokeNode(bytes32)" 0x<32-byte> --rpc-url $RPC --private-key "$PK" -v

cast send $CONTRACT "approveFirmware(bytes32)" 0x<32-byte> --rpc-url $RPC --private-key "$PK" -v
cast send $CONTRACT "revokeFirmware(bytes32)" 0x<32-byte> --rpc-url $RPC --private-key "$PK" -v

cast send $CONTRACT "transferOwnership(address)" 0x<newOwner> --rpc-url $RPC --private-key "$PK" -v
```

## Receipt verification transaction

```bash
cast send $CONTRACT "verifyReceipt(bytes32,bytes32,bytes32,uint64,bytes32)" \
  0x<hw_id> \
  0x<fw_hash> \
  0x<exec_hash> \
  <counter_uint64> \
  0x<claimed_digest> \
  --rpc-url $RPC --private-key "$PK" -v
```

## Batch receipt verification (new)

```bash
# Generate packed receipts off-chain (see scripts/benchmark_receipts.py)
# Then submit as a single bytes blob
cast send $CONTRACT "verifyReceiptsBatchBitsetBytes(bytes)" \
  0x<packed_receipts_blob> \
  --rpc-url $RPC \
  --private-key $PK -v
```

## ABI export (Stylus SDK 0.6.x)

```bash
cd stylus_anchor/stylus_hardware_anchor
cargo stylus export-abi --rust-features export-abi > abi.json
```

`abi.json` is a Solidity `interface` representation of the ABI. This does **not** mean the contract is Solidity.
