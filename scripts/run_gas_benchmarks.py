#!/usr/bin/env python3

import argparse
import json
import os
import subprocess
import sys
from typing import Any, Dict, List, Tuple

from benchmark_receipts import (
    CHAIN_ID_DEFAULT,
    _hex32_to_bytes,
    make_packed_batch,
    make_single_args,
)


def _must_env(name: str) -> str:
    v = os.environ.get(name, "")
    if not v:
        raise SystemExit(f"Missing env var: {name}")
    return v


def _run(cmd: List[str]) -> str:
    p = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if p.returncode != 0:
        raise SystemExit(
            "Command failed:\n"
            + " ".join(cmd)
            + "\n\nSTDOUT:\n"
            + p.stdout
            + "\n\nSTDERR:\n"
            + p.stderr
        )
    return p.stdout.strip()


def _cast_send_json(args: List[str]) -> Dict[str, Any]:
    out = _run(["cast", "send", "--json"] + args)
    try:
        return json.loads(out)
    except json.JSONDecodeError:
        raise SystemExit(f"Failed to parse JSON from cast send output:\n{out}")


def _cast_receipt_json(tx_hash: str, rpc_url: str) -> Dict[str, Any]:
    out = _run(["cast", "receipt", "--json", tx_hash, "--rpc-url", rpc_url])
    try:
        return json.loads(out)
    except json.JSONDecodeError:
        raise SystemExit(f"Failed to parse JSON from cast receipt output:\n{out}")


def _gas_used(receipt: Dict[str, Any]) -> int:
    v = receipt.get("gasUsed")
    if isinstance(v, str):
        return int(v, 0)
    if isinstance(v, int):
        return v
    raise SystemExit(f"Unexpected gasUsed in receipt: {v}")


def _status(receipt: Dict[str, Any]) -> int:
    v = receipt.get("status")
    if isinstance(v, str):
        return int(v, 0)
    if isinstance(v, int):
        return v
    raise SystemExit(f"Unexpected status in receipt: {v}")


def _send_and_measure(
    label: str, send_args: List[str], rpc_url: str
) -> Tuple[str, int, int]:
    send = _cast_send_json(send_args)
    tx = send.get("transactionHash") or send.get("hash")
    if not tx:
        raise SystemExit(f"cast send JSON missing transaction hash: {send}")

    receipt = _cast_receipt_json(tx, rpc_url)
    return tx, _gas_used(receipt), _status(receipt)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--contract", default=os.environ.get("CONTRACT_ADDRESS", ""))
    ap.add_argument("--rpc-url", default=os.environ.get("RPC_URL", ""))
    ap.add_argument("--private-key", default=os.environ.get("PRIVATE_KEY", ""))
    ap.add_argument("--hw-id", default=os.environ.get("HW_ID", ""))
    ap.add_argument("--fw-hash", default=os.environ.get("FW_HASH", ""))
    ap.add_argument(
        "--chain-id",
        type=int,
        default=int(os.environ.get("CHAIN_ID", CHAIN_ID_DEFAULT)),
    )
    ap.add_argument("--setup", action="store_true")
    ap.add_argument("--batch-fn", choices=["bitset", "bool"], default="bitset")
    ap.add_argument("--sizes", default="5,10,20,50")
    ap.add_argument("--gas-limit", type=int, default=5_000_000)
    args = ap.parse_args()

    contract = args.contract or _must_env("CONTRACT_ADDRESS")
    rpc_url = args.rpc_url or _must_env("RPC_URL")
    pk = args.private_key or _must_env("PRIVATE_KEY")
    hw_hex = args.hw_id or _must_env("HW_ID")
    fw_hex = args.fw_hash or _must_env("FW_HASH")

    hw_id = _hex32_to_bytes(hw_hex)
    fw_hash = _hex32_to_bytes(fw_hex)

    sizes = [int(x.strip()) for x in args.sizes.split(",") if x.strip()]

    results: List[Dict[str, Any]] = []

    if args.setup:
        for fn, fn_args in [
            ("initialize()", []),
            ("authorizeNode(bytes32)", [hw_hex]),
            ("approveFirmware(bytes32)", [fw_hex]),
        ]:
            tx, gas, st = _send_and_measure(
                fn,
                [
                    contract,
                    fn,
                    *fn_args,
                    "--rpc-url",
                    rpc_url,
                    "--private-key",
                    pk,
                    "--gas-limit",
                    str(args.gas_limit),
                ],
                rpc_url,
            )
            results.append({"label": fn, "tx": tx, "gasUsed": gas, "status": st})

    # Batch benchmarks
    for n in sizes:
        packed = make_packed_batch(args.chain_id, hw_id, fw_hash, 1, n)
        packed_hex = "0x" + packed.hex()

        if args.batch_fn == "bitset":
            sig = "verifyReceiptsBatchBitsetBytes(bytes)"
        else:
            sig = "verifyReceiptsBatchBytes(bytes)"

        tx, gas, st = _send_and_measure(
            f"batch_{n}",
            [
                contract,
                sig,
                packed_hex,
                "--rpc-url",
                rpc_url,
                "--private-key",
                pk,
                "--gas-limit",
                str(args.gas_limit),
            ],
            rpc_url,
        )
        results.append(
            {
                "label": f"{sig} N={n}",
                "tx": tx,
                "gasUsed": gas,
                "status": st,
                "n": n,
                "gasPerReceipt": gas / n,
            }
        )

    # Single success + single failure (invalid digest)
    single = make_single_args(args.chain_id, hw_id, fw_hash, 1)

    def _hex32(b: bytes) -> str:
        return "0x" + b.hex()

    sig_single = "verifyReceipt(bytes32,bytes32,bytes32,uint64,bytes32)"

    tx, gas, st = _send_and_measure(
        "single_success",
        [
            contract,
            sig_single,
            hw_hex,
            fw_hex,
            _hex32(single.exec_hash),
            "1",
            _hex32(single.claimed_digest),
            "--rpc-url",
            rpc_url,
            "--private-key",
            pk,
            "--gas-limit",
            str(args.gas_limit),
        ],
        rpc_url,
    )
    results.append(
        {"label": "verifyReceipt success", "tx": tx, "gasUsed": gas, "status": st}
    )

    tx, gas, st = _send_and_measure(
        "single_invalid_digest",
        [
            contract,
            sig_single,
            hw_hex,
            fw_hex,
            _hex32(single.exec_hash),
            "1",
            _hex32(single.claimed_digest_bad),
            "--rpc-url",
            rpc_url,
            "--private-key",
            pk,
            "--gas-limit",
            str(args.gas_limit),
        ],
        rpc_url,
    )
    results.append(
        {
            "label": "verifyReceipt invalid digest",
            "tx": tx,
            "gasUsed": gas,
            "status": st,
        }
    )

    any_failed = any(r.get("status") != 1 for r in results)
    print(
        json.dumps(
            {
                "contract": contract,
                "rpc_url": rpc_url,
                "results": results,
                "warning": "one or more transactions reverted" if any_failed else "",
            },
            indent=2,
        )
    )


if __name__ == "__main__":
    main()
