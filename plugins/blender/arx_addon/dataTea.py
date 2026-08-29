# Copyright 2014-2019 Arx Libertatis Team (see the AUTHORS file)
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
    c_int32,
    c_uint32,
    c_float
)

try:
    from .dataCommon import (
        SavedVec3,
        ArxQuat,
        SerializationException,
        UnexpectedValueException
    )
except ImportError:
    from dataCommon import (
        SavedVec3,
        ArxQuat,
        SerializationException,
        UnexpectedValueException
    )

class THEA_HEADER(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("identity",      c_char * 20), # THEO_SIZE_IDENTITY_ANIM
        ("version",       c_uint32),
        ("anim_name",     c_char * 256), # SIZE_NAME
        ("nb_frames",     c_int32),
        ("nb_groups",     c_int32),
        ("nb_key_frames", c_int32),
    ]

class THEA_KEYFRAME_2014(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("num_frame",        c_int32),
        ("flag_frame",       c_int32),
        ("master_key_frame", c_int32), # bool
        ("key_frame",        c_int32), # bool
        ("key_move",         c_int32), # bool
        ("key_orient",       c_int32), # bool
        ("key_morph",        c_int32), # bool
        ("time_frame",       c_int32),
    ]

class THEA_KEYFRAME_2015(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("num_frame",        c_int32),
        ("flag_frame",       c_int32),
        ("info_frame",       c_char * 256), # SIZE_NAME
        ("master_key_frame", c_int32), # bool
        ("key_frame",        c_int32), # bool
        ("key_move",         c_int32), # bool
        ("key_orient",       c_int32), # bool
        ("key_morph",        c_int32), # bool
        ("time_frame",       c_int32),
    ]

class THEA_MORPH(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("unknown1", c_int32),
        ("unknown2", c_int32),
        ("unknown3", c_int32),
        ("unknown4", c_int32),
    ]

class THEO_GROUPANIM(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("key_group",  c_int32),
        ("angle",      c_char * 8), # ignored
        ("Quaternion", ArxQuat),
        ("translate",  SavedVec3),
        ("zoom",       SavedVec3),
    ]

class THEA_SAMPLE(LittleEndianStructure):
    _pack_ = 1
    _fields_ = [
        ("sample_name",  c_char * 256), # SIZE_NAME
        ("sample_size",  c_int32)
    ]

from ctypes import sizeof
from typing import List
from collections import namedtuple
import logging

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

TeaFrame = namedtuple("TeaFrame", [
    'duration', 'flags', 'translation', 'rotation', 'groups', 'sampleName',
    'key_move', 'key_orient', 'key_morph', 'master_key_frame', 'key_frame', 'info_frame',
    # Position on the animation's own timeline, in 24ths of a second. This is the
    # only timing the engine uses: Animation.cpp puts keyframe i at num_frame / 24
    # and never reads time_frame at all.
    'num_frame'
])

#: What the engine needs to play an animation: the keyframes and the length of the
#: timeline they sit on. anim_time is nb_frames / 24 seconds.
TeaAnimation = namedtuple("TeaAnimation", ['frames', 'nb_frames', 'nb_groups'])


