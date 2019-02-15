
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

def arx_HandleType_helper(d, v):
    handleType = v.type.stripTypedefs()
    templateArgs = handleType.templateArguments()
    # TODO how to do asserts ?
    #d.check(len(templateArgs) == 3)
    invalidValue = templateArgs[2]
    value = v['t'].value()
    d.putValue(value if (value != invalidValue) else 'Invalid ('+str(value)+')')
    d.putPlainChildren(v)

def qdump__MixerId(d, v):
    arx_HandleType_helper(d, v)
def qdump__EnvId(d, v):
    arx_HandleType_helper(d, v)
def qdump__AmbianceId(d, v):
    arx_HandleType_helper(d, v)
def qdump__SavegameHandle(d, v):
    arx_HandleType_helper(d, v)
def qdump__EntityHandle(d, v):
    arx_HandleType_helper(d, v)
def qdump__SpellHandle(d, v):
    arx_HandleType_helper(d, v)
def qdump__PrecastHandle(d, v):
    arx_HandleType_helper(d, v)
def qdump__DamageHandle(d, v):
    arx_HandleType_helper(d, v)
def qdump__ActionPoint(d, v):
    arx_HandleType_helper(d, v)
def qdump__ObjSelection(d, v):
    arx_HandleType_helper(d, v)
def qdump__ObjVertGroup(d, v):
    arx_HandleType_helper(d, v)
def qdump__ObjVertHandle(d, v):
    arx_HandleType_helper(d, v)
def qdump__LightHandle(d, v):
    arx_HandleType_helper(d, v)

# Time Types ==================================================================
#            src/core/TimeTypes.h

def arx_TimeType_helper(d, v):
    d.putValue(v['t'].value())
    d.putPlainChildren(v)

def qdump__GameInstant(d, v):
    arx_TimeType_helper(d, v)
def qdump__GameDuration(d, v):
    arx_TimeType_helper(d, v)
def qdump__PlatformInstant(d, v):
    arx_TimeType_helper(d, v)
def qdump__PlatformDuration(d, v):
    arx_TimeType_helper(d, v)
def qdump__AnimationDuration(d, v):
    arx_TimeType_helper(d, v)
