### Force Budget Formula (plain)

$$
F_{out} = \min(F_{desired},\; F_{safe},\; \eta \cdot F_{actuator})
$$

* **$F_{desired}$** = how strong you *want* the sensation (e.g. “cookie feels snug in hand”).
* **$F_{safe}$** = absolute safety cap (what the body or cookie can handle without harm).
* **$\eta$** = efficiency factor (friction, clothing, energy loss).
* **$F_{actuator}$** = raw force your hardware can actually produce.
* **$F_{out}$** = final commanded force (what the player feels).

---

### How it works with the cookie example

* You want **just enough** grip force to hold the cookie against gravity.
* The system clamps that force against:

  1. The cookie’s breaking point (invariant).
  2. The child’s hand comfort limit (invariant).
  3. Actuator capability (reality check).

Anything variant (cookie size, sweaty hands, silly player movement) is folded into $F_{desired}$ and $\eta$.

---

### Inverse Kinematics tie-in

IK gives you **where** the hand should be.
Force budget ensures **how hard** you can move/press without wrecking the cookie or the player.

Together, they answer:

* *Where do I go?*
* *How much energy do I use without breaking invariants?*

