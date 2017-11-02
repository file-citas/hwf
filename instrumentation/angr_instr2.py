import pickle
import sys
import struct
import capstone as caps
#7e890

usage = "python2 angr_instr.py ../fw/SM-G920F/mfc_fw.bin bb.dict"

#   0:   00000000
#   4:   00000000
# //   set basic block bitmap (included once)
#   8:   e51f0010        ldr     r0, [pc, #-16]  ; 0x0
#   c:   e0210000        eor     r0, r1, r0
#  10:   e50f0018        str     r0, [pc, #-24]  ; 0x0
#  14:   ea000006        b       0x34
#        ...
# // jump stub code for each instrumentation
#  28:   e92d000e        push    {r1, r2, r3}       // save some state
#  2c:   e59f1008        ldr     r1, [pc, #8]    ; 0x3c // load bb id into r1
#  30:   eafffff4        b       0x8                // branch to prev function
#  34:   e8bd000e        pop     {r1, r2, r3}       // restore state
#  38:   eafffff0        b       0x0                // branch back to fw
#  3c:   00012345        andeq   r2, r1, r5, asr #6 // bb ID


set_bb_stub = {
    'bm0': [0x00, 0x00, 0x00, 0x00]
    'bm1': [0x00, 0x00, 0x00, 0x00]
    'ldr_bm2r0': [0xe5, 0x1f, 0x00, 0x10],
    'eor_r0_r1': [0xe0, 0x21, 0x00, 0x00],
    'str_bm2r0': [0xe5, 0x0f, 0x00, 0x18],
    'b_back': [0xea, 0x00, 0x00, 0x00],
}

intr_stub = {
    'push': [0xe9, 0x2d, 0x00, 0x0e],
    'ldr_id2r1': [0xe5, 0x9f, 0x10, 0x08],
    'b_bb_stub': [0xea, 0x00, 0x00, 0x00],
    'pop': [0xe8, 0xbd, 0x00, 0x0e],
    'b_back': [0xea, 0x00, 0x00, 0x00],
    'id': [0x00, 0x00, 0x00, 0x00]
}

md = caps.Cs(caps.CS_ARCH_ARM, caps.CS_MODE_ARM)

prog = None
with open(sys.argv[1], 'rb') as fd:
    prog = bytearray(fd.read())

print("bm_addr: %x" % bm_addr)
prog.extend(set_bb_stub['bm0'][::-1])
prog.extend(set_bb_stub['bm1'][::-1])
prog.extend(set_bb_stub[''][::-1])
prog.extend([0] * 4)

last_addr = len(prog) + 1

bbdict = pickle.load(open(sys.argv[2], 'rb'))


