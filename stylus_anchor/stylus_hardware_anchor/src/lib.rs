#![cfg_attr(not(feature = "export-abi"), no_main)]
extern crate alloc;

use alloc::vec::Vec;
use alloy_sol_types::sol;
use stylus_sdk::abi::Bytes;
use stylus_sdk::alloy_primitives::{keccak256, Address, FixedBytes, Uint};
use stylus_sdk::{block, msg, prelude::*};

type U64 = Uint<64, 1>;
const DOMAIN: &[u8; 13] = b"anchor_RCT_V1";
const PACKED_RECEIPT_LEN: usize = 137;
const PACKED_RECEIPT_V2_LEN: usize = 105;

sol! {
    error AlreadyInitialized();
    error UnauthorizedCaller();
    error UnauthorizedHardware();
    error FirmwareNotApproved();
    error ReplayDetected();
    error DigestMismatch();
    error InvalidOwner();
}

sol_storage! {
    #[entrypoint]
    pub struct StylusHardwareAnchor {
        address owner;
        mapping(bytes32 => bool) authorized_nodes;
        mapping(bytes32 => bool) approved_firmware;
        mapping(bytes32 => uint64) counters;
    }
}

#[derive(SolidityError)]
pub enum HardwareAnchorError {
    AlreadyInitialized(AlreadyInitialized),
    UnauthorizedCaller(UnauthorizedCaller),
    UnauthorizedHardware(UnauthorizedHardware),
    FirmwareNotApproved(FirmwareNotApproved),
    ReplayDetected(ReplayDetected),
    DigestMismatch(DigestMismatch),
    InvalidOwner(InvalidOwner),
}

#[public]
impl StylusHardwareAnchor {
    pub fn initialize(&mut self) -> Result<(), HardwareAnchorError> {
        if self.owner.get() != Address::ZERO {
            return Err(HardwareAnchorError::AlreadyInitialized(
                AlreadyInitialized {},
            ));
        }
        self.owner.set(msg::sender());
        Ok(())
    }

    pub fn verify_receipt(
        &mut self,
        hw_id: FixedBytes<32>,
        fw_hash: FixedBytes<32>,
        exec_hash: FixedBytes<32>,
        counter: u64,
        claimed_digest: FixedBytes<32>,
    ) -> Result<(), HardwareAnchorError> {
        if !self.authorized_nodes.get(hw_id) {
            return Err(HardwareAnchorError::UnauthorizedHardware(
                UnauthorizedHardware {},
            ));
        }
        if !self.approved_firmware.get(fw_hash) {
            return Err(HardwareAnchorError::FirmwareNotApproved(
                FirmwareNotApproved {},
            ));
        }

        // Convert u64 to U64 for comparison
        let counter_u64 = U64::from(counter);
        let last_counter = self.counters.get(hw_id);

        if counter_u64 <= last_counter {
            return Err(HardwareAnchorError::ReplayDetected(ReplayDetected {}));
        }

        let chain_id = block::chainid();
        let reconstructed = Self::compute_digest(chain_id, hw_id, fw_hash, exec_hash, counter);

        if reconstructed != claimed_digest {
            return Err(HardwareAnchorError::DigestMismatch(DigestMismatch {}));
        }

        // Store as U64
        self.counters.insert(hw_id, counter_u64);
        Ok(())
    }

    pub fn verify_receipts_batch(&self, packed: Vec<u8>) -> Vec<bool> {
        if packed.is_empty() {
            return Vec::new();
        }

        if !packed.len().is_multiple_of(PACKED_RECEIPT_LEN) {
            return Vec::new();
        }

        let chain_id = block::chainid();
        let count = packed.len() / PACKED_RECEIPT_LEN;
        let mut results = Vec::with_capacity(count);

        for i in 0..count {
            let start = i * PACKED_RECEIPT_LEN;
            let end = start + PACKED_RECEIPT_LEN;
            let receipt = &packed[start..end];
            results.push(self.verify_packed_receipt(chain_id, receipt));
        }

        results
    }

    pub fn verify_receipts_batch_bytes(&self, packed: Bytes) -> Vec<bool> {
        self.verify_receipts_batch(packed.to_vec())
    }

    pub fn verify_receipts_batch_bitset(&self, packed: Vec<u8>) -> FixedBytes<32> {
        if packed.is_empty() {
            return FixedBytes::<32>::ZERO;
        }

        if !packed.len().is_multiple_of(PACKED_RECEIPT_LEN) {
            return FixedBytes::<32>::ZERO;
        }

        let count = packed.len() / PACKED_RECEIPT_LEN;
        if count > 256 {
            return FixedBytes::<32>::ZERO;
        }

        let chain_id = block::chainid();
        let mut bits = [0u8; 32];

        for i in 0..count {
            let start = i * PACKED_RECEIPT_LEN;
            let end = start + PACKED_RECEIPT_LEN;
            let receipt = &packed[start..end];
            if self.verify_packed_receipt(chain_id, receipt) {
                let byte_index = i / 8;
                let bit_index = i % 8;
                bits[byte_index] |= 1u8 << bit_index;
            }
        }

        FixedBytes::<32>::from(bits)
    }

    pub fn verify_receipts_batch_bitset_bytes(&self, packed: Bytes) -> FixedBytes<32> {
        self.verify_receipts_batch_bitset(packed.to_vec())
    }

