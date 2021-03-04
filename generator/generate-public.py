#!/usr/bin/env python3

# Copyright 2021 Hein-Pieter van Braam-Stewart
#
# This file is part of ppelib (Portable Portable Executable LIBrary)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys
from generator import generate

outdir = sys.argv[1]
mydir = os.path.dirname(os.path.abspath(__file__))

headers = [
    ["coff_symbol", "coff_symbol"],
    ["header", "header"],
    ["dos_header", "dos_header"],
    ["section", "section"],
    ["vlv_signature", "vlv_signature"],
]

for header in headers:
    generate(f"{mydir}/structures/{header[0]}.yaml", f"{mydir}/templates/public_header.h", f"{outdir}/ppelib-{header[1]}.h")
    generate(f"{mydir}/structures/{header[0]}.yaml", f"{mydir}/templates/public_header_lowlevel.h", f"{outdir}/ppelib-{header[1]}-lowlevel.h")
