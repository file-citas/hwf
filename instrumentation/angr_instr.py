import angr
import pickle
import sys
import struct
import capstone as caps

usage = "python2 angr_instr.py ../fw/SM-G920F/mfc_fw.bin bb.dict"

#   0:   e58f0014        str     r0, [pc, #20]   ; 0x1c
#   4:   e3a000ff        mov     r0, #255        ; 0xff
#   8:   e58f0010        str     r0, [pc, #16]   ; 0x20
#   c:   e59f0008        ldr     r0, [pc, #8]    ; 0x1c
stubs = [
    #[0x10, 0x00, 0x8f, 0xe5],
    [0xff, 0x00, 0xa0, 0xe3],
    [0x08, 0x00, 0x8f, 0xe5],
    #[0x04, 0x00, 0x9f, 0xe5],
]

orr_r0_r0_pc = [0xe0, 0x20, 0x00, 0x0f]

md = caps.Cs(caps.CS_ARCH_ARM, caps.CS_MODE_ARM)

prog = None
with open(sys.argv[1], 'rb') as fd:
    prog = bytearray(fd.read())

bkp_addr = len(prog) + 1
print("bkp_addr: %x" % bkp_addr)
prog.extend([0] * 4)
bm_addr = len(prog) + 1
prog.extend([0] * 4)
last_addr = len(prog) + 1

bbdict = pickle.load(open(sys.argv[2], 'rb'))
bkp_places = [bkp_addr - 1, ]

cntr = 0
for addr, bbs in bbdict.items():
    n_insns = len(bbs)
    last_bb_idx = 0
    for bb_idx, bb in enumerate(bbs):
        contains_pc = False
        for i in md.disasm(str(bytearray(bb)), addr + bb_idx*4):
            #print("%x %s" % (addr + bb_idx * 4, i.op_str))
            if "pc" in i.op_str or i.mnemonic.startswith('b'):
                #print("%x: %s" %
                #      (last_addr,
                #       map(lambda t: "%2x" % t, list(bytearray(bb)))))
                #print("0x%x:\t%s\t%s" %(i.address, i.mnemonic, i.op_str))
                contains_pc = True
            if not contains_pc:
                break
        if not contains_pc:
            break
        last_bb_idx += 1

    if n_insns - last_bb_idx < 2:
        continue

    bkp_addr_diff = last_addr - bkp_addr
    # see http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.kui0100a/armasm_cihgjhed.htm
    if bkp_addr_diff > 0xfff - 0x1c:
        print("ERROR: bkp offset changed @ %x" % last_addr)
        bkp_addr = len(prog) + 1
        print("bkp_addr: %x" % bkp_addr)
        bkp_places.append(bkp_addr - 1)
        prog.extend([0] * 4)
        bm_addr = len(prog) + 1
        prog.extend([0] * 4)
        last_addr = len(prog) + 1

        #break
    addr_diff = (last_addr - (addr + last_bb_idx*4)) / 4

    if addr_diff > 0xffffff:
        print("ERROR: addr offset too big")
        break

    #print("last_addr:%x - addr:%x" % (last_addr, addr + last_bb_idx*4))

    strr0_instr = 0xe50f0000 | (last_addr - bkp_addr + 8)
    strr0_data=[0] * 4
    strr0_data = list(bytearray(struct.pack(">I", strr0_instr)))

    ldrr0bm_instr = 0xe51f0000 | (last_addr - bm_addr + 0xc)
    ldrr0bm_data=[0] * 4
    ldrr0bm_data = list(bytearray(struct.pack(">I", ldrr0bm_instr)))

    # .. orr

    strr0bm_instr = 0xe50f0000 | (last_addr - bm_addr + 0x14)
    strr0bm_data=[0] * 4
    strr0bm_data = list(bytearray(struct.pack(">I", strr0bm_instr)))

    ldrr0_instr = 0xe51f0000 | (last_addr - bkp_addr + 0x18)
    ldrr0_data=[0] * 4
    ldrr0_data = list(bytearray(struct.pack(">I", ldrr0_instr)))

    back_instr = 0xea000000 | ((-addr_diff - 7) & 0xffffff)
    back_data=[0] * 4
    back_data = list(bytearray(struct.pack(">I", back_instr)))

    patch_instr = 0xea000000 + addr_diff - 2
    patch_data=[0] * 4
    patch_data = list(bytearray(struct.pack(">I", patch_instr)))

    #print("patching %x: %s - %s" %
    #      (addr,
    #       #map(lambda t: "%2x" % t, prog[addr:addr+4]),
    #       map(lambda t: "%2x" % t, bbs[last_bb_idx]),
    #       map(lambda t: "%2x" % t, list(bytearray(patch_data)))))

    #print("appending %x: %s" %
    #      (last_addr,
    #       map(lambda t: "%2x" % t, list(bytearray(back_data)))))

    #for i in md.disasm(str(bytearray(bbs[last_bb_idx])), addr + last_bb_idx*4):
    #    print("0x%x:\t%s\t%s" %(i.address, i.mnemonic, i.op_str))


    prog[addr+ last_bb_idx*4:addr+ last_bb_idx*4+4] = patch_data[::-1]
    prog.extend(strr0_data[::-1])
    prog.extend(ldrr0bm_data[::-1])
    prog.extend(orr_r0_r0_pc[::-1])
    prog.extend(strr0bm_data[::-1])
    #prog.extend(stubs[0])
    #prog.extend(stubs[1])
    prog.extend(ldrr0_data[::-1])
    #prog.extend(stubs[2])
    #prog.extend(stubs[3])
    prog.extend(bbs[last_bb_idx])
    prog.extend(back_data[::-1])
    #prog.extend([0] * 4)
    #prog.extend([0] * 4)

    last_addr = len(prog) + 1

    #if last_addr >= 0x100000 - 0x1000:
    #    break
    if cntr > 128:
        break
    cntr += 1

print("#Patches: %d" % cntr)
with open("newfw.bin", "wb") as fd:
    fd.write(prog)

print("unsigned int bm_addr_len = %d;" % len(bkp_places))
print("unsigned int bm_addr[] = {" + ", ".join(map(lambda t: "%#x" % t, bkp_places)) + ", 0};")
