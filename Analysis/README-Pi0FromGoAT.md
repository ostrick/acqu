# Neutral pi0 reconstruction from GoAT files

`analyze_pi0_goat` reads the event-synchronous GoAT trees `tracks` and
`tagger`, plus the single-entry tree `setupParameters`. It selects neutral CB
tracks without PID or MWPC hits. It treats them as photons, combines all photon
pairs and accepts pairs inside the configured invariant-mass window.

For every accepted pi0 and tagger hit it calculates

```
coincidenceTime = taggedTime - pi0Time
missingMass     = (target + taggedPhoton - pi0).M()
missingEnergy   = measured pi0 CM energy - expected two-body pi0 CM energy
pi0LabEnergy    = expected coherent two-body pi0 energy in the laboratory
```

`pi0OpeningAngle` contains the laboratory opening angle of the two photons in
degrees. It is indexed by the pi0 candidate. `missingEnergy` is indexed by the
pi0-tagger combination, like `missingMass`. The nominal pion mass used in the
two-body calculation is read from `pi0_mass` in the configuration.

The output file contains the prompt-minus-random histograms `missingEnergy`,
`openingAngle`, `pi0Mass`, `missingMass`, and `pi0ThetaCM`. The `openingAngle`
histogram contains `pi0OpeningAngle - minimumOpeningAngle`, not the absolute
measured angle. Prompt entries have weight +1 and random entries enter with the
negative, width-normalized `timingWeight`. Histogram binning is configured in
`pi0_input.dat`.

Each one-dimensional spectrum has a corresponding two-dimensional histogram
named `<quantity>ByTaggerChannel`. Its x-axis contains the integer tagger
channel and its y-axis the respective observable. The y-axis uses one tenth of
the configured number of bins of the one-dimensional histogram; ranges and
prompt-random weights are identical.

The tree branch `pi0ThetaCM` contains the pion polar angle after boosting into
the photon-target center-of-mass system. It is indexed by the pi0-tagger
combination, like `missingEnergy` and `missingMass`.

The combination-level branch `minimumOpeningAngle` contains the smallest
kinematically possible laboratory opening angle of the two decay photons in
degrees. It is calculated from `pi0LabEnergy` as
`2 asin(pi0_mass / pi0LabEnergy)`. Both quantities use the configured nominal
pion and target masses and assume coherent two-body production.

The inclusive ranges `opening_angle_cut_min/max`, `pi0_mass_cut_min/max`,
`missing_energy_cut_min/max`, and `tagger_channel_cut_min/max` select the
pi0-tagger combinations stored in the combination-level tree vectors and used
for all histograms. The opening-angle cut is applied to `pi0OpeningAngle -
minimumOpeningAngle`. The reconstructed pi0 candidate vectors themselves
retain the broader `pi0_mass_min/max` selection.

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
