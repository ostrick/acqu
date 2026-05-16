import re

E_BEAM = 855.0

old_config = "FPD_old.dat"
new_egamma_file = "new_egamma.dat"
output_config = "FPD_855_new.dat"

# ------------------------------------------------------------
# Read new gamma-energy table.
# Expected lines like:
#   1. 797.626 0.455
#   2. 797.190 0.467
# or:
#   1 797.626 0.455
# ------------------------------------------------------------
rows = []

with open(new_egamma_file, "r") as f:
    for line in f:
        line = line.strip()

        if not line or line.startswith("#"):
            continue

        parts = line.split()

        if len(parts) < 3:
            continue

        # Accept both "1" and "1."
        idx_str = parts[0].replace(".", "")

        try:
            idx = int(idx_str)
            egamma = float(parts[1])
            width = float(parts[2])
        except ValueError:
            continue

        rows.append((idx, egamma, width))

print(f"Loaded {len(rows)} gamma-energy rows")

if len(rows) < 328:
    raise RuntimeError("The new energy table has fewer than 328 rows.")

# ------------------------------------------------------------
# Reverse mapping:
#
# Element 0   <- last row of new table
# Element 1   <- second-last row
# ...
# Element 327 <- 328th row from the end
#
# Since the FPD file has Size: 328, only 328 elements are used.
# ------------------------------------------------------------
rows_reversed = list(reversed(rows))

egamma_for_element = {}

for element in range(328):
    idx, egamma, width = rows_reversed[element]
    egamma_for_element[element] = egamma

# ------------------------------------------------------------
# Replace the electron-energy field in Element lines.
#
# In your FPD lines the final fields look like:
#   ... 840.3 0.0 2871
#
# The energy field is tokens[-3].
# The config wants electron energy:
#   E_e = E_beam - E_gamma
# ------------------------------------------------------------
element_re = re.compile(r"^\s*Element:\s+(\d+)\s+")

n_replaced = 0

with open(old_config, "r") as fin, open(output_config, "w") as fout:
    for line in fin:
        m = element_re.match(line)

        if not m:
            fout.write(line)
            continue

        element = int(m.group(1))

        if element not in egamma_for_element:
            fout.write(line)
            continue

        Egamma = egamma_for_element[element]
        Ee = E_BEAM - Egamma

        tokens = line.split()

        old_energy = tokens[-3]
        tokens[-3] = f"{Ee:.3f}"

        fout.write(" ".join(tokens) + "\n")

        n_replaced += 1

print(f"Replaced {n_replaced} Element energies")
print(f"Wrote output file: {output_config}")

print("\nSanity check:")
for element in [0, 1, 2, 58, 59, 327]:
    Egamma = egamma_for_element[element]
    Ee = E_BEAM - Egamma
    print(
        f"Element {element:3d}: "
        f"Egamma = {Egamma:8.3f} MeV, "
        f"Ee = 855 - Egamma = {Ee:8.3f} MeV"
    )
