import angr
import pickle
import sys

usage = "python2 create_bbdict.py mfc_fw.elf(!!elf!)"

proj = angr.Project(sys.argv[1], auto_load_libs=False)
cfg = proj.analyses.CFGFast()
outdata = {}
for addr, block in cfg.get_bbl_dict().items():
    outdata[addr - 0x400000] = []
    for i in range(block.size/4):
        b = list(bytearray(block.byte_string))[i*4:(i+1)*4]
        outdata[addr - 0x400000].append(b)
pickle.dump(outdata, open("bbdict", "wb"))
