import numpy as np

# 512x512 for the denex
# +1 for the monitor
spec_numbers = np.arange(512*512 + 1, dtype=int)
det_numbers = spec_numbers

detspecmap = np.hstack((spec_numbers.reshape(spec_numbers.size, 1), det_numbers.reshape(det_numbers.size, 1)))

output_filename = "denex_detspecmap.dat"
with open(output_filename, 'w') as output_file:
    output_file.write("Number_of_entries\n")
    output_file.write(f"{spec_numbers.size}\n")
    output_file.write("Detector\tSpectrum\n")
    np.savetxt(output_file, detspecmap, delimiter="\t", fmt='%d')
