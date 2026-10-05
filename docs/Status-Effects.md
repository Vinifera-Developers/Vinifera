# Status effects

Define a named effect that changes health over time in `[StatusEffectTypes]`, then bind it to a warhead, Tiberium type, or gas particle type. The name and health behavior are configurable; poisoning is one example. Non-gas particle bindings are rejected when loading configuration. The target owns its timer. Animation visibility and removal do not control health changes.

| Use | Effect configuration | Application |
|---|---|---|
| Burning | Damage over time with a fire warhead | Flame weapon impacts |
| Poisoning or bleeding | Damage over time with a chosen warhead | Weapon impacts or configured hazards |
| Corrosion | Damage over time with vehicle eligibility and a chosen warhead | Chemical weapons or configured hazards |
| Repair or regeneration | `Response=Heal` | Repair weapons or configured hazards |

Each effect has its own damage magnitude, timing, warhead and target rules. Different named effects can coexist. Burning can damage a cyborg while a separate poison effect heals it. Tiberium and gas can apply any registered effect; they are not restricted to poisoning.

Append effect names using unused keys in the existing registry, and register any new warheads in `[Warheads]`. The examples below share this effect registry:

```ini
[StatusEffectTypes]
0=Status.TiberiumPoisoning
1=Status.Burning
2=Status.Repair
```

## Burning from a flame weapon

```ini
[Status.Burning]
EligibleTypes=Infantry,Vehicle
Response=Damage
Damage.PerTick=3
Damage.Warhead=Fire
Tick.Count=5
Tick.Interval=15
Tick.FirstDelay=15
TiberiumHeal.Response=Damage

; Add this to the flame weapon's existing warhead section.
[Fire]
Status.Apply=Status.Burning

; Optional immunity on a target type:
; [YourFireproofUnit]
; Status.Burning.Immune=yes
```

This adds burning to impacts using `Fire`, while retaining their immediate damage. Use a separate applying warhead to limit burning to particular weapons. Each source supports one `Status.Apply` binding.

For an ordinary projectile or railgun that should inflict only DoT, set its immediate `Damage` and `AmbientDamage` to zero. Fire particles and damaging animations have separate damage settings; zero weapon damage does not disable them. Replacing an existing burn-down mechanic requires identifying and removing its legacy damage path. Applying a status does not automatically replace that path.

## Repair and regeneration

```ini
[Status.Repair]
EligibleTypes=Infantry,Vehicle
Response=Heal
Damage.PerTick=3
Damage.Warhead=SA
Tick.Count=5
Tick.Interval=15

; Register this warhead in the existing [Warheads] list.
[RepairApplyWH]
Verses=100%,100%,100%,100%,100%
AffectsAllies=yes
Status.Apply=Status.Repair
```

Give a repair weapon `Warhead=RepairApplyWH` and zero immediate damage. `Response=Heal` works without `TiberiumHeal=yes`. Healing uses the same timer and refresh rules as harmful effects, and is capped at maximum health. Configure normal weapon targeting separately; status eligibility does not change attack cursors or AI weapon selection.

For a regeneration hazard, bind `Status.Repair` to a Tiberium or gas type instead. Use `Status.ReplaceLegacyHealth=yes` to replace that hazard's existing health handling. Set `Status.Repair.PersistAfterExit=no` if healing must stop when the target leaves.

## Poisoning from weapons and hazards

```ini

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

## Timing and reapplication

Timing uses simulation frames. The first tick occurs after `Tick.FirstDelay`, which initially defaults to `Tick.Interval`; zero allows a tick at the application frame's completion. Every attempt consumes a tick, including blocked damage and healing at maximum health. Invalid values, missing references, unsupported durations, and stack limits other than one fail configuration loading.

Repeated application refreshes the remaining count and preserves the next deadline. Sources with the same effect, Tiberium-healing response, and persistence policy share one timer. Different policies have separate timers. Environmental exposure refreshes while present and leaves a finite tail. Set `Status.TiberiumPoisoning.PersistAfterExit=no` on a hazard to remove its channel immediately on exit. Bound hazards replace their legacy health handling even when a target rejects the effect; unbound hazards retain existing behavior.

## Target responses

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

## Damage and lifecycle

Ticks use native damage handling with the configured warhead. Effects apply to explosion targets selected by existing `CellSpread` and air-targeting logic; ticks do not create another area explosion. Iron Curtain can block tick damage while the attached timer continues and consumes attempts. Positive damage retains armor and other native modifiers; negative damage clamps at maximum health. Zero resistance skips the damage call. The first accepted source keeps attribution; destruction clears its live pointer while the timer continues. Environmental sources are neutral. Friendly-fire checks use the original source house and current target owner.

Limbo pauses countdowns; cloaking and EMP do not. Successful supported deployment and transformation paths transfer eligible state. Death removes it. Saves stream records and remap pointers; the save format is incompatible with older builds. Status state contributes to the native network checksum.

Tick-origin damage cannot apply effects by default. `Tick.ApplyStatuses=yes` enables deferred application on the following frame, with deduplication. Cumulative stacks, arbitrary stat modifiers, owned-cloud attribution, posthumous unit credit, icons, and delayed visceroid conversion are outside this implementation.

## Other debuffs

The shared definitions and application sources provide a basis for more status behaviors, but this version changes health only. Reduced speed, firepower or armor would need modifier hooks and expiration rules. There are no settings for those stat debuffs in this implementation.
