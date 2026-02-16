#!/usr/bin/env python3

import argparse
import os
from dataclasses import dataclass

from eth_hash.auto import keccak

DOMAIN = b"anchor_RCT_V1"
CHAIN_ID_DEFAULT = 421614
PACKED_V1_LEN = 137


def _strip_0x(s: str) -> str:
    return s[2:] if s.startswith("0x") else s


def _hex32_to_bytes(s: str) -> bytes:
    s = _strip_0x(s)
    if len(s) != 64:
        raise ValueError(f"expected 32-byte hex (64 chars), got {len(s)}")
    return bytes.fromhex(s)


def _u64be(x: int) -> bytes:
    if x < 0 or x >= 2**64:
        raise ValueError("counter must fit into uint64")
    return x.to_bytes(8, "big")


def compute_digest(
    chain_id: int, hw_id: bytes, fw_hash: bytes, exec_hash: bytes, counter: int
) -> bytes:
    material = DOMAIN + _u64be(chain_id) + hw_id + fw_hash + exec_hash + _u64be(counter)
    return keccak(material)


def pack_v1(
    chain_id: int,
    hw_id: bytes,
    fw_hash: bytes,
    exec_hash: bytes,
    counter: int,
    claimed_digest: bytes,
) -> bytes:
    packed = b"\x01" + hw_id + fw_hash + exec_hash + _u64be(counter) + claimed_digest
    if len(packed) != PACKED_V1_LEN:
        raise RuntimeError(f"packed receipt length mismatch: {len(packed)}")
    return packed


def exec_hash_for(counter: int) -> bytes:
    return keccak(b"exec:" + _u64be(counter))


@dataclass
class SingleArgs:
    exec_hash: bytes
    claimed_digest: bytes
    claimed_digest_bad: bytes


def make_single_args(
    chain_id: int, hw_id: bytes, fw_hash: bytes, counter: int
) -> SingleArgs:
    ex = exec_hash_for(counter)
    d = compute_digest(chain_id, hw_id, fw_hash, ex, counter)
    bad = bytearray(d)
    bad[-1] ^= 1
    return SingleArgs(exec_hash=ex, claimed_digest=d, claimed_digest_bad=bytes(bad))


def make_packed_batch(
    chain_id: int, hw_id: bytes, fw_hash: bytes, start_counter: int, n: int
) -> bytes:
    out = bytearray()
    for i in range(n):
        counter = start_counter + i
        ex = exec_hash_for(counter)
        d = compute_digest(chain_id, hw_id, fw_hash, ex, counter)
        out += pack_v1(chain_id, hw_id, fw_hash, ex, counter, d)
    return bytes(out)


def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument(
        "--chain-id",
        type=int,
        default=int(os.environ.get("CHAIN_ID", CHAIN_ID_DEFAULT)),
    )
    p.add_argument("--hw-id", type=str, default=os.environ.get("HW_ID"))
    p.add_argument("--fw-hash", type=str, default=os.environ.get("FW_HASH"))
    p.add_argument("--start-counter", type=int, default=1)
    p.add_argument("--n", type=int, default=50)
    args = p.parse_args()

    if not args.hw_id or not args.fw_hash:
        raise SystemExit(
            "Missing HW_ID or FW_HASH (set env vars or pass --hw-id/--fw-hash)"
        )

    hw_id = _hex32_to_bytes(args.hw_id)
    fw_hash = _hex32_to_bytes(args.fw_hash)

    single = make_single_args(args.chain_id, hw_id, fw_hash, args.start_counter)
    packed = make_packed_batch(
        args.chain_id, hw_id, fw_hash, args.start_counter, args.n
    )

    print("SINGLE_ARGS")
    print(f"EXEC_HASH=0x{single.exec_hash.hex()}")
    print(f"CLAIMED_DIGEST=0x{single.claimed_digest.hex()}")
    print(f"CLAIMED_DIGEST_BAD=0x{single.claimed_digest_bad.hex()}")
    print()
    print(f"PACKED_V1_N_{args.n}=0x{packed.hex()}")


if __name__ == "__main__":
    main()
