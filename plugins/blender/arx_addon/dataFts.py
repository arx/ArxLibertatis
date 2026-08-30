# Copyright 2014-2020 Arx Libertatis Team (see the AUTHORS file)
#
# This file is part of Arx Libertatis.
#
# Arx Libertatis is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Arx Libertatis is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Arx Libertatis. If not, see <http://www.gnu.org/licenses/>.

from ctypes import (
    LittleEndianStructure,
    c_char,
    c_uint32,
    c_int16,
    c_int32,
    c_float
)

from .dataCommon import SavedVec3, PolyTypeFlag
from collections import deque
import os

class UNIQUE_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("path",             c_char * 256),
        ("count",            c_int32),
        ("version",          c_float),
        ("uncompressedsize", c_int32),
        ("pad",              c_int32 * 3)
    ]

class UNIQUE_HEADER3(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("path",  c_char * 256), # In the c code this is currently in a separate struct
        ("check", c_char * 512)
    ]

class FAST_SCENE_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("version",     c_float),
        ("sizex",       c_int32),
        ("sizez",       c_int32),
        ("nb_textures", c_int32),
        ("nb_polys",    c_int32),
        ("nb_anchors",  c_int32),
        ("playerpos",   SavedVec3),
        ("Mscenepos",   SavedVec3),
        ("nb_portals",  c_int32),
        ("nb_rooms",    c_int32)
    ]

class FAST_TEXTURE_CONTAINER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("tc",   c_int32),
        ("temp", c_int32),
        ("fic",  c_char * 256)
    ]

class FAST_SCENE_INFO(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("nbpoly",     c_int32),
        ("nbianchors", c_int32),
    ]

class FAST_VERTEX(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("sy",  c_float),
        ("ssx", c_float),
        ("ssz", c_float),
        ("stu", c_float),
        ("stv", c_float)
    ]

class FAST_EERIEPOLY(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("v",        FAST_VERTEX * 4),
        ("tex",      c_int32),
        ("norm",     SavedVec3),
        ("norm2",    SavedVec3),
        ("nrml",     SavedVec3 * 4),
        ("transval", c_float),
        ("area",     c_float),
        ("type",     PolyTypeFlag),
        ("room",     c_int16),
        ("paddy",    c_int16)
    ]

class FAST_ANCHOR_DATA(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("pos",       SavedVec3),
        ("radius",    c_float),
        ("height",    c_float),
        ("nb_linked", c_int16),
        ("flags",     c_int16)
    ]

class SavedTextureVertex(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("pos",      SavedVec3),
        ("rhw",      c_float),
        ("color",    c_uint32),
        ("specular", c_uint32),
        ("tu",       c_float),
        ("tv",       c_float)
    ]

class SAVE_EERIEPOLY(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("type",     c_int32),
        ("min",      SavedVec3),
        ("max",      SavedVec3),
        ("norm",     SavedVec3),
        ("norm2",    SavedVec3),
        ("v",        SavedTextureVertex * 4),
        ("tv",       SavedTextureVertex * 4),
        ("nrml",     SavedVec3 * 4),
        ("tex",      c_int32),
        ("center",   SavedVec3),
        ("transval", c_float),
        ("area",     c_float),
        ("room",     c_int16),
        ("misc",     c_int16)
    ]

class EERIE_SAVE_PORTALS(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("poly",      SAVE_EERIEPOLY),
        ("room_1",    c_int32),
        ("room_2",    c_int32),
        ("useportal", c_int16),
        ("paddy",     c_int16)
    ]
    
class EERIE_SAVE_ROOM_DATA(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("nb_portals", c_int32),
        ("nb_polys",   c_int32),
        ("padd",       c_int32 * 6)
    ]

class FAST_EP_DATA(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("px",   c_int16),
        ("py",   c_int16),
        ("idx",  c_int16),
        ("padd", c_int16)
    ]
    
class ROOM_DIST_DATA_SAVE(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("distance", c_float),
        ("startpos", SavedVec3),
        ("endpos",   SavedVec3),
    ]


from collections import namedtuple

FtsData = namedtuple('FtsData', ['sceneOffset', 'textures', 'cells', 'cell_anchors', 'anchors', 'portals', 'room_data'])

import logging

from ctypes import sizeof
from .lib import ArxIO

def portal_centre(portal):
    """Centre of a portal quad, from either a parsed struct or raw bytes."""
    if isinstance(portal, bytes):
        portal = EERIE_SAVE_PORTALS.from_buffer_copy(portal)
    poly = portal.poly
    return (sum(poly.v[i].pos.x for i in range(4)) / 4.0,
            sum(poly.v[i].pos.y for i in range(4)) / 4.0,
            sum(poly.v[i].pos.z for i in range(4)) / 4.0), portal.room_1, portal.room_2


