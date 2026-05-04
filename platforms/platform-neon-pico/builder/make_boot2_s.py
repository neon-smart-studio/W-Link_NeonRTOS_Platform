import sys

bin_path = sys.argv[1].replace("\\", "/")
out_path = sys.argv[2]

with open(out_path, "w", encoding="utf-8") as f:
    f.write('.section .boot2, "a"\n')
    f.write('.global __boot2_blob_start\n')
    f.write('__boot2_blob_start:\n')
    f.write(f'.incbin "{bin_path}"\n')
    f.write('.global __boot2_blob_end\n')
    f.write('__boot2_blob_end:\n')