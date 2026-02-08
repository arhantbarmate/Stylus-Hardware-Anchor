# Privacy Policy

## 🔐 Privacy-First Design

Stylus Hardware Anchor is built with **Privacy by Design** principles. We are committed to protecting user privacy through cryptographic verification rather than data collection.

## 📊 What Data We Process

### On-Chain Data (Public Blockchain)
The following data is permanently stored on Arbitrum blockchain:

**Hardware Receipts:**
- Hardware ID (SHA-256 hash of ESP32 MAC address)
- Firmware hash (SHA-256 of approved firmware)
- Execution hash (Keccak-256 of computation result)
- Receipt counter (monotonic integer)
- Receipt digest (Keccak-256 of combined data)

**Important:** All on-chain data is **cryptographic hashes**, not raw identifiable information.

### Off-Chain Data (Not Collected)
Stylus Hardware Anchor **does not collect**:
- ❌ Personal names or email addresses
- ❌ IP addresses or network metadata
- ❌ Location data or GPS coordinates
- ❌ Device fingerprints beyond hardware ID
- ❌ Usage analytics or telemetry
- ❌ Cookies or tracking pixels

## 🛡️ Privacy Guarantees

### Hardware Privacy
- **Hardware IDs are irreversible hashes** - The original ESP32 MAC address cannot be recovered from the hash
- **No device tracking** - Multiple receipts from the same device are linked only by the anonymous hardware ID
- **Opt-in authorization** - Devices must be explicitly authorized before receipts are accepted

### Data Minimization
- **Zero PII** - No personally identifiable information is required to operate a node
- **Pseudonymous operation** - Hardware IDs serve as pseudonyms, not identities
- **No third-party sharing** - All verification happens on-chain without intermediaries

### Cryptographic Privacy
- **Hash-based verification** - Original data never leaves the device
- **No plaintext storage** - Only cryptographic commitments are recorded
- **Forward secrecy** - Compromising one receipt doesn't reveal others

## 🌐 Network Privacy

### RPC Endpoints
When interacting with Arbitrum Sepolia:
- **Your IP address** may be logged by public RPC providers (e.g., Alchemy, Infura)
- **Transaction metadata** is public on blockchain explorers
- **Recommendation:** Use your own RPC node for maximum privacy

### Wallet Privacy
- **Private keys** remain on your device and are never transmitted
- **Transaction signing** happens locally
- **Nonce management** is client-side only

## 🔍 Third-Party Services

### Services We Use
**Arbitrum Sepolia Testnet:**
- Purpose: Blockchain infrastructure
- Data shared: Transaction data (public blockchain)
- Privacy policy: https://arbitrum.io/privacy-policy

**GitHub (Optional):**
- Purpose: Code hosting and issue tracking
- Data shared: Only if you contribute code or open issues
- Privacy policy: https://docs.github.com/privacy

### Services We Don't Use
- ❌ Analytics platforms (Google Analytics, Mixpanel, etc.)
- ❌ Error tracking services (Sentry, Rollbar, etc.)
- ❌ Marketing trackers
- ❌ Social media pixels

## 👥 Data Access

### Who Can Access On-Chain Data?
**Anyone** - Blockchain data is public by design:
- Anyone can query hardware receipts via the smart contract
- Anyone can verify receipt authenticity
- Anyone can view transaction history on Arbiscan

### Who Cannot Access Private Data?
**No one** - Because we don't collect it:
- Stylus Hardware Anchor developers have no access to private keys
- We cannot identify users beyond their blockchain addresses
- We cannot track device usage or behavior

## 🗑️ Data Retention

### On-Chain Data
- **Permanent** - Blockchain data is immutable and stored forever
- **No deletion** - Smart contract state cannot be erased
- **No "right to be forgotten"** - This is a technical limitation of blockchain

### Off-Chain Data
- **None retained** - We don't store any off-chain user data
- **Logs are ephemeral** - Build logs and terminal output are local-only

## 🔒 Security Measures

### Protecting Your Privacy
- **Open source** - All code is publicly auditable
- **No telemetry** - Firmware and middleware don't "phone home"
- **Local execution** - Receipt generation happens entirely on your device
- **Encrypted communications** - RPC calls use HTTPS

### Your Responsibilities
- **Secure your private keys** - We cannot recover lost keys
- **Choose RPC providers carefully** - They may log your IP
- **Review smart contract interactions** - Verify what you're signing
- **Don't reuse hardware IDs** - Each device should have a unique ID

## 🌍 Jurisdiction & Compliance

### Legal Framework
- **Decentralized protocol** - No central entity controls data
- **GDPR considerations** - On-chain data is pseudonymous, not anonymous
- **No data controller** - Users control their own private keys and data

### Regulatory Compliance
- **KYC/AML:** Not applicable (no financial transactions, no user accounts)
- **COPPA:** Not applicable (no data collection from minors)
- **CCPA:** Not applicable (no sale of personal information)

## 📧 Privacy Questions

If you have questions about how Stylus Hardware Anchor handles privacy:
- **Email:** arhant6armate@gmail.com
- **GitHub Discussions:** Open a discussion in our repository
- **Security Concerns:** See SECURITY.md for responsible disclosure

## 🔄 Changes to This Policy

We may update this privacy policy as the protocol evolves. Changes will be:
- Documented in Git commit history
- Announced via GitHub releases
- Effective immediately upon publication

**Current Version:** 1.0  
**Last Updated:** February 8, 2026  
**Effective Date:** February 8, 2026

## 🎯 Summary

**In Plain English:**
- Stylus Hardware Anchor only stores cryptographic hashes on-chain
- We don't collect your name, email, location, or any personal data
- Everything is public blockchain data - no secrets, no tracking
- You control your private keys - we can't access them
- Privacy through cryptography, not through data policies

---

© 2026 Stylus Hardware Anchor · Built for Privacy
