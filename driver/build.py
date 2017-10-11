import argparse
import sys
sys.path.append("dr_checker/helper_scripts/runner_scripts")
from components import *

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('-l', action='store', dest='llvm_bc_out', required=True,
                        help='Destination directory where all the generated bitcode files should be stored.')
    parser.add_argument('-g', action='store', dest='compiler_name', required=True,
                        help='Name of the compiler used in the makeout.txt')
    parser.add_argument('-k', action='store', dest='kernel_src_dir', required=True,
                        help='Base directory of the kernel sources.')
    parser.add_argument('-m', action='store', dest='makeout', required=True,
                        help='Path to the makeout.txt file.')
    return parser.parse_args()

def main():
    args = parse_args()
    if not os.path.exists(args.llvm_bc_out):
        print("%s does not exist." % args.llvm_bc_out)
        return
    if not os.path.exists(args.kernel_src_dir):
        print("%s does not exist." % args.kernel_src_dir)
        return
    if not os.path.exists(args.makeout):
        print("%s does not exist." % args.makeout)
        return

    arg_dict = dict()
    arg_dict['llvm_bc_out'] = args.llvm_bc_out
    arg_dict['kernel_src_dir'] = args.kernel_src_dir
    arg_dict['makeout'] = args.makeout
    arg_dict['compiler_name'] = args.compiler_name
    arg_dict['arch_num'] = 2 # 32 bit (1) or 64 bit (2)
    arg_dict['clangbin'] = os.path.abspath('llvm/build/bin/clang')

    target_components = list()
    target_components.append(LLVMBuild(arg_dict))

    component_times = {}
    for curr_comp in target_components:
        component_name = curr_comp.get_name()
        ret_val = __run_component(curr_comp, component_times)

def __run_component(component_obj, component_times):
    print("Running component: ", component_obj.get_name())
    setup_msg = component_obj.setup()
    if setup_msg is None:
        ret_val = component_obj.perform()
        if ret_val:
            return True
        else:
            if not component_obj.is_critical():
                return True
    else:
        print("Setup failed for component: ", component_obj.get_name(), " with error: ", setup_msg)
    return False

if __name__ == "__main__":
    main()
