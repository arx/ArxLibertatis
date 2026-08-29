# Copyright 2019-2020 Arx Libertatis Team (see the AUTHORS file)
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

import os
import logging
from pathlib import Path

#: Where LOADANIM looks for its file, decided by the entity's kind rather than by
#: anything in the name (ScriptedAnimation.cpp: the player and NPCs read from
#: anims/npc, everything else from anims/fix_inter).
ANIM_ROOT_NPC = 'graph/obj3d/anims/npc'
ANIM_ROOT_FIX = 'graph/obj3d/anims/fix_inter'


def split_asl_words(line):
    """Split an ASL line into words, keeping quoted strings whole."""
    words = []
    current = ''
    quoted = False
    for char in line:
        if char == '"':
            quoted = not quoted
            continue
        if char.isspace() and not quoted:
            if current:
                words.append(current)
                current = ''
            continue
        current += char
    if current:
        words.append(current)
    return words


def parse_loadanim(text):
    """Every animation a script binds, as {slot: file stem}.

    LOADANIM is what actually associates an animation with an entity; matching
    file names against a model name only ever guesses. A later declaration of the
    same slot replaces an earlier one, which is how an instance script overrides
    one animation without restating the rest. A file of "none" clears the slot,
    exactly as the command does.
    """
    animations = {}
    for raw in text.splitlines():
        line = raw.split('//', 1)[0]
        words = split_asl_words(line)
        if not words or words[0].lower() != 'loadanim':
            continue

        # Skip the option flags, for example the -p that targets the player.
        rest = [word for word in words[1:] if not word.startswith('-')]
        if len(rest) < 2:
            continue

        slot, target = rest[0].upper(), rest[1]
        if target.lower() == 'none':
            animations.pop(slot, None)
        else:
            animations[slot] = target
    return animations