def compute_room_distances(portals, room_count):
    """Shortest route between every pair of rooms, in the form the engine wants.

    SP_GetRoomDist does not use the stored distance by itself. It returns

        fdist(from, startpos) + distance + fdist(endpos, to)

    so startpos and endpos have to be real waypoints on the route - the centres of
    the first and last doorway along it - and distance the length of the run
    between those doorways. Zeroed waypoints make every cross room measurement as
    large as the level's distance from the world origin, which pushes entities out
    of the treat zone (PrepareIOTreatZone) and freezes them: no physics, no
    drawing, until the player walks into the same room.

    Returns {(start, end): (distance, startpos, endpos)}. Pairs with no route are
    absent, and the caller should leave those at distance 0, which the engine
    reads as "no route" and answers with straight line distance instead.
    """
    import heapq
    import math

    centres = []
    links = []
    for portal in portals:
        centre, room_1, room_2 = portal_centre(portal)
        centres.append(centre)
        links.append({room_1, room_2})

    def gap(a, b):
        return math.sqrt(sum((centres[a][i] - centres[b][i]) ** 2 for i in range(3)))

    result = {}
    for start in range(1, room_count + 1):
        best = [math.inf] * len(portals)
        entry = [None] * len(portals)
        queue = []
        for i, rooms in enumerate(links):
            if start in rooms:
                best[i] = 0.0
                entry[i] = i
                heapq.heappush(queue, (0.0, i))

        while queue:
            cost, i = heapq.heappop(queue)
            if cost > best[i]:
                continue
            for j, rooms in enumerate(links):
                if i == j or not (links[i] & rooms):
                    continue
                step = cost + gap(i, j)
                if step < best[j]:
                    best[j] = step
                    entry[j] = entry[i]
                    heapq.heappush(queue, (step, j))

        for end in range(1, room_count + 1):
            if end == start:
                continue
            reachable = [(best[i], entry[i], i) for i, rooms in enumerate(links)
                         if end in rooms and best[i] < math.inf]
            if not reachable:
                continue
            cost, first, last = min(reachable)
            # Adjacent rooms come out at zero, which the engine reads as no route.
            result[(start, end)] = (max(cost, 1.0), centres[first], centres[last])

    return result


