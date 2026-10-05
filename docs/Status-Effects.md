# Status effects

Define a named effect that changes health over time in `[StatusEffectTypes]`, then bind it to a warhead, Tiberium type, or gas particle type. The name and health behavior are configurable; poisoning is one example. Non-gas particle bindings are rejected when loading configuration. The target owns its timer. Animation visibility and removal do not control health changes.

| Use | Effect configuration | Application |
|---|---|---|
| Burning | Damage over time with a fire warhead | Flame weapon impacts |
| Poisoning or bleeding | Damage over time with a chosen warhead | Weapon impacts or configured hazards |
| Corrosion | Damage over time with vehicle eligibility and a chosen warhead | Chemical weapons or configured hazards |
| Repair or regeneration | `Response=Heal` | Repair weapons or configured hazards |

Each effect has its own damage magnitude, timing, warhead and target rules. Different named effects can coexist. Burning can damage a cyborg while a separate poison effect heals it. Tiberium and gas can apply any registered effect; they are not restricted to poisoning.

Append effect names using unused keys in the existing registry, and register any new warheads in `[Warheads]`. Effect names must not contain periods. Target overrides use `StatusEffect<EffectName>Eligible`, `StatusEffect<EffectName>Immune`, `StatusEffect<EffectName>DamageMultiplier`, and `StatusEffect<EffectName>Response`, replacing `<EffectName>` with the registered name. The examples below share this effect registry:

```ini
[StatusEffectTypes]
0=TiberiumPoisoning
1=Burning
2=Repair
```

## Burning from a flame weapon

```ini
[Burning]
EligibleTypes=Infantry,Vehicle
Response=Damage
DamagePerTick=3
DamageWarhead=Fire
TickCount=5
TickInterval=15
TickFirstDelay=15
TiberiumHealResponse=Damage

; Add this to the flame weapon's existing warhead section.
[Fire]
StatusEffect=Burning

; Optional immunity on a target type:
; [YourFireproofUnit]
; StatusEffectBurningImmune=yes
```

This adds burning to impacts using `Fire`, while retaining their immediate damage. Use a separate applying warhead to limit burning to particular weapons. Each source supports one `StatusEffect` binding.

For an ordinary projectile or railgun that should inflict only DoT, set its immediate `Damage` and `AmbientDamage` to zero. Fire particles and damaging animations have separate damage settings; zero weapon damage does not disable them. Replacing an existing burn-down mechanic requires identifying and removing its legacy damage path. Applying a status does not automatically replace that path.

## Repair and regeneration

```ini
[Repair]
EligibleTypes=Infantry,Vehicle
Response=Heal
DamagePerTick=3
DamageWarhead=SA
TickCount=5
TickInterval=15

; Register this warhead in the existing [Warheads] list.
[RepairApplyWH]
Verses=100%,100%,100%,100%,100%
AffectsAllies=yes
StatusEffect=Repair
```

Give a repair weapon `Warhead=RepairApplyWH` and zero immediate damage. `Response=Heal` works without `TiberiumHeal=yes`. Healing uses the same timer and refresh rules as harmful effects, and is capped at maximum health. Configure normal weapon targeting separately; status eligibility does not change attack cursors or AI weapon selection.

For a regeneration hazard, bind `Repair` to a Tiberium or gas type instead. Use `StatusEffectReplaceLegacyHealth=yes` to replace that hazard's existing health handling. Set `StatusEffectPersistAfterExit=no` if healing must stop when the target leaves.

## Poisoning from weapons and hazards

```ini

[TiberiumPoisoning]
EligibleTypes=Infantry
Response=Damage
DamagePerTick=2
DamageWarhead=StatusTiberiumWH
TickCount=6
TickInterval=15
TickFirstDelay=15
TiberiumHealResponse=Heal
Reapply=Refresh
StackLimit=1
TickApplyStatuses=no

; Register this warhead in the existing [Warheads] list.
[StatusTiberiumWH]
Verses=100%,100%,100%,100%,100%
ProneDamage=100%
MinDamage=0

[InfectorPoisonWH]
Verses=100%,100%,100%,100%,100%
AffectsAllies=yes
StatusEffect=TiberiumPoisoning

[Riparius]
StatusEffect=TiberiumPoisoning
StatusEffectReplaceLegacyHealth=yes

[GasCloud1]
StatusEffect=TiberiumPoisoning
StatusEffectReplaceLegacyHealth=yes
```