def compute_voidgroups(frames: List[TeaFrame], num_groups: int) -> List[bool]:
    """
    Compute void groups matching engine logic (Animation.cpp:431-449).

    A group is void if it has identity transforms across ALL frames:
    - quaternion == (1, 0, 0, 0) (identity)
    - translate == (0, 0, 0)
    - zoom == (0, 0, 0)

    Void groups should NOT be keyframed during import, allowing them to
    retain their pose from base animations (important for layered animations
    like combat/casting overlays on walk cycles).

    Args:
        frames: List of TeaFrame objects from TeaSerializer.read()
        num_groups: Number of bone groups in the animation

    Returns:
        List of booleans, True if group is void (has no animation data)
    """
    if not frames or num_groups <= 0:
        return []

    # Start assuming all groups are void
    voidgroups = [True] * num_groups

    # Tolerance for floating point comparison
    EPSILON = 0.0001

    for group_idx in range(num_groups):
        for frame in frames:
            if group_idx >= len(frame.groups):
                continue

            group = frame.groups[group_idx]

            # Check if this group has a non-identity transform in this frame
            # Identity quaternion is (w=1, x=0, y=0, z=0)
            quat_not_identity = (
                abs(group.Quaternion.w - 1.0) > EPSILON or
                abs(group.Quaternion.x) > EPSILON or
                abs(group.Quaternion.y) > EPSILON or
                abs(group.Quaternion.z) > EPSILON
            )

            # Non-zero translation
            trans_not_zero = (
                abs(group.translate.x) > EPSILON or
                abs(group.translate.y) > EPSILON or
                abs(group.translate.z) > EPSILON
            )

            # Non-zero zoom/scale
            zoom_not_zero = (
                abs(group.zoom.x) > EPSILON or
                abs(group.zoom.y) > EPSILON or
                abs(group.zoom.z) > EPSILON
            )

            if quat_not_identity or trans_not_zero or zoom_not_zero:
                # This group has animation data - not void
                voidgroups[group_idx] = False
                break  # No need to check more frames for this group

    # Log summary
    animated_count = sum(1 for v in voidgroups if not v)
    void_count = sum(1 for v in voidgroups if v)
    logger.debug("Voidgroups computed: %d animated, %d void out of %d total groups",
                 animated_count, void_count, num_groups)

    return voidgroups

def compute_protected_groups(frames: List[TeaFrame], num_groups: int) -> List[bool]:
    """Groups deliberately kept out of the void groups, by negated identity.

    -1,0,0,0 is the same rotation as 1,0,0,0 but is not equal to it, so the
    engine's exact comparison does not see it as identity and the group keeps
    hold of its bone instead of letting a lower layer drive it. Authors use that
    on purpose, so it has to survive a round trip rather than being tidied away.
    """
    if not frames or num_groups <= 0:
        return []

    protected = [False] * num_groups
    for group_idx in range(num_groups):
        negated = False
        plain = False
        for frame in frames:
            if group_idx >= len(frame.groups):
                continue
            quat = frame.groups[group_idx].Quaternion
            if quat.x or quat.y or quat.z:
                plain = True
                break
            if quat.w == -1.0:
                negated = True
            elif quat.w == 1.0:
                continue
            else:
                plain = True
                break
        protected[group_idx] = negated and not plain

    return protected


