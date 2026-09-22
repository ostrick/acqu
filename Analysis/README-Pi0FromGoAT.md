# Neutral pi0 reconstruction from GoAT files

`analyze_pi0_goat` reads the event-synchronous GoAT trees `tracks` and
`tagger`, plus the single-entry tree `setupParameters`. It selects neutral
calorimeter tracks: CB tracks without a PID hit and TAPS tracks without a veto
hit. It treats them as photons, combines all photon pairs and accepts pairs
inside the configured invariant-mass window.

For every accepted pi0 and tagger hit it calculates

```
coincidenceTime = taggedTime - pi0Time
missingMass     = (target + taggedPhoton - pi0).M()
```

Only combinations in the prompt or either random window are retained.
`timingClass` is 1 for prompt and 0 for random. `timingWeight` is 1 for
prompt and minus the prompt-width/random-width ratio for random combinations.

Build and run:

```
cd Analysis
make analyze_pi0_goat
./analyze_pi0_goat pi0_input.dat
```

The configuration accepts repeated `input = ...` lines. All input files are
written to one output tree called `pi0`; `inputFileIndex` and `sourceEntry`
identify the source event. `setupParameters/TaggerPhotonEnergy` supplies the
tagged-photon energy when the optional GoAT branch `tagger/taggedEnergy` is
absent.

The output contains vectors for neutral tracks, all gamma-gamma pairs,
accepted pi0 candidates, tagger hits and accepted pi0-tagger combinations.
No PDG-based charged-particle reconstruction and no CB-sector multiplicity are
performed.