class ASLReader:
    """Module for reading ASL (Arx Scripting Language) files with ISO-8559-15 encoding"""
    
    def __init__(self, data_path):
        self.data_path = Path(data_path)
        self.log = logging.getLogger('ASLReader')
        
    @staticmethod
    def class_path_from_name(entity_name):
        """The class path the engine derives from the name stored in the DLF.

        LoadLevel lowercases the string, keeps everything from "graph" onwards and
        drops the extension. Doing the same here is what makes an entity's scripts
        findable wherever it lives, rather than only under graph/obj3d/interactive.
        """
        text = entity_name.replace('\\', '/').lower()
        if text.endswith('.teo') or text.endswith('.ftl') or text.endswith('.asl'):
            text = text.rsplit('.', 1)[0]
        marker = text.find('graph')
        if marker != -1:
            text = text[marker:]
        return text.strip('/')

    def class_script_path(self, class_path):
        """The script shared by every instance of this class."""
        return self.data_path / (class_path + '.asl')

    def instance_script_path(self, class_path, entity_ident):
        """The script belonging to one placed entity, which overrides the class one.

        LoadInter_Ex looks it up as <class directory>/<id>/<class name>.asl, where
        the id is the class name and the four digit instance number.
        """
        parts = class_path.split('/')
        name = parts[-1]
        folder = f"{name}_{entity_ident:04d}"
        return self.data_path.joinpath(*parts[:-1], folder, name + '.asl')

    def resolve(self, class_path, entity_ident, scope='auto'):
        """Locate a script. Returns (path, scope, exists).

        `scope` is 'instance', 'class' or 'auto'. Auto prefers the instance script
        and falls back to the class one, which is right for reading. Saving must
        never do that: silently redirecting an edit of one entity onto the script
        shared by every entity of its type is how a change to a single marker ends
        up rewriting all of them.
        """
        instance = self.instance_script_path(class_path, entity_ident)
        klass = self.class_script_path(class_path)

        if scope == 'instance':
            return instance, 'instance', instance.exists()
        if scope == 'class':
            return klass, 'class', klass.exists()

        if instance.exists():
            return instance, 'instance', True
        if klass.exists():
            return klass, 'class', True
        return instance, 'instance', False

    def get_asl_file_path(self, entity_ident, object_id=None, class_path=None,
                          scope='auto'):
        """Backwards compatible lookup; prefer resolve() with a real class path."""
        if class_path is None:
            if not object_id:
                return None
            # Older scenes stored only the fragment below graph/obj3d/interactive.
            class_path = 'graph/obj3d/interactive/' + object_id.strip('/')
            class_path = class_path + '/' + class_path.rsplit('/', 1)[-1]

        path, _scope, exists = self.resolve(class_path, entity_ident, scope)
        return path if exists else None

    def animation_set(self, class_path, entity_ident=None, npc=True):
        """The animations an entity actually uses, as {slot: absolute tea path}.

        The class script is read first and the instance script layered over it, so
        an instance that rebinds one slot keeps the rest.
        """
        declared = {}
        for scope in ('class', 'instance'):
            if scope == 'instance' and entity_ident is None:
                continue
            path, _scope, exists = self.resolve(class_path, entity_ident or 0, scope)
            if not exists:
                continue
            text = self.read_path(path)
            if text:
                declared.update(parse_loadanim(text))

        root = ANIM_ROOT_NPC if npc else ANIM_ROOT_FIX
        return {slot: self.data_path / root / (name + '.tea')
                for slot, name in declared.items()}

    def find_animation_sets(self):
        """Every script in the data that binds animations, as {class path: {slot: file}}.

        Useful because the set a mesh should use is not always declared by that
        mesh's own script: the human animations live in the player's script, and a
        new humanoid borrowing them wants to start from that list.
        """
        sets = {}
        interactive = self.data_path / 'graph' / 'obj3d' / 'interactive'
        if not interactive.exists():
            return sets

        for script in interactive.rglob('*.asl'):
            text = self.read_path(script)
            if not text:
                continue
            declared = parse_loadanim(text)
            if not declared:
                continue
            relative = script.relative_to(self.data_path).as_posix()
            sets[relative[:-4]] = declared

        return sets

    def read_path(self, asl_path):
        """Read one script file, or None if it cannot be read."""
        try:
            with open(asl_path, 'r', encoding='iso-8859-15') as handle:
                return handle.read()
        except OSError as error:
            self.log.error(f"Error reading ASL file {asl_path}: {error}")
            return None

    def read_asl_file(self, entity_ident, object_id=None, class_path=None, scope='auto'):
        """Read and return the contents of an ASL file for the given entity identifier"""
        asl_path = self.get_asl_file_path(entity_ident, object_id, class_path, scope)
        
        if not asl_path:
            self.log.warning(f"ASL file not found for entity {entity_ident:04d}")
            return None
            
        try:
            with open(asl_path, 'r', encoding='iso-8859-15') as f:
                content = f.read()
                self.log.info(f"Successfully read ASL file: {asl_path}")
                return content
        except Exception as e:
            self.log.error(f"Error reading ASL file {asl_path}: {e}")
            return None
    
    def get_asl_file_info(self, entity_ident):
        """Get information about an ASL file without reading its contents"""
        asl_path = self.get_asl_file_path(entity_ident)
        
        if not asl_path:
            return None
            
        try:
            stat = asl_path.stat()
            return {
                'path': str(asl_path),
                'size': stat.st_size,
                'modified': stat.st_mtime,
                'exists': True
            }
        except Exception as e:
            self.log.error(f"Error getting ASL file info {asl_path}: {e}")
            return None
    
    def list_all_asl_files(self):
        """List all ASL files found in the data directory"""
        asl_files = []
        
        try:
            for root, dirs, files in os.walk(self.data_path):
                for file in files:
                    if file.endswith('.asl'):
                        full_path = Path(root) / file
                        # Extract entity ID from filename
                        try:
                            entity_id = int(file[:-4])  # Remove .asl extension
                            asl_files.append({
                                'entity_id': entity_id,
                                'path': str(full_path),
                                'relative_path': str(full_path.relative_to(self.data_path))
                            })
                        except ValueError:
                            # Skip files that don't have numeric names
                            continue
        except Exception as e:
            self.log.error(f"Error listing ASL files: {e}")
            
        return sorted(asl_files, key=lambda x: x['entity_id'])