    pub fn compute_receipt_digests_batch(&self, packed: Vec<u8>) -> Vec<FixedBytes<32>> {
        if packed.is_empty() {
            return Vec::new();
        }

        if !packed.len().is_multiple_of(PACKED_RECEIPT_V2_LEN) {
            return Vec::new();
        }

        let chain_id = block::chainid();
        let count = packed.len() / PACKED_RECEIPT_V2_LEN;
        let mut digests = Vec::with_capacity(count);

        for i in 0..count {
            let start = i * PACKED_RECEIPT_V2_LEN;
            let end = start + PACKED_RECEIPT_V2_LEN;
            let receipt = &packed[start..end];
            digests.push(Self::compute_digest_from_packed_v1(chain_id, receipt));
        }

        digests
    }

    pub fn authorize_node(&mut self, node_id: FixedBytes<32>) -> Result<(), HardwareAnchorError> {
        if msg::sender() != self.owner.get() {
            return Err(HardwareAnchorError::UnauthorizedCaller(
                UnauthorizedCaller {},
            ));
        }
        self.authorized_nodes.insert(node_id, true);
        Ok(())
    }

    pub fn revoke_node(&mut self, node_id: FixedBytes<32>) -> Result<(), HardwareAnchorError> {
        if msg::sender() != self.owner.get() {
            return Err(HardwareAnchorError::UnauthorizedCaller(
                UnauthorizedCaller {},
            ));
        }
        self.authorized_nodes.insert(node_id, false);
        Ok(())
    }

    pub fn approve_firmware(&mut self, fw_hash: FixedBytes<32>) -> Result<(), HardwareAnchorError> {
        if msg::sender() != self.owner.get() {
            return Err(HardwareAnchorError::UnauthorizedCaller(
                UnauthorizedCaller {},
            ));
        }
        self.approved_firmware.insert(fw_hash, true);
        Ok(())
    }

    pub fn revoke_firmware(&mut self, fw_hash: FixedBytes<32>) -> Result<(), HardwareAnchorError> {
        if msg::sender() != self.owner.get() {
            return Err(HardwareAnchorError::UnauthorizedCaller(
                UnauthorizedCaller {},
            ));
        }
        self.approved_firmware.insert(fw_hash, false);
        Ok(())
    }

    pub fn transfer_ownership(&mut self, new_owner: Address) -> Result<(), HardwareAnchorError> {
        if msg::sender() != self.owner.get() {
            return Err(HardwareAnchorError::UnauthorizedCaller(
                UnauthorizedCaller {},
            ));
        }
        if new_owner == Address::ZERO {
            return Err(HardwareAnchorError::InvalidOwner(InvalidOwner {}));
        }
        self.owner.set(new_owner);
        Ok(())
    }

    pub fn get_owner(&self) -> Address {
        self.owner.get()
    }

    pub fn is_node_authorized(&self, node_id: FixedBytes<32>) -> bool {
        self.authorized_nodes.get(node_id)
    }

    pub fn is_firmware_approved(&self, fw_hash: FixedBytes<32>) -> bool {
        self.approved_firmware.get(fw_hash)
    }

    pub fn get_counter(&self, node_id: FixedBytes<32>) -> u64 {
        // Convert U64 to u64 for return
        self.counters.get(node_id).try_into().unwrap_or(0)
    }
}

impl StylusHardwareAnchor {
    fn verify_packed_receipt(&self, chain_id: u64, receipt: &[u8]) -> bool {
        if receipt.len() != PACKED_RECEIPT_LEN {
            return false;
        }

        let version = receipt[0];
        if version != 1 {
            return false;
        }

        let hw_id = FixedBytes::<32>::from_slice(&receipt[1..33]);
        let fw_hash = FixedBytes::<32>::from_slice(&receipt[33..65]);
        let _exec_hash = FixedBytes::<32>::from_slice(&receipt[65..97]);
        let counter = u64::from_be_bytes(receipt[97..105].try_into().unwrap());
        let claimed_digest = FixedBytes::<32>::from_slice(&receipt[105..137]);

        if !self.authorized_nodes.get(hw_id) {
            return false;
        }
        if !self.approved_firmware.get(fw_hash) {
            return false;
        }

        let counter_u64 = U64::from(counter);
        let last_counter = self.counters.get(hw_id);
        if counter_u64 <= last_counter {
            return false;
        }


        let hw_id = FixedBytes::<32>::from_slice(&receipt[1..33]);
        let fw_hash = FixedBytes::<32>::from_slice(&receipt[33..65]);
        let exec_hash = FixedBytes::<32>::from_slice(&receipt[65..97]);
        let counter = u64::from_be_bytes(receipt[97..105].try_into().unwrap());

        Self::compute_digest(chain_id, hw_id, fw_hash, exec_hash, counter) == claimed_digest
    }

    fn compute_digest_from_packed_v1(chain_id: u64, receipt: &[u8]) -> FixedBytes<32> {
        let hw_id = FixedBytes::<32>::from_slice(&receipt[1..33]);
        let fw_hash = FixedBytes::<32>::from_slice(&receipt[33..65]);
        let exec_hash = FixedBytes::<32>::from_slice(&receipt[65..97]);
        let counter = u64::from_be_bytes(receipt[97..105].try_into().unwrap());
        Self::compute_digest(chain_id, hw_id, fw_hash, exec_hash, counter)
    }

    fn compute_digest(
        chain_id: u64,
        hw_id: FixedBytes<32>,
        fw_hash: FixedBytes<32>,
        exec_hash: FixedBytes<32>,
        counter: u64,
    ) -> FixedBytes<32> {
        let mut material = [0u8; 125];
        material[0..13].copy_from_slice(DOMAIN);
        material[13..21].copy_from_slice(&chain_id.to_be_bytes());
        material[21..53].copy_from_slice(hw_id.as_slice());
        material[53..85].copy_from_slice(fw_hash.as_slice());
        material[85..117].copy_from_slice(exec_hash.as_slice());
        material[117..125].copy_from_slice(&counter.to_be_bytes());
        keccak256(material)
    }
}
