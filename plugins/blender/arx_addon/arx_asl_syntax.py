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

import re
import bpy
import blf
import gpu
from gpu_extras.batch import batch_for_shader
from mathutils import Vector

class ASLSyntaxHighlighter:
    """Syntax highlighter for ASL (Arx Scripting Language) files"""
    
    def __init__(self):
        self.keywords = {
            # Control flow
            'if', 'else', 'elseif', 'endif', 'while', 'endwhile', 'for', 'endfor', 'switch', 'endswitch',
            'case', 'break', 'continue', 'return', 'goto', 'gosub', 'call',
            
            # Events
            'on', 'init', 'initend', 'die', 'main', 'inventory2_open', 'inventory2_close',
            'clicked', 'collision', 'chat', 'action', 'combine', 'load', 'reload', 'unload',
            'timer', 'zone_enter', 'zone_leave', 'game_ready', 'cine_end', 'key_pressed',
            
            # Functions
            'accept', 'refuse', 'dodamage', 'playanim', 'playsound', 'teleport', 'spawn',
            'destroy', 'sendevent', 'setevent', 'setname', 'setweight', 'setprice',
            'heal', 'poison', 'inventory', 'equip', 'unequip', 'worldfade',
            
            # Variables and constants
            'me', 'it', 'sender', 'player', 'true', 'false', 'none', 'null',
            'random', 'rnd', 'system', 'global', 'local', 'const',
            
            # Operators
            'and', 'or', 'not', 'mod', 'div',
        }
        
        self.colors = {
            'keyword': (0.5, 0.7, 1.0, 1.0),      # Light blue
            'comment': (0.5, 0.7, 0.5, 1.0),      # Green
            'string': (1.0, 0.8, 0.4, 1.0),       # Orange
            'number': (1.0, 0.6, 0.6, 1.0),       # Light red
            'reference': (0.8, 0.4, 1.0, 1.0),    # Purple
            'event': (1.0, 1.0, 0.4, 1.0),        # Yellow
            'function': (0.4, 0.8, 1.0, 1.0),     # Cyan
            'default': (0.9, 0.9, 0.9, 1.0),      # Light gray
        }
        
        # Regex patterns for different syntax elements
        self.patterns = {
            'comment': re.compile(r'//.*$|/\*.*?\*/', re.MULTILINE | re.DOTALL),
            'string': re.compile(r'"[^"]*"', re.MULTILINE),
            'number': re.compile(r'\b\d+\.?\d*\b'),
            'reference': re.compile(r'@\w+|\^\w+|\$\w+'),  # @object, ^item, $variable
            'event': re.compile(r'\bon\s+\w+'),
            'function': re.compile(r'\b\w+\s*\('),
        }
    
    def parse_line(self, line):
        """Parse a line of ASL code and return tokens with their types"""
        tokens = []
        pos = 0
        
        while pos < len(line):
            # Skip whitespace
            if line[pos].isspace():
                pos += 1
                continue
            
            # Check for comments
            if line[pos:pos+2] == '//':
                tokens.append(('comment', line[pos:], pos, len(line)))
                break
            
            # Check for strings
            if line[pos] == '"':
                end_pos = line.find('"', pos + 1)
                if end_pos == -1:
                    end_pos = len(line)
                else:
                    end_pos += 1
                tokens.append(('string', line[pos:end_pos], pos, end_pos))
                pos = end_pos
                continue
            
            # Check for numbers
            if line[pos].isdigit():
                end_pos = pos
                while end_pos < len(line) and (line[end_pos].isdigit() or line[end_pos] == '.'):
                    end_pos += 1
                tokens.append(('number', line[pos:end_pos], pos, end_pos))
                pos = end_pos
                continue
            
            # Check for references
            if line[pos] in '@^$':
                end_pos = pos + 1
                while end_pos < len(line) and (line[end_pos].isalnum() or line[end_pos] == '_'):
                    end_pos += 1
                tokens.append(('reference', line[pos:end_pos], pos, end_pos))
                pos = end_pos
                continue
            
            # Check for words (keywords, functions, etc.)
            if line[pos].isalpha() or line[pos] == '_':
                end_pos = pos
                while end_pos < len(line) and (line[end_pos].isalnum() or line[end_pos] == '_'):
                    end_pos += 1
                
                word = line[pos:end_pos]
                
                # Check if it's a keyword
                if word.lower() in self.keywords:
                    tokens.append(('keyword', word, pos, end_pos))
                # Check if it's followed by '(' (function call)
                elif end_pos < len(line) and line[end_pos] == '(':
                    tokens.append(('function', word, pos, end_pos))
                else:
                    tokens.append(('default', word, pos, end_pos))
                
                pos = end_pos
                continue
            
            # Default character
            tokens.append(('default', line[pos], pos, pos + 1))
            pos += 1
        
        return tokens
    
    def find_references(self, text_content):
        """Find all references in the text that can be navigated to"""
        references = []
        lines = text_content.split('\n')
        
        for line_num, line in enumerate(lines):
            # Find object references (@object_name)
            for match in re.finditer(r'@(\w+)', line):
                references.append({
                    'type': 'object',
                    'name': match.group(1),
                    'line': line_num,
                    'start': match.start(),
                    'end': match.end()
                })
            
            # Find item references (^item_name)
            for match in re.finditer(r'\^(\w+)', line):
                references.append({
                    'type': 'item',
                    'name': match.group(1),
                    'line': line_num,
                    'start': match.start(),
                    'end': match.end()
                })
            
            # Find variable references ($variable_name)
            for match in re.finditer(r'\$(\w+)', line):
                references.append({
                    'type': 'variable',
                    'name': match.group(1),
                    'line': line_num,
                    'start': match.start(),
                    'end': match.end()
                })
        
        return references