class TeaSerializer(object):
    def __init__(self):
        self.log = logging.getLogger(__name__)

    def read(self, fileName) -> List[TeaFrame]:
        f = open(fileName, "rb")
        data = f.read()
        f.close()
        self.log.debug("Read %i bytes from file %s", len(data), fileName)
        
        # Basic file size validation
        if len(data) < sizeof(THEA_HEADER):
            raise SerializationException(f"File too small ({len(data)} bytes), need at least {sizeof(THEA_HEADER)} for header")

        pos = 0
        try:
            header = THEA_HEADER.from_buffer_copy(data, pos)
        except Exception as e:
            raise SerializationException(f"Failed to read TEA header: {str(e)}")
        pos += sizeof(THEA_HEADER)

        self.log.debug("Header: Frames=%d, KeyFrames=%d", header.nb_frames, header.nb_key_frames)
        self.log.debug(
            "Header - Identity: {0}; Version: {1}; Frames: {2}; Groups {3}; KeyFrames {4}".format(
                header.identity, header.version, header.nb_frames, header.nb_groups, header.nb_key_frames))

        # Validate header values for reasonable bounds
        if header.nb_frames < 0 or header.nb_frames > 100000:
            raise UnexpectedValueException(f"Invalid header.nb_frames = {header.nb_frames} (should be 0-100000)")
        if header.nb_groups < 0 or header.nb_groups > 1000:
            raise UnexpectedValueException(f"Invalid header.nb_groups = {header.nb_groups} (should be 0-1000)")
        if header.nb_key_frames < 0 or header.nb_key_frames > 10000:
            raise UnexpectedValueException(f"Invalid header.nb_key_frames = {header.nb_key_frames} (should be 0-10000)")

        results = []
        default_frame_rate = 24.0  # Configurable default
        for i in range(header.nb_key_frames):
            # Check if we have enough data left for keyframe structure
            if header.version == 2014:
                if pos + sizeof(THEA_KEYFRAME_2014) > len(data):
                    raise SerializationException(f"File truncated at keyframe {i}, need {sizeof(THEA_KEYFRAME_2014)} bytes but only {len(data) - pos} left")
                try:
                    kf = THEA_KEYFRAME_2014.from_buffer_copy(data, pos)
                except Exception as e:
                    raise SerializationException(f"Failed to read keyframe {i} (2014): {str(e)}")
                pos += sizeof(THEA_KEYFRAME_2014)
            elif header.version == 2015:
                if pos + sizeof(THEA_KEYFRAME_2015) > len(data):
                    raise SerializationException(f"File truncated at keyframe {i}, need {sizeof(THEA_KEYFRAME_2015)} bytes but only {len(data) - pos} left")
                try:
                    kf = THEA_KEYFRAME_2015.from_buffer_copy(data, pos)
                except Exception as e:
                    raise SerializationException(f"Failed to read keyframe {i} (2015): {str(e)}")
                pos += sizeof(THEA_KEYFRAME_2015)
                if kf.info_frame:
                    try:
                        self.log.info("Keyframe str: %s", kf.info_frame.decode('iso-8859-1', errors='replace'))
                    except Exception:
                        self.log.warning("Failed to decode info_frame for keyframe %d", i)
            else:
                raise SerializationException("Unknown version: " + str(header.version))

            self.log.debug("Keyframe %d: raw time_frame=%d", i, kf.time_frame)
            
            # Validate time_frame for reasonable bounds
            if kf.time_frame > 0 and kf.time_frame < 10000000:  # Less than 10 seconds in microseconds
                duration = kf.time_frame / 1000000.0  # Convert microseconds to seconds
            else:
                duration = 1.0 / default_frame_rate
                if kf.time_frame <= 0:
                    self.log.warning("Invalid time_frame=%d for keyframe %d, using default duration %.3fs",
                                     kf.time_frame, i, duration)
                else:
                    self.log.warning("Suspicious time_frame=%d for keyframe %d (%.2f seconds), using default duration %.3fs",
                                     kf.time_frame, i, kf.time_frame/1000000.0, duration)

            flags = kf.flag_frame
            # Validate flag_frame (should be -1 for normal frames or 9 for step sound frames)
            # Large values indicate corrupted data or wrong byte order
            if abs(flags) > 1000:  # Clearly corrupted data
                self.log.warning("Invalid flag_frame=%d for keyframe %d, likely corrupted data. Using -1.", flags, i)
                flags = -1
            elif flags not in (-1, 9):
                self.log.warning("Unexpected flag_frame=%d for keyframe %d, using -1. Valid values are -1 or 9.", flags, i)
                flags = -1

            translation = None
            if kf.key_move != 0:
                if pos + sizeof(SavedVec3) > len(data):
                    raise SerializationException(f"File truncated at keyframe {i} translation data")
                translation = SavedVec3.from_buffer_copy(data, pos)
                pos += sizeof(SavedVec3)

            rotation = None
            if kf.key_orient != 0:
                if pos + 8 + sizeof(ArxQuat) > len(data):
                    raise SerializationException(f"File truncated at keyframe {i} rotation data")
                pos += 8  # skip THEO_ANGLE
                rotation = ArxQuat.from_buffer_copy(data, pos)
                pos += sizeof(ArxQuat)

            if kf.key_morph != 0:
                if pos + sizeof(THEA_MORPH) > len(data):
                    raise SerializationException(f"File truncated at keyframe {i} morph data")
                morph = THEA_MORPH.from_buffer_copy(data, pos)
                pos += sizeof(THEA_MORPH)

            groups_size = sizeof(THEO_GROUPANIM) * header.nb_groups
            if pos + groups_size > len(data):
                raise SerializationException(f"File truncated at keyframe {i} groups data, need {groups_size} bytes")
            groupsList = (THEO_GROUPANIM * header.nb_groups).from_buffer_copy(data, pos)
            pos += sizeof(groupsList)

            num_sample = c_int32.from_buffer_copy(data, pos)
            pos += sizeof(c_int32)

            sampleName = None
            if num_sample.value != -1:
                sample = THEA_SAMPLE.from_buffer_copy(data, pos)
                pos += sizeof(THEA_SAMPLE)
                sampleName = sample.sample_name.decode('iso-8859-1')
                pos += sample.sample_size

            # Read num_sfx but validate it
            try:
                num_sfx = c_int32.from_buffer_copy(data, pos)
                pos += sizeof(c_int32)
                if abs(num_sfx.value) > 1000:  # Sanity check
                    self.log.warning("Suspicious num_sfx value: %d for keyframe %d", num_sfx.value, i)
            except Exception as e:
                self.log.warning("Error reading num_sfx for keyframe %d: %s", i, str(e))
                pos += 4  # Continue anyway
            results.append(TeaFrame(
                duration=duration,
                flags=flags,
                translation=translation,
                rotation=rotation,
                groups=groupsList,
                sampleName=sampleName,
                key_move=bool(kf.key_move),
                key_orient=bool(kf.key_orient),
                key_morph=bool(kf.key_morph),
                master_key_frame=bool(kf.master_key_frame),
                key_frame=bool(kf.key_frame),
                info_frame=kf.info_frame.decode('iso-8859-1') if header.version == 2015 and kf.info_frame else "",
                num_frame=kf.num_frame
            ))

        self.log.debug("File loaded with %d frames", len(results))
        
        # Perform interpolation like the engine does
        interpolated_results = self._interpolate_missing_transforms(results)
        
        return interpolated_results
    
    def _interpolate_missing_transforms(self, frames):
        """
        Interpolate missing translation and rotation data between keyframes,
        following the same logic as the Arx engine (Animation.cpp lines 374-422).
        """
        if not frames:
            return frames
            
        # Convert to mutable list for modification
        interpolated_frames = []
        for frame in frames:
            # Create a copy with the same structure
            interpolated_frames.append(TeaFrame(
                duration=frame.duration,
                flags=frame.flags,
                translation=frame.translation,
                rotation=frame.rotation,
                groups=frame.groups,
                sampleName=frame.sampleName,
                key_move=frame.key_move,
                key_orient=frame.key_orient,
                key_morph=frame.key_morph,
                master_key_frame=frame.master_key_frame,
                key_frame=frame.key_frame,
                info_frame=frame.info_frame,
                num_frame=frame.num_frame
            ))
        
        # Interpolate missing translations
        self._interpolate_translations(interpolated_frames)
        
        # Interpolate missing rotations  
        self._interpolate_rotations(interpolated_frames)
        
        return interpolated_frames
    
    def _interpolate_translations(self, frames):
        """
        Interpolate missing translation data between keyframes.
        Based on Animation.cpp lines 374-395.
        """
        for i in range(len(frames)):
            if frames[i].translation is None:
                # Find previous frame with translation data
                k = i - 1
                while k >= 0 and frames[k].translation is None:
                    k -= 1
                
                # Find next frame with translation data
                j = i + 1
                while j < len(frames) and frames[j].translation is None:
                    j += 1
                
                if j < len(frames) and k >= 0:
                    # Linear interpolation between keyframes
                    r1 = self._get_time_between_keyframes(frames, k, i)
                    r2 = self._get_time_between_keyframes(frames, i, j)
                    
                    if r1 + r2 > 0:
                        tot = 1.0 / (r1 + r2)
                        r1 *= tot
                        r2 *= tot
                        
                        # Interpolate translation
                        trans_k = frames[k].translation
                        trans_j = frames[j].translation
                        
                        interpolated_translation = SavedVec3()
                        interpolated_translation.x = trans_j.x * r1 + trans_k.x * r2
                        interpolated_translation.y = trans_j.y * r1 + trans_k.y * r2
                        interpolated_translation.z = trans_j.z * r1 + trans_k.z * r2
                        
                        # Update frame with interpolated translation
                        frames[i] = TeaFrame(
                            duration=frames[i].duration,
                            flags=frames[i].flags,
                            translation=interpolated_translation,
                            rotation=frames[i].rotation,
                            groups=frames[i].groups,
                            sampleName=frames[i].sampleName,
                            key_move=frames[i].key_move,
                            key_orient=frames[i].key_orient,
                            key_morph=frames[i].key_morph,
                            master_key_frame=frames[i].master_key_frame,
                            key_frame=frames[i].key_frame,
                            info_frame=frames[i].info_frame,
                            num_frame=frames[i].num_frame
                        )
                        
                        self.log.debug("Interpolated translation for frame %d: x=%.3f, y=%.3f, z=%.3f", 
                                     i, interpolated_translation.x, interpolated_translation.y, interpolated_translation.z)
    
    def _interpolate_rotations(self, frames):
        """
        Interpolate missing rotation data between keyframes.
        Based on Animation.cpp lines 397-422.
        """
        for i in range(len(frames)):
            if frames[i].rotation is None:
                # Find previous frame with rotation data
                k = i - 1
                while k >= 0 and frames[k].rotation is None:
                    k -= 1
                
                # Find next frame with rotation data
                j = i + 1
                while j < len(frames) and frames[j].rotation is None:
                    j += 1
                
                if j < len(frames) and k >= 0:
                    # Linear interpolation between keyframes
                    r1 = self._get_time_between_keyframes(frames, k, i)
                    r2 = self._get_time_between_keyframes(frames, i, j)
                    
                    if r1 + r2 > 0:
                        tot = 1.0 / (r1 + r2)
                        r1 *= tot
                        r2 *= tot
                        
                        # Interpolate rotation (quaternion)
                        rot_k = frames[k].rotation
                        rot_j = frames[j].rotation
                        
                        interpolated_rotation = ArxQuat()
                        interpolated_rotation.w = rot_j.w * r1 + rot_k.w * r2
                        interpolated_rotation.x = rot_j.x * r1 + rot_k.x * r2
                        interpolated_rotation.y = rot_j.y * r1 + rot_k.y * r2
                        interpolated_rotation.z = rot_j.z * r1 + rot_k.z * r2
                        
                        # Update frame with interpolated rotation
                        frames[i] = TeaFrame(
                            duration=frames[i].duration,
                            flags=frames[i].flags,
                            translation=frames[i].translation,
                            rotation=interpolated_rotation,
                            groups=frames[i].groups,
                            sampleName=frames[i].sampleName,
                            key_move=frames[i].key_move,
                            key_orient=frames[i].key_orient,
                            key_morph=frames[i].key_morph,
                            master_key_frame=frames[i].master_key_frame,
                            key_frame=frames[i].key_frame,
                            info_frame=frames[i].info_frame,
                            num_frame=frames[i].num_frame
                        )
                        
                        self.log.debug("Interpolated rotation for frame %d: w=%.3f, x=%.3f, y=%.3f, z=%.3f", 
                                     i, interpolated_rotation.w, interpolated_rotation.x, interpolated_rotation.y, interpolated_rotation.z)
    
    def _get_time_between_keyframes(self, frames, start_idx, end_idx):
        """
        Calculate time between two keyframes based on frame durations.
        """
        if start_idx >= end_idx:
            return 0.0
            
        total_time = 0.0
        for i in range(start_idx, end_idx):
            total_time += frames[i].duration
            
        return total_time

    def write(self, frames: List[TeaFrame], fileName: str, anim_name: str = "", version: int = 2015):
        """
        Write TEA animation data to a file.
        Args:
            frames: List of TeaFrame objects containing animation data
            fileName: Output file path
            anim_name: Animation name (default empty)
            version: TEA version (2014 or 2015, default 2015)
        """
        if not frames:
            raise SerializationException("No frames to write")
        
        # Validate version
        if version not in [2014, 2015]:
            raise SerializationException(f"Unsupported TEA version: {version}")
        
        # Calculate header values based on actual animation data
        # nb_frames: Total animation duration in frame units (engine expects this for timing calculations)
        # nb_key_frames: Actual number of keyframes in the file
        # nb_groups: Number of bone groups per keyframe
        
        total_duration_seconds = sum(frame.duration for frame in frames)
        nb_frames = max(1, int(total_duration_seconds * 24.0))  # Total duration at 24fps for engine timing
        nb_groups = len(frames[0].groups) if frames else 0
        nb_key_frames = len(frames)  # Actual keyframe count
        
        self.log.debug("Animation metadata: duration=%.3fs, nb_frames=%d, nb_groups=%d, nb_key_frames=%d", 
                       total_duration_seconds, nb_frames, nb_groups, nb_key_frames)
        
        # Create header
        header = THEA_HEADER()
        header.identity = b"THEO_TEA_FILE_VERSION"[:20].ljust(20, b'\x00')  # Use proper identity
        header.version = version
        header.anim_name = anim_name.encode('iso-8859-1')[:255].ljust(256, b'\x00')  # Null-terminated
        header.nb_frames = nb_frames
        header.nb_groups = nb_groups
        header.nb_key_frames = nb_key_frames
        
        self.log.debug("Writing TEA file: %s", fileName)
        self.log.debug("Header - Version: %d, Frames: %d, Groups: %d, KeyFrames: %d", 
                       version, nb_frames, nb_groups, nb_key_frames)
        
        # Write to file
        with open(fileName, "wb") as f:
            # Write header
            f.write(header)
            
            # Write keyframes
            for i, frame in enumerate(frames):
                self._write_keyframe(f, frame, i, version)
                
            file_size = f.tell()
                
        self.log.debug("Successfully wrote %d bytes to %s", file_size, fileName)
    
    def _write_keyframe(self, f, frame: TeaFrame, frame_index: int, version: int):
        """Write a single keyframe to file."""
        # Based on engine Animation.cpp:298 timing calculation:
        # eerie->frames[i].time = std::chrono::microseconds(s64(tkf2015->num_frame)) * 1000 * 1000 / 24;
        # This means num_frame should be the frame number at 24fps
        
        # Calculate frame number for 24fps timing (this is what engine uses for timing)
        # The frame's own position on the 24 fps timeline. Deriving it from the
        # duration instead gives every keyframe the same number, which collapses
        # the animation to a single instant as far as the engine is concerned.
        num_frame = getattr(frame, 'num_frame', None)
        if num_frame is None:
            num_frame = max(0, int(frame.duration * 24.0))
        
        # time_frame is duration in microseconds (this is what we read back as duration)
        time_frame = max(1000, int(frame.duration * 1000000.0))  # Convert seconds to microseconds
        
        # Create keyframe structure using the metadata from the frame
        if version == 2014:
            kf = THEA_KEYFRAME_2014()
            kf.num_frame = num_frame
            kf.flag_frame = frame.flags
            kf.master_key_frame = 1 if frame.master_key_frame else 0
            kf.key_frame = 1 if frame.key_frame else 0
            kf.key_move = 1 if frame.key_move else 0
            kf.key_orient = 1 if frame.key_orient else 0
            kf.key_morph = 1 if frame.key_morph else 0
            kf.time_frame = time_frame
        else:  # version == 2015
            kf = THEA_KEYFRAME_2015()
            kf.num_frame = num_frame
            kf.flag_frame = frame.flags
            # Handle info_frame - it can contain binary data
            if frame.info_frame:
                info_bytes = frame.info_frame.encode('iso-8859-1', errors='replace')[:255]
                kf.info_frame = info_bytes.ljust(256, b'\x00')
            else:
                kf.info_frame = b'\x00' * 256
            kf.master_key_frame = 1 if frame.master_key_frame else 0
            kf.key_frame = 1 if frame.key_frame else 0
            kf.key_move = 1 if frame.key_move else 0
            kf.key_orient = 1 if frame.key_orient else 0
            kf.key_morph = 1 if frame.key_morph else 0
            kf.time_frame = time_frame
            
        # Write keyframe
        f.write(kf)
        
        # Write translation if flag is set
        if kf.key_move:
            f.write(frame.translation)
            
        # Write rotation if flag is set
        if kf.key_orient:
            # Write 8 bytes for THEO_ANGLE (ignored)
            f.write(b'\x00' * 8)
            # Write quaternion
            f.write(frame.rotation)
            
        # Write morph data if flag is set (currently always skipped)
        if kf.key_morph:
            morph = THEA_MORPH()
            f.write(morph)
            
        # Write groups
        for group in frame.groups:
            f.write(group)
            
        # Write sample data
        num_sample = c_int32(-1)  # No sample
        if frame.sampleName:
            # TODO: Implement sample writing if needed
            pass
            
        f.write(num_sample)
        
        # Write sample if present
        if frame.sampleName:
            sample = THEA_SAMPLE()
            sample.sample_name = frame.sampleName.encode('iso-8859-1')[:256].ljust(256, b'\x00')
            sample.sample_size = 0  # No sample data
            f.write(sample)
            
        # Write num_sfx (always 0)
        num_sfx = c_int32(0)
        f.write(num_sfx)
