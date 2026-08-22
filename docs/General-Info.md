# General Info

This page lists general info that should be known about the project.

## Build types

There are three main types of Vinifera builds:
- *stable builds* - those are numbered like your regular versions (something close to semantic versioning, e.g. version 1.2.3 for example) and ideally should contain no bugs, therefore are safe to use in mods;
- *development builds* - those are the builds which contain functionality that needs to be tested. They are numbered plainly starting from 0 and incrementing the number on each release. Mod authors still can include those versions with their mods if they want latest features, though we can't guarantee lack of bugs;
- *nightly builds* - bleeding edge versions which can include prototypes, proofs of concepts, scrapped features etc., in other words - we can't guarantee anything in those builds and they absolutely should NOT be used in mod releases and should only be used to help with development and testing.

Stable builds are produced starting with version 1.0.0.0. Nightly builds continue to be produced daily from the `develop` branch.

## Compatibility

Vinifera includes its own multiplayer spawner and supersedes the external ts-patches spawner. A ts-patches-compatible executable can still be used where a client installation requires its remaining integration patches.