class ASLNavigator:
    """Handle navigation between ASL files and scene objects"""
    
    def __init__(self, addon):
        self.addon = addon
        self.syntax_highlighter = ASLSyntaxHighlighter()
    
    def find_entity_by_name(self, context, entity_name):
        """Find an entity object in the scene by name"""
        scene = context.scene
        
        # Look for entity objects (those starting with 'e:')
        for obj in scene.objects:
            if obj.name.startswith('e:'):
                # Check if the object ID matches
                object_id = obj.get("arx_object_id")
                if object_id and entity_name.lower() in object_id.lower():
                    return obj
                
                # Check entity name
                entity_name_prop = obj.get("arx_entity_name")
                if entity_name_prop and entity_name.lower() in entity_name_prop.lower():
                    return obj
        
        return None
    
    def find_asl_by_entity_reference(self, context, entity_name):
        """Find ASL file by constructing direct path from entity reference"""
        # First, try to find the entity in the scene to get its details
        obj = self.find_entity_by_name(context, entity_name)
        if obj:
            # Get the object_id and entity_ident from the found object
            object_id = obj.get("arx_object_id")
            entity_ident = obj.get("arx_entity_ident")
            
            if object_id and entity_ident:
                # Construct the direct path
                return self.construct_asl_path(object_id, entity_ident)
        
        # Fallback: try to construct path from the reference name directly
        # Parse entity name to extract base name (e.g., "human_guard" -> "human_base")
        return self.construct_asl_path_from_name(entity_name)
    
    def construct_asl_path(self, object_id, entity_ident):
        """Construct direct ASL file path from object_id and entity_ident"""
        if not hasattr(self.addon, 'sceneManager') or not hasattr(self.addon.sceneManager, 'dataPath'):
            return None
        
        from .arx_asl_reader import ASLReader
        asl_reader = ASLReader(self.addon.sceneManager.dataPath)
        
        # object_id is like "npc/human_base" or "items/sword"
        # Construct path: /build/graph/obj3d/interactive/{object_id}/{base_name}_{entity_ident:04d}/{base_name}.asl
        
        object_parts = object_id.split('/')
        if len(object_parts) >= 2:
            category = object_parts[0]  # npc, items, fix_inter
            base_name = object_parts[1]  # human_base, goblin_base, etc.
            
            folder_name = f"{base_name}_{entity_ident:04d}"
            asl_filename = f"{base_name}.asl"
            
            asl_path = (asl_reader.data_path / "graph" / "obj3d" / "interactive" / 
                       category / base_name / folder_name / asl_filename)
            
            if asl_path.exists():
                return asl_path
        
        return None
    
    def construct_asl_path_from_name(self, entity_name):
        """Construct ASL path from entity name (fallback method)"""
        # This is a fallback - try common mappings
        # You could extend this with a mapping table if needed
        
        # For now, assume entity_name maps to base names
        name_mappings = {
            'kultar': 'human_base',
            'human_guard': 'human_base', 
            'goblin_lord': 'goblin_base',
            'spider': 'spider_base',
            # Add more mappings as needed
        }
        
        base_name = name_mappings.get(entity_name.lower(), entity_name.lower())
        
        if not hasattr(self.addon, 'sceneManager') or not hasattr(self.addon.sceneManager, 'dataPath'):
            return None
        
        from .arx_asl_reader import ASLReader
        asl_reader = ASLReader(self.addon.sceneManager.dataPath)
        
        # Try common categories
        categories = ['npc', 'items', 'fix_inter']
        
        for category in categories:
            base_path = asl_reader.data_path / "graph" / "obj3d" / "interactive" / category / base_name
            if base_path.exists():
                # Look for the first numbered subdirectory
                for subdir in base_path.iterdir():
                    if subdir.is_dir() and subdir.name.startswith(f"{base_name}_"):
                        asl_file = subdir / f"{base_name}.asl"
                        if asl_file.exists():
                            return asl_file
        
        return None
    
    def navigate_to_reference(self, context, reference):
        """Navigate to a reference (object, item, or variable)"""
        if reference['type'] == 'object':
            # Try to find by constructing direct path
            asl_path = self.find_asl_by_entity_reference(context, reference['name'])
            if asl_path:
                return self.open_asl_file_by_path(context, asl_path)
        
        elif reference['type'] == 'item':
            # Try to find items by constructing direct path
            asl_path = self.find_asl_by_entity_reference(context, reference['name'])
            if asl_path:
                return self.open_asl_file_by_path(context, asl_path)
        
        elif reference['type'] == 'variable':
            # For variables, we could search for other uses of the same variable
            # This would require text search functionality
            pass
        
        return False
    
    def open_entity_asl(self, context, entity_ident):
        """Open ASL file for an entity"""
        from .arx_asl_reader import ASLReader
        
        # Get data path from addon
        if not hasattr(self.addon, 'sceneManager') or not hasattr(self.addon.sceneManager, 'dataPath'):
            return False
        
        # Read ASL file
        asl_reader = ASLReader(self.addon.sceneManager.dataPath)
        asl_content = asl_reader.read_asl_file(entity_ident)
        
        if asl_content is None:
            return False
        
        # Create or update text block
        text_name = f"ASL_{entity_ident:04d}"
        text_block = bpy.data.texts.get(text_name)
        
        if text_block:
            text_block.clear()
            text_block.write(asl_content)
        else:
            text_block = bpy.data.texts.new(text_name)
            text_block.write(asl_content)
        
        # Switch to text editor
        for area in context.screen.areas:
            if area.type == 'TEXT_EDITOR':
                area.spaces.active.text = text_block
                break
        
        return True
    
    def open_asl_file_by_path(self, context, asl_path):
        """Open an ASL file by its direct path"""
        try:
            with open(asl_path, 'r', encoding='iso-8859-15') as f:
                content = f.read()
                
            # Create text block name from the path
            # Extract entity name and ID from path if possible
            path_parts = str(asl_path).split('/')
            if len(path_parts) >= 2:
                parent_dir = path_parts[-2]  # e.g., "human_base_0028"
                text_name = f"ASL_{parent_dir}"
            else:
                text_name = f"ASL_{asl_path.stem}"
            
            # Create or update text block
            text_block = bpy.data.texts.get(text_name)
            if text_block:
                text_block.clear()
                text_block.write(content)
            else:
                text_block = bpy.data.texts.new(text_name)
                text_block.write(content)
            
            # Switch to text editor
            for area in context.screen.areas:
                if area.type == 'TEXT_EDITOR':
                    area.spaces.active.text = text_block
                    break
            
            return True
            
        except Exception as e:
            print(f"Error opening ASL file {asl_path}: {e}")
            return False