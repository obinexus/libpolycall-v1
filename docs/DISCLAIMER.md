# DISCLAIMER

**LibPolyCall** (`github.com/obinexus/libpolycall`) is an **experimental, research-grade polyglot interoperability framework** implementing zero-trust FFI bridges, hot-swappable language adapters, hybrid self-balancing tree structures (NLM-Atlas), and unified microservice orchestration.

## Not for Production Use
This software is **not designed, intended, or verified** for use in production environments, critical systems, financial applications, medical devices, safety-critical systems, or any scenario where failure could result in financial loss, data corruption, privacy violations, or harm to individuals or infrastructure.

It is provided strictly for:
- Research
- Education
- Prototyping
- Development and testing in isolated, non-critical environments

**Do NOT deploy in production without independent security auditing, formal verification, and compliance review.**

## Security Warning
While the project implements zero-trust principles, cryptographic seeding, JWT-based authorization, and adapter isolation:
- These mechanisms are **experimental**
- Vulnerabilities may exist in FFI boundaries, bridge implementations, dynamic loading, or tree transformation logic
- Shared libraries (`.so`, `.dll`, `.node`) and hot-swapping introduce attack surfaces
- Misuse or misconfiguration can lead to privilege escalation, memory corruption, or remote code execution

**Use only in sandboxed, air-gapped, or containerized environments.**

## No Warranties
This software is provided **"AS IS"** without warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose, security, availability, or non-infringement.

The authors, contributors, and OBINexus make no representations about the suitability, reliability, or accuracy of this software.

## Limitation of Liability
In no event shall the authors, contributors, OBINexus, or any affiliated parties be liable for any direct, indirect, incidental, special, exemplary, or consequential damages (including but not limited to loss of data, profits, goodwill, or business interruption) arising from the use or inability to use this software — even if advised of the possibility of such damage.

This includes, but is not limited to, damages resulting from:
- Security breaches via FFI or bridge exploits
- Data corruption due to tree rebalancing or memory mapping bugs
- System instability from hot-swapping or dynamic loading
- Failures in zero-trust validation logic

If your jurisdiction does not permit such broad exclusion of liability, you must not use this software.

## Experimental Features
The following components are explicitly **research prototypes**:
- NLM-Atlas Geomorphic Schema (hybrid RB-AVL trees with dimensional collapse)
- Hot-swappable language adapters with semantic versioning
- Bidirectional polyglot bridges (e.g., cobol2python, java2go)
- Zero-trust REPL agents and WebPolyCall streaming

These have **not been formally verified** and may change or be removed without notice.

## License
See the `LICENSE` file. Generally MIT or Apache-2.0 — but the disclaimer above applies regardless of license.

---

**Maintained by OBINexus**  
**Last updated: November 23, 2025**

By using this repository, you acknowledge that you have read, understood, and agree to this disclaimer.
