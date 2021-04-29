#!/usr/bin/env python3

import logging
logging.basicConfig(level=logging.INFO)
log = logging.getLogger('ArxDataVaidator')

import argparse

from math import isclose

from arx_addon.lib import ArxIO
from arx_addon.files import ArxFiles
from arx_addon.dataFts import FtsSerializer

def validateScenes():
    ioLib = ArxIO()
    ftsSerializer = FtsSerializer(ioLib)
    
    totalTriangles = 0
    totalQuads = 0
    
    minPolyPerTile =  999999
    maxPolyPerTile = -999999
    
    avgPolyPerTileSum = 0
    avgPolyPerTileCount = 0
    
    for levelId in arxFiles.levels.levels:
        parts = arxFiles.levels.levels[levelId]
        
        if not parts.fts:
            log.warn("Level {} is missing fts file".format(levelId))
            continue
        
        data = ftsSerializer.read_fts_container(parts.fts)
        
        for tileX in data.cells:
            for tile in tileX:
                if tile is not None:
                    count = len(tile)
                    minPolyPerTile = min(minPolyPerTile, count)
                    maxPolyPerTile = max(maxPolyPerTile, count)
                    
                    avgPolyPerTileSum += count
                    avgPolyPerTileCount += 1
                    
                    for poly in tile:
                        if poly.type.POLY_QUAD:
                            totalQuads += 1
                        else:
                            totalTriangles += 1
        
        
        for portal in data.portals:
            # Types other than none or quad make no sense for portals
            if not portal.poly.type in {0, 64}:
                log.warn("Portal Poly: Weird type: {}".format(portal.poly.type))
            if portal.poly.type == 0:
                log.info("Portal Poly: Not properly supported type found in {}".format(levelId))
            
            # portal.poly.norm2.x is unused but filled with data
            if isclose(portal.poly.norm2.x, 0) or isclose(portal.poly.norm2.x, 0):
                log.info("Portal Poly: bad poly.norm2 value: {}".format(portal.poly.norm2))
            
            for n in portal.poly.nrml:
                if n.x != 0 or n.y != 0 or n.z != 0:
                    log.info("Portal Poly: bad poly.nrml value: {}".format(n))
            
            if portal.poly.tex != 0:
                log.info("Portal Poly: bad poly.tex value: {}".format(portal.poly.tex))
            
            # transval makes no sense for a portal poly, the latter value was found in the data
            if portal.poly.transval != 0 and portal.poly.transval != 2.763092896901618e+29:
                log.info("Portal Poly: bad poly.transval value: {}".format(portal.poly.transval))
            
            if (portal.poly.area != 0 and
                portal.poly.area != 63903090343936 and 
                portal.poly.area != 2.8182821021424183e+20 and 
                portal.poly.area != 8.030455756812317e+33):
                
                log.info("Portal Poly: bad poly.area value: {}".format(portal.poly.area))
            
            if portal.poly.room != 0 and portal.poly.room != 26988 and portal.poly.room != 3:
                log.info("Portal Poly: bad poly.room value: {}".format(portal.poly.room))
            
            if portal.poly.misc != 0 and portal.poly.misc != 25978:
                log.info("Portal Poly: bad poly.misc value: {}".format(portal.poly.misc))
    
    print("Scene data Stats:")
    print("Tris: {} Quads: {} ratio: {}".format(str(totalTriangles), str(totalQuads), str(totalTriangles / totalQuads)))
    print("maxPolyPerTile: {}".format(maxPolyPerTile))
    print("minPolyPerTile: {}".format(minPolyPerTile))
    print("avgPolyPerTile: {}".format(avgPolyPerTileSum / avgPolyPerTileCount))

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--data-dir', help='Where to find the data files', required=True)
    args = parser.parse_args()
    print("Using data dir: " + args.data_dir)
    
    arxFiles = ArxFiles(args.data_dir)
    arxFiles.updateAll()
    
    # TODO Currently too verbose readd later
    if arxFiles.danglingPaths and False:
        print("Found unexpected files:")
        for p in arxFiles.danglingPaths:
            print(p);
    
    validateScenes()
