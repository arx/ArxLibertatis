#!/usr/bin/env python3

import argparse

from arx_addon import dataFtl, lib

parser = argparse.ArgumentParser()
parser.add_argument('-data')

args = parser.parse_args()

from arx_addon.files import *

import logging

logging.basicConfig(level=logging.DEBUG)
log = logging.getLogger('ArxAssetChecker')


def roundtripModel(path):
    f = open(fileName, "rb")
    data = f.read()
    f.close()
    unpacked = ioLib.unpack(data)

    log.info("Loaded %i bytes from file %s" % (len(unpacked), fileName))
    with open("test1.ftl", 'wb') as f:
        f.write(unpacked)

    result1 = serializer.read(unpacked)

    log.info("Verts: %i" % len(result1.verts))
    firstResult = serializer.write(result1)
    with open("test2.ftl", 'wb') as f:
        f.write(firstResult)

    result3 = serializer.read(firstResult)

    secondResult = serializer.write(result3)
    with open("test3.ftl", 'wb') as f:
        f.write(secondResult)

    if firstResult != secondResult:
        raise AssertionError("Result mismatch")


if args.data:
    print("Using data dir: " + args.data)

    arxFiles = ArxFiles(args.data)
    arxFiles.updateAll()

    ioLib = lib.ArxIO()
    serializer = dataFtl.FtlSerializer(ioLib)

    # problematic stuff
    # interactive/fix_inter/hanged_gob
    # key = ('npc', 'goblin_king')
    # if 1:
    for key in arxFiles.models.data:
        val = arxFiles.models.data[key]

        fileName = os.path.join(val.path, val.model)
        roundtripModel(fileName)

        for tweak in val.tweaks:
            fileName = os.path.join(val.path, "tweaks", tweak)
            roundtripModel(tweak)
