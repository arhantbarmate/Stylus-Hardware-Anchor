#![cfg_attr(not(any(test, feature = "export-abi")), no_main)]
extern crate alloc;

use stylus_sdk::prelude::*;
use stylus_sdk::alloy_primitives::{FixedBytes, U256};

// In Stylus 0.10.x, the storage struct IS the entrypoint.
// No wrapper struct is needed - #[entrypoint] goes INSIDE sol_storage!
sol_storage! {
    #[entrypoint]
    pub struct anchorAnchor {
        uint256 verified_count;
        bytes32 last_receipt;
    }
}

// Implement logic directly on the storage struct.
// The #[public] macro generates the Router for this struct.
#[public]
impl anchorAnchor {
    pub fn verify_execution(&mut self, receipt_digest: FixedBytes<32>) {
        let current = self.verified_count.get();
        self.verified_count.set(current + U256::from(1));
        self.last_receipt.set(receipt_digest);
    }

    pub fn get_verified_count(&self) -> U256 {
        self.verified_count.get()
    }

    pub fn get_last_receipt(&self) -> FixedBytes<32> {
        self.last_receipt.get()
    }
}
