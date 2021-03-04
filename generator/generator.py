#!/usr/bin/env python

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
import yaml
import itertools
from jinja2 import Template, Environment, FileSystemLoader

import pprint
from builtins import max
pp = pprint.PrettyPrinter(indent=4).pprint

sizes = {
    "uint8_t": 1,
    "uint16_t": 2,
    "uint32_t": 4,
    "uint64_t": 8,
    "string_name": 8,
}

max_sizes = {
    "uint8_t": "UINT8_MAX",
    "uint16_t": "UINT16_MAX",
    "uint32_t": "UINT32_MAX",
    "uint65_t": "UINT64_MAX",
}

mydir = os.path.dirname(os.path.abspath(__file__))


def snake_case(string):
    block_of_upper = False
    prev_upper = False
    at_start = True
    retval = ""

    char_iter, next_char_iter = itertools.tee(iter(string))
    next_char = next(next_char_iter, 'o')

    for char in char_iter:
        next_char = next(next_char_iter, 'o')

        if char.isupper():
            if prev_upper:
                if next_char.islower():
                    retval += "_"

            if not prev_upper and not at_start:
                retval += "_"
            prev_upper = True

        else:
            prev_upper = False

        retval += char.lower()
        at_start = False

    return retval


def generate_extradata(structure):
    common_size = 0
    pe_size = 0
    peplus_size = 0

    for field in structure["fields"]:
        field["struct_name"] = snake_case(field["name"])
        field["pe_offset"] = pe_size
        field["peplus_offset"] = peplus_size
        field["common"] = False

        if "peplus_type" in field:
            if field["peplus_type"]:
                peplus_size += sizes[field["peplus_type"]]
                field["getset_type"] = field["peplus_type"]

        else:
            if not "pe_only" in field:
                peplus_size += sizes[field["pe_type"]]

            field["getset_type"] = field["pe_type"]

        pe_size += sizes[field["pe_type"]]
        if "peplus_offset" in field and field["pe_offset"] == field["peplus_offset"]:
            if "peplus_type" in field and field["pe_type"] != field["peplus_type"]:
                continue
            if "pe_only" in field:
                continue

            field["common"] = True
            common_size = max(common_size, pe_size)

    structure["pe_size"] = pe_size
    structure["peplus_size"] = peplus_size
    structure["common_size"] = common_size
    structure["max_sizes"] = max_sizes


def generate(template, infile, outfile):
    f = {}
    with open(template, 'r') as stream:
        try:
            f = yaml.safe_load(stream)
        except yaml.YAMLError as exc:
            print(exc)
            sys.exit(1)

    generate_extradata(f)

    with open(infile, "r") as file_:
        template = Environment(loader=FileSystemLoader(f".")).from_string(file_.read())
        with open(f'{outfile}', 'w') as outfile:
            outfile.write(template.render(s=f))
            outfile.write("\n")


if __name__ == "__main__":
    template = sys.argv[1]
    infile = sys.argv[2]
    outfile = sys.argv[3]

    generate(template, infile, outfile)
