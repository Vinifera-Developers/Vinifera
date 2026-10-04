# Status effects

Define a named damage-over-time effect in `[StatusEffectTypes]`, then bind it to a warhead, Tiberium type, or gas particle type. Non-gas particle bindings are rejected when loading configuration. The target owns its timer. Animation visibility and removal do not control health changes.

```ini
[StatusEffectTypes]
0=Status.TiberiumPoisoning

[Status.TiberiumPoisoning]
EligibleTypes=Infantry
Response=Damage
Damage.PerTick=2
Damage.Warhead=StatusTiberiumWH
Tick.Count=6
Tick.Interval=15
Tick.FirstDelay=15
TiberiumHeal.Response=Heal
Reapply=Refresh
StackLimit=1
Tick.ApplyStatuses=no

; Register this warhead in the existing [Warheads] list.
[StatusTiberiumWH]
Verses=100%,100%,100%,100%,100%
ProneDamage=100%
MinDamage=0

[InfectorPoisonWH]
Verses=100%,100%,100%,100%,100%
AffectsAllies=yes
Status.Apply=Status.TiberiumPoisoning

[Riparius]
Status.Apply=Status.TiberiumPoisoning
Status.ReplaceLegacyHealth=yes

[GasCloud1]
Status.Apply=Status.TiberiumPoisoning
Status.ReplaceLegacyHealth=yes
```

Give the applying weapon `Warhead=InfectorPoisonWH`. Zero immediate damage is supported, including railgun damage dispatch. Existing immediate damage still happens unless the weapon's `Damage` and `AmbientDamage` are set to zero.

Timing uses simulation frames. The first tick occurs after `Tick.FirstDelay`, which initially defaults to `Tick.Interval`; zero allows a tick at the application frame's completion. Every attempt consumes a tick, including blocked damage and healing at maximum health. Invalid values, missing references, unsupported durations, and stack limits other than one fail configuration loading.

Repeated application refreshes the remaining count and preserves the next deadline. Sources with the same effect, Tiberium-healing response, and persistence policy share one timer. Different policies have separate timers. Environmental exposure refreshes while present and leaves a finite tail. Set `Status.TiberiumPoisoning.PersistAfterExit=no` on a hazard to remove its channel immediately on exit. Bound hazards replace their legacy health handling even when a target rejects the effect; unbound hazards retain existing behavior.

Use these settings on target types:

```ini
[HTNK]
Status.TiberiumPoisoning.Eligible=yes

[E1]
Status.TiberiumPoisoning.Immune=yes
Status.TiberiumPoisoning.DamageMultiplier=50%
Status.TiberiumPoisoning.Response=Inherit
```

`EligibleTypes` accepts `Infantry`, `Vehicle`/`Unit`, `Aircraft`, and `Building`. Explicit eligibility overrides the category. Immunity rejects application and clears existing instances. The percentage multiplier affects harmful ticks. Explicit target `Response=Damage`, `Heal`, or `Ignore` takes precedence over source and effect responses.

`Response=Damage`, `Heal`, or `Ignore` on an effect selects its default health response; it defaults to `Damage`. For example, a `Status.Repair` effect with `Response=Heal` can repair eligible vehicles without `TiberiumHeal=yes`. `TiberiumHeal.Response` defaults to `Inherit`, so those targets also use the effect default unless a conditional override is configured.

The example converts ticks into healing for targets with the existing type boolean `TiberiumHeal=yes`, regardless of application source. A source can override this with `Status.TiberiumPoisoning.TiberiumHeal.Response=Damage`, `Heal`, `Ignore`, or `Inherit`. The veteran ability is not included automatically. `TiberiumProof` is not generalized status immunity.

Ticks use native damage handling with the configured warhead. Effects apply to explosion targets selected by existing `CellSpread` and air-targeting logic; ticks do not create another area explosion. Iron Curtain can block tick damage while the attached timer continues and consumes attempts. Positive damage retains armor and other native modifiers; negative damage clamps at maximum health. Zero resistance skips the damage call. The first accepted source keeps attribution; destruction clears its live pointer while the timer continues. Environmental sources are neutral. Friendly-fire checks use the original source house and current target owner.

Limbo pauses countdowns; cloaking and EMP do not. Successful supported deployment and transformation paths transfer eligible state. Death removes it. Saves stream records and remap pointers; the save format is incompatible with older builds. Status state contributes to the native network checksum.

Tick-origin damage cannot apply effects by default. `Tick.ApplyStatuses=yes` enables deferred application on the following frame, with deduplication. Cumulative stacks, arbitrary stat modifiers, owned-cloud attribution, posthumous unit credit, icons, and delayed visceroid conversion are outside this implementation.
