from dumper import *

def arx_vec2_helper(value):
    return (value['#1']['x'].value(), value['#2']['y'].value())

def arx_vec3_helper(value):
    return (value['#1']['x'].value(), value['#2']['y'].value(), value['#3']['z'].value())

def qdump__Vec2s(d, value):
    x, y = arx_vec2_helper(value)
    d.putValue('({}, {})'.format(x, y))
    d.putPlainChildren(value)

def qdump__Vec2i(d, value):
    x, y = arx_vec2_helper(value)
    d.putValue('({}, {})'.format(x, y))
    d.putPlainChildren(value)

def qdump__Vec2f(d, value):
    x, y = arx_vec2_helper(value)
    d.putValue('{:10.4f} {:10.4f}'.format(x, y))
    d.putPlainChildren(value)

def qdump__Vec3f(d, value):
    x, y, z = arx_vec3_helper(value)
    d.putValue('{:10.4f} {:10.4f} {:10.4f}'.format(x, y, z))
    d.putPlainChildren(value)

def qdump__Anglef(d, value):
    yaw   = value['m_yaw'].value()
    pitch = value['m_pitch'].value()
    roll  = value['m_roll'].value()
    valueStr = "{:10.4f} {:10.4f} {:10.4f}".format(yaw, pitch, roll)
    d.putType("Anglef")
    d.putValue(valueStr)
    d.putPlainChildren(value)

# Color =======================================================================

def qdump__Color3(d, value):
    r = value['r'].value()
    g = value['g'].value()
    b = value['b'].value()
    d.putType('Color3')
    d.putValue('{:03d} {:03d} {:03d}'.format(r, g, b))
    d.putPlainChildren(value)

def qdump__Color(d, value):
    r = value['r'].value()
    g = value['g'].value()
    b = value['b'].value()
    a = value['a'].value()
    d.putType('Color')
    d.putValue('{:03d} {:03d} {:03d} {:03d}'.format(r, g, b, a))
    d.putPlainChildren(value)

def qdump__Color3f(d, value):
    r = value['r'].value()
    g = value['g'].value()
    b = value['b'].value()
    valueStr = "{:10.4f} {:10.4f} {:10.4f}".format(r, g, b)
    d.putType("Color3f")
    d.putValue(valueStr)
    d.putPlainChildren(value)

# Entity ======================================================================

def qdump__Entity(d, value):
    index = value['m_index'].value()
    d.putType("Entity")
    d.putValue("Idx: " + str(index))
    d.putPlainChildren(value)

# HandleType ==================================================================
#            src/util/HandleType.h

def qdump__HandleType(d, v):
    handleType = v.type.stripTypedefs()
    invalidValue = handleType.templateArgument(2)
    value = v['t'].value()
    if value == invalidValue:
        d.putValue('Invalid ('+str(value)+')')
    else:
        d.putValue(value)
    d.putPlainChildren(v)

# Time Types ==================================================================
#            src/core/TimeTypes.h

def qdump__InstantType(d, v):
    d.putValue(v['t'].value())
    d.putPlainChildren(v)

def qdump__DurationType(d, v):
    d.putValue(v['t'].value())
    d.putPlainChildren(v)
