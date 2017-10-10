import argparse
import sys
sys.path.append("dr_checker/helper_scripts/runner_scripts")
from components import *

def main():
    parser = argparse.ArgumentParser()
    arg_dict = dict()
    target_components = list()
    target_components.append(LLVMBuild(arg_dict))
    target_components.append(DriverLinker(arg_dict))

if __name__ == "__main__":
    main()