class FtsSerializer(object):
    def __init__(self, ioLib):
        self.log = logging.getLogger('FtsSerializer')
        self.ioLib = ioLib
    def read_fts(self, data) -> FtsData:
        """If you want to read a fts file use read_fts_container"""

        pos = 0
        ftsHeader = FAST_SCENE_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(FAST_SCENE_HEADER)
        self.log.debug("Fts Header version: %f" % ftsHeader.version)
        self.log.debug("Fts Header size x,z: %i,%i" % (ftsHeader.sizex, ftsHeader.sizez))
        self.log.debug("Fts Header playerpos: %f,%f,%f" % (ftsHeader.playerpos.x, ftsHeader.playerpos.y, ftsHeader.playerpos.z))
        self.log.debug("Fts Header Mscenepos: %f,%f,%f" % (ftsHeader.Mscenepos.x, ftsHeader.Mscenepos.y, ftsHeader.Mscenepos.z))
        # --- Added: Log nb_rooms ---
        self.log.debug("Fts Header nb_rooms: %i" % ftsHeader.nb_rooms)
        
        sceneOffset = (ftsHeader.Mscenepos.x, ftsHeader.Mscenepos.y, ftsHeader.Mscenepos.z)

        texturesType = FAST_TEXTURE_CONTAINER * ftsHeader.nb_textures
        textures_array = texturesType.from_buffer_copy(data, pos)
        pos += sizeof(texturesType)
        # Convert ctypes structures to Python dicts to avoid pickle issues
        textures = []
        for tex in textures_array:
            texture_dict = {
                'tc': tex.tc,
                'temp': tex.temp,
                'fic': bytes(tex.fic)  # Convert to bytes for pickle safety
            }
            textures.append(texture_dict)
        self.log.debug("Loaded %i textures" % len(textures))

        #for i in textures:
        #    log.info(i.fic.decode('iso-8859-1'))
        
        cells = [[None for x in range(ftsHeader.sizex)] for x in range(ftsHeader.sizez)]
        cell_anchors = [[None for x in range(ftsHeader.sizex)] for x in range(ftsHeader.sizez)]  # Store anchor indices per cell
        
        for z in range(ftsHeader.sizez):
            for x in range(ftsHeader.sizex):
                cellHeader = FAST_SCENE_INFO.from_buffer_copy(data, pos)
                pos += sizeof(FAST_SCENE_INFO)

                try:
                    if cellHeader.nbpoly <= 0:
                        cells[z][x] = None
                    else:
                        polysType = FAST_EERIEPOLY * cellHeader.nbpoly
                        polys = polysType.from_buffer_copy(data, pos)
                        pos += sizeof(polysType)

                        cells[z][x] = polys
                except ValueError as e:
                    print("Failed reading cell data, x:%i z:%i polys:%i" % (x, z, cellHeader.nbpoly))
                    raise e

                    
                if cellHeader.nbianchors > 0:
                    AnchorsArrayType = c_int32 * cellHeader.nbianchors
                    anchors = AnchorsArrayType.from_buffer_copy(data, pos)
                    pos += sizeof(AnchorsArrayType)
                    # Store anchor indices for this cell
                    cell_anchors[z][x] = list(anchors)
                else:
                    cell_anchors[z][x] = []
                        
        anchors = []
        for i in range(ftsHeader.nb_anchors):
            anchor = FAST_ANCHOR_DATA.from_buffer_copy(data, pos)
            pos += sizeof(FAST_ANCHOR_DATA)
            
            if anchor.nb_linked > 0:
                nb_linked = int(anchor.nb_linked)  # Ensure it's an integer
                LinkedAnchorsArrayType = c_int32 * nb_linked
                linked = LinkedAnchorsArrayType.from_buffer_copy(data, pos)
                pos += sizeof(LinkedAnchorsArrayType)
                # Convert ctypes array to Python list to avoid pickle issues
                linked_list = [int(linked[i]) for i in range(nb_linked)]
                anchors.append( ((float(anchor.pos.x), float(anchor.pos.y), float(anchor.pos.z)), linked_list, float(anchor.radius), float(anchor.height), int(anchor.flags)) )
            else:
                anchors.append( ((float(anchor.pos.x), float(anchor.pos.y), float(anchor.pos.z)), [], float(anchor.radius), float(anchor.height), int(anchor.flags)) )
        
        portals = []
        for i in range(ftsHeader.nb_portals):
            portal = EERIE_SAVE_PORTALS.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_SAVE_PORTALS)
            # Convert to binary data for pickle safety
            portals.append(bytes(portal))

        # Read room data structures and convert to Python data
        room_data = []
        for i in range(ftsHeader.nb_rooms + 1): # Off by one in data
            room = EERIE_SAVE_ROOM_DATA.from_buffer_copy(data, pos)
            pos += sizeof(EERIE_SAVE_ROOM_DATA)
            
            # Convert room info to Python dict
            room_info_dict = {
                'nb_portals': room.nb_portals,
                'nb_polys': room.nb_polys,
                'padd': [room.padd[j] for j in range(6)]  # Convert ctypes array to Python list
            }
            
            room_portal_indices = []
            if room.nb_portals > 0:
                PortalsArrayType = c_int32 * room.nb_portals
                portals2 = PortalsArrayType.from_buffer_copy(data, pos)
                pos += sizeof(PortalsArrayType)
                room_portal_indices = list(portals2)  # This creates Python ints, not ctypes
                
            room_poly_refs = []
            if room.nb_polys > 0:
                PolysArrayType = FAST_EP_DATA * room.nb_polys
                polys2 = PolysArrayType.from_buffer_copy(data, pos)
                pos += sizeof(PolysArrayType)
                # Convert to Python dicts instead of ctypes
                for poly_ref in polys2:
                    poly_dict = {
                        'px': poly_ref.px,
                        'py': poly_ref.py,
                        'idx': poly_ref.idx,
                        'padd': poly_ref.padd
                    }
                    room_poly_refs.append(poly_dict)
            
            room_data.append((room_info_dict, room_portal_indices, room_poly_refs))
        
        # Read room distance matrix and convert to binary data
        room_distances = []
        distance_matrix_size = ftsHeader.nb_rooms + 1
        for i in range(distance_matrix_size):
            row = []
            for j in range(distance_matrix_size):
                dist = ROOM_DIST_DATA_SAVE.from_buffer_copy(data, pos)
                pos += sizeof(ROOM_DIST_DATA_SAVE)
                # Convert to binary data for pickle safety
                row.append(bytes(dist))
            room_distances.append(row)
                
        self.log.debug("Loaded %i bytes of %i" % (pos, len(data)))

        # --- Modified: Return header along with FtsData ---
        return ftsHeader, FtsData(
            sceneOffset=sceneOffset,
            textures=textures,
            cells=cells,
            cell_anchors=cell_anchors,
            anchors=anchors,
            portals=portals,
            room_data=(room_data, room_distances)
        )

    def read_fts_container(self, filepath) -> FtsData:
        f = open(filepath, "rb")
        data = f.read()
        f.close()

        self.log.debug("Loaded %i bytes from file %s" % (len(data), filepath))
        
        pos = 0
        
        primaryHeader = UNIQUE_HEADER.from_buffer_copy(data, pos)
        pos += sizeof(UNIQUE_HEADER)
        self.log.debug("Header path: %s" % primaryHeader.path.decode('iso-8859-1'))
        self.log.debug("Header count: %i" % primaryHeader.count)
        self.log.debug("Header version: %f" % primaryHeader.version)
        self.log.debug("Header uncompressedsize: %i" % primaryHeader.uncompressedsize)
            
        secondaryHeadersType = UNIQUE_HEADER3 * primaryHeader.count
        secondaryHeaders = secondaryHeadersType.from_buffer_copy(data, pos)
        pos += sizeof(secondaryHeadersType)
        
        for h in secondaryHeaders:
            self.log.debug("Header2 path: %s" % h.path.decode('iso-8859-1'))
        
        self.log.debug(f"About to unpack from position {pos} (0x{pos:x}), remaining data: {len(data) - pos} bytes")
        if primaryHeader.uncompressedsize:
            uncompressed = self.ioLib.unpack(data[pos:])
            if primaryHeader.uncompressedsize != len(uncompressed):
                self.log.warn("Uncompressed size mismatch, expected %i actual %i" % (primaryHeader.uncompressedsize, len(uncompressed)))
        else:
            # An uncompressedsize of 0 means the payload was never imploded. The
            # engine reads it as is (FastSceneLoad only calls blast when the field
            # is set), so hand written scenes are allowed to skip compression.
            self.log.debug("Container reports no compression, reading payload as is")
            uncompressed = data[pos:]
        
        # Store header for use in write operations
        ftsHeader, fts_data = self.read_fts(uncompressed)
        self._original_header = ftsHeader  # Store for write_fts_container
        self._original_uncompressed_size = primaryHeader.uncompressedsize  # Store size from container header
        self.log.info(f"Stored original header: uncompressed={primaryHeader.uncompressedsize}, nb_rooms={ftsHeader.nb_rooms}")
        return fts_data
    
    def write_fts_container(self, filepath, fts_data: FtsData, updated_cells=None):
        """Write FTS data to a file container with PKWare compression matching original"""
        self.log.info(f"Writing FTS file: {filepath}")
        
        # Use updated cells if provided, otherwise use original
        cells_to_write = updated_cells if updated_cells is not None else fts_data.cells
        
        # Build FTS data
        fts_binary_data = self.write_fts(fts_data, cells_to_write)
        self.log.info(f"Generated FTS binary data: {len(fts_binary_data)} bytes")
        
        # Debug: compare with original expected size
        if hasattr(self, '_original_uncompressed_size'):
            expected_size = self._original_uncompressed_size
            size_diff = len(fts_binary_data) - expected_size
            self.log.info(f"Size difference from original: {size_diff} bytes (generated: {len(fts_binary_data)}, expected: {expected_size})")
            if abs(size_diff) > 1000:
                self.log.warning(f"Large size difference detected - possible data corruption")
        
        # Use PKWare compression with dict=6 to match original
        compressed_data = self._encode_pkware(fts_binary_data)
        self.log.info(f"PKWare compressed data: {len(compressed_data)} bytes")
        
        # Create container headers with actual data size
        headers = self._create_fts_headers(len(fts_binary_data), len(compressed_data))
        self.log.info(f"Header claims uncompressed size: {len(fts_binary_data)}")
        
        # Calculate padding needed to match original file structure
        # Original: headers end at 1816, compressed data starts at 1816
        # Our headers: end at 1040, so we need 776 bytes of padding
        padding_size = 1816 - len(headers)
        padding = b'\x00' * padding_size
        
        # Write PKWare-compressed file matching original structure
        with open(filepath, "wb") as f:
            f.write(headers)  # Headers first (1040 bytes)
            f.write(padding)  # Padding to match original (776 bytes)
            f.write(compressed_data)  # PKWare compressed data (starts at 1816)
        
        self.log.info(f"Exported PKWare compressed FTS: {len(fts_binary_data)} → {len(compressed_data)} bytes")
    
    def _encode_pkware(self, data):
        """Clean PKWare encoding implementation based on C++ blast specification"""
        
        # Create bitstream encoder
        encoder = self._PKWareEncoder()
        
        # Write PKWare header as part of bitstream (not separate bytes)
        # From blast.cpp lines 359-366: lit and dict are read using bits(s, 8)
        # Use dict_size=6 to match original FTS files
        encoder.write_header(lit_flag=0, dict_size=6)
        
        # Encode all input bytes as uncoded literals
        for byte_val in data:
            encoder.write_literal(byte_val)
        
        # Write end-of-stream marker (length 519)
        encoder.write_end_of_stream()
        
        # Return bitstream as bytes (no separate header)
        return encoder.get_bytes()
    
    
    class _PKWareEncoder:
        """Clean PKWare encoder implementation based on ArxLibertatis blast.cpp"""
        
        def __init__(self):
            self.bits = []
            
            # Constants from ArxLibertatis/src/io/Blast.cpp lines 343-347
            self.BASE = [3, 2, 4, 5, 6, 7, 8, 9, 10, 12, 16, 24, 40, 72, 136, 264]
            self.EXTRA = [0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8]
            self.LENLEN = [2, 35, 36, 53, 38, 23]  # Line 340
            
            # Derived constants from C++ arrays
            self.MAX_LENGTH_SYMBOLS = len(self.BASE)  # 16 symbols (0-15)
            self.END_SYMBOL = self.MAX_LENGTH_SYMBOLS - 1  # Symbol 15 for end-of-stream
            self.END_LENGTH = self.BASE[self.END_SYMBOL] + ((1 << self.EXTRA[self.END_SYMBOL]) - 1)  # 519
            self.BYTE_BITS = 8
            
            # Build length code Huffman table
            self.length_codes = self._build_length_table()
        
        def _build_length_table(self):
            """Build Huffman table for length codes using C++ construct() algorithm"""
            # Decode compact lenlen format: each byte = (length & 15) | (count-1)<<4
            # From C++ construct() lines 224-233
            code_lengths = []
            for packed_val in self.LENLEN:
                length = (packed_val & 15) + 2  # Bottom 4 bits + 2
                count = (packed_val >> 4) + 1   # Top 4 bits + 1
                code_lengths.extend([length] * count)
            
            # Pad to MAX_LENGTH_SYMBOLS
            while len(code_lengths) < self.MAX_LENGTH_SYMBOLS:
                code_lengths.append(0)
            
            # Generate canonical Huffman codes with bit reversal
            # From C++ decode() lines 134-145: bits are inverted
            codes = [0] * self.MAX_LENGTH_SYMBOLS
            code = 0
            max_code_length = max(code_lengths) if code_lengths else 0
            
            for bit_length in range(1, max_code_length + 1):
                for symbol in range(self.MAX_LENGTH_SYMBOLS):
                    if code_lengths[symbol] == bit_length:
                        # Store bit-reversed code for PKWare format (blast.cpp line 165)
                        codes[symbol] = self._reverse_bits(code, bit_length)
                        code += 1
                code <<= 1
            
            return [(codes[i], code_lengths[i]) for i in range(self.MAX_LENGTH_SYMBOLS)]
        
        def _reverse_bits(self, value, num_bits):
            """Reverse bit order for PKWare compatibility (blast.cpp line 165)"""
            result = 0
            for i in range(num_bits):
                if value & (1 << i):
                    result |= (1 << (num_bits - 1 - i))
            return result
        
        def write_header(self, lit_flag, dict_size):
            """Write PKWare header as part of bitstream (blast.cpp lines 359-366)"""
            # Write lit flag (8 bits) - blast.cpp line 359: lit = bits(s, 8)
            for i in range(self.BYTE_BITS):
                self.bits.append((lit_flag >> i) & 1)
            
            # Write dict size (8 bits) - blast.cpp line 363: dict = bits(s, 8)  
            for i in range(self.BYTE_BITS):
                self.bits.append((dict_size >> i) & 1)
        
        def write_literal(self, byte_val):
            """Write uncoded literal: 0 prefix + 8 bits (blast.cpp line 292)"""
            # From blast.cpp line 292: "0 for literals"
            self.bits.append(0)
            
            # From blast.cpp line 294-297: "no bit-reversal is needed" for uncoded literals
            # Write in LSB-first order within byte
            for i in range(self.BYTE_BITS):
                self.bits.append((byte_val >> i) & 1)
        
        def write_end_of_stream(self):
            """Write simple end-of-stream marker like working C# implementation"""
            # From working C# code: much simpler EOS than complex Huffman
            # Try the pattern that the old _BitStream class used
            self.bits.append(1)    # EOS marker bit
            # Simple EOS pattern: 7 zeros + 8 ones (from old _BitStream.WriteEOS)
            for i in range(7):
                self.bits.append(0)
            for i in range(8):
                self.bits.append(1)
        
        def get_bytes(self):
            """Convert bit array to bytes with padding like C# GetBytePadded"""            
            result = bytearray()
            
            # Process complete bytes first
            complete_bytes = len(self.bits) // self.BYTE_BITS
            for i in range(complete_bytes):
                byte_val = 0
                for j in range(self.BYTE_BITS):
                    if self.bits[i * self.BYTE_BITS + j]:
                        byte_val |= (1 << j)
                result.append(byte_val)
            
            # Handle final partial byte (like C# GetBytePadded)
            remaining_bits = len(self.bits) % self.BYTE_BITS
            if remaining_bits > 0:
                byte_val = 0
                for j in range(remaining_bits):
                    bit_index = complete_bytes * self.BYTE_BITS + j
                    if self.bits[bit_index]:
                        byte_val |= (1 << j)
                result.append(byte_val)
            
            return bytes(result)
    
    
    
    
    def write_fts(self, fts_data: FtsData, cells_to_write):
        """Create uncompressed FTS binary data following exact C++ format"""
        import struct
        
        # Calculate counts
        total_polys = 0
        for z in range(160):
            for x in range(160):
                if z < len(cells_to_write) and x < len(cells_to_write[z]) and cells_to_write[z][x]:
                    total_polys += len(cells_to_write[z][x])
        
        # Create scene header matching C++ FAST_SCENE_HEADER
        header = FAST_SCENE_HEADER()
        header.version = 0.141000
        header.sizex = 160
        header.sizez = 160  
        header.nb_textures = len(fts_data.textures)
        header.nb_polys = total_polys
        header.nb_anchors = len(fts_data.anchors)
        header.nb_portals = len(fts_data.portals)
        # Calculate actual room count from portals and polygons (always recalculate for user modifications)
        max_room = 0
        room_ids_found = set()
        
        # Check all polygons for highest room ID
        for z in range(160):
            for x in range(160):
                if z < len(cells_to_write) and x < len(cells_to_write[z]) and cells_to_write[z][x]:
                    for poly in cells_to_write[z][x]:
                        if hasattr(poly, 'room'):
                            room_id = poly.room
                            max_room = max(max_room, room_id)
                            room_ids_found.add(room_id)
                        elif isinstance(poly, dict) and 'room' in poly:
                            room_id = poly['room']
                            max_room = max(max_room, room_id)
                            room_ids_found.add(room_id)
        
        # Check all portals for highest room ID
        for portal in fts_data.portals:
            if isinstance(portal, bytes):
                # Parse binary portal data to get room IDs
                from .dataFts import EERIE_SAVE_PORTALS
                try:
                    portal_struct = EERIE_SAVE_PORTALS.from_buffer_copy(portal)
                    max_room = max(max_room, portal_struct.room_1, portal_struct.room_2)
                except:
                    pass  # Skip invalid portal data
            else:
                max_room = max(max_room, portal.room_1, portal.room_2)
        
        header.nb_rooms = max_room  # nb_rooms = highest room ID, room data array is [0..max_room] inclusive
        
        self.log.info(f"Calculated room count: {header.nb_rooms} (max room ID found: {max_room})")
        self.log.info(f"Room IDs found in polygons: {sorted(room_ids_found)}")
        header.playerpos.x = fts_data.sceneOffset[0]
        header.playerpos.y = fts_data.sceneOffset[1] 
        header.playerpos.z = fts_data.sceneOffset[2]
        header.Mscenepos.x = fts_data.sceneOffset[0]
        header.Mscenepos.y = fts_data.sceneOffset[1]
        header.Mscenepos.z = fts_data.sceneOffset[2]
        
        # Build binary data
        data = bytearray()
        data.extend(bytes(header))
        
        # Write textures - handle both ctypes and dict formats
        for tex in fts_data.textures:
            if isinstance(tex, dict):
                # Convert dict to ctypes for serialization
                texture_struct = FAST_TEXTURE_CONTAINER()
                texture_struct.tc = tex['tc']
                texture_struct.temp = tex['temp']
                texture_struct.fic = tex['fic']
                data.extend(bytes(texture_struct))
            else:
                # Original ctypes structure
                data.extend(bytes(tex))
        
        # Write cells in correct order: Y (Z) rows, then X columns (row-major)
        # This matches the C++ loading order: for(z=0; z<sizez; z++) for(x=0; x<sizex; x++)
        for z in range(160):
            for x in range(160):
                # Check if this cell has data
                cell = None
                if (z < len(cells_to_write) and 
                    cells_to_write[z] is not None and 
                    x < len(cells_to_write[z])):
                    cell = cells_to_write[z][x]
                
                # Get preserved cell anchor indices
                cell_anchor_indices = []
                if (hasattr(fts_data, 'cell_anchors') and fts_data.cell_anchors and
                    z < len(fts_data.cell_anchors) and x < len(fts_data.cell_anchors[z]) and
                    fts_data.cell_anchors[z][x] is not None):
                    cell_anchor_indices = fts_data.cell_anchors[z][x]
                
                # Write scene info for this cell (even if empty)
                scene_info = FAST_SCENE_INFO()
                if cell is not None:
                    scene_info.nbpoly = len(cell)
                else:
                    scene_info.nbpoly = 0
                scene_info.nbianchors = len(cell_anchor_indices)  # Use preserved anchor count
                data.extend(bytes(scene_info))
                
                # Write polygons in this cell
                if cell is not None:
                    for poly in cell:
                        if isinstance(poly, dict):
                            # Convert dict to ctypes for serialization
                            poly_struct = FAST_EERIEPOLY()
                            
                            # Set vertices - only first 3 for triangles, all 4 for quads
                            num_verts = 3 if not poly.get('is_quad', False) else 4
                            for i in range(4):
                                if i < len(poly['vertices']):
                                    vert = poly['vertices'][i]
                                else:
                                    # Padding vertex for unused slot
                                    vert = {'ssx': 0, 'sy': 0, 'ssz': 0, 'stu': 0, 'stv': 0}
                                
                                poly_struct.v[i].ssx = vert['ssx']
                                poly_struct.v[i].sy = vert['sy']
                                poly_struct.v[i].ssz = vert['ssz']
                                poly_struct.v[i].stu = vert['stu']
                                poly_struct.v[i].stv = vert['stv']
                            
                            # Set polygon properties
                            poly_struct.tex = poly['tex']
                            poly_struct.transval = poly['transval']
                            poly_struct.area = poly['area']
                            poly_struct.room = poly['room']
                            
                            # Set normals
                            norm = poly['norm']
                            poly_struct.norm.x = norm['x']
                            poly_struct.norm.y = norm['y']
                            poly_struct.norm.z = norm['z']
                            
                            norm2 = poly['norm2']
                            poly_struct.norm2.x = norm2['x']
                            poly_struct.norm2.y = norm2['y']
                            poly_struct.norm2.z = norm2['z']
                            
                            # Set vertex normals - only first 3 for triangles, all 4 for quads
                            for i in range(4):
                                if i < len(poly['vertex_normals']):
                                    vnorm = poly['vertex_normals'][i]
                                else:
                                    # Padding normal for unused slot
                                    vnorm = {'x': 0, 'y': 1, 'z': 0}
                                
                                poly_struct.nrml[i].x = vnorm['x']
                                poly_struct.nrml[i].y = vnorm['y']
                                poly_struct.nrml[i].z = vnorm['z']
                            
                            # Set polygon type
                            from .dataCommon import PolyTypeFlag
                            poly_struct.type = PolyTypeFlag()

                            # Handle both integer and PolyTypeFlag values
                            poly_type_value = poly.get('poly_type', 0)
                            if hasattr(poly_type_value, 'asUInt'):
                                # It's already a PolyTypeFlag instance
                                poly_struct.type.asUInt = poly_type_value.asUInt
                            else:
                                # It's an integer value
                                poly_struct.type.asUInt = int(poly_type_value)

                            # Ensure POLY_QUAD flag matches actual vertex count
                            is_quad = poly.get('is_quad', False)
                            poly_struct.type.POLY_QUAD = is_quad
                            
                            data.extend(bytes(poly_struct))
                        else:
                            # Original ctypes structure
                            data.extend(bytes(poly))
                
                # Write anchor indices for this cell (preserve original data)
                for anchor_idx in cell_anchor_indices:
                    data.extend(struct.pack('<i', anchor_idx))
        
        # Write anchors (preserve original data)
        for anchor_data in fts_data.anchors:
            if len(anchor_data) >= 5:  # New format with preserved data
                anchor_pos, anchor_links, radius, height, flags = anchor_data
            else:  # Old format fallback
                anchor_pos, anchor_links = anchor_data[:2]
                radius, height, flags = 50.0, 100.0, 0
            
            anchor = FAST_ANCHOR_DATA()
            anchor.pos.x, anchor.pos.y, anchor.pos.z = anchor_pos
            anchor.radius = radius
            anchor.height = height
            anchor.nb_linked = len(anchor_links)
            anchor.flags = flags
            data.extend(bytes(anchor))
            
            # Write linked anchor indices
            for link in anchor_links:
                data.extend(struct.pack('<i', link))  # s32 not s16
        
        # Write portals - handle both ctypes and binary data
        for portal in fts_data.portals:
            if isinstance(portal, bytes):
                # Binary data from scene properties
                data.extend(portal)
            else:
                # Original ctypes structure
                data.extend(bytes(portal))
        
        # Write preserved room data structures
        nb_rooms = header.nb_rooms
        
        if hasattr(fts_data, 'room_data') and fts_data.room_data:
            room_data_list, room_distances = fts_data.room_data

            # The engine reads exactly nb_rooms + 1 room structures followed by an
            # (nb_rooms + 1)^2 distance matrix. nb_rooms above is derived from the room
            # ids actually referenced by polygons and portals, while room_data_list is
            # built from polygon room ids alone, so the two can disagree - and any
            # disagreement desynchronises the rest of the read. Force both to match the
            # header before writing anything.
            required_rooms = nb_rooms + 1

            room_data_list = list(room_data_list)
            if len(room_data_list) > required_rooms:
                self.log.warning(f"Truncating room data from {len(room_data_list)} to {required_rooms} entries")
                room_data_list = room_data_list[:required_rooms]
            while len(room_data_list) < required_rooms:
                room_data_list.append(({'nb_portals': 0, 'nb_polys': 0, 'padd': [0] * 6}, [], []))

            # Routes derived from the portal graph, used to fill in any entry the
            # caller could not supply. A room added in Blender used to get a
            # distance of 999999 with the waypoints left on the world origin,
            # which puts every entity in it permanently outside the treat zone.
            routes = compute_room_distances(fts_data.portals, nb_rooms)

            def _computed_distance(i, j):
                dist_data = ROOM_DIST_DATA_SAVE()
                route = routes.get((i, j))
                if route and i != j:
                    dist_data.distance = route[0]
                    dist_data.startpos.x, dist_data.startpos.y, dist_data.startpos.z = route[1]
                    dist_data.endpos.x, dist_data.endpos.y, dist_data.endpos.z = route[2]
                else:
                    # A room to itself, or two rooms no chain of portals connects.
                    # Zero tells the engine to fall back to straight line distance.
                    dist_data.distance = 0.0
                return bytes(dist_data)

            def _unusable(cell):
                entry = ROOM_DIST_DATA_SAVE.from_buffer_copy(cell) if isinstance(cell, bytes) else cell
                if entry.distance >= 999999.0:
                    return True
                # Real data always names waypoints; all zeroes means it was invented.
                return (entry.distance > 0.0
                        and entry.startpos.x == 0.0 and entry.startpos.y == 0.0
                        and entry.startpos.z == 0.0 and entry.endpos.x == 0.0
                        and entry.endpos.y == 0.0 and entry.endpos.z == 0.0)

            old_distances = room_distances or []
            room_distances = []
            replaced = 0
            for i in range(required_rooms):
                row = []
                for j in range(required_rooms):
                    if i < len(old_distances) and j < len(old_distances[i]) \
                            and not _unusable(old_distances[i][j]):
                        row.append(old_distances[i][j])
                    else:
                        row.append(_computed_distance(i, j))
                        replaced += 1
                room_distances.append(row)

            self.log.info(f"Writing {len(room_data_list)} room structures and a "
                          f"{required_rooms}x{required_rooms} room distance matrix "
                          f"({replaced} entries computed from the portal graph)")

            # Write room structures and their portal/polygon references
            for room_info, room_portal_indices, room_poly_refs in room_data_list:
                # Handle both ctypes structures and dict-based structures
                if isinstance(room_info, dict):
                    # Create ctypes structure from dict for serialization
                    room_data_struct = EERIE_SAVE_ROOM_DATA()
                    room_data_struct.nb_portals = room_info['nb_portals']
                    room_data_struct.nb_polys = room_info['nb_polys']
                    for i in range(6):
                        room_data_struct.padd[i] = room_info['padd'][i]
                    data.extend(bytes(room_data_struct))
                else:
                    # Original ctypes structure
                    data.extend(bytes(room_info))
                
                # Write portal indices for this room
                for portal_idx in room_portal_indices:
                    data.extend(struct.pack('<i', portal_idx))
                
                # Write polygon references for this room  
                for poly_ref in room_poly_refs:
                    if isinstance(poly_ref, dict):
                        # Create ctypes structure from dict for serialization
                        ep_data_struct = FAST_EP_DATA()
                        ep_data_struct.px = poly_ref['px']
                        ep_data_struct.py = poly_ref['py']
                        ep_data_struct.idx = poly_ref['idx']
                        ep_data_struct.padd = poly_ref['padd']
                        data.extend(bytes(ep_data_struct))
                    else:
                        # Original ctypes structure
                        data.extend(bytes(poly_ref))
            
            # Write preserved room distance matrix - handle binary data
            for row in room_distances:
                for dist_info in row:
                    if isinstance(dist_info, bytes):
                        # Binary data from scene properties
                        data.extend(dist_info)
                    else:
                        # Original ctypes structure
                        data.extend(bytes(dist_info))
        else:
            # Fallback: create empty room data (shouldn't happen with preserved data)
            for room_id in range(nb_rooms + 1):  # Off-by-one in original data
                room_data_struct = EERIE_SAVE_ROOM_DATA()
                room_data_struct.nb_portals = 0
                room_data_struct.nb_polys = 0
                data.extend(bytes(room_data_struct))
            
            # Write an empty distance matrix (size is (nb_rooms + 1) x (nb_rooms + 1)).
            # Every entry stays at zero, which SP_GetRoomDist reads as "no route
            # recorded" and answers with straight line distance - the same thing the
            # engine does for a level with no matrix at all. 999999 used to go here
            # instead, and since the waypoints beside it were left on the world
            # origin the engine returned roughly a million for any cross room pair.
            # PrepareIOTreatZone compares that against a limit of a few thousand, so
            # every entity the player was not sharing a room with dropped out of the
            # treat zone: not simulated, and not drawn.
            distance_matrix_size = nb_rooms + 1
            for _ in range(distance_matrix_size * distance_matrix_size):
                data.extend(bytes(ROOM_DIST_DATA_SAVE()))
        
        return bytes(data)
    
    def _create_fts_headers(self, uncompressed_size, compressed_size):
        """Create FTS container headers with exact size matching engine expectations"""
        # Primary header
        header1 = UNIQUE_HEADER()
        header1.path = b"Level\\FTS\0" + b"\0" * (256 - len(b"Level\\FTS\0"))
        header1.count = 2
        header1.version = 0.141000
        header1.uncompressedsize = uncompressed_size
        
        # Secondary header
        header2 = UNIQUE_HEADER3()  
        header2.path = b"fast.fts\0" + b"\0" * (256 - len(b"fast.fts\0"))
        header2.check = b"DANAE_FILE\0" + b"\0" * (512 - len(b"DANAE_FILE\0"))
        
        headers = bytes(header1) + bytes(header2)
        
        # Ensure headers are exactly 0x410 bytes (1040) to match engine expectations
        expected_size = 0x410
        if len(headers) > expected_size:
            # Truncate if too large
            headers = headers[:expected_size]
            self.log.warning(f"Headers truncated from {len(headers)} to {expected_size} bytes")
        elif len(headers) < expected_size:
            # Pad if too small  
            padding = expected_size - len(headers)
            headers += b'\x00' * padding
            self.log.info(f"Headers padded with {padding} bytes to reach {expected_size} bytes")
        
        return headers
    
    def _create_uncompressed_fts_headers(self, data_size):
        """Create headers for uncompressed FTS format (engine compatible)"""
        # Primary header - for uncompressed, uncompressedsize = data_size
        header1 = UNIQUE_HEADER()
        header1.path = b"Level\\FTS\0" + b"\0" * (256 - len(b"Level\\FTS\0"))
        header1.count = 2  # Use same count as compressed version
        header1.version = 0.141000
        header1.uncompressedsize = data_size  # Set to actual data size for uncompressed
        
        # Secondary header  
        header2 = UNIQUE_HEADER3()
        header2.path = b"fast.fts\0" + b"\0" * (256 - len(b"fast.fts\0"))
        header2.check = b"DANAE_FILE\0" + b"\0" * (512 - len(b"DANAE_FILE\0"))
        
        return bytes(header1) + bytes(header2)
    
    
    def _validate_blast_compatibility(self, compressed_data):
        """Validate that compressed data follows PKWare format for C++ blast function"""
        if len(compressed_data) < 2:
            self.log.error("Compressed data too small (missing header)")
            return False
        
        # Check header
        lit_flag = compressed_data[0]
        dict_size = compressed_data[1]
        
        if lit_flag != 0:
            self.log.error(f"Invalid lit flag: {lit_flag} (expected 0)")
            return False
        
        if dict_size != 4:
            self.log.error(f"Invalid dict size: {dict_size} (expected 4)")
            return False
        
        self.log.info(f"PKWare header valid: lit={lit_flag}, dict={dict_size}")
        
        # Basic bitstream validation
        bitstream_size = len(compressed_data) - 2
        if bitstream_size == 0:
            self.log.error("Empty bitstream after header")
            return False
        
        self.log.info(f"Bitstream size: {bitstream_size} bytes")
        return True