Give the applying weapon `Warhead=InfectorPoisonWH`. Zero immediate damage is supported, including railgun damage dispatch. Existing immediate damage still happens unless the weapon's `Damage` and `AmbientDamage` are set to zero.

## Timing and reapplication

Timing uses simulation frames. The first tick occurs after `TickFirstDelay`, which initially defaults to `TickInterval`; zero allows a tick at the application frame's completion. Every attempt consumes a tick, including blocked damage and healing at maximum health. Invalid values, missing references, unsupported durations, and stack limits other than one fail configuration loading.

Repeated application refreshes the remaining count and preserves the next deadline. Sources with the same effect, Tiberium-healing response, and persistence policy share one timer. Different policies have separate timers. Environmental exposure refreshes while present and leaves a finite tail. Set `StatusEffectPersistAfterExit=no` on a hazard to remove its channel immediately on exit. Bound hazards replace their legacy health handling even when a target rejects the effect; unbound hazards retain existing behavior.

## Target responses

Use these settings on target types:

```ini
[HTNK]
StatusEffectTiberiumPoisoningEligible=yes

[E1]
StatusEffectTiberiumPoisoningImmune=yes
StatusEffectTiberiumPoisoningDamageMultiplier=50%
StatusEffectTiberiumPoisoningResponse=Inherit
```

`EligibleTypes` accepts `Infantry`, `Vehicle`/`Unit`, `Aircraft`, and `Building`. Explicit eligibility overrides the category. Immunity rejects application and clears existing instances. The percentage multiplier affects harmful ticks. Explicit target `Response=Damage`, `Heal`, or `Ignore` takes precedence over source and effect responses.

`Response=Damage`, `Heal`, or `Ignore` on an effect selects its default health response; it defaults to `Damage`. For example, a `Repair` effect with `Response=Heal` can repair eligible vehicles without `TiberiumHeal=yes`. `TiberiumHealResponse` defaults to `Inherit`, so those targets also use the effect default unless a conditional override is configured.

The example converts ticks into healing for targets with the existing type boolean `TiberiumHeal=yes`, regardless of application source. A source can override this with `StatusEffectTiberiumHealResponse=Damage`, `Heal`, `Ignore`, or `Inherit`. The veteran ability is not included automatically. `TiberiumProof` is not generalized status immunity.

## Damage and lifecycle

Ticks use native damage handling with the configured warhead. Effects apply to explosion targets selected by existing `CellSpread` and air-targeting logic; ticks do not create another area explosion. Iron Curtain can block tick damage while the attached timer continues and consumes attempts. Positive damage retains armor and other native modifiers; negative damage clamps at maximum health. Zero resistance skips the damage call. The first accepted source keeps attribution; destruction clears its live pointer while the timer continues. Environmental sources are neutral. Friendly-fire checks use the original source house and current target owner.

Limbo pauses countdowns; cloaking and EMP do not. Successful supported deployment and transformation paths transfer eligible state. Death removes it. Saves stream records and remap pointers; the save format is incompatible with older builds. Status state contributes to the native network checksum.

Tick-origin damage cannot apply effects by default. `TickApplyStatuses=yes` enables deferred application on the following frame, with deduplication. Cumulative stacks, arbitrary stat modifiers, owned-cloud attribution, posthumous unit credit, icons, and delayed visceroid conversion are outside this implementation.

## Other debuffs

The shared definitions and application sources provide a basis for more status behaviors, but this version changes health only. Reduced speed, firepower or armor would need modifier hooks and expiration rules. There are no settings for those stat debuffs in this implementation.
