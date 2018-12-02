# Copyright 2018 Arx Libertatis Team (see the AUTHORS file)
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

import bpy

#from arx_addon.materials import arx_create_material_node_group
#arx_create_material_node_group()
#am = D.node_groups['Arx Material']

from mathutils import Vector

def arx_get_material_node_group():
    arx_material_node_group_name = 'Arx Material'

    if arx_material_node_group_name in bpy.data.node_groups:
        return bpy.data.node_groups[arx_material_node_group_name]

    group = bpy.data.node_groups.new(arx_material_node_group_name, 'ShaderNodeTree')

    group.outputs.new('NodeSocketShader', 'Shader')
    group.inputs.new('NodeSocketColor', 'Color')

    n_out = group.nodes.new('NodeGroupOutput')
    n_out.name = 'n_out'
    n_out.location = Vector((0.0, 0.0))

    n_mix = group.nodes.new('ShaderNodeMixShader')
    n_mix.name = 'n_mix'
    n_mix.location = Vector((-220.0, 0.0))

    n_transparent = group.nodes.new('ShaderNodeBsdfTransparent')
    n_transparent.name = 'n_transparent'
    n_transparent.location = Vector((-520.0, -140.0))

    n_diffuse = group.nodes.new('ShaderNodeBsdfDiffuse')
    n_diffuse.name = 'n_diffuse'
    n_diffuse.location = Vector((-520.0, 0.0))

    n_less_than = group.nodes.new('ShaderNodeMath')
    n_less_than.name = 'n_less_than'
    n_less_than.location = Vector((-520.0, 180.0))

    n_in = group.nodes.new('NodeGroupInput')
    n_in.name = 'n_in'
    n_in.location = Vector((-1120.0, 0.0))

    group.links.new(n_mix.outputs['Shader'], n_out.inputs['Shader'])

    group.links.new(n_less_than.outputs['Value'], n_mix.inputs['Fac'])
    group.links.new(n_diffuse.outputs['BSDF'], n_mix.inputs[1])
    group.links.new(n_transparent.outputs['BSDF'], n_mix.inputs[2])

    group.links.new(n_in.outputs['Color'], n_diffuse.inputs['Color'])
    group.links.new(n_in.outputs['Color'], n_less_than.inputs['Value'])

    return group