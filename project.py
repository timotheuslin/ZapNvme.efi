#!/usr/bin/env python
#
# -*- coding: utf-8 -*-
# pylint: disable=invalid-name, line-too-long
#
# (c) 2020 Timothy Lin <timothy.gh.lin@gmail.com>, BSD 3-Clause License.
#

""" This is the project configuration file as well the starter script for iPug."""

import os

DEFAULT_EDK2_TAG = 'edk2-stable202008'
DEFAULT_UDK_DIR = os.environ.get('UDK_DIR', os.path.join(os.getcwd(), 'edk2'))
#DEFAULT_EDK2_REPO = os.environ.get('EDK2_REPO', 'https://github.com/tianocore/edk2.git')

CODETREE = {
    'Build_Tip'        : {
        'path'          : os.getcwd(),
        'multiworkspace': True,
    },
}


###################################################################################################


if __name__ == '__main__':
    import sys
    sys.dont_write_bytecode = True      # To inhibit the creation of .pyc file

    PKG_DSC = 'ZapNvme.dsc'
    CMDX = " ".join(sys.argv[1:])
    IPUG_CMD = f"ipug -p {PKG_DSC} {CMDX}"
    print(IPUG_CMD)
    os.environ['PYTHON_COMMAND']='python3'
    os.system(IPUG_CMD)
