#!/usr/bin/env python3
"""
ArxLibertatis Ambience File Generator

Creates .amb files compatible with the ArxLibertatis engine.
Usage: python3 ambiencegen.py <wav_path> [options]
"""

import struct
import os
import argparse
from typing import List, NamedTuple


class KeySetting(NamedTuple):
    """Represents a setting that can vary over time"""
    min_val: float
    max_val: float
    interval: int  # milliseconds
    flags: int  # FLAG_RANDOM=1, FLAG_INTERPOLATE=2


class AmbienceKey(NamedTuple):
    """Represents a key event in an ambience track"""
    flags: int
    start: int  # milliseconds
    loop: int  # loop count (stored as count-1)
    delay_min: int  # milliseconds
    delay_max: int  # milliseconds
    volume: KeySetting
    pitch: KeySetting
    pan: KeySetting
    x: KeySetting  # 3D position
    y: KeySetting
    z: KeySetting


class AmbienceTrack(NamedTuple):
    """Represents a single track in an ambience file"""
    sample_path: str
    track_name: str
    flags: int  # FLAG_POSITION=1, FLAG_MASTER=4
    keys: List[AmbienceKey]


class AmbienceFile:
    """Generator for ArxLibertatis .amb files"""
    
    MAGIC = 0x424d4147  # 'GAMB'
    VERSION_1003 = 0x01000003  # Latest version with reversed keys
    
    FLAG_POSITION = 0x00000001
    FLAG_MASTER = 0x00000004
    
    def __init__(self):
        self.tracks: List[AmbienceTrack] = []
    
    def add_simple_track(self, sample_path: str, volume: float = 1.0, 
                        loop_count: int = 0, is_master: bool = True,
                        use_3d: bool = False, x: float = 0.0, y: float = 0.0, z: float = 0.0):
        """Add a simple track with basic parameters"""
        flags = 0
        if is_master:
            flags |= self.FLAG_MASTER
        if use_3d:
            flags |= self.FLAG_POSITION
        
        # Create a simple key with the specified parameters
        key = AmbienceKey(
            flags=0,
            start=0,
            loop=max(0, loop_count - 1),  # Stored as count-1
            delay_min=0,
            delay_max=0,
            volume=KeySetting(volume, volume, 0, 0),
            pitch=KeySetting(1.0, 1.0, 0, 0),
            pan=KeySetting(0.0, 0.0, 0, 0),
            x=KeySetting(x, x, 0, 0),
            y=KeySetting(y, y, 0, 0),
            z=KeySetting(z, z, 0, 0)
        )
        
        track = AmbienceTrack(
            sample_path=sample_path.lower(),
            track_name="",
            flags=flags,
            keys=[key]
        )
        
        self.tracks.append(track)
    
    def _write_string(self, f, s: str):
        """Write a null-terminated string"""
        f.write(s.encode('utf-8'))
        f.write(b'\x00')
    
    def _write_key_setting(self, f, setting: KeySetting):
        """Write a KeySetting structure"""
        f.write(struct.pack('<ffII', setting.min_val, setting.max_val, 
                           setting.interval, setting.flags))
    
    def _write_key(self, f, key: AmbienceKey):
        """Write an AmbienceKey structure"""
        f.write(struct.pack('<IIIII', key.flags, key.start, key.loop, 
                           key.delay_min, key.delay_max))
        self._write_key_setting(f, key.volume)
        self._write_key_setting(f, key.pitch)
        self._write_key_setting(f, key.pan)
        self._write_key_setting(f, key.x)
        self._write_key_setting(f, key.y)
        self._write_key_setting(f, key.z)
    
    def save(self, filename: str):
        """Save the ambience file to disk"""
        with open(filename, 'wb') as f:
            # Write header
            f.write(struct.pack('<III', self.MAGIC, self.VERSION_1003, len(self.tracks)))
            
            # Write tracks
            for track in self.tracks:
                # Write sample path
                self._write_string(f, track.sample_path)
                
                # Write track name (empty for version 1003)
                self._write_string(f, track.track_name)
                
                # Write track header
                f.write(struct.pack('<II', track.flags, len(track.keys)))
                
                # Write keys in reverse order (version 1003 requirement)
                for key in reversed(track.keys):
                    self._write_key(f, key)


def main():
    parser = argparse.ArgumentParser(description='Generate ArxLibertatis .amb files')
    parser.add_argument('wav_path', help='Path to the WAV file')
    parser.add_argument('--volume', type=float, default=1.0, help='Volume (0.0-1.0)')
    parser.add_argument('--loop', type=int, default=0, help='Loop count (0=infinite)')
    parser.add_argument('--master', action='store_true', default=True, help='Is master track')
    parser.add_argument('--3d', action='store_true', help='Use 3D positioning')
    parser.add_argument('--x', type=float, default=0.0, help='X position')
    parser.add_argument('--y', type=float, default=0.0, help='Y position')
    parser.add_argument('--z', type=float, default=0.0, help='Z position')
    
    args = parser.parse_args()
    
    # Check if input file exists
    if not os.path.exists(args.wav_path):
        print(f"Error: File {args.wav_path} not found")
        return 1
    
    # Create output filename in same directory as input
    input_dir = os.path.dirname(args.wav_path)
    base_name = os.path.splitext(os.path.basename(args.wav_path))[0]
    output_path = os.path.join(input_dir, f"{base_name}.amb")
    
    # Create ambience file
    amb = AmbienceFile()
    
    # Convert to relative path format expected by engine
    # Use relative path from sfx directory
    if "sfx" in args.wav_path:
        rel_path = args.wav_path.split("sfx")[1].lstrip("/").replace("/", "\\")
        rel_path = "sfx\\" + rel_path
    else:
        rel_path = os.path.relpath(args.wav_path, start="/").replace("/", "\\")
    
    amb.add_simple_track(
        sample_path=rel_path,
        volume=args.volume,
        loop_count=args.loop,
        is_master=args.master,
        use_3d=getattr(args, '3d'),
        x=args.x,
        y=args.y,
        z=args.z
    )
    
    amb.save(output_path)
    print(f"Created {output_path}")
    
    return 0


if __name__ == "__main__":
    exit(main())