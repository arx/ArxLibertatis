#!/usr/bin/env python3

import logging
logging.basicConfig(level=logging.INFO)
log = logging.getLogger('ArxDataVaidator')

import argparse

from math import isclose

from arx_addon.files import ArxFiles
from arx_addon.dataCin import CinSerializer

def checkCinematics():
    cinSerializer = CinSerializer()

    log.info('Found {} cinematics'.format((len(arxFiles.cinematics.cins))))
    for cinematicFile in arxFiles.cinematics.cins:
        cinematic = cinSerializer.read(cinematicFile)
        for keyframe in cinematic.keyframes:
            img = keyframe.image[1]
            if img not in arxFiles.cinematics.textures.keys():
                log.warning('Texture {} not found !', img)

            if keyframe.sound:
                for lang, files in arxFiles.audioSpeech.speeches.items():
                    if keyframe.sound not in files:
                        log.warning('Sound {} for Lang {} not found !'.format(keyframe.sound, lang))
                    #else:
                    #    log.info('Sound {} found'.format(keyframe.sound))


    log.info('Done')

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--data-dir', help='Where to find the data files', required=True)
    args = parser.parse_args()
    print("Using data dir: " + args.data_dir)
    
    arxFiles = ArxFiles(args.data_dir)
    arxFiles.updateAll()
    
    checkCinematics()
